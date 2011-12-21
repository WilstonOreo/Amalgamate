#pragma once

#include <stdlib.h>
#include <Magick++.h> 
#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <list>
#include <vector>

#include "amalgamate/descriptor.hpp"
#include "amalgamate/config.hpp"

using namespace std;
using namespace Magick;
using namespace boost;

namespace amalgamate
{
	class Database 
	{
	public:
		Database(Config* _config);
		Database(string inputFile);

		size_t size() { return database_.size(); }

		ImageDescriptor& at(size_t index) { return database_[index]; }
		ImageDescriptor& operator[] (size_t index) { return at(index); }

		void read(string inputFile);
		void write(string outputFile);
		void generate(string inputDir, string outputFile);
		string toString();
		
		Config* config() { return config_; }
		void config(Config* _config) { config_=_config; }

	private:
		static const string HEADER;
		
		void buildDescriptors(const vector<string>& imageFileList);
		void scanDir(string inputDir, vector<string>* imageFileList);

		vector<ImageDescriptor> database_;
		Config* config_;
	};

}



