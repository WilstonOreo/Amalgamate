#include "amalgamate/mosaic.hpp"

#include <stack>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace Magick;
using namespace boost;


namespace amalgamate 
{
	static u16 clip(int i)
	{
		return (i > 65535) ? 65535 : (i < 0) ? 0 : i;
	}

	static u16 blend(u16 a, u16 b, int f)
	{
		return clip((a/16*f/16 + b/16*(65535-f)/16)/256);
	}

	TileMatches::TileMatches( Rect _rect, ImageDescriptor* _desc )
	{
		desc = _desc;
		rect = _rect;
	}

	bool TileMatches::addHistMatch(Match& match, size_t histCount)
	{ 
		match.histMatch =  match.desc->histogram().compare(&(desc->histogram()));
		match.histMatch += match.histMatch*match.desc->used()/2;

		list<Match>::iterator it = matches.begin();
		bool found = false;
		
		BOOST_FOREACH( Match& m, matches)
		{
			if (m.histMatch < match.histMatch) { found = true; break; }
			++it;
		}
		if (!found && matches.size()>histCount) return false;

		matches.insert(it,match);
		if (matches.size()>histCount)
			matches.pop_front();

		return true;
	}

	bool compareThumbMatch(Match a, Match b)
	{
		return (a.thumbMatch.diff < b.thumbMatch.diff);
	}

	void TileMatches::getThumbMatch()
	{
		if (matches.empty()) return;

		BOOST_FOREACH( Match& match, matches )
		{
			match.thumbMatch = desc->compare(match.desc);
			match.thumbMatch.diff += match.thumbMatch.diff*match.desc->used()/2;
		}	
		matches.sort(compareThumbMatch);
		bestMatch = &matches.front();
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
		BOOST_FOREACH( Match& match, matches )
		{
			bestMatch = &match;
			if (equalNeighbors() < 1)
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
		bestMatch = &matches.back();
		desc->filename(bestMatch->desc->filename());
	}

	Mosaic::Mosaic()
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
		render(img,mosaicImg);
		mosaicImg.write(outputFile);
	}

	void Mosaic::render(Image& motif, Image& mosaic)
	{
		clearMatches();

		cout << "Phase 1: Get tile descriptors ... " << endl;
		matches_.reserve(tileList_->size());

		int count = 0;
		BOOST_FOREACH( Tile tile, *tileList_ )
		{
			Image tileImage(motif);
			Rect rect = tile.get(motif);
			tileImage.crop(Geometry(rect.width(),rect.height(),rect.x1(),rect.y1()));

			ImageDescriptor* desc = new ImageDescriptor(tileImage);
			desc->offset(rect.x1(),rect.y2());
			matches_.push_back(TileMatches(rect,desc));
		}

		cout << "Phase 2: Get histogram matches ... " << endl;
		int histCount = config()->as<size_t>("MOSAIC_HISTOGRAM");
		
		for (size_t i = 0; i < database_->size(); i++)
			BOOST_FOREACH( TileMatches& tileMatches, matches_ ) 
			{
				Match match;
				match.thumbMatch.diff = INF; match.histMatch = 1 << 30;
				match.desc = &database_->at(i);

				if (tileMatches.addHistMatch(match,histCount))
					database_->at(i).usedOnceMore();
			}

		cout << "Phase 3: Get thumbnail matches ... " << endl;
		int maxDist = mosaic.rows()*mosaic.rows() + mosaic.columns()*mosaic.columns();
		maxDist = maxDist * 10 / (tileList_->size());

		int smatches = matches_.size(); count = 0;
		BOOST_FOREACH( TileMatches& tileMatches, matches_ )
		{
			tileMatches.getThumbMatch();
			getNeighbors(tileMatches,maxDist);
			cout << "Please wait ... " << (100*count/smatches) << "\r"; 
			count++;
		}

		cout << "Phase 4: Get best matches ... " << endl;
		BOOST_FOREACH( TileMatches& tileMatches, matches_ )
			tileMatches.getBestMatch();

		cout << "Phase 5: Place matching tiles into final image" << endl;
		count = 0;
		BOOST_FOREACH( TileMatches& tileMatches, matches_ ) 
		{
			if (!tileMatches.bestMatch || tileMatches.desc->filename().empty()) continue;
			
			cout << tileMatches.desc->filename() << ", #" << count << endl;
			Image tileImg(tileMatches.desc->filename());
			drawTile(mosaic,tileImg,tileMatches.rect,tileMatches.bestMatch->thumbMatch.rect);
			count++;
		}

		cout << "Phase 6: Blend image ... " << endl; 
		blendImage(motif,mosaic);
	}

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
				imgPixels[y*w+x] = tileImgPixels[dy*matchRect.width()+dx];
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

		int blendFactor = int(config_->as<float>("MOSAIC_BLENDFACTOR")*65535);
		for (int i = 0; i < n; i++)
		{
			mImgPixels[i].red 	= blend(bImgPixels[i].red, 	 mImgPixels[i].red, 	blendFactor);
			mImgPixels[i].green = blend(bImgPixels[i].green, mImgPixels[i].green, 	blendFactor);
			mImgPixels[i].blue 	= blend(bImgPixels[i].blue,  mImgPixels[i].blue, 	blendFactor);
		}
		mosaic.syncPixels();
	}


}

