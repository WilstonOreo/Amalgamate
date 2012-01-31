#include "amalgamate/Tile.hpp"

#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Magick;
using namespace boost;


namespace amalgamate
{
	void Tile::set(Image& img, Geometry& geom)
	{
		x1 = double(img.columns())/double(geom.width());
		y1 = double(img.rows())/double(geom.height());
		x2 = double(img.columns())/double(geom.width());
		y2 = double(img.rows())/double(geom.height());	
		validate();
	}

	void Tile::set(double _x1, double _y1, double _x2, double _y2)
	{
		x1 = _x1; y1 = _y1; x2 = _x2; y2 = _y2; validate();
	}

	Rect Tile::get(Image& img) 
	{ 
		return Rect(int(img.columns()*x1),
					int(img.rows()*y1),
					int(img.columns()*x2),
					int(img.rows()*y2));
	}

	string Tile::toString()
	{
		stringstream ss; ss << x1 << ";" << y1 << ";";
		ss << x2 << ";" << y2; return ss.str();
	}

	bool Tile::fromString(string str)
	{
		vector<string> values;
		split( values, str, is_any_of(";"), token_compress_on );

		if (values.size()==4)
		{
			x1 = atof(values[0].c_str());
			y1 = atof(values[1].c_str());
			x2 = atof(values[2].c_str());
			y2 = atof(values[3].c_str());			
			return true;
		} else
			return false;
	}
}

