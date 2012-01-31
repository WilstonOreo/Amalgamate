#include <iostream>

#include <opencv/cv.h> 
#include <Magick++.h>

#include "amalgamate/Database.hpp"
#include "amalgamate/Config.hpp"

using namespace std;
using namespace Magick;
using namespace amalgamate;

class Panorama
{
public:
	Panorama(int _width, int _height, Config* _config = NULL);

	void generate(Image& image);
	void generate(string outputImageFile);

	void generateDatabase(string inputDir, string databaseFileLeft, string databaseFileRight);
	void generateDatabase(vector<string>& imageFileList, string databaseFileLeft, string databaseFileRight);

	void loadDatabases(string databaseFileLeft, string databaseFileRight);

	void config(Config* _config) 
	{
		config_=_config;
		left.config(_config); 
		right.config(_config);
	}

	Config* config() { return config_; }

	int height() { return height_; }
	void height(int _height);

	int width() { return width_; }
	void width(int _width);

private:
	Database left,right;
	Config *config_;

	IplImage* MagickToCv(const Magick::Image& image);
	void drawImage(const Image& src, Image& dest, int offX, int offY);
	int templateMatching(const Image& left, const Image& right, int leftOffY = 0);
	
	vector<bool> graphCut(const Image& left, const Image& right);
	Image linearBlending(const Image& left, const Image& right, const vector<bool>& mask);

	Image poissonBlending(const Image& left, const Image& right, const vector<bool>& mask);

	bool stitch(Image& pan, string prev, string next, int& stitchOffX, int& stitchOffY);

	void setPanGeometry();

	float border;
	int panHeight;
	int height_, width_;
};
