#pragma once
#include <vector>
#include <Magick++.h>
#include <tbd/config.h>

#include "amalgamate/utils.hpp"
#include "amalgamate/Tile.hpp"

using namespace tbd;
using namespace std;
using namespace Magick;

namespace amalgamate
{
	class TileList : public vector<Tile> 
	{
	public:
		TileList(string inputFile = string());

		void read(string inputFile);
		void write(string outputFile);

		void visualize(Image& image);
		void visualize(string& inputImageFile, string& outputImageFile);
	};
}
