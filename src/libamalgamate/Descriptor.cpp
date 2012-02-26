#include "amalgamate/Descriptor.hpp"

#include "boost/format.hpp"
#include "boost/foreach.hpp"

#include "amalgamate/DescriptorFilter.hpp"

using namespace Magick;
using namespace boost;

namespace amalgamate
{
	// Descriptor class /////////////////////////////////////////////////
	Descriptor::Descriptor(): statInfo(NULL), index_(0)
	{
	};

	Descriptor::Descriptor( Image& image ): statInfo(NULL), index_(0)
	{ 
		build(image); 
	}


	Descriptor::Descriptor(Image& image, int _width, int _height, int _offX, int _offY): statInfo(NULL), index_(0)
	{
		build(image,_width,_height,_offX,_offY); 
	}

	Descriptor::Descriptor( string _filename ): statInfo(NULL), index_(0)
	{ 
		build(_filename); 
	}

	Descriptor::~Descriptor()
	{
//		if (statInfo)
//			delete statInfo;
	}


	bool Descriptor::build( string _filename, 
							float _width, float _height, 
							float _offX,  float _offY)
	{
		try
		{
			Image image; image.read(_filename);

			if (_width != 1.0 || _height != 1.0)
			{
				int iWidth  = int(_width*image.columns());
				int iHeight = int(_height*image.rows());
				int iOffX   = int(_offX*image.columns());
				int iOffY 	= int(_offY*image.rows());

				image.crop( Geometry( iWidth, iHeight, iOffX, iOffY ));
			}

			filename_ = _filename;
			build(image);
		}
		catch (Exception e) { return false; }
		return true;
	}

	void Descriptor::build(Image& image)
	{
		offset_.set(0,0);
		width_ = image.columns(); height_ = image.rows();
		descriptor::YUVImage yuvImage(image);
		histogramSmall_.build(yuvImage,8);
		histogramLarge_.build(yuvImage,256);
		gist_.build(yuvImage);
		thumbnail_.build(yuvImage);
	}

	void Descriptor::build(Image& image, int _width, int _height, int _offX, int _offY)
	{
		Image img = image;
		img.crop(Geometry(_width,_height,_offX,_offY));
		build(img);
		offset_.set(_offX,_offY);
	}

	string Descriptor::toString()
	{
		stringstream ss;
		ss << filename_ <<";"<< width_ <<";"<< height_ <<";";
		ss << offset_.x << ";" << offset_.y << ";";
		ss << histogramSmall_.toString();
		ss << histogramLarge_.toString();
		return ss.str();
	}

	float Descriptor::compare(Descriptor& desc)
	{
		float histLargeDiff = desc.histogramLarge_.compare(histogramLarge_); 
		float histSmallDiff = desc.histogramSmall_.compare(histogramSmall_);
		float thumbnailDiff = desc.thumbnail_.compare(thumbnail_);
		float gistDiff 		= desc.gist_.compare(gist_);

		LOG_MSG_(1) << fmt("% % % %") % histSmallDiff % histLargeDiff % gistDiff % thumbnailDiff;	
		if (histLargeDiff == INVALID_MATCH || histSmallDiff == INVALID_MATCH ||
				thumbnailDiff == INVALID_MATCH || gistDiff == INVALID_MATCH) return INVALID_MATCH;

		float sum = (histLargeDiff + histSmallDiff + thumbnailDiff + gistDiff) / 4.0f;
		//		cout << "Sum = " << sum << endl;
		return sum;
	}

}
