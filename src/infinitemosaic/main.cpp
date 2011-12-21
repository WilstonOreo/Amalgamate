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

int main(int ac, char* av[])
{
	cout << "InfiniteMosaic. -- written by Wilston Oreo." << endl;
	cout << "Released under GPLv2." << endl << endl;

	stringstream descStr; 
	descStr << "Examplary usage: " << endl;
	descStr << "\tinfinitemosaic -d database.dat -s 10x10" << endl << endl;
	descStr << endl;
	descStr << "Allowed options";


	string databaseFile,sizeStr;
	// Declare the supported options.
	po::options_description desc(descStr.str());

	desc.add_options()
		("databasefile,d", po::value<string>(&databaseFile),"Database")
		("size,s", po::value<string>(&sizeStr) , "Size of image / Number of tiles")	
		("config,c", po::value<string>(),"Configurate file")
		("verbose,v","Verbose text output")
	;

	// Parse the command line arguments for all supported options
	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
	    cout << desc << endl;
	    return 1;
	}

	amalgamate::Database database(databaseFile);
 	Magick::Geometry size(sizeStr);

	if (vm.count("config"))
			amalgamate::loadConfigFile(vm["config"].as<string>());



	return EXIT_SUCCESS;
}



