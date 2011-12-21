#pragma once
#include <stdlib.h>
#include <iostream>
#include <Magick++.h> 

#include "amalgamate/tiles.hpp"
#include "amalgamate/database.hpp"
#include "amalgamate/mosaic.hpp"

#include "amalgamate/tilegenerators/bsptree.hpp"
#include "amalgamate/tilegenerators/collage.hpp"
#include "amalgamate/tilegenerators/regular.hpp"
#include "amalgamate/tilegenerators/warp.hpp"
#include "amalgamate/config.hpp"

using namespace std;


namespace amalgamate
{
	Config config;

	void generateDatabase(string inputDir, string outputFile)
	{
		cout << "Generate Database ... " << endl;
		Database database(&config);
		database.generate(inputDir,outputFile);
	}

	void generateTileList(string inputFile, string outputFile)
	{
		cout << "Generating tile list..." << endl;
		TileList tileList(&config);
		tileList.generate(inputFile,outputFile); 
	}

	void visualizeTileList(string inputFile, string tileListFile, string outputFile)
	{
		cout << "Visualizing Tile List ... " << endl;
		TileList tileList(tileListFile);
		tileList.visualize(inputFile,outputFile);
	}

	void generateMosaic(string inputFile, string outputFile, 
						string tileListFile, string databaseFile)
	{

		cout << "Generate Mosaic ... " << endl;
		Database db(databaseFile);
		TileList tileList(tileListFile);

		Mosaic mosaic(&db,&tileList,&config);
		mosaic.render(inputFile,outputFile);
	}

	void loadConfigFile(string configFileName) 
	{
		config.read(configFileName);
		config.print();
	}
}

