#pragma once
#include <stdlib.h>
#include <Magick++.h>
#include <vector>

#include "amalgamate/utils.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate
{
	struct Tile {
		union {
			struct { double x1,y1,x2,y2; };
			double coords[4];
		};

		void validate() {		
			if (x1 > x2) swap(x1,x2);
			if (y1 > y2) swap(y1,y2);
		}

		void set(Image& img, Geometry& geom);
		void set(double _x1, double _y1, double _x2, double _y2);
		Rect get(Image& img); 

		string toString();
		bool fromString(string str);

		inline double area() const
		{
			double A=(x2-x1)*(y2-y1);
			if (A < 0.0) A = -A;
			return A;
		}
	};
}


