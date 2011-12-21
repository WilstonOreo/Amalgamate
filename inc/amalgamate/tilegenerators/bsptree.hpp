#pragma once 
#include <Magick++.h>
#include <stack>
#include <boost/foreach.hpp>

#include "amalgamate/tiles.hpp"
#include "amalgamate/utils.hpp"

using namespace std;
using namespace Magick;
using namespace boost;


namespace amalgamate 
{
	class BSPTree : public TileGenerator
	{
	public:
		string nameString() { return string("BSPTree"); }



		void genTiles(Image& img, TileList* tileList)
		{
			Image image = img;
			image.type(TrueColorType);
			image.modifyImage();
			image.depth(8);
			image.quantizeColorSpace( GRAYColorspace );
    		image.quantizeColors( 256 );
    		image.quantize( ); 
			image.edge(1.0f);
			tileList->clear();
			
			bspTree(Rect(0,0,image.columns()-1,image.rows()-1),&image,tileList);
			cout << tileList->size() << " tiles generated. " << endl;
		}

	private:
		typedef bool SplitAxis;
		typedef pair<Rect,Rect> Split;

		void bspTree(Rect rect, Image* img, TileList* tileList)
		{
			if (rect.width()*size().width()<img->columns() ||
				rect.height()*size().height()<img->rows())
			{
				addTile(rect,img,tileList);
				return; 
			}

			int sA = X_AXIS; // sA = splitAxis
			if (rect.height() > rect.width()) sA = Y_AXIS;
			
			vector<int> sums;
			int minDiff = 1 << 30, minSplit = (rect.p1(sA)+rect.p2(sA))/2;

			sums.reserve(rect.dim(sA));
			for (int i = rect.p1(sA); i < rect.p2(sA); i++)
				sums.push_back(getSumLine(i,sA,rect,img));

			int pixelsum = 0;
			BOOST_FOREACH( int sum, sums )
				pixelsum += sum;
			double relSum = double(pixelsum)/double(rect.width()*rect.height())/256.0;

			cout << relSum << endl;
			if (relSum < 0.1) 
			{
				addTile(rect,img,tileList);
				return;
			}

			for ( int i = rect.p1(sA)+rect.dim(sA)/4; i < rect.p2(sA)-rect.dim(sA)/4; i++)
			{
				int pos = i - rect.p1(sA);
				int sumLeft = 0, sumRight = 0; 
				for (int j = 0; j < pos; j++) sumLeft += sums[j];
				for (size_t j = pos; j < sums.size(); j++) sumRight += sums[j];
				int diff = abs(sumLeft - sumRight);

				if (diff < minDiff)
				{ 
					minDiff = diff;
					minSplit = i;
				}
			}

			Split split;
			split.first = rect; split.first.p2(minSplit,sA);
			split.second = rect; split.second.p1(minSplit,sA); 

			bspTree(split.first,img,tileList);
			bspTree(split.second,img,tileList);
		}

	};
}

