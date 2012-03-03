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
	class TileGenerator;
	enum TileGenType { TGTYPE_REGULAR, TGTYPE_BSP, TGTYPE_WARP, TGTYPE_COLLAGE };

	class TileList : public ConfigurableObject, public vector<Tile> {
	public:
		TileList(Config* _config = NULL);
		TileList(string inputFile);

		void read(string inputFile);
		void write(string outputFile);
		void generate(Image& image); 
		void generate(string inputFile, string outputFile);

		void visualize(Image& image);
		void visualize(string inputImageFile, string outputImageFile);

		TBD_DECLARE_PROPERTY_RO(TileGenerator*,tileGen);

	private:
		TileGenType getTileGenType();
	};
}


