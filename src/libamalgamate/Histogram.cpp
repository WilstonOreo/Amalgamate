#include "amalgamate/descriptor/Histogram.hpp"


namespace amalgamate
{
	namespace descriptor
	{
	// Histogram class /////////////////////////////////////////////////
	
	Histogram::Histogram()
	{
		setColorRange(256);
	}

	Histogram::Histogram(int _colorRange)
	{
		setColorRange(_colorRange);
	}

	Histogram::Histogram(Image& image, u16 _colorRange)
	{
		YUVImage yuvImage(image); 
		build(yuvImage,_colorRange); 
	}

	Histogram::Histogram(YUVImage& yuvImage, u16 _colorRange)
	{
		build(yuvImage,_colorRange); 
	}


	float Histogram::compare(Histogram& hist)
	{
		if (hist.colorRange_ != colorRange_) return INVALID_MATCH;

		LOG_MSG_(3) << fmt("%, %") % colorRange_ % hist.colorRange_; 
		int sum = 0;
		for (int i = 0; i < colorRange_; i++)
		{
			int ydiff = ((int)y_[i] - (int)hist.y_[i]) >> 4;
			int udiff = ((int)u_[i] - (int)hist.u_[i]) >> 4;
			int vdiff = ((int)v_[i] - (int)hist.v_[i]) >> 4;
			sum +=  4*ydiff*ydiff+udiff*udiff+vdiff*vdiff;
		}
		sum >>= 8;

		return float(sum) / YUV_MAX_DIFF;
	}

	void Histogram::build(YUVImage& yuvImage, u16 _colorRange)
	{
		setColorRange(_colorRange);
		orgWidth(yuvImage.orgWidth()); orgHeight(yuvImage.orgHeight());
		// Clear histogram
		for (size_t i = 0; i < YUVImage::nPixels; i++)
		{
			y_[yuvImage.y(i)*_colorRange/256]++;
			u_[yuvImage.u(i)*_colorRange/256]++;
			v_[yuvImage.v(i)*_colorRange/256]++;
		}	
		for (size_t i = 0; i < colorRange_; i++)
		{
			y_[i] = y_[i]*256*256/YUVImage::nPixels;
			u_[i] = u_[i]*256*256/YUVImage::nPixels;
			v_[i] = v_[i]*256*256/YUVImage::nPixels;
		}
	}

	string Histogram::toString() 
	{
		stringstream ss;
		#define N 1
		#define PRINT_CHANNEL(c) for (int i=0; i<colorRange_; i+=N) { \
									int sum = 0; \
									for (int j=0; j<N; j++) sum += (c)[i+j]; \
									ss << (sum/N); if (i<colorRange_-1) ss << ","; } \
									ss << ";"; 
		ss << "Hist: ";
		ss << "Y: "; PRINT_CHANNEL(y_)
		ss << "U: "; PRINT_CHANNEL(u_)
		ss << "V: "; PRINT_CHANNEL(v_)
		#undef PRINT_CHANNEL
		#undef N
		return ss.str();
	}

	void Histogram::setColorRange(u16 _colorRange)
	{ 
		_colorRange = (_colorRange > 256) ? 256 : 
					  (_colorRange < 4  ) ? 4   : _colorRange;
		colorRange_=_colorRange;
		y_.resize(colorRange_);
		u_.resize(colorRange_);
		v_.resize(colorRange_);
		for (int i = 0; i < colorRange_; i++) 
			{ y_[i] = 0; u_[i] = 0; v_[i] = 0; }
	}
}
}

