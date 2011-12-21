#include "amalgamate/database.hpp"


#include "boost/foreach.hpp"

using namespace boost;


namespace amalgamate
{
	const string Database::HEADER = "PassoireDatabase_v0.1";

	Database::Database(Config* _config)
	{
		config_=_config;
	}
	
	Database::Database(string inputFile)
	{
		read(inputFile);
	}

	string Database::toString()
	{
		stringstream ss;
		BOOST_FOREACH(ImageDescriptor& desc, database_)
		{
			ss << desc.toString() << endl;	
		}
		return ss.str();
	}

	void Database::generate(string inputDir, string outputFile)
	{
		vector<string> imageFileList;
		scanDir(inputDir,&imageFileList);
		buildDescriptors(imageFileList);
		write(outputFile);
	}

	void Database::write(string outputFile)
	{
		cout << "Write database to " << outputFile << " ... ";
		Writer databaseWriter(outputFile) ;
		string header(HEADER);
		databaseWriter << header;
		int n = database_.size(); 
		databaseWriter << n;
		
		BOOST_FOREACH ( ImageDescriptor& desc, database_ )
			databaseWriter << desc;

		databaseWriter.close();
		cout << "done." << endl;
	}

	void Database::read(string inputFile)
	{
		Reader databaseReader(inputFile);
		int n; string header; 
		databaseReader >> header;
		databaseReader >> n;

		if (header!=HEADER) 
		{ 
			cout << "Wrong header type. Got " << header << ", expected '" << HEADER << "'" << endl;
			return;
		}

		cout << "Database " << inputFile << " contains " << n << " descriptors." << endl; 
		database_.clear();

		for (int i = 0; i < n; i++) 
		{
			ImageDescriptor desc;
			databaseReader >> desc;
			database_.push_back(desc);
		}
		databaseReader.close();
	}


	void Database::scanDir(string inputDir, vector<string>* imageFileList)
	{
		cout << "Scanning directory " << inputDir << endl;
		if (!filesystem::is_directory(inputDir))
		{
			cout << inputDir << " is not a directory, aborted." << endl; return;
		}

		for ( filesystem::recursive_directory_iterator end, dir(inputDir); dir != end; ++dir ) 
		{
			filesystem::path imageFileName(*dir);
			string ext = to_upper_copy(imageFileName.extension().string());

			if ((ext==".JPG") || (ext==".PNG"))
			{
			//	cout << imageFileName; XXX
				imageFileList->push_back(imageFileName.string());                   	
			}
		}
	}

	void Database::buildDescriptors(const vector<string>& imageFileList)
	{
		int index = 0;
		BOOST_FOREACH(const string& imageFileName, imageFileList)
		{
			try {
				cout << "Build descriptor for image " << "#" << (index+1) << " '" << imageFileName << "' ... ";
				ImageDescriptor desc(imageFileName);
				database_.push_back(desc);
				cout << "done." << endl;
				index++;
			}
			catch ( std::exception e )
			{
				cout << "Error reading " << imageFileName << e.what() << endl;
			}
		}
	}
}



