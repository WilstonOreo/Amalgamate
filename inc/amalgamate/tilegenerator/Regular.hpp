#pragma once 
#include <Magick++.h>
#include "amalgamate/TileGenerator.hpp"

using namespace std;
using namespace Magick;	

namespace amalgamate
{		
	class Regular : public TileGenerator 	
	{	
	public:
		Regular(Config* _config = NULL) : TileGenerator(_config)
		{

		}

		string nameString() { return string("Regular"); }

		TileList generate(Image& img)	
		{	
			double 	tileWidth  = 1.0/double(tilesHorz()),
					tileHeight = 1.0/double(tilesVert());
			TileList tileList;

			for (size_t y = 0; y < tilesVert(); y++)
				for (size_t x = 0; x < tilesHorz(); x++)
				{
					Tile tile; 
					double xW  = double(x)*tileWidth, 	xH  = double(y)*tileHeight,
						   xW1 = double(x+1)*tileWidth, xH1 = double(y+1)*tileHeight;

					tile.push_back(Point(xW ,xH ));
					tile.push_back(Point(xW1,xH ));
					tile.push_back(Point(xW1,xH1));
					tile.push_back(Point(xW ,xH1));
					tileList.push_back(tile);
				}

			LOG_MSG << fmt("% tiles generated. ") % tileList.size();
			return tileList;
		}	

		TBD_DECLARE_PROPERTY_CFG(unsigned,tilesHorz,"REGULAR_TILES_HORZ",10);
		TBD_DECLARE_PROPERTY_CFG(unsigned,tilesVert,"REGULAR_TILES_VERT",10);
	};
}
