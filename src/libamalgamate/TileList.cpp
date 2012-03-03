#include "amalgamate/TileList.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "boost/foreach.hpp"
#include <stack>

#include "amalgamate/TileGenerator.hpp"
#include "amalgamate/tilegenerator/BSPTree.hpp"
#include "amalgamate/tilegenerator/Warp.hpp"
#include "amalgamate/tilegenerator/Collage.hpp"
#include "amalgamate/tilegenerator/Regular.hpp"


using namespace std;
using namespace Magick;
using namespace boost;



namespace amalgamate
{
	TileList::TileList(Config* _config) : ConfigurableObject(_config)
	{
		cout << "Generate ";
		switch (getTileGenType())
		{
			#define IF(a) case (a): tileGen_ = new tilegenerator::
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
		if (!config()) return TGTYPE_REGULAR;

		string tileGenStr(config()->get<string>("TILEGEN"));
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

