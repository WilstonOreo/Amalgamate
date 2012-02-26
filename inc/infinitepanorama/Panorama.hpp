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
	typedef enum { BLEND_LINEAR, BLEND_POISSON } BlendingMode;

	Panorama(int _width, int _height, Config* _config = NULL);

	void generate(Image& image, BlendingMode blend = BLEND_LINEAR);
	void generate(string outputImageFile, BlendingMode blend = BLEND_LINEAR);

	void generateDatabase(string inputDir, string databaseFileLeft, string databaseFileRight);
	void generateDatabase(vector<string>& imageFileList, string databaseFileLeft, string databaseFileRight);

	void loadDatabases(string databaseFileLeft, string databaseFileRight, bool append = false);

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

	IplImage* MagickToCv(const Magick::Image& image, bool grayScale = false);
	Magick::Image CvToMagick(IplImage* image);

	void drawOnImage(const Image& src, Image& dest, int offX, int offY);
	int templateMatching(const Image& left, const Image& right, int leftOffY = 0);
	
	vector<bool> graphCut(const Image& left, const Image& right);
	Image linearBlending(const Image& left, const Image& right, const Image& mask);
	Image poissonBlending(Image& left, Image& right,Image& mask);
	bool stitch(Image& pan, string prev, string next, int& stitchOffX, int& stitchOffY, BlendingMode blend = BLEND_LINEAR);

	void setPanGeometry();

	float border;
	unsigned panHeight;
	unsigned height_, width_;
};
