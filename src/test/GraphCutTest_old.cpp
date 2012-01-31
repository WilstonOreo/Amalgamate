#include <iostream>
#include <stdio.h>
#include <boost/program_options.hpp>
#include <Magick++.h>

#include <opencv/highgui.h>
#include <opencv/cv.h> 
#include "log/Log.hpp"

#include "graphcut/graph.h"

using namespace boost;
namespace po = program_options;

using namespace std;
using namespace Magick;

LOG_INIT;

int pixelDiff(PixelPacket a, PixelPacket b)
{
	int gA = a.red + a.green * 2 + a.blue;
	int gB = b.red + b.green * 2 + b.blue;
	return abs(gA + gB);
}


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

//Image& 



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

	double m,M;

	Image image1CropExt( Geometry(image1Crop.columns(),image1Crop.rows()+image1Crop.rows()/4), Color(0,0,0));
	image1CropExt.modifyImage();
	PixelPacket* pixels = image1Crop.getPixels(0,0,image1Crop.columns(),image1Crop.rows());
	PixelPacket* pixelsExt = image1CropExt.getPixels(0,0,image1CropExt.columns(),image1CropExt.rows());

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
	cvNamedWindow( "output", 1 ); 
	cvShowImage( "output", result ); 

	cvRectangle( image, point2, 
				cvPoint( point2.x + icon->width, point2.y + icon->height ), 
				cvScalar( 0, 0, 255, 0 ), 1, 0, 0 ); 

	cvNamedWindow( "output", 1 ); 
	cvShowImage( "output", image ); 
	cvWaitKey(); 
*/	

	image1CropExt.gaussianBlur( 2, 0.6);
	image1CropExt.edge( 1.0 );

	image2Crop.gaussianBlur( 2, 0.6);
	image2Crop.edge( 1.0 );

	image1CropExt.display();
	image2Crop.display();

	PixelPacket* image1Pixels = 
		image1CropExt.getPixels(0,0,image1CropExt.columns(),image1CropExt.rows());
	PixelPacket* image2Pixels = 
		image2Crop.getPixels(0,0,image2Crop.columns(),image2Crop.rows());

	typedef Graph<int,int,int> GraphType;
	GraphType *g = new GraphType(w*h,2*w*h); 

	g -> add_node(w*h); 


	for (int y = 1; y < h-1; y++)
	{
		for (int x = 1; x < w-1; x++)
		{
			int dHorzRight = pixelDiff(pixels[y*w+x],pixels[y*w+x+1]);
			g -> add_edge( y*w+x, y*w +x+1, dHorzRight, dHorzRight );
			
			int dHorzLeft = pixelDiff(pixels[y*w+x-1],pixels[y*w+x]);
			g -> add_edge( y*w+x-1, y*w +x, dHorzLeft, dHorzLeft );
			
			int dVertDown = pixelDiff(pixels[y*w+x],pixels[(y+1)*w+x]);
			g -> add_edge( y*w+x,(y+1)*w+x, dVertDown, dVertDown );
			
			int dVertUp   = pixelDiff(pixels[y*w+x],pixels[(y-1)*w+x]);
			g -> add_edge( y*w+x,(y-1)*w+x, dVertUp, dVertUp );
		}
		
		g->add_tweights( y*w+1 , 0, 4*65536);
		g->add_tweights( y*w+w-2 , 4*65536 ,0);
	}
	int flow = g -> maxflow();
	printf("Flow = %d\n", flow);
	printf("Minimum cut:\n");

	for (int i = 0; i < w*h; i++)
	{
		if (g->what_segment(i) == GraphType::SOURCE)
		{
			pixels[i].red = 65535;
		} else
		{
			pixels[i].blue = 65535;
		}
	}
	image.syncPixels();
	image.display();
	delete g;
*/
	
	
	Image weightImage(
				Geometry(image1CropExt.columns(),image1CropExt.rows()),
				Color(65535,65535,65535));
	//weightImage.display();
	return EXIT_SUCCESS;
}



