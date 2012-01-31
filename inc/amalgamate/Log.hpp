#pragma once 
#define LOGGING

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <time.h>

using namespace std;

namespace amalgamate 
{
	class Log 
	{
		int level_,curLevel_;
		ostream* output_;
		static Log *_instance;
		Log() { init(); };
		Log(const Log&) {}
		Log& operator=(const Log&) { return *this; }

		vector<string> fmtParams;
		string lastParam;
	public:
			void init()
			{
				level(1); 
				output(&cout);
			}

			ostream* output() { return output_; }
			void output(ostream* _output) { output_ = _output; }


			int level() { return level_; }

			void level(int _level)
			{
				level_ = (_level > 5) ? 5 : 
						 (_level < 0) ? 0 : _level;  
			}

			inline Log& log(string file, 
					int linenumber, int _level, string type)
			{
#ifdef LOGGING
				if (level_ < _level) return *this;  
				(*output_) << logHeader(file,linenumber,_level,type); 
			//	(*output_) << text << endl; 
#endif
				return *this;
			}

			Log& operator<< (const string s) { (*output_) << "MEGATest" << s; return *this; }
			Log& operator<< (const char* s) { (*output_) << "MEGATestChar" << s; return *this; }

			template <typename T> Log& operator%(T t) 
			{

			}

			template <typename T> Log& operator<< (T t) 
			{
#ifdef LOGGING
				(*output_) << t; 
				return *this; 
#endif
			}

			inline string logHeader(string file, int linenumber, int _level, string type)
			{
#ifdef LOGGING
				time_t rawtime;
				struct tm * timeinfo;
				char buffer[30];
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				strftime(buffer,30,"%Y/%m/%d-%H:%M:%S",timeinfo); 
				string time(buffer);

				int pPos = 0;
				for (int i = file.length()-1; i > 0; i--)
					if (file.substr(i,1)==string("/"))
					{ pPos = i+1; break; }
				file = file.substr(pPos,file.length()-pPos);
			
				stringstream ss; ss << endl << time << "|" << _level << "|" << type << "|" << file; 
				ss << ":" << linenumber << "|"; 
				return ss.str();
#endif
#ifndef LOGGING		
				return string();
#endif
			}
			
			static Log *instance()
			{
				if (!_instance) _instance = new Log;
				return _instance;
			}
	};

#define LOG Log::instance() 
#define LOG_INIT amalgamate::Log *amalgamate::Log::_instance = 0
#define LOG_(lvl,type) LOG->log(__FILE__,__LINE__,(lvl),(type))
#define LOG_MSG LOG_(0,"MSG")
#define LOG_MSG_(lvl) LOG_((lvl),"MSG")
#define LOG_ERR LOG_(0,"ERR")
#define LOG_WRN LOG_(0,"WRN")
#define LOG_WRN_(lvl) LOG((lvl),"WRN") 

}
