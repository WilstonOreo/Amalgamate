#include <stdlib.h>
#include <iostream>
#include <boost/gil/extension/io/jpeg_io.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <Magick++.h>

//#include <boost/algorithm/string/split.hpp>

#include "amalgamate.hpp"

using namespace boost;
namespace po = program_options;

using namespace std;

typedef enum { MODE_NONE = -1, MODE_MOSAIC, MODE_TILES, MODE_DATABASE } PassoireMode;


PassoireMode getMode(po::variables_map& vm)
{
	if (vm.count("input") && vm.count("output"))
	{
		if (vm.count("mosaic") && !vm.count("database") && !vm.count("tiles")) return MODE_MOSAIC; else
		if (!vm.count("mosaic") && !vm.count("database") && vm.count("tiles")) return MODE_TILES; else
		if (!vm.count("mosaic") && vm.count("database") &&  !vm.count("tiles")) return MODE_DATABASE;	
	}
	else
		return MODE_NONE;
}

int main(int ac, char* av[])
{
	cout << "LaPassoire. -- written by Wilston Oreo." << endl;
	cout << "Released under GPLv2." << endl << endl;

	stringstream descStr; 
	descStr << "Examplary usage: " << endl;
	descStr << "\tGenerate Database:" << endl;
	descStr << "\tamalgamate -D -i /dir/to/imagefiles/ -o database.dat" << endl << endl;
	descStr << "\tGenerate Tile list:" << endl;
	descStr << "\tamalgamate -T -i imagefile.jpg -o tiles.txt" << endl << endl;
	descStr << "\tVisualize tiles:" << endl;
	descStr << "\tamalgamate -T -i image.png -t tiles.txt  -o tileimage.png" << endl << endl;
	descStr << "\tRender mosaic:" << endl;
	descStr << "\tamalgamate -M -i imagefile.jpg -d database.dat -t tiles.txt -o mosaic.jpg" << endl << endl;
	descStr << endl;
	descStr << "Allowed options";

	amalgamate::Database database();

	string input, output, sizeStr;
	// Declare the supported options.
	po::options_description desc(descStr.str());

	desc.add_options()
		("help,h", "Display help message.")
		("input,i", po::value<string>(&input), "Input directory")
		("output,o", po::value<string>(&output), "Output files")
		("size,s", po::value<string>(&sizeStr) , "Size of image / Number of tiles")	
		("database,D","Database mode")
		("mosaic,M","Mosaic mode")
		("tiles,T","Tile mode")
		("config,c", po::value<string>(),"Configurate file")
		("verbose,v","Verbose text output")
		("databasefile,d", po::value<string>(),"Database")
		("tilelist,t", po::value<string>(),"List of tiles")
		("verbose,v","Verbose output")
	;

	// Parse the command line arguments for all supported options
	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	PassoireMode mode = getMode(vm);

	if ((vm.count("help")) || (mode==MODE_NONE)) {
	    cout << desc << endl;
	    return 1;
	}

 	Magick::Geometry size(sizeStr);

	if (vm.count("config"))
			amalgamate::loadConfigFile(vm["config"].as<string>());

	switch (mode)
	{
		case MODE_NONE: {
			cout << desc << endl;
			return 0; 
			break; 		
		}
		case MODE_DATABASE: {
			amalgamate::generateDatabase(input,output);
			break;
		}
		case MODE_TILES: {

			if (!vm.count("tilelist"))
			{ 
				amalgamate::generateTileList(input,output);
			} else
			{ 
				string tilelistFile = vm["tilelist"].as<string>();
				amalgamate::visualizeTileList(input,tilelistFile,output);
			}
			break;
		}
		case MODE_MOSAIC: {
			if (!vm.count("tilelist") || !vm.count("databasefile"))
			{
				cout << "No tile list and/or no databasefile given." << endl;
				return 2;
			}

			string tileListFile = vm["tilelist"].as<string>();
			string databaseFile = vm["databasefile"].as<string>();
			amalgamate::generateMosaic(input,output,tileListFile,databaseFile);
			break;
		}
	}


	return EXIT_SUCCESS;
}



