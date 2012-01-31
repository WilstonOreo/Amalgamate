#pragma once

#include <stdlib.h>
#include <Magick++.h> 

#include "amalgamate/descriptor/GIST.hpp"
#include "amalgamate/descriptor/Histogram.hpp"
#include "amalgamate/descriptor/Thumbnail.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate {

	enum DescriptorType { DT_HISTSMALL, DT_HISTLARGE, DT_GIST, DT_THUMBNAIL, DT_ };

	class Descriptor 
	{
		public:
			Descriptor();
			Descriptor(Image& image); 
			Descriptor(Image& image, int _width, int _height, int _offX, int _offY); 
			Descriptor(string _filename);

			long long		id() 		{ return (long long)this; }
			int 			width() 	{ return width_; }
			int 			height() 	{ return height_; }
			Point 			offset() 	{ return offset_; }
			string 			filename() 	{ return filename_; }
			void 			filename(string _filename) { filename_=_filename; }
			int 			index() 	{ return index_; }
			void 			index(int _index) { index_=_index; }

			descriptor::Histogram& 	histogramSmall() { return histogramSmall_; }
			descriptor::Histogram& 	histogramLarge() { return histogramLarge_; }
			descriptor::Thumbnail& 	thumbnail() { return thumbnail_; }
			descriptor::GIST& 		gist() 		{ return gist_; }

			bool 			build(string filename_, 
								  float _width = 1.0, float _height = 1.0, 
								  float _offX = 0.0,  float _offY = 0.0);

			void			build(Image& image);
			void 			build(Image& image, int _width, int _height, int _offX, int _offY);
			string 			toString();

			float compare(Descriptor& desc);

			friend Writer& operator<< (Writer& os, const Descriptor& id)
			{
				os << id.index_ << id.filename_; 
				os << id.width_ << id.height_ << id.offset_.x << id.offset_.y;
				os << id.histogramSmall_ << id.histogramLarge_ << id.gist_ << id.thumbnail_;
				return os;
			}

			friend Reader& operator>> (Reader& is, Descriptor& id)
			{
				is >> id.index_ >> id.filename_;
				is >> id.width_ >> id.height_ >> id.offset_.x >> id.offset_.y;
				is >> id.histogramSmall_ >> id.histogramLarge_ >> id.gist_ >> id.thumbnail_;
				return is;
			}

		private:
			static int count; 

			int pixelDiff(PixelPacket a, PixelPacket b );
			int width_, height_, index_;
			Point offset_;
			string 		filename_;
			descriptor::Histogram 	histogramSmall_, histogramLarge_;
			descriptor::GIST 		gist_;
			descriptor::Thumbnail 	thumbnail_;
	};

	typedef vector<Descriptor*> Descriptors;
}
