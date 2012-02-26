#include <iostream>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <GL/glut.h>

#include "amalgamate/Config.hpp"
#include "amalgamate/DescriptorFilter.hpp"
#include "amalgamate/Database.hpp"
#include "infinitepanorama/Panorama.hpp"


using namespace boost;
namespace po = program_options;

using namespace std;

static amalgamate::Config config;

LOG_INIT;

typedef pair<string,string> left_right;

void loadConfig(string configFile)
{
	config.read(configFile);
}

vector<string> loadFileList(string fileList)
{
	LOG_MSG << "Loading filelist...";
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

vector< left_right > loadDatabaseList(string fileList)
{
	vector<string> lines = loadFileList(fileList);
	
	vector< pair<string, string> > result;
	BOOST_FOREACH( string line, lines)
	{
		if (line[0] == '#') continue;
		vector<string> splitVec(2);
		split(splitVec, line, is_any_of(";"), token_compress_on);
		if (splitVec.size()==2)
		{
			left_right leftRight(trim_copy(splitVec[0]),trim_copy(splitVec[1]));
			result.push_back(leftRight);
		}
	}

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
	descStr << "Preprocessing mode (generate database from filelist):" << endl;
	descStr << "\tinfinitepanorama -S -f filelist -i input.jpg -l left.dat -r right.dat" << endl;

	po::options_description desc(descStr.str());
	string inputDir, fileList, databaseFileLeft, databaseFileRight; 
	string outputImageFile, configFile;
	int width = 10240;
	int height = 768;

	int gistCount = 10000,histLargeCount = 100, histSmallCount = 0, thumbCount = 0;

	desc.add_options()
		("help", 		"Display help message.")
		("database,D", 	"Database mode")
		("panorama,P", 	"Panorama mode")
		("preprocess,S","Preprocessing mode")
		("config,c", 	po::value<string>(&configFile), "Config file")
		("inputdir,i", 	po::value<string>(&inputDir), "Input directory")
		("filelist,f",  po::value<string>(&fileList), "File list or data base list")
		("left,l", 		po::value<string>(&databaseFileLeft), "Database for left part of images")
		("right,r", 	po::value<string>(&databaseFileRight),"Database for right part of images")
		("output,o", 	po::value<string>(&outputImageFile), "Output image file")
		("width,w", 	po::value<int>(&width), "Panorama width. Default: 10240")
		("height,h", 	po::value<int>(&height), "Panorama height. Default: 768")
		("gist",		po::value<int>(&gistCount),		"Gist count")
		("histlarge",	po::value<int>(&histLargeCount), "Histogram (large) count")
		("histsmall",	po::value<int>(&histSmallCount), "Histogram (small) count")
		("thumb", 		po::value<int>(&thumbCount), 	"Thumbnail count")
		("linear", 		"Enable linear blending (default)")
		("poisson", 	"Enable poisson blending")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	#define V vm.count
	if (V("help")) { cout << desc << endl; return 1; }
	if (V("config")) loadConfig(configFile); 

	if (V("gist")) 		config.set("FILTER_GIST_MATCHES",gistCount);
	if (V("histsmall")) config.set("FILTER_HISTSMALL_MATCHES",histSmallCount);
	if (V("histlarge")) config.set("FILTER_HISTLARGE_MATCHES",histLargeCount);
	if (V("thumb")) 	config.set("FILTER_THUMBNAIL_MATCHES",thumbCount);

	Panorama pan(width,height,&config);
	config.print();

	Panorama::BlendingMode blend = Panorama::BLEND_LINEAR;
	if (V("poisson")) blend = Panorama::BLEND_POISSON;

	if (V("preprocess") && V("left") && V("right") && V("inputdir"))
	{
		Image image(inputDir);
		float border = 1.0f/6.0f;
		Descriptor leftDesc(image,int(image.columns()*border),image.rows(),0,0);
		Descriptor rightDesc(image,int(image.columns()*border),image.rows(),int((1.0f-border)*image.columns()),0);

		LOG->level(2);

		config.set("FILTER_GIST_MATCHES",5000);
		config.set("FILTER_HISTSMALL_MATCHES",0);
		config.set("FILTER_HISTLARGE_MATCHES",0);
		config.set("FILTER_THUMBNAIL_MATCHES",0);
		Database leftFinal(&config), rightFinal(&config);

		vector< left_right > files = loadDatabaseList(fileList);

		BOOST_FOREACH( left_right& file, files)
		{
			Statistics statistics(0,true);
			DescriptorFilter filter(&config,&statistics);

			LOG_MSG << fmt("Reading databases '%' (left) and '%' (right)...") % file.first % file.second; 
			Database leftTmp(file.first), rightTmp(file.second);
			Descriptors leftDescs = leftTmp.descriptors(), rightDescs = rightTmp.descriptors();

			Matches leftMatches = filter.getMatches(leftDesc,leftDescs);

			BOOST_FOREACH ( const Match& m, leftMatches)
			{
				if (m.desc)
				{
					statistics.exclude(leftDescs[m.desc->index()]);
					statistics.exclude(rightDescs[m.desc->index()]);
				}
			}

			Matches rightMatches = filter.getMatches(rightDesc,rightDescs);

			Matches matches = leftMatches;
			BOOST_FOREACH ( const Match& m, rightMatches )
				matches.insert(m);

			BOOST_FOREACH ( const Match& m, matches)
			{
				if (m.desc)
				{
					Descriptor* newLeft = new Descriptor(leftTmp[m.desc->index()]);
					Descriptor* newRight = new Descriptor(rightTmp[m.desc->index()]);	
					leftFinal.push_back(newLeft);
					rightFinal.push_back(newRight);
				}
			}
			LOG_MSG << fmt("Left database contains %, right database contains %") % leftFinal.size() % rightFinal.size();
			leftTmp.clear();
			rightTmp.clear();
		}
		leftFinal.write(databaseFileLeft);
		rightFinal.write(databaseFileRight);
		return EXIT_SUCCESS;
	}

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
	if (V("panorama")  && V("output")==1)
	{
		if (V("left")==1 && V("right")==1)
		{
			pan.loadDatabases(databaseFileLeft,databaseFileRight);
			pan.generate(outputImageFile,blend);
			return EXIT_SUCCESS;
		}
		else
		if (V("filelist")==1)
		{
			vector< left_right > files = loadDatabaseList(fileList);

			BOOST_FOREACH( left_right& file, files)
			{
				LOG_MSG << fmt("Reading databases '%' (left) and '%' (right)...") % file.first % file.second; 
				pan.loadDatabases(file.first,file.second,true);
			}
				
			pan.generate(outputImageFile,blend);
			return EXIT_SUCCESS;
		}
	}

	cout << desc << endl;
	return EXIT_FAILURE;
}



