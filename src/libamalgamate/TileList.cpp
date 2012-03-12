#include "amalgamate/TileList.hpp"

#include "amalgamate/tilegenerator/Regular.hpp"

using namespace std;
using namespace Magick;
using namespace boost;


namespace amalgamate
{
	TileList::TileList(string inputFile)
	{
		if (!inputFile.empty()) read(inputFile);
	}

	void TileList::read(string inputFile)
	{
		Reader r(inputFile);
		size_t n = 0; r >> n;
		resize(n);
		for (size_t i = 0; i < n; i++) r >> at(i);
		r.close();

		LOG_MSG << fmt("Tile list '%' read, has % tiles." ) % inputFile % size();
	}

	void TileList::write(string outputFile)
	{
		Writer w(outputFile);
		size_t n = size(); w << n;
		BOOST_FOREACH( Tile& t, *this) w << t;
		w.close();

		LOG_MSG << fmt("Tile list written to '%'.") % outputFile;
	}

	void TileList::visualize(Image& image)
	{
		image.strokeColor("red");
		image.strokeWidth(2);
		image.fillPattern(image);

		LOG_MSG << "Drawing tiles ... ";
		BOOST_FOREACH( Tile& t, *this)
		{
			list<Coordinate> coords = t.getCoords(image.columns(),image.rows());
			image.draw( DrawablePolygon(coords) );
		}
	}

	void TileList::visualize(string& inputImageFile, string& outputImageFile)
	{
		Image img;
		img.read(inputImageFile);
		visualize(img);	
		LOG_MSG << fmt("Writing output image to '%' ... ") % outputImageFile; 
		img.write(outputImageFile);
	}

}

