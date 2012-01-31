#include "amalgamate/Database.hpp"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <omp.h>

using namespace boost;


namespace amalgamate
{
	const string Database::HEADER = "PassoireDatabase_v0.2";

#define FOREACH_DESC(i)  for(size_t (i) = 0; (i) < size(); (i)++)

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
		FOREACH_DESC(i) 
			ss << at(i).toString() << endl; 	
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
		LOG_MSG_(1) << fmt("Write database to '%' ... ") % outputFile;
		Writer databaseWriter(outputFile) ;
		string header(HEADER);
		databaseWriter << header;
		databaseWriter << size();
		FOREACH_DESC(i) 
		{ 
			at(i).index(i);  
			databaseWriter << at(i); 
			if ((i % 10000 == 0) && i > 0) 
				LOG_MSG << fmt("Wrote % descriptors, % left") % i % (size() - i);	
		} 
		databaseWriter.close();
		*LOG << "done.";
	}

	void Database::read(string inputFile)
	{
		Reader databaseReader(inputFile);
		size_t n = 0; string header; 
		databaseReader >> header;
		databaseReader >> n;

		if (header!=HEADER) 
		{
			LOG_ERR << fmt("Wrong header type for '%'. Got '%', expected '%'.") % inputFile % header % HEADER;
			return;
		}
		if (!n)
		{
			LOG_WRN << "Database contains no elements.";
			return;
		}

		LOG_MSG << fmt("Database % contains % descriptors.") % inputFile % n;
		clear(); resize(n);

		FOREACH_DESC(i)
		{
			databaseReader >> at(i);
			at(i).index(i);
		}
		databaseReader.close();
	}

	Descriptors Database::descriptors()
	{
		Descriptors result; result.resize(size());
		for (size_t i = 0; i < size(); i++) 
			result[i] = &at(i);
		LOG_MSG_(2) << fmt("Got % descriptors") % result.size();
		return result;
	}


	void Database::removeSimilarDescriptors(float threshold)
	{
		typedef pair<Descriptor*,Descriptor*> IdPair; 
		set<IdPair> pairs; vector<int> indices;

		FOREACH_DESC(i)
		{
			FOREACH_DESC(j)
			{
				IdPair pair1(&at(i),&at(j));
				IdPair pair2(&at(j),&at(i));
				if (pairs.find(pair1) != pairs.end() ||
						pairs.find(pair2) != pairs.end()) continue;
				if (at(i).compare(at(j)) > threshold)
				{
					pairs.insert(pair1);
					indices.push_back(i);
				}
			}
			i++;
		}
		for (int i = indices.size()-1; i >0; i--)
			erase(begin()+indices[i]);
	}

	void Database::scanDir(string inputDir, vector<string>* imageFileList)
	{
		LOG_MSG_(1) << fmt("Scanning directory '%'") % inputDir;
		if (!filesystem::is_directory(inputDir))
		{
			LOG_MSG_(1) << fmt("'%' is not a directory, aborted.") % inputDir; return;
		}

		for ( filesystem::recursive_directory_iterator end, dir(inputDir); dir != end; ++dir ) 
		{
			filesystem::path imageFileName(*dir);
			string ext = to_upper_copy(imageFileName.extension().string());

			if ((ext==".JPG") || (ext==".PNG"))
				imageFileList->push_back(imageFileName.string());                   	
		}
	}

	void Database::buildDescriptors(const vector<string>& imageFileList, 
							  float width, float height, float offX, float offY)
	{
		size_t fSize = imageFileList.size();
		resize(fSize);
		LOG_MSG_(1) << "Build descriptors ";

		#pragma omp parallel
		{
			int n = omp_get_num_threads();
			for (size_t i = 0; i < (fSize+n)/n; i++) 
			{
				int threadIdx = omp_get_thread_num();
				size_t pos = i*n+threadIdx;
				if (pos >= fSize) continue;
				try { at(pos).build(imageFileList[pos],width,height,offX,offY); }
				catch ( std::exception e )
					{ LOG_ERR << fmt("Error reading % %") % imageFileList[pos] % e.what(); }
				if ((pos % 100 == 0) && pos > 0) 
					LOG_MSG << fmt("Built % descriptors, % left") % pos % (fSize - pos);
			}
		}
	}

#undef FOREACH_DESC
}



