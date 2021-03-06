#include <stdlib.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "amalgamate.hpp"


using namespace boost;
namespace po = program_options;

using namespace std;

LOG_INIT;

int main(int ac, char* av[])
{
	cout << "PassoireTileGenTestApp. -- written by Wilston Oreo." << endl;

	stringstream descStr; 
	descStr << "Allowed options";

	string outputFile,inputFile,configFile,tileGen;

	// Declare the supported options.
	po::options_description desc(descStr.str());

	desc.add_options()
		("help,h", "Display help message.")
		("input,i", po::value<string>(&inputFile), "Input image file")
		("output,o", po::value<string>(&outputFile), "Output image file")
		("tilegen,t",po::value<string>(&tileGen), "Tile generator.")		
		("config,c", po::value<string>(&configFile),"Configurate file")	
	;

	// Parse the command line arguments for all supported options
	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	if (vm.count("help")) 
	{
	    cout << desc << endl;
	    return 1;
	}

	tbd::Config config(configFile);
	if (vm.count("tilegen")) config.set("TILEGEN",tileGen);
	cout <<	config;

	Magick::Image image(inputFile);
	
	amalgamate::Regular regular(&config); 
	amalgamate::TileList tileList = regular.generate(image);

	tileList.visualize(image);

	image.display();
	image.write(outputFile);

	tileList.write("testTiles.dat");
	tileList.read("testTiles.dat");
	tileList.visualize(image);
	image.display();

	return EXIT_SUCCESS;
}



