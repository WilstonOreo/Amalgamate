#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <GL/glut.h>

#include <Magick++.h>
#include "amalgamate.hpp"
#include "amalgamate/utils.hpp"
#include "amalgamate/Config.hpp"


LOG_INIT;

using namespace boost;
namespace po = program_options;

using namespace std;
using namespace Magick;

static amalgamate::Config config;

#define MAX_COUNT 10

void testHistSmall(amalgamate::Descriptor& desc, amalgamate::Descriptors& descs)
{
	LOG_MSG << "Get Hist small matches ... ";
	amalgamate::DescriptorFilter filter(&config);

	amalgamate::MatchList matches = filter.getMatches(desc,amalgamate::DT_HISTSMALL,MAX_COUNT,descs,true);
	LOG_MSG << fmt("Found % matches.") % matches.size();
	int i = 0;
	BOOST_FOREACH( amalgamate::Match m, matches )
	{
		LOG_MSG << fmt("%: %, %") % i % m.desc->filename() % m.result;
		i++;
	}
}

void testHistLarge(amalgamate::Descriptor& desc, amalgamate::Descriptors& descs)
{
	LOG_MSG << "Get Hist large matches ... ";
	amalgamate::DescriptorFilter filter(&config);

	amalgamate::MatchList matches = filter.getMatches(desc,amalgamate::DT_HISTLARGE,MAX_COUNT,descs,true);
	LOG_MSG << fmt("Found % matches.") % matches.size();
	int i = 0;
	BOOST_FOREACH( amalgamate::Match m, matches )
	{
		LOG_MSG << fmt("%: %, %") % i % m.desc->filename() % m.result;
		i++;
	}
}

void testGist(amalgamate::Descriptor& desc, amalgamate::Descriptors& descs)
{
	LOG_MSG << "Get Gist matches ... ";
	amalgamate::DescriptorFilter filter(&config);

	amalgamate::MatchList matches = filter.getMatches(desc,amalgamate::DT_GIST,MAX_COUNT,descs,true);
	LOG_MSG << fmt("Found % matches.") % matches.size();
	int i = 0;
	BOOST_FOREACH( amalgamate::Match m, matches )
	{
		LOG_MSG << fmt("%: %, %") % i % m.desc->filename() % m.result;
		i++;
	}
}


void testThumbnail(amalgamate::Descriptor& desc, amalgamate::Descriptors& descs)
{
	LOG_MSG << "Get Thumbnail matches ... ";
	amalgamate::DescriptorFilter filter(&config);

	amalgamate::MatchList matches = filter.getMatches(desc,amalgamate::DT_THUMBNAIL,MAX_COUNT,descs,true);
	LOG_MSG << fmt("Found % matches.") % matches.size();
	int i = 0;
	BOOST_FOREACH( amalgamate::Match m, matches )
	{
		LOG_MSG << fmt("%: %, %, Rect = %") % i % m.desc->filename() % m.result % m.rect.toString();
		i++;
	}
}

int main(int ac, char* av[])
{
	cout << "DescriptorFilterTest. v0.1 -- written by Wilston Oreo." << endl;

	stringstream descStr; 

	descStr << "Usage:" << endl;
	descStr << "\tinfinitepanorama -d database.dat -n 100 -c amalgamate.cfg" << endl;

	descStr << "Allowed options" << endl;
	po::options_description cmdDesc(descStr.str());
	string databaseFile, configFile, imageFile;

	cmdDesc.add_options()
		("help", 		"Display help message.")
		("imagefile,i", po::value<string>(&imageFile), "Image file")
		("database,d", 	po::value<string>(&databaseFile), "Database file")
		("config,c", 	po::value<string>(&configFile), "Config file")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, cmdDesc), vm);
	po::notify(vm);

#define V vm.count
	if (V("help")) { cout << cmdDesc << endl; return 1; }
	if (V("config")) config.read(configFile);

	LOG_MSG << fmt("Load image '%' ...") % imageFile; 
	Image image(imageFile);

	LOG_MSG << "Build descriptor for image ... ";
	amalgamate::Descriptor desc(image);

	LOG_MSG << "Load database ... ";
	amalgamate::Database database(databaseFile); 

	LOG_MSG << "Get descriptors ... ";
	amalgamate::Descriptors descs = database.descriptors();
	
	LOG->level(2);
	testHistSmall(desc,descs);
	testHistLarge(desc,descs);
	testGist(desc,descs);
	testThumbnail(desc,descs);
	
	LOG_MSG << "Test with statistics.";
	amalgamate::Statistics statistics(1.0);
	amalgamate::DescriptorFilter filter(&config,&statistics);
	amalgamate::MatchList matches = filter.getMatches(desc,descs,true);
	LOG_MSG << fmt("Found % matches.") % matches.size();

	int i = 0;
	BOOST_FOREACH( amalgamate::Match m, matches )
	{
		LOG_MSG << fmt("%: %, %, Rect = %") % i % m.desc->filename() % m.result % m.rect.toString();
		i++;
	}

	LOG_MSG << "Test with best match and type preference defined by config.";

	filter




	return EXIT_SUCCESS;
}



