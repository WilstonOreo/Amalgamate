#pragma once

#include <Magick++.h> 
#include <vector>


#include "amalgamate/Descriptor.hpp"
#include "amalgamate/Config.hpp"

using namespace std;
using namespace Magick;

namespace amalgamate
{
	class Database : public vector<Descriptor>
	{
	public:
		Database(Config* _config);
		Database(string inputFile);

		void read(string inputFile);
		void write(string outputFile);
		void generate(string inputDir, string outputFile);
		string toString();

		Descriptors descriptors();

		void removeSimilarDescriptors(float threshold);
		
		Config* config() { return config_; }
		void config(Config* _config) { config_=_config; }
		void scanDir(string inputDir, vector<string>* imageFileList);
		void buildDescriptors(const vector<string>& imageFileList, 
							  float width = 1.0, float height = 1.0,
							  float offX = 0.0, float offY = 0.0);
	private:
		static const string HEADER;
		Config* config_;
	};
}
