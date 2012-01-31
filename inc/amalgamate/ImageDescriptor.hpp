#pragma once

#include <stdlib.h>
#include <Magick++.h> 
#include <vector>

#include "amalgamate/Config.hpp"
#include "amalgamate/descriptor/YUVImage.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate {

	class ImageDescriptor {
		public:
			ImageDescriptor() {}

			static Config* config;
	};
}
