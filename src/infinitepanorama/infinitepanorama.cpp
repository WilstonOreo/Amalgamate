#include <iostream>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <GL/glut.h>

#include "amalgamate/Config.hpp"
#include "infinitepanorama/Panorama.hpp"

using namespace boost;
namespace po = program_options;

using namespace std;

static amalgamate::Config config;

LOG_INIT;

void loadConfig(string configFile)
{
	config.read(configFile);
}

vector<string> loadFileList(string fileList)
{
	ifstream is;
	is.open(fileList.c_str());
	vector<string> result;

	while (is.good())
	{
		char line[2048];
		is.getline(line,sizeof(line));
		string l(line); trim(l);
		if (l.empty()) continue;

		result.push_back(l);
	}
	is.close();
	return result;
}


int main(int ac, char* av[])
{
	cout << "InfinitePanorama. v0.2 -- (C) 2012 by Wilston Oreo." << endl;

	stringstream descStr; 
	descStr << "Allowed options:" << endl;

	//descStr << "Panorama mode (interactive):" << endl;
	//descStr << "\tinfinitepanorama -P -l left.dat -r right.dat" << endl << endl;
	descStr << "Panorama mode (generate output file):" << endl;
	descStr << "\tinfinitepanorama -P -l left.dat -r right.dat -n 10 -h 1024 -o output.png" << endl << endl;
	descStr << "Database mode (generate database from directory):" << endl;
	descStr << "\tinfinitepanorama -D -i inputdir -l left.dat -r right.dat" << endl;
	descStr << "Database mode (generate database from filelist):" << endl;
	descStr << "\tinfinitepanorama -D -f filelist -l left.dat -r right.dat" << endl;

	po::options_description desc(descStr.str());
	string inputDir, fileList, databaseFileLeft, databaseFileRight; 
	string outputImageFile, configFile;
	int width = 10240;
	int height = 1024;

	desc.add_options()
		("help", 		"Display help message.")
		("database,D", 	"Database mode")
		("panorama,P", 	"Panorama mode")
		("config,c", 	po::value<string>(&configFile), "Config file")
		("inputdir,i", 	po::value<string>(&inputDir), "Input directory")
		("filelist,f",  po::value<string>(&fileList), "File list")
		("left,l", 		po::value<string>(&databaseFileLeft), "Database for left part of images")
		("right,r", 	po::value<string>(&databaseFileRight),"Database for right part of images")
		("output,o", 	po::value<string>(&outputImageFile), "Output image file")
		("width,w", 	po::value<int>(&width), "Panorama width")
		("height,h", 	po::value<int>(&height), "Panorama height")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	#define V vm.count
	if (V("help")) { cout << desc << endl; return 1; }

	if (V("config")) loadConfig(configFile); 

	Panorama pan(width,height,&config);
	config.print();

	if (V("database") && V("left") && V("right"))
	{
		if (V("inputdir"))
		{
			pan.generateDatabase(inputDir,databaseFileLeft,databaseFileRight);
			return EXIT_SUCCESS;
		}
		else
			if (V("filelist"))
			{
				vector<string> files = loadFileList(fileList);
				pan.generateDatabase(files,databaseFileLeft,databaseFileRight);

				return EXIT_SUCCESS;
			}
	} else
	if (V("panorama") && V("left")==1 && V("right")==1 && V("output")==1)
	{
		pan.loadDatabases(databaseFileLeft,databaseFileRight);
		pan.generate(outputImageFile);
		return EXIT_SUCCESS;
	}

	cout << desc << endl;
	return EXIT_FAILURE;
}



