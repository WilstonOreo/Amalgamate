#pragma once

#include <stdlib.h>
#include <Magick++.h> 

#include "amalgamate/utils.hpp"
#include "amalgamate/descriptor/YUVImage.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate 
{

	namespace descriptor 
	{
		class Thumbnail : public ImageDescriptor {
			public:
#define SIZE_FACTOR 8
#define THUMBNAIL_SIZE IMAGE_SIZE/SIZE_FACTOR
#define NPIXELS THUMBNAIL_SIZE*THUMBNAIL_SIZE
				static const size_t nPixels = NPIXELS;

				Thumbnail() {}
				Thumbnail(YUVImage& yuvImage) { build(yuvImage); }

				void build(YUVImage& yuvImage);
				float compare(const Thumbnail& thumbnail);
				float compare(Thumbnail& thumbnail, Rect& rect);

				PixelPacket get(int x, int y);
				friend Writer& operator<< (Writer& os, const Thumbnail& tn)
				{
					os << tn.orgWidth() << tn.orgHeight();
					FOR_1D(nPixels)   os << tn.y_[i];
					FOR_1D(nPixels/4) os << tn.u_[i];
					FOR_1D(nPixels/4) os << tn.v_[i];
					return os;
				}

				friend Reader& operator>> (Reader& is, Thumbnail& tn)
				{
					int buf; 
					is >> buf; tn.orgWidth(buf);
					is >> buf; tn.orgHeight(buf);
					FOR_1D(nPixels)   is >> tn.y_[i];
					FOR_1D(nPixels/4) is >> tn.u_[i];
					FOR_1D(nPixels/4) is >> tn.v_[i];
					return is;
				}

				static float border() { return Thumbnail::border_; }
				static void border(float _border) 
				{ 
					if (_border<0.0) _border = 0; 
					Thumbnail::border_=_border; 
				}

			private:
				int pixelDiff(PixelPacket a, PixelPacket b);
				u8 y_[NPIXELS];
				u8 u_[NPIXELS/4];
				u8 v_[NPIXELS/4];
				int orgWidth_, orgHeight_;
				static float border_;

		};
	}
}
