#include "amalgamate/tiles.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "boost/foreach.hpp"
#include <stack>

#include "amalgamate/tilegenerators/bsptree.hpp"
#include "amalgamate/tilegenerators/warp.hpp"
#include "amalgamate/tilegenerators/collage.hpp"
#include "amalgamate/tilegenerators/regular.hpp"


using namespace std;
using namespace Magick;
using namespace boost;



namespace amalgamate
{
	void Tile::set(Image& img, Geometry& geom)
	{
		x1 = double(img.columns())/double(geom.width());
		y1 = double(img.rows())/double(geom.height());
		x2 = double(img.columns())/double(geom.width());
		y2 = double(img.rows())/double(geom.height());	
		validate();
	}

	void Tile::set(double _x1, double _y1, double _x2, double _y2)
	{
		x1 = _x1; y1 = _y1; x2 = _x2; y2 = _y2; validate();
	}

	Rect Tile::get(Image& img) 
	{ 
		return Rect(int(img.columns()*x1),
					int(img.rows()*y1),
					int(img.columns()*x2),
					int(img.rows()*y2));
	}

	string Tile::toString()
	{
		stringstream ss; ss << x1 << ";" << y1 << ";";
		ss << x2 << ";" << y2; return ss.str();
	}

	bool Tile::fromString(string str)
	{
		vector<string> values;
		split( values, str, is_any_of(";"), token_compress_on );

		if (values.size()==4)
		{
			x1 = atof(values[0].c_str());
			y1 = atof(values[1].c_str());
			x2 = atof(values[2].c_str());
			y2 = atof(values[3].c_str());			
			return true;
		} else
			return false;
	}

	TileList::TileList(Config* _config)
	{
		config(_config);
		cout << "Generate ";
		switch (getTileGenType())
		{
			#define IF(a) case (a): tileGen_ = new
			#define FI ; break;
			IF (TGTYPE_REGULAR) Regular() 	FI
			IF (TGTYPE_BSP) 	BSPTree() 	FI
			IF (TGTYPE_COLLAGE) Collage() 	FI
			IF (TGTYPE_WARP) 	Warp()  	FI
			#undef IF
		}
		tileGen_->config(_config);
		cout << tileGen_->nameString() << " tiles ... " << endl;
	}

	TileList::TileList(string inputFile)
	{
		read(inputFile);
	}

	void TileList::read(string inputFile)
	{
		ifstream tileListFile;
		tileListFile.open(inputFile.c_str());

		while (tileListFile.good())
		{
			char line[1024];
			tileListFile.getline(line,1024);
			string strLine(line);
			Tile tile; 
			if (tile.fromString(line))
				push_back(tile);
		}
		tileListFile.close();
		cout << "Tile list '" << inputFile << "' read, has " << size() << " tiles." << endl;
	}

	void TileList::write(string outputFile)
	{
		ofstream tileListFile;
		tileListFile.open(outputFile.c_str());
		TileList::iterator it;
	 	for(it=begin(); it!=end(); it++)
		{
			string tileStr = it->toString();
			tileListFile << tileStr << endl;	
		}
		tileListFile.close();

		cout << "Tile list written to " << outputFile << endl;
	}

	void TileList::generate(Image& image)
	{
		clear();
		tileGen_->genTiles(image,this);
	}

	void TileList::generate(string inputImageFile, string outputFile)
	{
		Image img(inputImageFile);
		generate(img);
		write(outputFile);
	}

	void TileList::visualize(Image& image)
	{
		image.strokeColor("red");
		image.strokeWidth(2);
		image.fillPattern(image);

		cout << "Drawing ... " << endl;
		TileList::iterator it;
		for(it=begin(); it!=end(); it++)
			image.draw( DrawableRectangle(
						int(it->x1*image.columns()),
						int(it->y1*image.rows()),
						int(it->x2*image.columns()),
						int(it->y2*image.rows())));
	}

	void TileList::visualize(string inputImageFile, string outputImageFile)
	{
		Image img;
		img.read(inputImageFile);
		visualize(img);	
		cout << "Writing output image to " << outputImageFile << " ... " << endl; 
		img.write(outputImageFile);
	}


	TileGenType TileList::getTileGenType()
	{
		if (!config_) return TGTYPE_REGULAR;

		string tileGenStr(config_->get("TILEGEN"));
		to_upper(tileGenStr);

		#define IF(a) if (tileGenStr == (a)) return 
		IF 	("REGULAR") TGTYPE_REGULAR;
		IF 	("COLLAGE") TGTYPE_COLLAGE;
		IF 	("WARP") 	TGTYPE_WARP;
		IF 	("BSP") 	TGTYPE_BSP;
		#undef IF
		return TGTYPE_REGULAR;
	}

}

