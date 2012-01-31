#pragma once

#include <vector>

#include "amalgamate/descriptor/YUVImage.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate {

	namespace descriptor 
	{
	class Histogram : public ImageDescriptor {
		typedef vector<u16> HistogramChannel;
	public:
		Histogram();
		Histogram(int _colorRange);
		Histogram(Image& image, u16 _colorRange);
		Histogram(YUVImage& yuvImage, u16 _colorRange);

		void build(YUVImage& yuvImage, u16 _colorRange = 256);
		float compare(Histogram& hist);
		string toString();

		int colorRange() { return colorRange_; }
		
		inline u16 y(size_t idx) { return y_[idx]; }
		inline u16 u(size_t idx) { return u_[idx]; }
		inline u16 v(size_t idx) { return v_[idx]; }

		friend Writer& operator<< (Writer& os, const Histogram& dh)
		{
			os << dh.colorRange_;
			FOR_1D(dh.colorRange_) os << dh.y_[i] << dh.u_[i] << dh.v_[i]; 
			return os;
		}

		friend Reader& operator>> (Reader& is, Histogram& dh)
		{			
			is >> dh.colorRange_; dh.setColorRange(dh.colorRange_);
			FOR_1D(dh.colorRange_) is >> dh.y_[i] >> dh.u_[i] >> dh.v_[i]; 
			return is;
		}

	private:
		void setColorRange(u16 _colorRange); 

		HistogramChannel y_, u_, v_;
		u16 colorRange_;
	};
	}
}
