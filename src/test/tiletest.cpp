#include <stdlib.h>
#include <iostream>
#include <boost/program_options.hpp>

//#include <boost/algorithm/string/split.hpp>

#include "amalgamate.hpp"

using namespace boost;
namespace po = program_options;

using namespace std;

int main(int ac, char* av[])
{
	cout << "PassoireTileTestApp. -- written by Wilston Oreo." << endl;

	stringstream descStr; 
	descStr << "Allowed options";

	string outputFile,inputFile,tilelistFile,tileImageFile,configFile,databaseFile;
	float blendFactor;
	
	// Declare the supported options.
	po::options_description desc(descStr.str());

	desc.add_options()
		("help,h", "Display help message.")
		("input,i", po::value<string>(&inputFile), "Input image file")
		("tileimage,T", po::value<string>(&tileImageFile), "Input tile image")
		("tilelist,t", po::value<string>(&tilelistFile), "Input tile list")
		("blendfactor,b", po::value<float>(&blendFactor), "Blend factor, between 0.0 and 1.0")
		("output,o", po::value<string>(&outputFile), "Output image")
		("database,d", po::value<string>(&databaseFile), "Database file")
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
	
	if (!vm.count("blendfactor")) 
		blendFactor = config.as<float>("MOSAIC_BLENDFACTOR");
	else
		config.set("MOSAIC_BLENDFACTOR",blendFactor);

	config.print();
	
	amalgamate::TileList tileList(tilelistFile);
//	amalgamate::Database database(databaseFile);
//	amalgamate::Mosaic mosaic(&database,&tileList,&config);


	Magick::Image img(inputFile);
	//img.rotate(90);
	//Magick::Image mosaicImg(img);
	Magick::Image tileImg(tileImageFile);
	tileImg.resize( "25%" );
	//tileImg.rotate(90);

	amalgamate::ImageDescriptor tileDesc(tileImg);
	amalgamate::ImageDescriptor imgDesc(img);

	tileDesc.offset(100,100);

	amalgamate::ImageDescriptorDiff result;

	

	result = tileDesc.compare(&imgDesc);
	
	Magick::Image image( Geometry(img.columns(),img.rows()) , Color(0,0,0) );

	image.strokeColor("red");
	image.strokeWidth(2);
	image.fillColor(Color(16384,16384,16384,16384));
	//image.fillPattern(image);

	image.draw( DrawableRectangle(tileDesc.offset().x,
								  tileDesc.offset().y,
								  tileDesc.offset().x+tileDesc.width(),
								  tileDesc.offset().y+tileDesc.height()));
	image.draw( DrawableRectangle(result.rect.x1(),result.rect.y1(),
								  result.rect.x2(),result.rect.y2()));
	image.display();


	return EXIT_SUCCESS;
}



