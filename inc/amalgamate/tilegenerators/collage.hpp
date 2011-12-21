#pragma once 
#include <deque>
#include <cstdlib>
#include "amalgamate/tiles.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate
{
	class Collage : public TileGenerator 
	{
	public:
		string nameString() { return string("Collage"); }

		#define BUFSIZE 256

		void genTiles(Image& img, TileList* tileList)
		{
			Image image = img;
			image.type(TrueColorType);
			image.modifyImage();
			image.depth(8);
			image.resize( Geometry("256x256!"));
			image.quantizeColorSpace( GRAYColorspace );
    		image.quantizeColors( 256 );
    		image.quantize( ); 
			image.edge(1.0f);
			tileList->clear();
	
			double tileWidth  = 1.0f/double(size().width());
			double tileHeight = 1.0f/double(size().height());

			vector<int> edgeBuf (BUFSIZE*BUFSIZE,0);

			PixelPacket* p = image.getPixels(0,0,BUFSIZE,BUFSIZE);
			for (int i = 0; i < BUFSIZE*BUFSIZE; i++)
				edgeBuf[i] = p[i].red;

			vector<bool> drawnBuf (BUFSIZE*BUFSIZE,false);
			Rect rect(-1,-1,-1,-1);
		//	while (!drawnBufFull(drawnBuf))
			{
				newRect(rect,drawnBuf,edgeBuf);
				drawRect(rect,drawnBuf);
				addTile(rect,&img,tileList);
			}

			cout << tileList->size() << " tiles generated. " << endl;
		}

		Geometry& size() { return size_; }
		void size( Geometry& _size ) { size_=_size; }

		float variation() { return variation_; }
		void variation( float _variation ) { variation_=_variation; }

	private:	
		int randint(int n)
		{
			return int((double)rand()/(double)RAND_MAX*n);
		}


		bool drawnBuf(int x, int y) { return drawnBuf_[y*BUFSIZE+x]; }

		void getBestEdge()
		{
		}

		enum { NOTHING, OUTERCORNER_TOPLEFT, OUTERCORNER_TOPRIGHT, EDGE_TOP,
			   OUTERCORNER_BOTTOMLEFT, EDGE_LEFT, SHRINK_RIGHT, INNERCORNER_TOPLEFT,
			   OUTERCORNER_BOTTOMRIGHT, SHRINK_LEFT, EDGE_RIGHT, INNERCORNER_TOPRIGHT,
			   EDGE_BOTTOM, INNERCORNER_BOTTOMRIGHT, INNERCORNER_BOTTOMLEFT, FULL } PixelType;

		PixelType pixelType(int x, int y)
		{
			if (x >= BUFSIZE-1 || y >= BUFSIZE-1 || x < 0 || y < 0) return INVALID;

			int pType = drawBuf(x,y) | (drawBuf(x+1,y) << 1) | 
					   (drawBuf(x,y+1) << 2) | (drawBuf(x+1,y+1) << 3);

			return PixelType(pType);
		}


		Rect getHole()
		{
			// Step1: Find top-left corner
			vector<Point> topLeftCorners;
			int x,y;
			FOR_2D(x,y,BUFSIZE-1) 
				if (pixelType(x,y)==INNERCORNER_TOPLEFT || pixelType(x,y)==SHRINK_RIGHT)
					topLeftCorners.push_back(Point(x,y));


			// Step2: For all top-left corners, find top-right corner
			BOOST_FOREACH( Point topLeftCorner, topLeftCorners)
			{
				posX = topLeftCorner.x, posY = topLeftCorner.y;
				// If top-right corner found, find bottom-left corner
				for (int x = posX+1 ; x < BUFSIZE-1; x++)
				{
					if (pixelType(x,posY)==EDGE_TOP) continue; else
					if (pixelType(x,posY)==INNERCORNER_TOPRIGHT ||
						pixelType(x,posY)==SHRINK_LEFT)
					{
						for (int y = posY+1; y < BUFSIZE-1; y++)
						{
							if (pixelType(posX,y)==EDGE_RIGHT) continue; else
							if (pixelType(posX,y)==INNERCORNER_BOTTOMLEFT ||
								pixelType(posX,y)==SHRINK_LEFT)
							{

							}
							else 
								break;
						}
					

					} else 
						break;
				
					// If bottom-left corner found, find bottom-right corner
						
						// bottom-right corner found, we have a hole --> return



		}


		void drawRect(Rect rect, vector<bool>& drawnBuf)
		{
			for (int y = rect.y1(); y < rect.y2(); y++)
				for (int x = rect.x1(); x < rect.x2(); x++)
					drawnBuf[y*BUFSIZE+x] = true;
		}

		bool drawnBufFull(vector<bool>& drawnBuf)
		{
			for (int i = 0; i < drawnBuf.size(); i++)
				if (!drawnBuf[i]) return false;

			return true;
		}


		void newRect(Rect& rect, vector<bool>& drawnBuf, vector<int>& edgeBuf)
		{
			Rect oldRect = rect;

			int minWidth = 32, minHeight = 32;

			bool firstRect = rect.x1() == -1 && rect.x2() == -1 && 
							 rect.y1() == -1 && rect.y2() == -1;
			
			if (firstRect)
			{
				int maxEdge = 0, maxPosX = BUFSIZE/2, maxPosY = BUFSIZE/2;
			/*	for (int x = 0; x < BUFSIZE; x++)
					for (int y = 0; y < BUFSIZE; y++)
						if (edgeBuf[y*BUFSIZE+x] > maxEdge)
						{
							maxEdge = edgeBuf[y*BUFSIZE+x];
							maxPosX = x; 
							maxPosY = y;
						}
			*/	
				rect.x1(maxPosX - minWidth/2);
				rect.x2(maxPosX + minWidth/2);
				rect.y1(maxPosY - minHeight/2);
				rect.y2(maxPosY + minHeight/2);

			} else
			{

			}

			if (rect.x1() < 0) 
			{
				rect.x2(rect.x2()-rect.x1());
				rect.x1(0);
			}
			if (rect.y1() < 0)
			{
				rect.y2(rect.y2()-rect.y1());
				rect.y1(0);
			}

			if (rect.x2() > BUFSIZE-1)
			{
				rect.x1(rect.x2()-rect.x1()+BUFSIZE-1);
				rect.x2(BUFSIZE-1);
			}
			if (rect.y2() > BUFSIZE-1)
			{
				rect.y1(rect.y2()-rect.y1()+BUFSIZE-1);
				rect.y2(BUFSIZE-1);
			}
			
			cout << rect.x1() << "," << rect.y1() << "," << rect.x2() << "," << rect.y2() << endl;
		}

		


		float variation_;
	
	};
}
