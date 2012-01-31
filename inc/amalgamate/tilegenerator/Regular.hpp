#pragma once 
#include <Magick++.h>
#include "amalgamate/TileGenerator.hpp"

using namespace std;
using namespace Magick;	

namespace amalgamate
{		
	namespace tilegenerator 
	{
	class Regular : public TileGenerator 	
	{	
	public:	
		string nameString() { return string("Regular"); }

		void genTiles(Image& img, TileList* tileList)	
		{	
			double tileWidth  = 1.0f/double(size().width());
			double tileHeight = 1.0f/double(size().height());

			for (size_t y = 0; y < size().height(); y++)
				for (size_t x = 0; x < size().width(); x++)
				{
					Tile tile; tile.set(double(x)*tileWidth,
										double(y)*tileHeight,
										double(x+1)*tileWidth,
										double(y+1)*tileHeight);
					tileList->push_back(tile);
				}

			cout << tileList->size() << " tiles generated. " << endl;
		}	
	};	
}
}

