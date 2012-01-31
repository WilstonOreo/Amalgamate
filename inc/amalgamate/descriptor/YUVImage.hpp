#pragma once

#include <stdlib.h>
#include <Magick++.h> 

#include "amalgamate/utils.hpp"
#include "amalgamate/descriptor/ImageDescriptor.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate 
{
	namespace descriptor
	{
	#define YUV_MAX_DIFF float(4*255*255 + 255*255 + 255*255)
	#define INVALID -1.0f
	class YUVImage : public ImageDescriptor {
		public:
		#define IMAGE_SIZE 256
			static const size_t nPixels = IMAGE_SIZE*IMAGE_SIZE;
			typedef u8 PixBuf[IMAGE_SIZE*IMAGE_SIZE];

			YUVImage(Image& image);
			inline u8 y(int idx) { return y_[idx]; }
			inline u8 u(int idx) { return u_[idx]; }
			inline u8 v(int idx) { return v_[idx]; }
			inline u8 y(int _x, int _y) { return y_[_y*IMAGE_SIZE+_x]; }
			inline u8 u(int _x, int _y) { return u_[_y*IMAGE_SIZE+_x]; }
			inline u8 v(int _x, int _y) { return v_[_y*IMAGE_SIZE+_x]; }
		private:
			PixBuf y_,u_,v_;
	};
	}
}

