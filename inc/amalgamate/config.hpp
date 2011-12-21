#pragma once

#include <set>
#include <map>
#include <string>
#include <fstream>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include "boost/lexical_cast.hpp"

using namespace std;
using namespace Magick;
using namespace boost;

namespace amalgamate 
{
	typedef pair<string,string> ParameterTableElement;
	static set<ParameterTableElement>& parameterTable()
	{
		static set<ParameterTableElement> table;

		if (table.empty())
		{
			#define ADD(a,b) table.insert(ParameterTableElement((a),(b)));
			ADD("MOSAIC_SIZE","300%");
			ADD("MOSAIC_BLENDFACTOR","0.25");
			ADD("MOSAIC_FILENAME","mosaic.jpg");
			ADD("MOSAIC_HISTOGRAM","20");
			ADD("TILEGEN","regular");
			ADD("TILEGEN_SIZE","10x10");
			ADD("TILEGEN_COLLAGE_COEFF","0.2");
			#undef ADD 
		}

		return table;
	}

	class Config {
	public:
		Config() { init(); }
		Config(string filename) { init(); read(filename); }

		string get(string key) { return parameters[key]; }

		template <class T> T as(string key) 
		{ 		
			return lexical_cast<T>(parameters[key]); 
		}

		template <typename T> void set(string key, T value) { parameters[key] = lexical_cast<string>(value); }
		void set(string key, string value) { parameters[key] = value; }

		void read(string filename)
		{
			ifstream is;
			is.open(filename.c_str());

			while (is.good())
			{
				char line[1024];
				is.getline(line,sizeof(line));
				string l(line); trim(l);
				vector<string> splitVec;
				split( splitVec, l, is_any_of("="), token_compress_on);
				if (splitVec.size()<2) continue;

				string key   = splitVec[0]; trim(key); to_upper(key);
				string value = splitVec[1]; 

				// Remove comment 
				size_t commentPos = value.find("#");
				if (commentPos != string::npos) value = value.substr(0,commentPos-1);
				trim(value);

				if (key.length()==0) continue;
				if (key[0] < 65 || key[0] > 90) continue;
				
				parameters.insert(pair<string,string>(key,value));
				set(key,value);
			}
			is.close();

		}

		void print()
		{
			map<string,string>::iterator it;
			for (it = parameters.begin(); it != parameters.end(); ++it)
				cout << (*it).first << " = " << (*it).second << endl;
		}

	private:
		void init()
		{
			BOOST_FOREACH( ParameterTableElement e, parameterTable()) 
				parameters[e.first] = e.second;
		}

		map<string,string> parameters;
	};
}
