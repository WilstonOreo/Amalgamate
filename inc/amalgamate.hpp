#pragma once
#include "amalgamate/DescriptorFilter.hpp"
#include "amalgamate/TileList.hpp"
#include "amalgamate/Database.hpp"
#include "amalgamate/Mosaic.hpp"
#include "amalgamate/Config.hpp"

using namespace std;


namespace amalgamate
{
	Config config;

	void generateDatabase(string inputDir, string outputFile)
	{
		LOG_MSG << "Generate Database ... ";
		Database database(&config);
		database.generate(inputDir,outputFile);
	}

	void generateTileList(string inputFile, string outputFile)
	{
		LOG_MSG << "Generating tile list...";
		TileList tileList(&config);
		tileList.generate(inputFile,outputFile); 
	}

	void visualizeTileList(string inputFile, string tileListFile, string outputFile)
	{
		LOG_MSG << "Visualizing Tile List ... ";
		TileList tileList(tileListFile);
		tileList.visualize(inputFile,outputFile);
	}

	void generateMosaic(string inputFile, string outputFile, 
						string tileListFile, string databaseFile)
	{

		LOG_MSG << "Generate Mosaic ... ";
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

