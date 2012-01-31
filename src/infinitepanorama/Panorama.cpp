#include "infinitepanorama/Panorama.hpp"
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <time.h>

#include <boost/filesystem.hpp>
#include <opencv/highgui.h>

#include "amalgamate/DescriptorFilter.hpp"

#include "graphcut/graph.h"

using namespace boost;

bool sameResolution(const Image& image1, const Image& image2)
{
	return (image1.rows() 	 == image2.rows() && 
			image1.columns() == image2.columns());
}


Panorama::Panorama(int _width, int _height, Config* _config):
	left(_config), right(_config)
{
	height(_height);
	width(_width);
	config(_config);

	border = 1.0 / 6.0;
}

void Panorama::setPanGeometry()
{
	panHeight = height_ + height_/2;
}

void Panorama::height(int _height) 
{ 
	height_=_height; 
	setPanGeometry();
}

void Panorama::width(int _width) 
{ 
	width_=_width;
	setPanGeometry();
}


void Panorama::drawImage(const Image& src, Image& dest, int offX, int offY)
{
	int w = src.columns(), h = src.rows();

	const PixelPacket* srcPixels = src.getConstPixels(0,0,w,h);
	PixelPacket* destPixels = dest.getPixels(0,0,dest.columns(),dest.rows());

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++) 
		{
			if (y+offY > int(dest.rows()) || 
					x+offX > int(dest.columns()) ||
					y+offY < 0 || x+offX < 0 ) continue;

			destPixels[(y+offY)*dest.columns()+x+offX] = srcPixels[y*w+x];
		}

	dest.syncPixels();
}


vector<bool> Panorama::graphCut(const Image& left, const Image& right)
{
	if (!sameResolution(left,right))
	{
		LOG_ERR << fmt("Resolution differs: %x% vs. %x%") 
			% left.columns() % left.rows() % right.columns() % right.rows(); 
		return vector<bool>();
	}

	Image edges1 = left, edges2 = right;

	float blurFactor = left.columns() / 100.0f;

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
			int pos = y*w+x;
			int maxDiff = max(edge1Pixels[pos].red,
					edge2Pixels[pos].red);
			g -> add_edge( pos,pos+1, maxDiff, maxDiff );
			g -> add_edge( pos,pos+w, maxDiff, maxDiff );
		}

		g->add_tweights( y*w , 0, 65536);
		g->add_tweights( y*w+w-1 , 65536 ,0);
	}
	g -> maxflow();

	vector<bool> mask(w*h);
	for (int i = 0; i < w*h; i++)
		mask[i] = (g->what_segment(i) != GraphType::SOURCE);

	delete g;

	return mask;
}

Image Panorama::linearBlending(const Image& left, const Image& right, const vector<bool>& mask)
{
	if (!sameResolution(left,right))
	{
		LOG_ERR << fmt("Resolution differs: %x% vs. %x%") 
			% left.columns() % left.rows() % right.columns() % right.rows(); 
		return Image();
	}

	int w = left.columns(), h = left.rows();
	float blurFactor = w / 8.0f;

	Image weightImage( Geometry(w,h), Color(0,0,0) );
	PixelPacket* weightPixels = weightImage.getPixels(0,0,w,h);

	for (int i = 0; i < w*h; i++)
	{
		int p = int(mask[i])*65535;
		weightPixels[i] = Color(p,p,p);
	}

	for (int y = 0; y < h; y++)
		for (int x = 0; x < int(blurFactor); x++)
		{
			int pos = y*w;
			weightPixels[pos+x] = Color(65535,65535,65535);
			weightPixels[pos+w-1-x] = Color(0,0,0);
		}

	weightImage.syncPixels();
	weightImage.gaussianBlur( blurFactor, blurFactor);
	weightImage.contrast( 2.0);
	//	weightImage.display();

	weightPixels = weightImage.getPixels(0,0,w,h);

	const PixelPacket* leftPixels = left.getConstPixels(0,0,w,h);
	const PixelPacket* rightPixels = right.getConstPixels(0,0,w,h);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int pos = y*w+x;
			int blend = weightPixels[pos].red;
			PixelPacket p1 = leftPixels[pos],
						p2 = rightPixels[pos];

			weightPixels[pos].red   = BLEND_u16(p1.red  ,p2.red  ,blend);
			weightPixels[pos].green = BLEND_u16(p1.green,p2.green,blend);
			weightPixels[pos].blue  = BLEND_u16(p1.blue ,p2.blue ,blend); 
		}
	}

	return weightImage;
}

Image Panorama::poissonBlending(const Image& left, const Image& right, const vector<bool>& mask)
{ 
	if (!sameResolution(left,right))
	{
		LOG_ERR << fmt("Resolution differs: %x% vs. %x%") 
			% left.columns() % left.rows() % right.columns() % right.rows(); 
		return Image();
	}

	int w = left.columns(), h = left.rows();

	Image finalImage( Geometry(w,h), Color(0,0,0,0));
	/*
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
}
weightPixels[y*w+x].red   = BLEND_u16(p1.red  ,p2.red  ,blend);
weightPixels[y*w+x].green = BLEND_u16(p1.green,p2.green,blend);
weightPixels[y*w+x].blue  = BLEND_u16(p1.blue ,p2.blue ,blend); 

}
}*/
return Image();
}

IplImage* Panorama::MagickToCv(const Magick::Image& image)
{
	int w = image.columns(), h = image.rows();
	LOG_MSG_(3) << fmt("% x %") % w % h;
	IplImage* cvImage = cvCreateImage( cvSize(w,h), IPL_DEPTH_8U, 3);

	Image img = image;
	const Magick::PixelPacket* pixels = image.getConstPixels(0,0,w,h);

	LOG_MSG_(3) << fmt("% %") % cvImage->widthStep % cvImage->nChannels; 
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

int Panorama::templateMatching(const Image& left, const Image& right, int leftOffY)
{
	if (left.columns() != right.columns() ||
			left.rows() != panHeight)
	{
		LOG_ERR << "Width of images must be equal, height of left image must be equal to "; 
		LOG_ERR << fmt("panorama height (=%). %x% vs. %x%") 
			% panHeight % left.columns() % left.rows() % right.columns() % right.rows(); 
		return -1;	
	}

	IplImage* image = MagickToCv(left); 
	IplImage* icon  = MagickToCv(right);

	//LOG->level(3);

	if (LOG->level() > 2)
	{
		cvNamedWindow( "image1", 1 ); cvShowImage( "image1", image ); 
		cvNamedWindow( "image2", 1 ); cvShowImage( "image2", icon );  
		cvWaitKey();
	}
	CvPoint point1; CvPoint point2;     

	int resultW = 1; 
	int resultH = image->height - icon->height +1; 
	IplImage* result = cvCreateImage(cvSize(resultW, resultH), IPL_DEPTH_32F, 1); 
	cvMatchTemplate(image, icon, result, CV_TM_SQDIFF);

	float* resultData = (float*)result->imageData;

	for (int i = 0; i < resultH; i++)
	{
		LOG_MSG_(3) << i << " " << resultData[i];
		if (resultData[i] < 0) resultData[i] = -resultData[i];

		float mid = leftOffY-i; 

		float weight = float(mid*mid*8.0/(resultH*resultH)); 
		resultData[i] *= (1 + weight);
		*LOG << " " << resultData[i];
	}

	double m,M;
	cvMinMaxLoc(result, &m, &M, &point2, &point1, NULL);

	if (LOG->level() > 2)
	{
		cvRectangle( image, point2, 
				cvPoint( point2.x + icon->width-1, point2.y + icon->height-1 ), 
				cvScalar( 0, 0, 255, 0 ), 1, 0, 0 ); 
		cvNamedWindow( "image1", 1 ); cvShowImage( "image1", image ); 
		cvWaitKey();
	}

	cvReleaseImage(&image);
	cvReleaseImage(&icon);
	cvReleaseImage(&result);

	return point2.y;
}

void Panorama::loadDatabases(string databaseFileLeft, string databaseFileRight)
{
	LOG_MSG << fmt("Reading database '%' for left descriptors...") % databaseFileLeft;
	left.read(databaseFileLeft);
	LOG_MSG << fmt("Reading database '%' for right descriptors...") % databaseFileRight;
	right.read(databaseFileRight);
	LOG_MSG << "Reading databases done.";
}

bool Panorama::stitch(Image& pan, string prev, string next, int& stitchOffX, int& stitchOffY)
{
	LOG_MSG << fmt("Stitching '%' and '%' @ (%x%)") % prev % next % stitchOffX % stitchOffY;
	LOG_MSG << fmt("Load and resize image1 '%' and image2 '%' ...") % prev % next;

	Image image1(prev); 
	image1.modifyImage();
	image1.resize( Geometry(width_, height_)) ; 

	Image image2(next);
	image2.modifyImage();
	image2.resize( Geometry(width_, height_)) ; 

	int w = int(border*min(image1.columns(),image2.columns()));

	LOG_MSG_(2) << fmt("New image sizes: %x%, %x%") 
		% image1.columns() % image1.rows() % image2.columns() % image2.rows();

	if (image1.rows() != image2.rows())
	{
		LOG_ERR << fmt("Height differs: %x% vs. %x%") 
			% image1.columns() % image1.rows() % image2.columns() % image2.rows(); 
		return false;
	}


	LOG_MSG_(2) << fmt("Border width = %") % w;

	int off1 = (panHeight-image1.rows())/2;
	if (stitchOffY != -1) off1 = stitchOffY;
	int offX = image1.columns() - w;

	Image image1Crop( Geometry(w,panHeight), Color(0,0,0) );
	Image image2Crop( Geometry(w,image2.rows()), Color(0,0,0) );

	drawImage(image1,image1Crop,w-image1.columns(),off1);
	drawImage(image2,image2Crop,0,0);

	int off2 = templateMatching(image1Crop,image2Crop,off1);
	if (off2 < 0)
	{
		LOG_ERR << fmt("Offset is negative! offset = %") % off2;
		return false;
	}
	LOG_MSG_(2) << fmt("Got offsets = (%,%)") % off1 % off2;

	Image image2CropExt( Geometry(w,panHeight) , Color(0,0,0));
	drawImage(image2Crop,image2CropExt,0,off2);

	vector<bool> mask = graphCut(image1Crop,image2CropExt);
	if (mask.empty())
	{ LOG_ERR << "No mask was generated."; return false; }

	Image blendImage = linearBlending(image1Crop,image2CropExt,mask);

	if (stitchOffX == 0) drawImage(image1,pan,stitchOffX,off1);
	stitchOffX += offX;

	if (stitchOffX+image2.columns() < width_)
	{
		drawImage(image2,pan,stitchOffX,off2);
		drawImage(blendImage,pan,stitchOffX,0);
	}
	else
	{	
		stitchOffX += w;
		return false;
	}
	stitchOffY = off2;
	return true;
}


void Panorama::generate(Image& image)
{
	LOG->level(1);
	LOG_MSG << fmt("Generating panorama (width = %, height = %)") % width_ % panHeight;
	Image tmpImage( Geometry(width_,panHeight) ,Color(0,0,0));
	image = tmpImage;
	int stitchPos = 0;
	int offY = -1;

	Descriptors leftDescs  = left.descriptors(), rightDescs = right.descriptors();
	
	Statistics statistics(0.0,true);
	DescriptorFilter descFilter(config_,&statistics);

	amalgamate::Descriptor *descLeft, *descRight;
	
	srand ( time(NULL) );

	
	// if First image, select random image
	int rnd = int(rand()/float(RAND_MAX)*left.size());
	descRight = rightDescs[rnd];
	
	statistics[descRight].excluded = true;

	LOG_MSG << fmt("%") % rnd;

	while (1)
	{
		amalgamate::Match match = descFilter.getBestMatch(*descRight,leftDescs);
		
		if (!match.desc)
		{
			LOG_WRN << "Match is associated with no descriptor.";
			break;
		}
		
		LOG_MSG_(2) << fmt("%: %, %") % match.desc->filename() % match.desc->index() % match.result;

		descLeft = match.desc;
		
		if (!stitch(image,descRight->filename(),descLeft->filename(),stitchPos,offY)) break;
		
		descRight = rightDescs[descLeft->index()];

	//	descLeft = descRight;
	}	
/*
	string image1FileName("./data/image1.jpg");
	string image2FileName("./data/image2.jpg");
	string image3FileName("./data/Forrest 03 - 1024x768.jpg");
	string image4FileName("./data/Forrest 01 - 1024x768.jpg");
	stitch(image,image2FileName,image3FileName,stitchPos,offY);
	stitch(image,image3FileName,image4FileName,stitchPos,offY);
*/
	image.crop( Geometry(stitchPos,panHeight,0,0) ); 
	image.display();

}

void Panorama::generate(string outputImageFile )
{
	Image image;
	generate(image);
	image.write(outputImageFile);
}

void Panorama::generateDatabase(vector<string>& imageFileList, string databaseFileLeft, string databaseFileRight)
{
	float border = 1.0f/6.0f;
	left.buildDescriptors(imageFileList,border,1.0,0,0);
	left.write(databaseFileLeft);
	left.clear();

	right.buildDescriptors(imageFileList,border,1.0,1.0-border,0);
	right.write(databaseFileRight);
	right.clear();
}


void Panorama::generateDatabase(string inputDir, string databaseFileLeft, string databaseFileRight)
{
	LOG_MSG << fmt("Scanning directory '%' ...") % inputDir;

	if (!filesystem::is_directory(inputDir))
	{
		LOG_MSG << fmt("'%' is not a directory, aborted.") % inputDir; return;
	}

	vector<string> imageFileList;
	for ( filesystem::recursive_directory_iterator end, dir(inputDir); dir != end; ++dir ) 
	{
		filesystem::path imageFileName(*dir);
		string ext = to_upper_copy(imageFileName.extension().string());
		if ((ext==".JPG") || (ext==".PNG"))
			imageFileList.push_back(imageFileName.string());            
	}
	cout << "Found " << imageFileList.size() << " files in '" << inputDir << "'." << endl;

	generateDatabase(imageFileList,databaseFileLeft,databaseFileRight);
}






