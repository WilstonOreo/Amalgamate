#pragma once

#include <stdlib.h>
#include <Magick++.h> 
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <list>
#include <vector>

#include "amalgamate/utils.hpp"

using namespace std;
using namespace Magick;
using namespace boost;

namespace amalgamate {

	class TileDescImage {
		public:
#define IMAGE_SIZE 256
			static const size_t nPixels = IMAGE_SIZE*IMAGE_SIZE;
			typedef u8 PixBuf[IMAGE_SIZE*IMAGE_SIZE];

			TileDescImage(Image& image);
			inline u8 y(int idx) { return y_[idx]; }
			inline u8 u(int idx) { return u_[idx]; }
			inline u8 v(int idx) { return v_[idx]; }
			inline u8 y(int _x, int _y) { return y_[_y*IMAGE_SIZE+_x]; }
			inline u8 u(int _x, int _y) { return u_[_y*IMAGE_SIZE+_x]; }
			inline u8 v(int _x, int _y) { return v_[_y*IMAGE_SIZE+_x]; }
		private:
			inline u8 clip(int i) { return (i < 0) ? 0 : ((i > 255) ? 255 : i ); }
			PixBuf y_,u_,v_;
	};

	class DescHistogram {
#define COLORRANGE 256
		typedef u16 Histogram[COLORRANGE];
		public:
		DescHistogram() {}
		DescHistogram(Image& image) { TileDescImage tdImage(image); build(tdImage); }
		DescHistogram(TileDescImage& tdImage) { build(tdImage); }

		int compare(DescHistogram* hist);
		void build(TileDescImage& tdImage);
		string toString();
		
		inline u16 y(size_t idx) { return y_[idx]; }
		inline u16 u(size_t idx) { return u_[idx]; }
		inline u16 v(size_t idx) { return v_[idx]; }

		friend Writer& operator<< (Writer& os, const DescHistogram& dh)
		{
			for (int i = 0; i < COLORRANGE; i++) 
				{  os << dh.y_[i] << dh.u_[i] << dh.v_[i]; }
			return os;
		}

		friend Reader& operator>> (Reader& is, DescHistogram& dh)
		{			
			for (int i = 0; i < COLORRANGE; i++) 
				{  is >> dh.y_[i] >> dh.u_[i] >> dh.v_[i]; } 
			return is;
		}
		private:
		Histogram y_, u_, v_;
	};

	class DescThumbnail {
		public:
			#define SIZE_FACTOR 8
			#define THUMBNAIL_SIZE IMAGE_SIZE/SIZE_FACTOR
			#define NPIXELS THUMBNAIL_SIZE*THUMBNAIL_SIZE
			static const size_t nPixels = NPIXELS;

			DescThumbnail() {}
			DescThumbnail(TileDescImage& tdImage) { build(tdImage); }

			void build(TileDescImage& tdImage);
			PixelPacket get(int x, int y);

			friend Writer& operator<< (Writer& os, const DescThumbnail& tn)
			{
				for (size_t i = 0; i < nPixels  ; i++) { os << tn.y_[i]; }
				for (size_t i = 0; i < nPixels/4; i++) { os << tn.u_[i]; }
				for (size_t i = 0; i < nPixels/4; i++) { os << tn.v_[i]; }
				return os;
			}

			friend Reader& operator>> (Reader& is, DescThumbnail& tn)
			{
				for (size_t i = 0; i < nPixels  ; i++) { is >> tn.y_[i]; }
				for (size_t i = 0; i < nPixels/4; i++) { is >> tn.u_[i]; }
				for (size_t i = 0; i < nPixels/4; i++) { is >> tn.v_[i]; }
				return is;
			}
		private:
			u8 y_[NPIXELS];
			u8 u_[NPIXELS/4];
			u8 v_[NPIXELS/4];
	};

	class DescGIST {
	
	
	};

	struct ImageDescriptorDiff {
		Rect rect;
		double diff;
	};
	class ImageDescriptor {
		public:
			ImageDescriptor();
			ImageDescriptor(Image& image); 
			ImageDescriptor(string _filename);

			int 			width() 	{ return width_; }
			int 			height() 	{ return height_; }
			Point 			offset() 	{ return offset_; }
			void 			offset(int x, int y) { offset_.set(x,y); } 
			int 			axis() 		{ return (width_>height_) ? 0 : 1; }
			DescHistogram& 	histogram() { return histogram_; }
			DescThumbnail& 	thumbnail() { return thumbnail_; }
			string 			filename() 	{ return filename_; }
			void 			filename(string _filename) { filename_=_filename; }

			int 			used() 		{ return used_; }
			void 			usedOnceMore() { used_++; }
			void 			build(Image& image);
			string 			toString();

			ImageDescriptorDiff compare(ImageDescriptor* desc);

			friend Writer& operator<< (Writer& os, const ImageDescriptor& id)
			{
				os << id.filename_; 
				os << id.width_ 	<< id.height_;
				os << id.histogram_ << id.thumbnail_;
				return os;
			}
			friend Reader& operator>> (Reader& is, ImageDescriptor& id)
			{
				is >> id.filename_;
				is >> id.width_ 	>> id.height_;
				is >> id.histogram_ >> id.thumbnail_;
				return is;
			}
		private:
			int pixelDiff(PixelPacket a, PixelPacket b );
			int width_, height_;
			Point offset_;
			string 			filename_;
			DescHistogram 	histogram_;
			DescThumbnail 	thumbnail_;
			int 			used_;
	};
}
