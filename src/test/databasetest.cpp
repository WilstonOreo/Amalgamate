#include <stdlib.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

//#include <boost/algorithm/string/split.hpp>

#include "amalgamate.hpp"
#include "amalgamate/Config.hpp"

LOG_INIT;

using namespace boost;
namespace po = program_options;

using namespace std;

template<class T> void cmp(T a, T b, bool& equal) 
{
	equal &= a == b;
	LOG_MSG << fmt("% == %") % a % b;
}


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


	if (databaseIn.size() != databaseOut.size())
	{
		cout << "Size of database is different: " << databaseIn.size() << " vs. " << databaseOut.size() << endl;
		return EXIT_FAILURE;
	}


	for (int i = 0; i < databaseIn.size(); i++)
	{
		float diff = databaseIn[i].compare(databaseOut[i]); 
		bool equal = diff == 0.0f;

		cmp(databaseIn[i].filename(),databaseOut[i].filename(),equal);
		cmp(databaseIn[i].width(),databaseOut[i].width(),equal);
		cmp(databaseIn[i].height(),databaseOut[i].height(),equal);
		cmp(databaseIn[i].offset().x,databaseOut[i].offset().x,equal);
		cmp(databaseIn[i].offset().y,databaseOut[i].offset().y,equal);
		cmp(databaseIn[i].index(),databaseOut[i].index(),equal);

		if (!equal)
		{	
			cout << "Databases not equal @" << i << ", diff = " << diff << endl;
		//	return EXIT_FAILURE;
		}
	}

	
	cout << "Databases equal!!!" << endl;
	//cout << databaseOut.toString() << endl;
	return EXIT_SUCCESS;
}



