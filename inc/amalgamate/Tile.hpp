#pragma once
#include <stdlib.h>
#include <Magick++.h>
#include <vector>

#include "amalgamate/utils.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate
{
	class Tile : public vector<Point>
	{
	public:
		Rect getRect(int width = 1, int height = 1);
		Image getMask(int width, int height);
		list<Coordinate> getCoords(int width = 1, int height = 1);

		friend Writer& operator<<(Writer& w, Tile& t)
		{
			w << t.size(); 
			BOOST_FOREACH( Point& p, t ) w << p;
			return w;
		}

		friend Reader& operator>>(Reader& r, Tile& t)
		{
			size_t n = 0; 
			r >> n; 
			t.resize(n);
			for (size_t i = 0; i < n; i++) r >> t[i];
			return r;
		}
	};
}


