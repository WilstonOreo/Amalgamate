#include <stdlib.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

//#include <boost/algorithm/string/split.hpp>

#include "amalgamate.hpp"
#include "amalgamate/utils.hpp"
#include "amalgamate/config.hpp"

using namespace boost;
namespace po = program_options;

using namespace std;



int main(int ac, char* av[])
{
	cout << "PassoireTestApp. -- written by Wilston Oreo." << endl;

	stringstream descStr; 
	descStr << "Allowed options";

	string databaseFile,inputDir,tilelistFile,configFile;
	// Declare the supported options.
	po::options_description desc(descStr.str());

	desc.add_options()
		("help,h", "Display help message.")
		("inputdir,i", po::value<string>(&inputDir), "Input dir")
		("database,d", po::value<string>(&databaseFile), "Database file")
		("config,c", po::value<string>(&configFile),"Configurate file")	
	;

	// Parse the command line arguments for all supported options
	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	if (vm.count("help")) { cout << desc << endl; return 1; }


	amalgamate::Config config(configFile);
	config.print();
	
	amalgamate::Database databaseIn(&config);
	amalgamate::Database databaseOut(&config); 
	
	databaseOut.generate(inputDir,databaseFile);
	string databaseOutStr = databaseOut.toString();

	databaseIn.read(databaseFile);
	string databaseInStr = databaseIn.toString();

	if (databaseInStr == databaseOutStr) 
	{
		cout << databaseInStr << endl;
		cout << "Databases equal!!!" << endl;
		return EXIT_SUCCESS;
	} else
	{
		cout << databaseInStr << endl;
		cout << databaseOutStr << endl;

		cout << "Databases not equal!!!" << endl;
		return EXIT_FAILURE;
	}

	
	return EXIT_SUCCESS;
}



