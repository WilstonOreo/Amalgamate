#pragma once
#include <Magick++.h>
#include <vector>
#include <tbd/config.h>

#include "amalgamate/utils.hpp"
#include "amalgamate/Tile.hpp"
#include "amalgamate/TileList.hpp"

using namespace tbd;
using namespace std;
using namespace Magick;

namespace amalgamate
{
	class TileGenerator : public ConfigurableObject {
	public: 

		virtual string nameString() = 0;
		virtual void genTiles(Image& img, TileList* tileList) = 0;
	protected:

		Geometry& size() { return size_; }
		void size( Geometry& _size ) { size_=_size; }

		void addTile(Rect rect, Image* img, TileList* tileList)
		{
			if (rect.x1() >= (int)img->columns() || 
				rect.y1() >= (int)img->rows() ||
				rect.x2() < 0 || rect.y2() < 0) return;

			if (rect.x1() <  0) rect.x1(0);
			if (rect.y1() < 0) rect.y1(0);
			if (rect.x2() >= (int)img->columns()) 	rect.x2(img->columns());
			if (rect.y2() >= (int)img->rows()) 		rect.y2(img->rows());

			if (rect.width()>0 && rect.height()>0)
			{
				Tile tile; 
				tile.set(double(rect.x1())/double(img->columns()),
						 double(rect.y1())/double(img->rows()),
						 double(rect.x2())/double(img->columns()),
			 		 	 double(rect.y2())/double(img->rows()));
				tileList->push_back(tile);
			}
		};


		int getSum(Rect rect, Image* img)
		{
			int sum = 0, w = rect.width(), h = rect.height();
			PixelPacket *pixels =  img->getPixels( rect.x1(), rect.y1(), w, h);
			
			for (int y = 0; y < h; y++)
				for (int x = 0; x < w; x++)
				{
					PixelPacket pixel = pixels[y*w+x];
					sum += (pixel.red + pixel.green + pixel.blue) >> 8;
					//cout << pixel.red << "," << pixel.green << "," << pixel.blue << "," << sum << endl; 
				}

			return sum;
		}


		int getSumColumn( int columnIdx, Rect rect, Image* img)
		{
			return getSum( Rect(columnIdx,rect.y1(),columnIdx+1,rect.y2()), img);
		}

		int getSumRow( int rowIdx, Rect rect, Image* img)
		{
			return getSum( Rect(rect.x1(),rowIdx,rect.x2(),rowIdx+1), img);
		}
		int getSumLine( int lineIdx, int splitAxis, Rect rect, Image* img)
		{
			if (splitAxis == X_AXIS) 
				return getSumColumn( lineIdx, rect, img ); else
					   getSumRow( lineIdx, rect, img );
			return -1;
		}

		Geometry size_;
	};

}


