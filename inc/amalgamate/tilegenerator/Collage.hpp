#pragma once 
#include <deque>
#include <cstdlib>
#include "amalgamate/TileGenerator.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate
{

	namespace tilegenerator
	{
	typedef enum { EDGE_BOTTOM, EDGE_LEFT, EDGE_RIGHT, EDGE_TOP, INVALID } EdgeType;
	
	struct Edge {
		int posA,posB,posC;
		EdgeType edgeType;

		void set(int _posA, int _posB, int _posC, EdgeType _edgeType) 
		{
			posA = _posA; posB = _posB; posC = _posC; edgeType = _edgeType;
		}
		int length() { int l = posB-posA; return (l>0) ? l : -l; }

		Point center()
		{
			switch (edgeType)
			{
				case EDGE_TOP: 
					return Point((posA+posB)/2,posC); break;
				case EDGE_BOTTOM:
					return Point((posA+posB)/2,posC+1); break;
				case EDGE_LEFT:
					return Point(posC,(posA+posB)/2); break;
				case EDGE_RIGHT:
					return Point(posC+1,(posA+posB)/2); break;
				case INVALID:
					return Point(-1,-1);
			}
		}
	};

	class Collage : public TileGenerator 
	{
	public:
		string nameString() { return string("Collage"); }

		#define BUFSIZE 256

	private:
		Geometry minSize, maxSize;
		vector<u8> edgeBuf;
		vector<bool> drawnBuf;
		
		float maxAspect_;
		int threshold_;
		float variation_;

	public:
		void genTiles(Image& img, TileList* tileList)
		{
			Image image = img;
			image.type(TrueColorType);
			image.modifyImage();
			image.resize( Geometry("256x256!"));
			image.quantizeColorSpace( GRAYColorspace );
			image.edge(1.0f);
			tileList->clear();

			variation_ = 1.0;
			threshold_ = 1024*256;
			maxAspect_ = 3.0;

			minSize.width(size_.width());
			minSize.height(size_.height());
			maxSize.width(int(minSize.width()*(variation_+1.0)));
			maxSize.height(int(maxSize.height()*(variation_+1.0)));

			edgeBuf.resize(BUFSIZE*BUFSIZE);
			drawnBuf.resize(BUFSIZE*BUFSIZE);
			PixelPacket* p = image.getPixels(0,0,BUFSIZE,BUFSIZE);
			for (int i = 0; i < BUFSIZE*BUFSIZE; i++)
			{
				edgeBuf[i] = p[i].red >> 8;
				drawnBuf[i] = false;
			}

			Edge edge;
			Rect rect = placeFirstRect();
			addTile(rect,&image,tileList);

			while (foundEdge(edge))
			{
				Rect rect = constructRect(edge);
				if (rect.width()==0 || rect.height()==0) break;

				addTile(rect,&image,tileList);
			}

			cout << tileList->size() << " tiles generated. " << endl;
		}

		float variation() { return variation_; }
		void variation( float _variation ) { variation_=_variation; }
	private:

		int drawBufSum(Rect& rect)
		{
			int sum = 0;
			for (int y = rect.y1(); y < rect.y2(); y++)
				for (int x = rect.x1(); x < rect.x2(); x++)
				{
					if (x<0 || y<0 || x>=BUFSIZE || y>=BUFSIZE) continue;
					sum += (int)drawBuf(x,y);
				}
			return sum;
		}

		bool drawBuf(int x, int y) 
		{ 
			if (x<0 || y<0 || x>=BUFSIZE || y>=BUFSIZE) return true;
			return drawnBuf[y*BUFSIZE+x]; 
		}

		int rectSum(Rect rect)
		{
			int sum = 0;
			for (int y = rect.y1(); y < rect.y2(); y++)
				for (int x = rect.x1(); x < rect.x2(); x++)
				{
					if (x<0 || y<0 || x>=BUFSIZE || y>=BUFSIZE) continue;
					sum += edgeBuf[y*BUFSIZE+x];
				}
			return sum;
		}



		void edgeOnLine(vector<int> line, int& a, int& b, int& edgeValue)
		{
			int state; edgeValue = 0;
			bool haveEdge = false;
			enum { OUT_OF_EDGE, ON_EDGE };
			state = OUT_OF_EDGE;

			for (int i = 0; i < line.size(); i++)
			{
				switch(state)
				{
					case OUT_OF_EDGE:
						if (line[i] != 0)
						{
							edgeValue = line[i];
							a = i;
							state++; haveEdge = true;
						}
						break;
					case ON_EDGE:
						if (line[i] != edgeValue || i == BUFSIZE-1)
						{
							b = i;
							state--;
						}
						break;
				}
			}
		}

		bool foundEdge(Edge& edge)
		{
			/*
			edge.set(0,512,0,INVALID);
			Edge curEdge;


			vector<int> vertEdges(BUFSIZE*BUFSIZE,0);
			vector<int> horzEdges(BUFSIZE*BUFSIZE,0);

			int x,y;
			FOR_2D(x,y,BUFSIZE)
			{
				vertEdges[y*BUFSIZE+x] = (int)drawBuf(x,y)-(int)drawBuf(x,y+1);
				horzEdges[y*BUFSIZE+x] = (int)drawBuf(x,y)-(int)drawBuf(x+1,y);
			}

			Image vertImg( Geometry(BUFSIZE,BUFSIZE), Color(0,0,0));
			Image horzImg( Geometry(BUFSIZE,BUFSIZE), Color(0,0,0));
			vertImg.modifyImage(); horzImg.modifyImage();
			
			PixelPacket* vertPix = vertImg.getPixels(0,0,BUFSIZE,BUFSIZE);
			PixelPacket* horzPix = horzImg.getPixels(0,0,BUFSIZE,BUFSIZE);
		
			for (int i = 0; i < BUFSIZE*BUFSIZE; i++)
			{
				if (vertEdges[i]==-1)
					vertPix[i].red = 65535; else
				if (vertEdges[i]==1)
					vertPix[i].green = 65535;

				if (horzEdges[i]==-1)
					horzPix[i].red = 65535; else
				if (horzEdges[i]==1)
					horzPix[i].green = 65535;
			}

			horzImg.syncPixels(); vertImg.syncPixels();
			horzImg.display();
			vertImg.display();
			int state = OUT_OF_EDGE;
			FOR_2D(y,x,BUFSIZE)
			{
				int v = vertEdges[y*BUFSIZE+x];
			}

			state = OUT_OF_EDGE;

			int a = -1, b = -1, edgeValue = 0;
			int minLength = 1 << 30;
			for (int y = 0; y < BUFSIZE; y++)
			{
				vector<int> line(BUFSIZE,0);
				for (int x = 0; x < BUFSIZE; x++)
					line[x] = horzEdges[y*BUFSIZE+x];
				edgeOnLine(line,a,b,edgeValue);
			
				if (edgeValue == -1)
					edge.set(a,b,y,EDGE_TOP); else
				if (edgeValue == 1)
					edge.set(a,b,y,EDGE_BOTTOM);
			}


			
			FOR_2D(x,y,BUFSIZE)
			{
				int h = horzEdges[y*BUFSIZE+x];



				switch(state)
				{
					case OUT_OF_EDGE:
						if (h != 0)
						{
							if 	(h==1)  curEdge.set(x,x,y,EDGE_BOTTOM);
							else if (h==-1) curEdge.set(x,x,y,EDGE_TOP);
							state++; haveEdge = true;
						}
						break;
					case ON_EDGE:
						if (h == 0 || x == BUFSIZE-1)
						{
							curEdge.posB = x; state--;
							if (curEdge.length() < edge.length() && curEdge.length()>0) 
								edge = curEdge;
						}
						break;
				}
			}

			if (edge.edgeType == INVALID) return false;
			return haveEdge;*/
		}

		void drawRect(Rect rect)
		{
			for (int y = rect.y1(); y < rect.y2(); y++)
				for (int x = rect.x1(); x < rect.x2(); x++)
					drawnBuf[y*BUFSIZE+x] = true;
		}

		Rect rectExtend(Edge edge)
		{
			Rect rect;
			Point center = edge.center();
			rect.set(center.x,center.y,center.x,center.y);
			
			while (rect.smallerThan(maxSize)) 
			{
		/*		int minSum = 1 << 30, sumExt; Rect extRect;
				int sum = rectSum(rect);
				#define MIN_SUM sumExt = rectSum(extRect) + drawBufSum(extRect)*128; \
									if(minSum > sumExt && sumExt >= 0)\
									{ minSum = sumExt; rect = extRect; }
				if (rect.width()<maxSize.width() && 
				   ((double)rect.width()/(double)rect.height()<maxAspect_))
				{
					if (rect.x1()>0)
						{ extRect  = rect.extendedLeft(1); MIN_SUM; }
					if (rect.x2()<BUFSIZE)
						{ extRect  = rect.extendedRight(1); MIN_SUM; }
				}
				if (rect.height()<maxSize.height() && 
				   ((double)rect.height()/(double)rect.width()<maxAspect_))
				{
					if (rect.y1()>0)
						{ extRect  = rect.extendedTop(1); MIN_SUM; }
					if (rect.y2()<BUFSIZE)
						{ extRect  = rect.extendedBottom(1); MIN_SUM; }
				}
 				#undef MIN_SUM
				if (minSum == 1 << 30) break;
				if (sum < threshold_ && 
					rect.width()>=minSize.width() && 
					rect.height()>=minSize.height()) break;
		*/


				
			}

			return rect;


		}

		Rect constructRect(Edge edge)
		{
			Rect rect = rectExtend(edge);
			keepInConstraints(rect);
			drawRect(rect);

			cout << "Construct rect: " << rect.toString()  << endl;
			return rect;
		}

		Rect placeFirstRect()
		{
			Rect rect;
			int maxEdge = 0, maxPosX = BUFSIZE/2, maxPosY = BUFSIZE/2;
			
			int x,y;
			FOR_2D(x,y,BUFSIZE)
				if (edgeBuf[y*BUFSIZE+x] > maxEdge)
				{
					maxEdge = edgeBuf[y*BUFSIZE+x];
					maxPosX = x; 
					maxPosY = y;
				}

			rect.set(maxPosX- minSize.width()/2, maxPosY - minSize.height()/2,
					 maxPosX + minSize.height()/2,maxPosY + minSize.height()/2);

			keepInConstraints(rect);
			drawRect(rect);

			cout << rect.x1() << "," << rect.y1() << "," << rect.x2() << "," << rect.y2() << endl;
			return rect;
		}

		void keepInConstraints(Rect& rect)
		{
			if (rect.x1() < 0) 			rect.move(-rect.x1(),0);
			if (rect.y1() < 0) 			rect.move(0,-rect.y1());
			if (rect.x2() >= BUFSIZE) 	rect.move(BUFSIZE-rect.x2(),0);
			if (rect.y2() >= BUFSIZE) 	rect.move(0,BUFSIZE-rect.y2());
		}

	};
	}
}
