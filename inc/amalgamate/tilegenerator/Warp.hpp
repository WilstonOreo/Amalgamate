#pragma once 
#include <Magick++.h>
#include <stack>
#include <boost/foreach.hpp>

#include "amalgamate/TileGenerator.hpp"
#include "amalgamate/utils.hpp"

using namespace std;
using namespace Magick;
using namespace boost;

namespace amalgamate 
{
	namespace tilegenerator 
	{
	#define X_AXIS 0
	#define Y_AXIS 1
/*	
		bool compTileWarp(const Tile& a, const Tile& b)
		{
			double A = a.area(), B = b.area();

			if (A==B) { 
				if (a.y1 == b.y1)
					return a.x1 < b.x1;
				else
					return a.y1 < b.y1;
			}
			else
				return A < B;	
		}
*/
	typedef pair<Tile,double> WarpTile;
/*
		bool compTileWarp(const WarpTile& a, const WarpTile& b)
		{
			return a.second < b.second;
		}
*/
	class Warp : public TileGenerator
	{
	public:
		string nameString() { return string("Warp"); }

		Geometry& size() { return size_; }
		void size( Geometry& _size ) { size_=_size; }

		void genTiles(Image& img, TileList* tileList)
		{
			Image image = img;
			image.type(TrueColorType);
			image.modifyImage();
			image.depth(8);
			image.quantizeColorSpace( GRAYColorspace );
    		image.quantizeColors( 256 );
    		image.quantize( ); 
			tileList->clear();
			
			vector<WarpTile> tmpList;

			double tileWidth  = 1.0/double(size().width());
			double tileHeight = 1.0/double(size().height());

			double imgTileWidth = image.columns()/size().width();
			double imgTileHeight = image.rows()/size().height();
			
			for (size_t y = 0; y < size().height(); y++)
				for (size_t x = 0; x < size().width(); x++)
				{
					Rect rect(x*imgTileWidth,
							  y*imgTileHeight,
							  (x+1)*imgTileWidth,
							  (y+1)*imgTileHeight);
					int sum = getSum(rect,&img);

					#define THRESHOLD 0.0
					#define FACTOR 0.5

					double relSum = double(sum)/double(imgTileWidth*imgTileHeight)/256.0;
					if (relSum < THRESHOLD ) relSum = THRESHOLD;

					double factor = (relSum - THRESHOLD)/(1.0-THRESHOLD)*FACTOR/2+0.5;

					double centerX = (x+0.5)*tileWidth;
					double centerY = (y+0.5)*tileHeight;

					Tile tile; tile.set(centerX-tileWidth*factor,
										centerY-tileHeight*factor,
										centerX+tileWidth*factor,
										centerY+tileHeight*factor);
					WarpTile bTile; bTile.first = tile; bTile.second = relSum;
					tmpList.push_back(bTile);
				}

//			sort(tmpList.begin(),tmpList.end(),compTileWarp);

			BOOST_FOREACH( WarpTile tile, tmpList )
				tileList->push_back(tile.first);
				

			cout << tileList->size() << " tiles generated. " << endl;
		}

	private:

		Geometry size_;
	};
	}
}


