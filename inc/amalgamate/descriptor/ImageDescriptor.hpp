#pragma once

#include <stdlib.h>
#include <Magick++.h> 
#include <vector>

#include "amalgamate/Config.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate {

	namespace descriptor
	{

		class ImageDescriptor {
			public:
				ImageDescriptor(int _orgWidth = 0, int _orgHeight = 0)
				{
					orgWidth(_orgWidth);
					orgHeight(_orgHeight);
				}

				int orgWidth() const { return orgWidth_; }
				int orgHeight() const { return orgHeight_; }
				void orgWidth(int _orgWidth) { orgWidth_=_orgWidth; }
				void orgHeight(int _orgHeight) { orgHeight_=_orgHeight; }

				static Config* config;
			private:
				int orgWidth_, orgHeight_;

		};
	}
}
