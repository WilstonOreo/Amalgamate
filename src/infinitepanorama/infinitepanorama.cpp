#include <stdlib.h>
#include <iostream>
#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <Magick++.h>

//#include <boost/algorithm/string/split.hpp>

#include "amalgamate.hpp"
#include "amalgamate/utils.hpp"
#include "amalgamate/config.hpp"

using namespace boost;
namespace po = program_options;

using namespace std;
using namespace Magick;

#define WEIGHT_WIDTH 384

void setPixel( PixelPacket* p, int r, int g, int b) 
{
	p->red = r;
	p->green = g;
	p->blue = b;
}


inline int sqr(PixelPacket a, PixelPacket b)
{
	int dR = a.red-b.red, dG = a.green-b.green, dB = a.blue-b.blue; 
	return int(((dR*dR + 2*dG*dG + dB*dB)/4));
}

#define SQR(a) (a)*(a)

void convoluteImage(Image& image, int* destBuf)
{
	PixelPacket* pix = image.getPixels(0,0,image.columns(),image.rows());

	int dR, w = image.columns(), h = image.rows();

	PixelPacket p = pix[0];
	dR =2*sqr(p,pix[1]);
	destBuf[0] = dR;

	p = pix[w-1];
	dR =2*sqr(p,pix[w-2]);
	destBuf[w-1] = dR;

	p = pix[(h-1)*w];
	dR =2*sqr(p,pix[(h-1)*w+1]);
	destBuf[(h-1)*w] = dR;

	p = pix[(h-1)*w+w-1];
	dR =2*sqr(p,pix[(h-1)*w+w-2]);
	destBuf[(h-1)*w+w-1] = dR;

	for (int x = 1; x < w-1; x++)
	{
		p = pix[x];
		dR= 2*(sqr(p,pix[x-1]) + sqr(p,pix[x+1]));
		destBuf[x] = dR;

		p = pix[(h-1)*w+x];
		dR= 2*(sqr(p,pix[(h-1)*w+x-1]) + sqr(p,pix[(h-1)*w+x+1]));
		destBuf[(h-1)*w+x] = dR;
	}

	for (int y = 1; y < h-1; y++)
	{
		p = pix[y*w];
		dR = 2*(sqr(p,pix[y*w+1]));
		destBuf[y*w] = dR;

		for (int x = 1; x < w-1; x++)
		{
			p = pix[y*w+x];
			dR= 2*( sqr(p,pix[y*w+x-1])+sqr(p,pix[y*w+x+1]));
			destBuf[y*w+x] = dR;
		}

		p = pix[y*w+w-1];
		dR = 2*(sqr(p,pix[y*w+w-2]));
		destBuf[y*w+w-1] = dR;
	}
}

struct Seam 
{
		vector< int > nodes;
		vector< int > intensities;
		int sum;

};


int getNode(int line, int pos, int minPos, int* buf, int w)
{
	int node = pos;
	int maxBuf = 0;// buf[line*w+pos];
	
	if (buf[line*w+pos  ] > maxBuf && pos > minPos  ) { node = pos; maxBuf = buf[line*w+pos]; }
	if (buf[line*w+pos-1] > maxBuf && pos > minPos  ) { node = pos-1; maxBuf = buf[line*w+pos-1]; }
	if (buf[line*w+pos+1] > maxBuf && pos < w/2-1 ) { node = pos+1; maxBuf = buf[line*w/2+pos+1]; }

	return node;	
}

void findSeam(Image& image1, Image& image2, Seam& seam1)
{
	int w = image1.columns(), h = image1.rows();
	int* buf1 = (int*)malloc(w*h*sizeof(int));
	int* buf2 = (int*)malloc(w*h*sizeof(int));

	convoluteImage(image1, buf1);
	convoluteImage(image2, buf2);
	
	for (int i = 0; i < h*w; i++) buf1[i] += buf2[i];

	Seam maxSeam;
	maxSeam.sum = 0;

	for (int y = 0; y < image1.rows(); y++)
	{
		Seam seam;
		seam.nodes.resize(image1.rows());
		seam.intensities.resize(image1.rows());

		int maxPoint = 0, max = 0;
		for (int x = 0; x < image1.columns()/2; x++)
			if (buf1[y*w+x] > max)
			{
				maxPoint = x;
				max = buf1[y*w+x];
			}

		int curPosX = maxPoint;
		seam.nodes[y] = maxPoint;
		seam.sum = max;
		seam.intensities[y] = max;

		for (int yy = y-1; yy >= 0; yy--)	
		{
			seam.nodes[yy] = getNode(yy,seam.nodes[yy+1],0,buf1,w);
			seam.intensities[yy] = buf1[y*w+seam.nodes[yy]];
			
			seam.sum += buf1[yy*w+seam.nodes[yy]];
		}

		for (int yy = y+1; yy < image1.rows(); yy++)	
		{
			seam.nodes[yy] = getNode(yy,seam.nodes[yy-1],0,buf1,w);
			seam.intensities[yy] = buf1[y*w+seam.nodes[yy]];
			seam.sum += buf1[yy*w+seam.nodes[yy]];
		}
		
		if (seam.sum > maxSeam.sum) maxSeam = seam;
	}

	seam1 = maxSeam;

	free(buf1); free(buf2);
}


void blendImages( Image& image1, Image& image2, Image& weight, Image& result )
{
	PixelPacket* pix1 = image1.getPixels(0,0,image1.columns(),image2.rows());
	PixelPacket* pix2 = image2.getPixels(0,0,image1.columns(),image2.rows());
	PixelPacket* pixW = weight.getPixels(0,0,weight.columns(),weight.rows());
	PixelPacket* pixR = result.getPixels(0,0,result.columns(),result.rows());

	for (int y = 0; y < result.rows(); y++)
		for (int x = 0; x < result.columns(); x++)
		{
			if (x < image1.columns()-WEIGHT_WIDTH)
				pixR[y*result.columns()+x] = pix1[y*image1.columns()+x]; else
			if (x >= image1.columns())
				pixR[y*result.columns()+x] = pix2[y*image2.columns()+x-image1.columns()+WEIGHT_WIDTH]; else
			{
				PixelPacket p1 = pix1[y*image1.columns()+x];
				PixelPacket p2 = pix2[y*image2.columns()+x-image1.columns()+WEIGHT_WIDTH];
				
				int weight = pixW[y*WEIGHT_WIDTH+x-image1.columns()+WEIGHT_WIDTH].red;

				int r = p1.red  *(65535-weight)/65536 + p2.red  *weight/65536;
				int g = p1.green*(65535-weight)/65536 + p2.green*weight/65536;
				int b = p1.blue *(65535-weight)/65536 + p2.blue *weight/65536;

				setPixel( &pixR[y*result.columns()+x], r, g ,b); 
			}
		}
}





int main(int ac, char* av[])
{
	cout << "InfinitePanoramaTest. -- written by Wilston Oreo." << endl;

	stringstream descStr; 
	descStr << "Allowed options";

	string image1FileName, image2FileName;
	// Declare the supported options.
	po::options_description desc(descStr.str());


	desc.add_options()
		("help,h", "Display help message.")
		("image1", po::value<string>(&image1FileName), "Input image1")
		("image2", po::value<string>(&image2FileName), "Input image2")
	;

	// Parse the command line arguments for all supported options
	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
	    cout << desc << endl;
	    return 1; 
	}


	Image image1(image1FileName);
	Image image2(image2FileName);
  //  image1.quantizeColorSpace( GRAYColorspace );
  //  image1.quantizeColors( 256 );
  //  image1.quantize( ); 
  //  image2.quantizeColorSpace( GRAYColorspace );
  //  image2.quantizeColors( 256 );
 //   image2.quantize( ); 

	Image result ( Geometry(image1.columns()+image2.columns()-WEIGHT_WIDTH,768) , Color(0,0,0));;
	Image weight ( Geometry(WEIGHT_WIDTH,768) , Color(0,0,0));

	Seam seam1;

	Image image1Weight( Geometry(WEIGHT_WIDTH,768), Color(0,0,0)); image1Weight.modifyImage();
	Image image2Weight( Geometry(WEIGHT_WIDTH,768), Color(0,0,0)); image2Weight.modifyImage();
	
	PixelPacket *pix1W = image1Weight.getPixels(0,0,WEIGHT_WIDTH,768); 
	PixelPacket *pix2W = image2Weight.getPixels(0,0,WEIGHT_WIDTH,768);
	PixelPacket *pix1 = image1.getPixels(0,0,image1.columns(),image1.rows()); 
	PixelPacket *pix2 = image2.getPixels(0,0,image2.columns(),image2.rows());
	for (int y = 0; y < 768; y++)
		for (int x = 0; x < WEIGHT_WIDTH; x++)
		{
			pix1W[y*WEIGHT_WIDTH+x] = pix1[y*image1.columns()+x+image1.columns()-WEIGHT_WIDTH];
			pix2W[y*WEIGHT_WIDTH+x] = pix2[y*image2.columns()+x];
		}

	image1Weight.syncPixels();
	image2Weight.syncPixels();

	findSeam(image1Weight,image2Weight,seam1);	
	

	result.modifyImage();

	weight.modifyImage();

	PixelPacket* pixW = weight.getPixels(0,0,weight.columns(),weight.rows());  
	for (int y = 0; y < 768; y++)
		for (int x = 0; x < WEIGHT_WIDTH; x++)
		{
			int v = 0;
			int a = seam1.nodes[y]; if (a < 0) a = 0;
			int b = seam1.nodes[y]+WEIGHT_WIDTH/2; if (b >=WEIGHT_WIDTH) b = WEIGHT_WIDTH-1; 

			if (x<=a) v = 0; else
			if (x >b) v = 65535; else { v = (x-a)*65535/(b-a);	}

	//		int v = seam1.nodes[y]  x*65535/WEIGHT_WIDTH;
			setPixel(&pixW[y*WEIGHT_WIDTH+x],v,v,v);
		}
	weight.syncPixels();


	blendImages(image1,image2,weight,result);
	result.syncPixels();

	image1.modifyImage();

	PixelPacket* p = result.getPixels(0,0,result.columns(),result.rows());
	for (int y = 0; y < result.rows(); y++)
	{
		setPixel(&p[y*result.columns()+image1.columns()-WEIGHT_WIDTH+seam1.nodes[y]],0,65535,0);
	}
	result.syncPixels();

	result.write("result.jpg");
	result.display();

	return EXIT_SUCCESS;
}



