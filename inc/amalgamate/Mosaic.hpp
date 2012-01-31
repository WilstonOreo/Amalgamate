#pragma once
#include <stdlib.h>
#include <Magick++.h>
#include <boost/foreach.hpp>
#include <list>

#include "amalgamate/DescriptorFilter.hpp"
#include "amalgamate/TileList.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate
{
	struct TileMatches
	{
		TileMatches( Rect _rect, Descriptor* _desc );
		Descriptor* desc;	
		Rect rect;
		MatchList			histSmallMatches, histLargeMatches, 
							gistMatches,thumbMatches;
		Match* 				bestMatch;
		list<TileMatches*> 	neighbors;

		void getBestMatch();
	private:
		int  equalNeighbors();
	};

	class Mosaic 
	{
	public:
		Mosaic();
		Mosaic(Database* _database, TileList* _tileList, Config* _config);
		~Mosaic(); 

		void render(string inputFile, string outputFile);
		void render(Image& motif, Image& mosaic);

		Database* database() 	{ return database_; }
		void database(Database* _database) { database_=_database; }

		Config* config() 		{ return config_; }
		void config(Config* _config) { config_=_config; }

		TileList* tileList() 	{ return tileList_; }
		void tileList(TileList* _tileList) { tileList_=_tileList; }

		void drawTile(Image& img, Image tileImg, Rect& tileRect, Rect& matchRect);
		void blendImage(const Image& motif, Image& mosaic);

	private:
		void getNeighbors(TileMatches& matches, int maxDist);
		void clearMatches();

		Config* config_;
		Database* database_;
		TileList* tileList_;
		vector<TileMatches> matches_;
	};
}

