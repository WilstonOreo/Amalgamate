#include <iostream>
#include <stdio.h>
#include <boost/program_options.hpp>
#include <Magick++.h>

#include <opencv/highgui.h>
#include <opencv/cv.h> 
#include "log/Log.hpp"

#include "amalgamate/utils.hpp"
#include "graphcut/graph.h"

using namespace boost;
namespace po = program_options;

using namespace std;
using namespace Magick;

LOG_INIT;

IplImage* MagickToCv(const Magick::Image& image)
{
	int w = image.columns(), h = image.rows();
	LOG_MSG << fmt("% x %") % w % h;
	IplImage* cvImage = cvCreateImage( cvSize(w,h), IPL_DEPTH_8U, 3);

	const Magick::PixelPacket* pixels = image.getConstPixels(0,0,w,h);

	LOG_MSG << fmt("% %") % cvImage->widthStep % cvImage->nChannels; 

	uchar * data    = (uchar*)cvImage->imageData;
	
	for (int i = 0; i < h; i++)
	for (int j = 0; j < w; j++)
	{
		data[i*cvImage->widthStep+j*3 + 0]=pixels[i*w+j].blue / 256; // B
		data[i*cvImage->widthStep+j*3 + 1]=pixels[i*w+j].green / 256; // G
		data[i*cvImage->widthStep+j*3 + 2]=pixels[i*w+j].red / 256; // R
	}
	return cvImage;
}

Magick::Image CvToMagick(const IplImage* image)
{
	int w = image->width, h = image->height;
	
	Magick::Image mImage( Magick::Geometry(w,h), Magick::Color(0,0,0,0));
	mImage.modifyImage();
	Magick::PixelPacket* pixels = mImage.getPixels(0,0,w,h);	

	for (int i = 0; i < w*h; i++)
		pixels[i] = Color( 	image->imageData[i*3+2]*256,
							image->imageData[i*3+1]*256,
							image->imageData[i*3]*256);
	mImage.syncPixels();
	return mImage;
}

void drawImage(const Image& src, Image& dest, int offX, int offY);
{
	int w = src.columns(), h = src.rows();
	
	const PixelPacket* srcPixels = src.getConstPixels(0,0,w,h);
	PixelPacket destPixels = dest.getPixels(0,0,dest.columns(),dest.rows());

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++) 
		{
			if (y+offY > dest.rows() || x+offX > dest.columns() ||
				y+offY < 0 || x+offX < 0 ) continue;

			dest[(y+offY)*dest.columns()+x+offX] = src[y*w+x];
		}
}

int templateMatching(const Image& left, const Image& right)
{
	Image image1CropExt( Geometry(image1Crop.columns(),image1Crop.rows()+image1Crop.rows()/4), Color(0,0,0));
	image1CropExt.modifyImage();
	PixelPacket* pixels = image1Crop.getPixels(0,0,image1Crop.columns(),image1Crop.rows());
	PixelPacket* pixelsExt = image1CropExt.getPixels(0,0,image1CropExt.columns(),image1CropExt.rows());
	
	double m,M;

	for (int i = 0; i < image1CropExt.columns()*image1CropExt.rows(); i++)
	{
		int pos = i-image1Crop.columns()*image1Crop.rows()/8;
		if (pos < 0) pos = (-pos) % image1Crop.columns();
		if (pos > image1Crop.columns()*image1Crop.rows())
			pos = pos % image1Crop.columns() + image1Crop.columns()*(image1Crop.rows()-1);
		pixelsExt[i] = pixels[pos];
	}

	image1CropExt.syncPixels();

	IplImage* image = MagickToCv(image1CropExt); 
	IplImage* icon  = MagickToCv(image2Crop);
/*
	cvNamedWindow( "image1", 1 ); 
	cvShowImage( "image1", image ); 
	cvNamedWindow( "image2", 1 ); 
	cvShowImage( "image2", icon ); 
*/

	CvPoint point1; CvPoint point2;     

	int resultW = 1; 
	int resultH = image->height - icon->height +1; 
	IplImage* result = cvCreateImage(cvSize(resultW, resultH), IPL_DEPTH_32F, 1); 
	cvMatchTemplate(image, icon, result, CV_TM_SQDIFF_NORMED);

	float* resultData = (float*)result->imageData;

	for (int i = 0; i < resultH; i++)
	{
	//	LOG_MSG << i << " " << resultData[i];
		if (resultData[i] < 0) resultData[i] = -resultData[i];
		float weight = float((i-resultH/2)*(i-resultH/2)*2.0/(resultH*resultH)); 
		resultData[i] *= (1 + weight);
	//	*LOG << " " << resultData[i];
	}

	cvMinMaxLoc(result, &m, &M, &point2, &point1, NULL);

/*
	cvRectangle( image, point2, 
				cvPoint( point2.x + icon->width, point2.y + icon->height ), 
				cvScalar( 0, 0, 255, 0 ), 1, 0, 0 ); 
*/	

	return point2.y;
}

vector<bool> graphCut(Image& left, Image& right, int offset)
{
	Image edges1 = left, edges2 = right;

	float blurFactor = left.columns() / 50.0f;

	edges1.quantizeColorSpace( GRAYColorspace );
	edges1.quantize();
	edges1.edge( 1.0 );
	edges1.gaussianBlur( blurFactor, blurFactor);

	edges2.quantizeColorSpace( GRAYColorspace );
	edges2.quantize();
	edges2.edge( 1.0 );
	edges2.gaussianBlur( blurFactor, blurFactor);

	PixelPacket* edge1Pixels = 
		edges1.getPixels(0,0,edges1.columns(),edges1.rows());
	PixelPacket* edge2Pixels = 
		edges2.getPixels(0,0,edges2.columns(),edges2.rows());

	int w = edges1.columns(), h = edges1.rows();

	typedef Graph<int,int,int> GraphType;
	GraphType *g = new GraphType(w*h,2*w*h); 

	g -> add_node(w*h); 

	for (int y = 0; y < h-1; y++)
	{
		for (int x = 0; x < w-1; x++)
		{
			int pos = y-point2.y; 
			if (pos <  0) pos = 0;
			if (pos >= edges2.rows()) pos = edges2.rows()-1;
			
			int p1 = edge1Pixels[y*w+x].red;
			int p2 = edge2Pixels[pos*w+x].red;			
			int maxDiff = max(p1,p2);

			g -> add_edge( y*w+x,y*w+x+1,   maxDiff, maxDiff );
			g -> add_edge( y*w+x,(y+1)*w+x, maxDiff, maxDiff );
		}

		g->add_tweights( y*w , 0, 65536);
		g->add_tweights( y*w+w-1 , 65536 ,0);
	}
	int flow = g -> maxflow();

	vector<bool> mask(w*h);
	for (int i = 0; i < w*h; i++)
		mask = (g->what_segment(i) == GraphType::SOURCE);

	delete g;
	
	return mask;
}

Image linearBlending(const Image& left, const Image& right, const vector<bool>& mask, int offset)
{

	weightImage.syncPixels();
	weightImage.gaussianBlur( 16, 16);

	weightPixels = weightImage.getPixels(0,0,w,h);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int pos = y-point2.y; 
			if (pos <  0) pos = 0;
			if (pos >= edges2.rows()) pos = edges2.rows()-1;

			int blend = weightPixels[y*w+x].red;
			PixelPacket p1 = image1Pixels[y*w+x],
						p2 = image2Pixels[pos*w+x];

			weightPixels[y*w+x].red   = BLEND_u16(p1.red  ,p2.red  ,blend);
			weightPixels[y*w+x].green = BLEND_u16(p1.green,p2.green,blend);
			weightPixels[y*w+x].blue  = BLEND_u16(p1.blue ,p2.blue ,blend); 
			
		}
	}
}

Image poissonBlending(const Image& left, const Image& right, const vector<bool>& mask)
{
	Image finalImage( Geometry(w,h), Color(0,0,0,0));
	vector<float> finalR(w*h,0); // = finalImage.getPixels(0,0,w,h);
	vector<float> finalG(w*h,0);
	vector<float> finalB(w*h,0);

	weightImage.syncPixels();
	weightImage.gaussianBlur( 16, 16);

	weightPixels = weightImage.getPixels(0,0,w,h);




	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int pos = y-point2.y; 
			if (pos <  0) pos = 0;
			if (pos >= edges2.rows()) pos = edges2.rows()-1;

			int blend = weightPixels[y*w+x].red;
			PixelPacket p1 = image1Pixels[y*w+x],
						p2 = image2Pixels[pos*w+x];
			// Poisson blending
/*
			int p = y*w+x;

			if (blend==0)
			{
				finalR[p] = p1.red/65535.0; 
				finalG[p] = p1.green/65535.0; 
				finalB[p] = p1.blue/65535.0; 
			} else
			{
				finalR[p] = finalG[p] = finalB[p] = 0;

				vector<int> n1(4); // Neighbors
				n1[0] = (x >  0)   ? y*w+x-1 : -1;
				n1[1] = (x <= w-1) ? y*w+x+1 : -1;
				n1[2] = (y >  0)   ? (y-1)*w+x : -1;
				n1[3] = (y <  h-1) ? (y+1)*w+x : -1;

				vector<int> n2(4); // Neighbors
				int posP = pos*w+x;
				n2[0] = (x >  0)   ? pos*w+x-1 : -1;
				n2[1] = (x < w-1) ? pos*w+x+1 : -1;
				n2[2] = (pos >  0)   ? (pos-1)*w+x : -1;
				n2[3] = (pos <  h-1) ? (pos+1)*w+x : -1;

				fmt f("% % % %");
				LOG_MSG << f % n1[0] % n1[1] % n1[2] % n1[3];
				LOG_MSG << f % n2[0] % n2[1] % n2[2] % n2[3];

				for (int i = 0; i < 4; i++)
				{
					if (n1[i] == -1 || n2[i] == -1) continue;

					finalR[p] += ((float)p2.red   - (float)image2Pixels[n2[i]].red)/65535.0;
					finalG[p] += ((float)p2.green - (float)image2Pixels[n2[i]].green)/65535.0;
					finalB[p] += ((float)p2.blue  - (float)image2Pixels[n2[i]].blue)/65535.0;

					LOG_MSG << fmt("%: % % %") % i % finalR[p] % finalG[p] % finalB[p];
					LOG_MSG << fmt("R = %, G = %, B = %") % image2Pixels[n2[i]].red % image2Pixels[n2[i]].green % image2Pixels[n2[i]].blue;
					LOG_MSG << fmt("R = %, G = %, B = %") % p2.red % p2.green % p2.blue;

					if (weightPixels[n1[i]].red==0)
					{
						finalR[p] += image1Pixels[n1[i]].red/65535.0f;
						finalG[p] += image1Pixels[n1[i]].green/65535.0f;
						finalB[p] += image1Pixels[n1[i]].blue/65535.0f;

					} else
					{
						finalR[p] += finalR[n1[i]];
						finalG[p] += finalG[n1[i]];
						finalB[p] += finalB[n1[i]];
					}
				}
			}*/
			weightPixels[y*w+x].red   = BLEND_u16(p1.red  ,p2.red  ,blend);
			weightPixels[y*w+x].green = BLEND_u16(p1.green,p2.green,blend);
			weightPixels[y*w+x].blue  = BLEND_u16(p1.blue ,p2.blue ,blend); 
			
		}
	}


}

int main(int ac, char* av[])
{
	cout << "GraphCutTest. v0.1 -- written by Wilston Oreo." << endl;

	stringstream descStr; 

	descStr << "Usage:" << endl;
	descStr << "\tgraphcuttest -i imagefile" << endl;

	descStr << "Allowed options" << endl;
	po::options_description cmdDesc(descStr.str());

	string image1FileName, image2FileName;
	int width = 1024,height = 768;

	cmdDesc.add_options()
		("help", 		"Display help message.")
		("image1", po::value<string>(&image1FileName), "Input image1")
		("image2", po::value<string>(&image2FileName), "Input image2")
		("width,w",  po::value<int>(&width), "Width") 
		("height,h", po::value<int>(&height), "Height") 
		;

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, cmdDesc), vm);
	po::notify(vm);

	if (vm.count("help")) { cout << cmdDesc << endl; return 1; }

	LOG_MSG << "Load images...";
	Image image1(image1FileName); 
	image1.modifyImage();
	image1.resize( Geometry(width,height) );

	Image image2(image2FileName);
	image2.modifyImage();
	image2.resize( Geometry(width,height) );

	int w1 = image1.columns(), h1 = image1.rows();
	int w2 = image2.columns(), h2 = image2.rows();
	
	LOG_MSG << "Crop images...";
	float border = 1.0/6.0;
	Image image1Crop = image1; image1Crop.modifyImage();
	Image image2Crop = image2; image2Crop.modifyImage();
	image1Crop.crop( Geometry( int(w1*border),h1,int((1.0-border)*w1),0));
	image2Crop.crop( Geometry( int(w2*border),h2,0,0));
//	image1Crop.display(); image2Crop.display();

	int offset = templateMatching(image1,image2);
	vector<bool> mask = graphCut(image1,image2,offset);
	Image blend = linearBlending(image1,image2,mask,offset);

	Image final( Geometry( image1.columns() + image2.columns() - w , image2.rows() + offset),
				 Color(0,0,0,0) );

	drawImage( image1, final, 0, 0);
	drawImage( image2, final, image1.columns()-w, offset);
	drawImage( blend, final, image1.columns()-w, 0);

	final.display();

	return EXIT_SUCCESS;
}



