#pragma once
#include <tbd/config.h>
#include "amalgamate/TileList.hpp"

using namespace tbd;
using namespace std;
using namespace Magick;

namespace amalgamate
{
	class TileGenerator : public ConfigurableObject 
	{
	public:
		TileGenerator(Config* _config = NULL) : ConfigurableObject(_config) 
		{
			LOG_MSG << fmt("Generate % tiles ...") % nameString();
		}

		virtual string nameString() { return "TileGenerator"; }
		virtual TileList generate(Image& img) = 0;
	};
}

