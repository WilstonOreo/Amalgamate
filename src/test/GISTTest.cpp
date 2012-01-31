#include <iostream>
#include <boost/program_options.hpp>

#include "gist/gist.h"
#include "amalgamate.hpp"

using namespace boost;
namespace po = program_options;

using namespace std;

int main(int ac, char* av[])
{
	cout << "GistDescriptorTestApp. -- written by Wilston Oreo." << endl;

	stringstream descStr; 
	descStr << "Allowed options";

	string inputFile,configFile;
	
	// Declare the supported options.
	po::options_description desc(descStr.str());

	desc.add_options()
		("help,h", "Display help message.")
		("input,i", po::value<string>(&inputFile), "Input image file")
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

	amalgamate::Config config(configFile);
	config.print();
	Magick::Image img(inputFile);

	amalgamate::descriptor::YUVImage yuvImg(img);
	amalgamate::descriptor::GIST gist(yuvImg);

	return EXIT_SUCCESS;
}



