#include "amalgamate/descriptor/Thumbnail.hpp"

namespace amalgamate
{
	namespace descriptor
	{
		float Thumbnail::border_ = 1.0f/6.0f;

		void Thumbnail::build(YUVImage& yuvImage)
		{
			int x,y; 
			u8 uBuf[NPIXELS], vBuf[NPIXELS];

			orgWidth(yuvImage.orgWidth()); orgHeight(yuvImage.orgHeight());

			#define TS THUMBNAIL_SIZE
			FOR_2D(x,y,TS) 
			{
				int ySum = 0, uSum = 0, vSum = 0, xx = 0, yy = 0;
				FOR_2D(xx,yy,SIZE_FACTOR)
				{
					ySum += yuvImage.y(x*SIZE_FACTOR+xx,y*SIZE_FACTOR+yy);
					uSum += yuvImage.u(x*SIZE_FACTOR+xx,y*SIZE_FACTOR+yy);
					vSum += yuvImage.v(x*SIZE_FACTOR+xx,y*SIZE_FACTOR+yy);
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

		PixelPacket Thumbnail::get(int x, int y)
		{
			PixelPacket p;
			p.red   = y_[y*THUMBNAIL_SIZE+x] << 8;
			p.green = u_[y/2*THUMBNAIL_SIZE/2+x/2] << 8;
			p.blue  = v_[y/2*THUMBNAIL_SIZE/2+x/2] << 8;
			return p;	
		}

		float Thumbnail::compare(const Thumbnail& thumbnail)
		{

			int sum = 0;
			FOR_1D(NPIXELS)
			{
				int diff = int(y_[i]) - int(thumbnail.y_[i]);
				sum += 4*diff*diff;
			}
			
			FOR_1D(NPIXELS/4)
			{
				int diffU = int(u_[i]) - int(thumbnail.u_[i]),
					diffV = int(v_[i]) - int(thumbnail.v_[i]);
				sum += diffU*diffU + diffV*diffV;
			}
			return float(sum) / YUV_MAX_DIFF / NPIXELS; 
		}

		float Thumbnail::compare(Thumbnail& thumbnail, Rect& rect)
		{
			LOG_MSG_(1) << fmt("Comparing Thumbnail ... Size %") % int(THUMBNAIL_SIZE);
			Geometry size(THUMBNAIL_SIZE,THUMBNAIL_SIZE);
			Image descImg(size,Color(0,0,0)); descImg.modifyImage();
			Image thisImg(size,Color(0,0,0)); thisImg.modifyImage();
			PixelPacket* descPixels = descImg.getPixels(0,0,THUMBNAIL_SIZE,THUMBNAIL_SIZE);
			PixelPacket* thisPixels = thisImg.getPixels(0,0,THUMBNAIL_SIZE,THUMBNAIL_SIZE);

			int X,Y; 
			FOR_2D(X,Y,THUMBNAIL_SIZE)
			{
				descPixels[Y*THUMBNAIL_SIZE+X] = thumbnail.get(X,Y);
				thisPixels[Y*THUMBNAIL_SIZE+X] = get(X,Y);
			}
			descImg.syncPixels(); thisImg.syncPixels();
			Geometry descSize; descSize.aspect(true);
			
			LOG_MSG_(2) << fmt("Calculate Aspect ratios... Border: %") % Thumbnail::border();
			double b = (1.0 + 2*Thumbnail::border());
			double descAspect = (double)thumbnail.orgWidth() / (double)thumbnail.orgHeight();
			double aspect = (double)orgWidth()/(double)orgHeight();
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

			w *= THUMBNAIL_SIZE*b;
			h *= THUMBNAIL_SIZE*b;

			LOG_MSG_(2) << fmt("thumbnail.orgWidth = %, thumbnail.orgHeight = %, orgWidth = %, orgHeight = %, w = %, h = %") 
				% thumbnail.orgWidth() % thumbnail.orgHeight() % orgWidth() % orgHeight() % w % h;

			descSize.width(int(w));
			descSize.height(int(h));
			descImg.resize( descSize );

			Point p(descSize.width()  - THUMBNAIL_SIZE, 
					descSize.height() - THUMBNAIL_SIZE);
			Point foundPos(p.x/2,p.y/2);

			LOG_MSG_(2) << "Comparing ... ";

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
			if (orgWidth()>orgHeight())
			{ dw = orgWidth()*b;  dh = orgWidth()*b; } 
			else
			{ dw = orgHeight()*b; dh = orgHeight()*b; }

			if (descAspect > 1.0) 	dh /= descAspect; 
			else					dw *= descAspect;

			double fx = foundPos.x/w*dw, fy = foundPos.y/h*dh; 
			foundPos.set(int(fx),int(fy));
			rect.set(-foundPos.x,-foundPos.y,
					-foundPos.x+dw,-foundPos.y+dh);

			return float(minDiff) / YUV_MAX_DIFF / NPIXELS;
		}

		int Thumbnail::pixelDiff(PixelPacket a, PixelPacket b)
		{
			int dy = a.red - b.red;
			int du = a.green - b.green;
			int dv = a.blue - b.blue;
			dy >>= 8; du >>= 8; dv >>= 8; 
			return (4*dy*dy + du*du + dv*dv);
		}
	}
}
