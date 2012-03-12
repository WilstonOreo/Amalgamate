#include "amalgamate/Tile.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate
{
	list<Coordinate>  Tile::getCoords(int width, int height)
	{
		if (empty()) return list<Coordinate>();

		list<Coordinate> coords;
		BOOST_FOREACH ( Point& p, *this )
			coords.push_back(Coordinate(p.x*width,p.y*height));
		return coords;
	}

	Rect Tile::getRect(int width, int height)
	{
		Rect rect(INF,INF,-INF,-INF);
		BOOST_FOREACH ( Point& p, *this )
			rect.set(min(rect.x1(),p.x),min(rect.y1(),p.y),
					 max(rect.x2(),p.x),max(rect.y2(),p.y));
		
		rect.scale(width,height);
		
		return rect;
	}

	Image Tile::getMask(int width, int height)
	{
		Image mask(Geometry(width,height), Color(0,0,0));
		mask.strokeWidth(1);
		mask.strokeColor("white");
		mask.fillColor("white");

		list<Coordinate> coords = getCoords(width,height);
		mask.draw( DrawablePolygon(coords) );
		return mask;
	}
}
