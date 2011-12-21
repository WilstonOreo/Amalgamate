#pragma once
#include <stdlib.h>
#include <Magick++.h>
#include <vector>

#include "amalgamate/utils.hpp"
#include "amalgamate/config.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate
{
	enum TileGenType { TGTYPE_BSP, TGTYPE_REGULAR, TGTYPE_COLLAGE, TGTYPE_WARP };

	struct Tile {
	public:
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

	class TileGenerator;

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

	class TileGenerator {
	public: 
		Config* config() 		{ return config_; }
		void config(Config* _config) 
		{
			config_=_config;
			if (_config) size_ = Geometry(config_->as<string>("TILEGEN_SIZE"));
		}
		
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

		Config* config_;
		Geometry size_;
	};

}


