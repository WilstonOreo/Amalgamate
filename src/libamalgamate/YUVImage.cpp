#include "amalgamate/descriptor/YUVImage.hpp"

#include "boost/foreach.hpp"

using namespace boost;

namespace amalgamate
{
	namespace descriptor
	{
		YUVImage::YUVImage(Image& image) 
		{
			Image img = image;
			Geometry geom(IMAGE_SIZE,IMAGE_SIZE);	 
			geom.aspect( true );
			img.resize( geom );

			orgWidth(image.columns());
			orgHeight(image.rows());

			PixelPacket* pixels = img.getPixels( 0,0,IMAGE_SIZE,IMAGE_SIZE);
			int xx,yy;
			FOR_2D(xx,yy,IMAGE_SIZE)
			{
				int pos = yy*IMAGE_SIZE+xx;
				float Y = 0.299f*pixels[pos].red + 0.587f*pixels[pos].green + 0.114f*pixels[pos].blue;
				y_[pos] = CLIP_u8(int(Y) / 256); 
				u_[pos] = CLIP_u8(128+int((pixels[pos].blue-Y)*0.496f) / 256);
				v_[pos] = CLIP_u8(128+int((pixels[pos].red-Y)*0.877f) / 256);
			}
		}
	}
}

