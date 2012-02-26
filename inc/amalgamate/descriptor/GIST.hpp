#pragma once

#include <stdlib.h>
#include <Magick++.h> 
#include <vector>

#include "amalgamate/descriptor/YUVImage.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate {

	namespace descriptor 
	{
	class GIST : public ImageDescriptor {
		public:
			GIST() {}
			GIST(YUVImage& yuvImage) { build(yuvImage); }

			void build(YUVImage& yuvImage);
			float compare(const GIST& gist);

			friend Writer& operator<< (Writer& os, const GIST& gist)
			{
				os << gist.desc_.size();
				FOR_1D(gist.desc_.size()) os << gist.desc_[i];
				return os;
			}

			friend Reader& operator>> (Reader& is, GIST& gist)
			{
				size_t desc_size;

				is >> desc_size; gist.desc_.resize(desc_size);
				FOR_1D(gist.desc_.size()) is >> gist.desc_[i];
				return is;
			}
			
			size_t size() { return desc_.size();  }

		private:
			vector<u16> desc_;
	};
	}
}
