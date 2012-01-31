#pragma once
#include <vector>
#include <Magick++.h>

#include "amalgamate/utils.hpp"
#include "amalgamate/Config.hpp"
#include "amalgamate/Tile.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate
{
	class TileGenerator;
	enum TileGenType { TGTYPE_REGULAR, TGTYPE_BSP, TGTYPE_WARP, TGTYPE_COLLAGE };

	class TileList : public vector<Tile> {
	public:
		TileList(Config* _config);
		TileList(string inputFile);

		Config* config() 		{ return config_; }
		void config(Config* _config) { config_=_config; }

		TileGenerator* tileGen() { return tileGen_; }

		void read(string inputFile);
		void write(string outputFile);
		void generate(Image& image); 
		void generate(string inputFile, string outputFile);

		void visualize(Image& image);
		void visualize(string inputImageFile, string outputImageFile);
	private:
		TileGenType getTileGenType();
		Config* config_;
		TileGenerator* tileGen_;
	};
}


