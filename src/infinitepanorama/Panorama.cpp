#include "infinitepanorama/Panorama.hpp"
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <time.h>

#include <boost/filesystem.hpp>
#include <opencv/highgui.h>

#include "amalgamate/DescriptorFilter.hpp"

#include "poisson/PoissonBlending.h"
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


void Panorama::drawOnImage(const Image& src, Image& dest, int offX, int offY)
{
	int w = src.columns(), h = src.rows();

	const PixelPacket* srcPixels = src.getConstPixels(0,0,w,h);
	PixelPacket* destPixels = dest.getPixels(0,0,dest.columns(),dest.rows());

	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++) 
		{
			if (y+offY >= int(dest.rows()) || 
					x+offX >= int(dest.columns()) ||
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
	float blurFactor = left.columns() / 200.0f;

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

Image Panorama::linearBlending(const Image& left, const Image& right, const Image& mask)
{
	if (!sameResolution(left,right) || !sameResolution(left,mask))
	{
		LOG_ERR << fmt("Resolution differs: %x% vs. %x%") 
			% left.columns() % left.rows() % right.columns() % right.rows(); 
		return Image();
	}

	int w = left.columns(), h = left.rows();
	float blurFactor = w / 100.0f;

	Image weightImage = mask;
	weightImage.gaussianBlur( blurFactor, blurFactor);
	PixelPacket* weightPixels = weightImage.getPixels(0,0,w,h);

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


Image Panorama::poissonBlending(Image& left, Image& right,Image& mask)
{ 
	if (!sameResolution(left,right))
	{
		LOG_ERR << fmt("Resolution differs: %x% vs. %x%") 
			% left.columns() % left.rows() % right.columns() % right.rows(); 
		return Image();
	}

	//left.display();
	//right.display();

	int w = left.columns(), h = left.rows();
	Image leftTmp = left;
	PixelPacket* leftPixels = leftTmp.getPixels(0,0,w,h);
	const PixelPacket* rightPixels = right.getConstPixels(0,0,w,h);

	PixelPacket* maskPixels = mask.getPixels(0,0,w,h);

	for (int y = 0; y < h; y++) 
		for (int x = 0; x < w; x++)
		{
			int pos = y*w+x;
			if (x < 4 || x >= w-4 || y < 4 || y >= h-4)
				maskPixels[pos] = Color(0,0,0);

			if (x > (w + w/6)/2) leftPixels[pos] = rightPixels[pos];
		}

	mask.syncPixels();
	leftTmp.syncPixels();

	//leftTmp.display();

	LOG->level(1);

	IplImage* leftCv = MagickToCv(leftTmp);
	IplImage* rightCv = MagickToCv(right);
	IplImage* maskCv = MagickToCv(mask,true);
	
	if (LOG->level() > 2)
	{
		cvNamedWindow( "image1", 1 ); cvShowImage( "image1", leftCv ); 
		cvNamedWindow( "image2", 1 ); cvShowImage( "image2", rightCv );  
		cvNamedWindow( "image3", 1 ); cvShowImage( "image3", maskCv );  
		cvWaitKey();
	}

	cv::Mat leftMat(leftCv);
	cv::Mat rightMat(rightCv);
	cv::Mat maskMat(maskCv);

	LOG_MSG_(2) << fmt("left: %x%, right: %x%, mask: %x%") 
		% leftMat.cols % leftMat.rows % rightMat.cols % rightMat.rows % maskMat.cols % maskMat.rows;
	


	//cv::Mat resultImg(leftMat);
	cv::Mat resultImg = PoissonBlend(rightMat,leftMat,maskMat);

	LOG_MSG_(2) << fmt("%x%") % resultImg.cols % resultImg.rows;

	IplImage result(resultImg);
	cvReleaseImage(&leftCv);
	cvReleaseImage(&rightCv);
	cvReleaseImage(&maskCv);

	LOG->level(1);
	
	return CvToMagick(&result);
}


IplImage* Panorama::MagickToCv(const Magick::Image& image, bool grayScale)
{
	int w = image.columns(), h = image.rows();
	LOG_MSG_(3) << fmt("% x %") % w % h;

	int nChannels = grayScale ? 1 : 3;

	IplImage* cvImage = cvCreateImage( cvSize(w,h), IPL_DEPTH_8U, nChannels);

	Image img = image;
	if (grayScale) img.quantizeColorSpace( GRAYColorspace );
	const Magick::PixelPacket* pixels = img.getConstPixels(0,0,w,h);

	LOG_MSG_(3) << fmt("% %") % cvImage->widthStep % cvImage->nChannels; 
	uchar * data    = (uchar*)cvImage->imageData;

	for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
		{
			data[i*cvImage->widthStep+j*nChannels + 0]=pixels[i*w+j].blue / 256; // B
			
			if (!grayScale)
			{
				data[i*cvImage->widthStep+j*nChannels + 1]=pixels[i*w+j].green / 256; // G
				data[i*cvImage->widthStep+j*nChannels + 2]=pixels[i*w+j].red / 256; // R
			}
		}
	return cvImage;
}

Magick::Image Panorama::CvToMagick(IplImage* image)
{
	int w = image->width, h = image->height;
	
	Magick::Image mImage( Magick::Geometry(w,h), Magick::Color(0,0,0,0));
	mImage.modifyImage();
	Magick::PixelPacket* pixels = mImage.getPixels(0,0,w,h);	

	uchar * data    = (uchar*)image->imageData;

	for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
			pixels[i*w+j] = Color( 	data[i*image->widthStep+j*3+2]*256,
									data[i*image->widthStep+j*3+1]*256,
									data[i*image->widthStep+j*3  ]*256);
	mImage.syncPixels();
	return mImage;
}

int Panorama::templateMatching(const Image& left, const Image& right, int leftOffY)
{
	if (left.columns() != right.columns() || left.rows() != panHeight)
	{
		LOG_ERR << "Width of images must be equal, height of left image must be equal to "; 
		LOG_ERR << fmt("panorama height (=%). %x% vs. %x%") 
			% panHeight % left.columns() % left.rows() % right.columns() % right.rows(); 
		return -1;	
	}
	IplImage* image = MagickToCv(left); 
	IplImage* icon  = MagickToCv(right);

//	LOG->level(3);


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

		float weight = float((resultH/2.0f-i)*(resultH/2.0f-i)*3.0/(resultH*resultH)); 
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

//	LOG->level(1);

	cvReleaseImage(&image);
	cvReleaseImage(&icon);
	cvReleaseImage(&result);
	return point2.y;
}


void Panorama::loadDatabases(string databaseFileLeft, string databaseFileRight, bool append)
{
	LOG_MSG << fmt("Reading database '%' for left descriptors...") % databaseFileLeft;
	left.read(databaseFileLeft,append);
	LOG_MSG << fmt("Reading database '%' for right descriptors...") % databaseFileRight;
	right.read(databaseFileRight,append);
	LOG_MSG << "Reading databases done.";
}

bool Panorama::stitch(Image& pan, string prev, string next, int& stitchOffX, int& stitchOffY, BlendingMode blend)
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
	drawOnImage(image1,image1Crop,w-image1.columns(),off1);
	drawOnImage(image2,image2Crop,0,0);

	int off2 = templateMatching(image1Crop,image2Crop,off1);
	if (off2 < 0)
		{ LOG_ERR << fmt("Offset is negative! offset = %") % off2; return false; }
	LOG_MSG_(2) << fmt("Got offsets = (%,%)") % off1 % off2;

	Image image2CropExt( Geometry(w,panHeight) , Color(0,0,0));
	drawOnImage(image2Crop,image2CropExt,0,off2);

	vector<bool> mask = graphCut(image1Crop,image2CropExt);
	if (mask.empty())
		{ LOG_ERR << "No mask was generated."; return false; }

	int halfWidth = (image1.columns()+image2.columns())/2-w;
	Image image1Blend( Geometry(halfWidth,panHeight), Color(0,0,0));
	Image image2Blend( Geometry(halfWidth,panHeight), Color(0,0,0));
	Image maskImage( Geometry(halfWidth,panHeight), Color(0,0,0) );
	PixelPacket* maskPixels = maskImage.getPixels(0,0,halfWidth,panHeight);

	for (unsigned y = 0; y < panHeight; y++)
		for (int x = 0; x < halfWidth; x++)
		{
			int pos = y*halfWidth+x;		
			if (x < (halfWidth - w) / 2) { maskPixels[pos] = Color(65535,65535,65535); continue; }
			if (x >=(halfWidth + w) / 2) { maskPixels[pos] = Color(0,0,0); continue; }
			int p = int(mask[y*w + x - (halfWidth - w)/2])*65535;
			maskPixels[pos] = Color(p,p,p);
		}

	if (blend == BLEND_POISSON)
	{
		for (unsigned y = 0; y < panHeight; y++)
			for (int x = 0; x < halfWidth; x++)
			{
				int pos = y*halfWidth+x;
				int p = 65535-maskPixels[pos].red;
				maskPixels[pos] = Color(p,p,p);			
			}
	}
	drawOnImage(image1,image1Blend,(halfWidth+w)/2-image1.columns(),off1);
	drawOnImage(image2,image2Blend,(halfWidth-w)/2,off2);
	
	Image blendImage;
	switch (blend)
	{
		case BLEND_LINEAR:  blendImage = linearBlending(image1Blend,image2Blend,maskImage); break;
		case BLEND_POISSON: blendImage = poissonBlending(image1Blend,image2Blend,maskImage); break;
	}

	if (stitchOffX == 0) drawOnImage(image1,pan,stitchOffX,off1);
	stitchOffX += offX;
	if (stitchOffX+image2.columns() < width_)
	{
		drawOnImage(image2,pan,stitchOffX,off2);
		drawOnImage(blendImage,pan,stitchOffX-(halfWidth-w)/2,0);
	}
	else
		{	stitchOffX += w; return false; }
	stitchOffY = off2;
	return true;
}


void Panorama::generate(Image& image, BlendingMode blend)
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
	statistics.exclude(descRight);

	descLeft = leftDescs[descRight->index()];
	statistics.exclude(descLeft);

	LOG_MSG << fmt("%") % rnd;

	while (1)
	{
		amalgamate::Match match = descFilter.getBestMatch(*descRight,leftDescs);
		if (!match.desc)
		{
			LOG_ERR << "Match is associated with no descriptor.";
			break;
		}
		LOG_MSG_(2) << fmt("%: %, %") % match.desc->filename() % match.desc->index() % match.result;
		descLeft = match.desc;

		statistics.exclude(descLeft);
		
		if (!stitch(image,descRight->filename(),descLeft->filename(),stitchPos,offY,blend)) break;	
		descRight = rightDescs[descLeft->index()];

		//statistics.exclude(descRight);
	}	
	image.crop( Geometry(stitchPos,panHeight,0,0) ); 
//	image.display();
}

void Panorama::generate(string outputImageFile, BlendingMode blend)
{
	Image image;
	generate(image,blend);
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
	int count = 0;
	vector<string> imageFileList;
	for ( filesystem::recursive_directory_iterator end, dir(inputDir); dir != end; ++dir ) 
	{
		filesystem::path imageFileName(*dir);
		string ext = to_upper_copy(imageFileName.extension().string());
		if ((ext==".JPG") || (ext==".PNG"))
		{
			imageFileList.push_back(imageFileName.string());            
			count++;
			if (count % 10000 == 0)
				LOG_MSG << fmt("Already found % images, still searching") % count;
		}
	}
	LOG_MSG << fmt("Found % files in '%'.") % imageFileList.size() % inputDir;
	generateDatabase(imageFileList,databaseFileLeft,databaseFileRight);
}

