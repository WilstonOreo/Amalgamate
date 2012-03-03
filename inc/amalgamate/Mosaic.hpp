#pragma once
#include <stdlib.h>
#include <Magick++.h>
#include <boost/foreach.hpp>
#include <list>

#include "amalgamate/DescriptorFilter.hpp"
#include "amalgamate/TileList.hpp"

using namespace tbd;
using namespace std;
using namespace Magick;

namespace amalgamate
{
	struct TileMatches
	{
		TileMatches( Rect _rect, Descriptor* _desc );
		Descriptor* desc;	
		Rect rect;
		Matches				histSmallMatches, histLargeMatches, 
							gistMatches,thumbMatches;
		Match* 				bestMatch;
		list<TileMatches*> 	neighbors;

		void getBestMatch();
	private:
		int  equalNeighbors();
	};

	class Mosaic : public ConfigurableObject 
	{
	public:
		Mosaic(Config* _config = NULL);
		Mosaic(Database* _database, TileList* _tileList, Config* _config = NULL);
		~Mosaic(); 

		void render(string inputFile, string outputFile);
		void render(Image& motif, Image& mosaic);

		TBD_DECLARE_PROPERTY_PTR(Database,database);
		TBD_DECLARE_PROPERTY_PTR(TileList,tileList);

		void drawTile(Image& img, Image tileImg, Rect& tileRect, Rect& matchRect);
		void blendImage(const Image& motif, Image& mosaic);

		TBD_DECLARE_PROPERTY_CFG(float,blendFactor,"MOSAIC_BLENDFACTOR",0.3);

	private:
		void getNeighbors(TileMatches& matches, int maxDist);
		void clearMatches();

		vector<TileMatches> matches_;
	};
}

