#include "amalgamate/descriptor.hpp"


#include "boost/foreach.hpp"

using namespace boost;


namespace amalgamate
{

	TileDescImage::TileDescImage(Image& image) 
	{
		Image img = image;
		Geometry geom(IMAGE_SIZE,IMAGE_SIZE);	 
		geom.aspect( true );
		img.resize( geom );

		PixelPacket* pixels = img.getPixels( 0,0,IMAGE_SIZE,IMAGE_SIZE);
		int xx,yy;
		FOR_2D(xx,yy,IMAGE_SIZE)
		{
			int pos = yy*IMAGE_SIZE+xx;
			float Y = 0.299f*pixels[pos].red + 0.587f*pixels[pos].green + 0.114f*pixels[pos].blue;
			y_[pos] = clip(int(Y) / 256); 
			u_[pos] = clip(128+int((pixels[pos].blue-Y)*0.496f) / 256);
			v_[pos] = clip(128+int((pixels[pos].red-Y)*0.877f) / 256);
		}
	}

	// DescHistogram class /////////////////////////////////////////////////
	int DescHistogram::compare(DescHistogram* hist)
	{
		int sum = 0;
		for (int i = 0; i < COLORRANGE; i++)
		{
			int ydiff = ((int)y_[i] - (int)hist->y_[i]) >> 8;
			int udiff = ((int)u_[i] - (int)hist->u_[i]) >> 8;
			int vdiff = ((int)v_[i] - (int)hist->v_[i]) >> 8;
			sum +=  4*ydiff*ydiff+udiff*udiff+vdiff*vdiff;
		}
		return sum;
	}

	void DescHistogram::build(TileDescImage& tdImage)
	{
		// Clear histogram
		for (int i = 0; i < COLORRANGE; i++) { y_[i] = 0; u_[i] = 0; v_[i] = 0; }
		for (size_t i = 0; i < TileDescImage::nPixels; i++)
		{
			y_[tdImage.y(i)]++;
			u_[tdImage.u(i)]++;
			v_[tdImage.v(i)]++;
		}		
	}

	string DescHistogram::toString() 
	{
		stringstream ss;
		#define PRINT_CHANNEL(c) for (int i=0; i<COLORRANGE; i+=16) { \
									int sum = 0; \
									for (int j=0; j<16; j++) sum += (c)[i+j]; \
									ss << (sum/16); if (i<COLORRANGE-1) ss << ","; } \
									ss << ";"; 
		ss << "Hist: ";
		ss << "Y: "; PRINT_CHANNEL(y_)
		ss << "U: "; PRINT_CHANNEL(u_)
		ss << "V: "; PRINT_CHANNEL(v_)
		#undef PRINT_CHANNEL
		return ss.str();
	}

	// DescThumbnail class /////////////////////////////////////////////////i
	void DescThumbnail::build(TileDescImage& tdImage)
	{
		int x,y; 
		u8 uBuf[NPIXELS], vBuf[NPIXELS];
		#define TS THUMBNAIL_SIZE
		FOR_2D(x,y,TS) 
		{
			int ySum = 0, uSum = 0, vSum = 0, xx = 0, yy = 0;
			FOR_2D(xx,yy,SIZE_FACTOR)
			{
				ySum += tdImage.y(x*SIZE_FACTOR+xx,y*SIZE_FACTOR+yy);
				uSum += tdImage.u(x*SIZE_FACTOR+xx,y*SIZE_FACTOR+yy);
				vSum += tdImage.v(x*SIZE_FACTOR+xx,y*SIZE_FACTOR+yy);
			}
			y_[y*TS+x] = ySum/(SIZE_FACTOR*SIZE_FACTOR);
			uBuf[y*TS+x] = uSum/(SIZE_FACTOR*SIZE_FACTOR);
			vBuf[y*TS+x] = vSum/(SIZE_FACTOR*SIZE_FACTOR);
		}
		FOR_2D(x,y,TS/2)
		{
			int x2 = x*2; int y2 = y*2;
			u_[y*TS/2+x] = (uBuf[ y2   *TS+x2] + uBuf[ y2   *TS+x2+1] +
						    uBuf[(y2+1)*TS+x2] + uBuf[(y2+1)*TS+x2+1])/4;
			v_[y*TS/2+x] = (vBuf[ y2   *TS+x2] + vBuf[ y2   *TS+x2+1] +
						    vBuf[(y2+1)*TS+x2] + vBuf[(y2+1)*TS+x2+1])/4;
		}
	}

	PixelPacket DescThumbnail::get(int x, int y)
	{
		PixelPacket p;
		p.red   = y_[y*THUMBNAIL_SIZE+x] << 8;
		p.green = u_[y/2*THUMBNAIL_SIZE/2+x/2] << 8;
		p.blue  = v_[y/2*THUMBNAIL_SIZE/2+x/2] << 8;
		return p;	
	}

	// ImageDescriptor class /////////////////////////////////////////////////
	ImageDescriptor::ImageDescriptor() 
	{ 
		used_ = 0;
		offset_.set(0,0);
		width_ = 0; height_ = 0;
	};
	
	ImageDescriptor::ImageDescriptor( Image& image )
	{ 
		used_ = 0;
		build(image); 
	} 

	ImageDescriptor::ImageDescriptor( string _filename )
	{ 
		used_ = 0;
		filename_ = _filename;
		Image image; image.read(filename());
		build(image); 
	}

	void ImageDescriptor::build(Image& image)
	{
		offset_.set(0,0);
		width_ = image.columns(); height_ = image.rows();
		TileDescImage tdImage(image);
		histogram_.build(tdImage);
		thumbnail_.build(tdImage);
	}

	string ImageDescriptor::toString()
	{
		stringstream ss;
		ss << filename_ <<";"<< width_ <<";"<< height_ <<";"<< histogram_.toString();
		return ss.str();
	}

	int ImageDescriptor::pixelDiff(PixelPacket a, PixelPacket b)
	{
		int dy = a.red - b.red;
		int du = a.green - b.green;
		int dv = a.blue - b.blue;
		dy >>= 8; du >>= 8; dv >>= 8; 
		return (4*dy*dy + du*du + dv*dv);
	}

	ImageDescriptorDiff ImageDescriptor::compare(ImageDescriptor* desc)
	{
		Geometry size(THUMBNAIL_SIZE,THUMBNAIL_SIZE);
		Image descImg(size,Color(0,0,0)); descImg.modifyImage();
		Image thisImg(size,Color(0,0,0)); thisImg.modifyImage();
		PixelPacket* descPixels = descImg.getPixels(0,0,THUMBNAIL_SIZE,THUMBNAIL_SIZE);
		PixelPacket* thisPixels = thisImg.getPixels(0,0,THUMBNAIL_SIZE,THUMBNAIL_SIZE);

		int X,Y; 
		FOR_2D(X,Y,THUMBNAIL_SIZE)
		{
			descPixels[Y*THUMBNAIL_SIZE+X] = desc->thumbnail().get(X,Y);
			thisPixels[Y*THUMBNAIL_SIZE+X] = thumbnail().get(X,Y);
		}
		descImg.syncPixels(); thisImg.syncPixels();
		ImageDescriptorDiff result;
		Geometry descSize; descSize.aspect(true);

		#define BORDER 1.0/6.0
		double border = (1.0 + 2*BORDER);
		double descAspect = (double)desc->width() / (double)desc->height();
		double aspect = (double)width()/(double)height();
		double w = 1.0, h = 1.0;

		if (descAspect < 1.0) 
		{
			h = 1.0/descAspect;
			if (aspect > 1.0) h /= aspect; 
			else 			  h *= aspect;
		}
		else
		{
			w = descAspect;
			if (aspect > 1.0) w /= aspect; 
			else 			  w *= aspect;
		}
		if (w < 1.0) { double tmp = h; h = h/w; w = tmp; } else
		if (h < 1.0) { double tmp = w; w = w/h; h = tmp; }
		
		w *= THUMBNAIL_SIZE*border;
		h *= THUMBNAIL_SIZE*border;
		descSize.width(int(w));
		descSize.height(int(h));
		descImg.resize( descSize );

		Point p(descSize.width()  - THUMBNAIL_SIZE, 
			    descSize.height() - THUMBNAIL_SIZE);
		Point foundPos(p.x/2,p.y/2);

		int minDiff = 1 << 30;
		for (int py = 0; py <= p.y; py+=2)
			for (int px = 0; px <= p.x; px+=2)
			{
				int diff = 0;
				FOR_2D(X,Y,THUMBNAIL_SIZE)
					diff += pixelDiff(thisPixels[Y*THUMBNAIL_SIZE+X],
									  descPixels[(Y+py)*THUMBNAIL_SIZE+X+px]);
				if (diff < minDiff) 
					{ minDiff = diff; foundPos.set(px,py); }
			}

		double dw = 0.0, dh = 0.0;
		if (width()>height())
		{
			dw = width()*border;
			dh = width()*border;
		} else
		{
			dw = height()*border;
			dh = height()*border;
		}
		if (descAspect > 1.0) 	dw *= descAspect; 
		else					dh /= descAspect;
	
		double fx = foundPos.x/w*dw; 
		double fy = foundPos.y/h*dh; 
		
		foundPos.set(int(fx),int(fy));
		result.diff = minDiff;
		result.rect.set(-foundPos.x,
						-foundPos.y,
						-foundPos.x+dw,
						-foundPos.y+dh);
		return result;
	}

}
