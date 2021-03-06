#include "amalgamate/Mosaic.hpp"

#include <stack>
#include <boost/filesystem.hpp>
#include <amalgamate/utils.hpp>

using namespace boost;

namespace amalgamate 
{
	TileMatches::TileMatches( Rect _rect, Descriptor* _desc )
	{
		desc = _desc;
		rect = _rect;
	}

	int TileMatches::equalNeighbors()
	{
		int equal = 0;
		BOOST_FOREACH( TileMatches* matches, neighbors)
			if (matches->desc->filename() == bestMatch->desc->filename()) 
				equal++;

		cout << equal << endl;
		return equal;
	}

	void TileMatches::getBestMatch()
	{
	/*
		BOOST_FOREACH( Match& match, thumbMatches )
		{
			bestMatch = &match;
			if (equalNeighbors() == 0)
			{
				string dfStr = match.desc->filename();
				filesystem::path df(match.desc->filename());

				if (filesystem::exists(df))
				{
					desc->filename(match.desc->filename());
					return;
				}
			}
		}
		desc->filename(bestMatch->desc->filename());
	*///	bestMatch = &(*thumbMatches.end());
	}

	Mosaic::Mosaic(Config* _config) : ConfigurableObject(_config)
	{
	}

	Mosaic::Mosaic(Database* _database, TileList* _tileList, Config* _config)
	{
		database(_database);
		tileList(_tileList);
		config(_config);
	}

	Mosaic::~Mosaic()
	{
		clearMatches();
	}

	void Mosaic::render(string inputFile, string outputFile)
	{
		Image img(inputFile);
		Image mosaicImg = img;
		img.resize("300x300%");
		mosaicImg.resize("300x300%");

		render(img,mosaicImg);
		mosaicImg.write(outputFile);
	}

	void Mosaic::render(Image& motif, Image& mosaic)
	{
		clearMatches();

		LOG_MSG << "Phase 1: Get tile descriptors ... ";

/*		int count = 0;
		BOOST_FOREACH( Tile& tile, *tileList() )
		{
			Image tileImage(motif);
			Rect rect = tile.getRect(motif.columns(),motif.rows());

			Descriptor* desc = new Descriptor(tileImage,rect.width(),rect.height(),rect.x1(),rect.y1());

			TileMatches tileMatches(rect,desc);
			matches_.push_back(tileMatches);
		}

	
		DescriptorFilter filter(config());
		blendImage(motif,mosaic);
*/	}

	void Mosaic::getNeighbors(TileMatches& matches, int maxDist)
	{
		Point p = matches.rect.center();
		BOOST_FOREACH( TileMatches& m, matches_)
		{
			if (&m == &matches) continue;
			int dist = p.dist(m.rect.center());
			if (dist < maxDist)
				matches.neighbors.push_back(&m);
		}
	}

	void Mosaic::clearMatches() 
	{
		BOOST_FOREACH( TileMatches& tileMatches, matches_ )
			delete tileMatches.desc;
		matches_.clear();
	}

	void Mosaic::drawTile(Image& img, Image tileImg, Rect& tileRect, Rect& matchRect)
	{
		Geometry newSize(matchRect.width(),matchRect.height());	
		newSize.aspect(true);
		tileImg.modifyImage();
		tileImg.resize(newSize);
		int offX = tileRect.x1(), 
			offY = tileRect.y1(); 
		int w = tileRect.width(), 
			h = tileRect.height();

		img.modifyImage();
		PixelPacket* imgPixels = img.getPixels(offX,offY,w,h);
		PixelPacket* tileImgPixels = 
			tileImg.getPixels(0,0,matchRect.width(),matchRect.height());

		if (!tileImgPixels || !imgPixels) return;

		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
			{
				int dx = x - matchRect.x1(); if (dx < 0) dx = 0; if (dx >= matchRect.width()) dx = matchRect.width()-1;
				int dy = y - matchRect.y1(); if (dy < 0) dy = 0; if (dy >= matchRect.height()) dy = matchRect.height()-1;
				imgPixels[y*w+x] = tileImgPixels[dy*int(matchRect.width())+dx];
			}
		img.syncPixels();
	}

	void Mosaic::blendImage(const Image& motif, Image& mosaic)
	{
		Image bImg(motif);
		Geometry newSize = mosaic.size();
		newSize.aspect(true);
		bImg.resize( newSize );
		const PixelPacket* bImgPixels = bImg.getConstPixels(0,0,bImg.columns(),bImg.rows());
		PixelPacket* mImgPixels = mosaic.getPixels(0,0,mosaic.columns(),mosaic.rows());
		int n = bImg.rows()*bImg.columns(); 

		int blF = int(blendFactor()*65535);
		for (int i = 0; i < n; i++)
		{
			mImgPixels[i].red 	= BLEND_u16(bImgPixels[i].red, 	 mImgPixels[i].red,  blF);
			mImgPixels[i].green = BLEND_u16(bImgPixels[i].green, mImgPixels[i].green,blF);
			mImgPixels[i].blue 	= BLEND_u16(bImgPixels[i].blue,  mImgPixels[i].blue, blF);
		}
		mosaic.syncPixels();
	}


}

