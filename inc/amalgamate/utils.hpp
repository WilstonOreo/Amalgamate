#pragma once 
#include <iostream>
#include <fstream>
#include <sstream>
#include <Magick++.h>

using namespace Magick;
using namespace std;

namespace amalgamate 
{
#define X_AXIS 0
#define Y_AXIS 1

#define INF 10e16

#define FOR_2D(x,y,size) for ((y) = 0; (y) < size; (y)++) \
	for ((x) = 0; (x) < size; (x)++) 

#define LOG

	typedef unsigned char u8;
	typedef unsigned short u16;

	class Log 
	{
		public:
			static Log& instance()
			{ 
				static Log _instance;
				return _instance;
			}

			static int loglevel() { return loglevel_; }
			static void loglevel(int _loglevel) { loglevel_ = (_loglevel >= 0 && _loglevel < 5) ? _loglevel : 1;  }
			inline void log(string text)
			{ 
				//	#ifdef LOG
				std::cout << text << endl; 
				//	#endif
			}

		private: 
			static int loglevel_;
			Log() { /*loglevel_ = 1;*/ };
			Log( const Log& );
			Log & operator = (const Log &);

	};

	class Writer {
		public:
			Writer(string filename) { os_.open(filename.c_str(), ios::binary | ios::out); }

			Writer& operator<< (int i) { os_.write(reinterpret_cast<char*>(&i),sizeof(i)); return *this; }
			Writer& operator<< (u8 u) { os_.write(reinterpret_cast<char*>(&u),sizeof(u8)); return *this;  }
			Writer& operator<< (u16 u) { os_.write(reinterpret_cast<char*>(&u),sizeof(u16)); return *this;  }
			Writer& operator<< (float f) { os_.write((char*)&f,sizeof(float)); return *this; }
			Writer& operator<< (double d) { os_.write((char*)&d,sizeof(double)); return *this;  }
			Writer& operator<< (string s) { u16 l = s.length(); (*this) << l; os_.write(s.c_str(),l); return *this;  }

			void close() { os_.close(); }
		private:
			ofstream os_;
	};

	class Reader {
		public:
			Reader(string filename) { is_.open(filename.c_str(),ios::binary| ios::in); }

			Reader&  operator>> (int& i) { is_.read(reinterpret_cast<char*>(&i),sizeof(int)); return *this; }
			Reader& operator>> (u8& u) { is_.read(reinterpret_cast<char*>(&u),sizeof(u8)); return *this; }
			Reader& operator>> (u16& u) { is_.read(reinterpret_cast<char*>(&u),sizeof(u16)); return *this; }
			Reader& operator>> (float& f) { is_.read((char*)&f,sizeof(float)); return *this; }
			Reader& operator>> (double& d) { is_.read((char*)&d,sizeof(double)); return *this; }
			Reader& operator>> (string& s) 
			{ 
				u16 l; 
				is_.read(reinterpret_cast<char*>(&l),sizeof(u16));
				char* buf = new char[l+1];
				buf[l] = 0; 
				is_.read(buf,l);
				s = buf;
				delete [] buf; 
				return *this; 
			}
			void close() { is_.close(); }
		private:
			ifstream is_;
	};

	struct Point {
		Point() { set(0,0); }
		Point(int _x, int _y) { set(_x,_y); }

		void set(int _x , int _y) { x = _x; y = _y; };

		int dist(const Point& p) { int dx = x-p.x, dy = y-p.y; return dx*dx + dy*dy; }

		string toString() { stringstream ss; ss << x << "|" << y; return ss.str(); }

		int x,y;
	};

	class Rect {
		public:
			Rect() { set(0,0,0,0); }
			Rect(int _x1, int _y1, int _x2, int _y2) { set(_x1,_y1,_x2,_y2); }

			void set(int _x1, int _y1, int _x2, int _y2)
			{
				x1_=_x1; y1_=_y1; x2_=_x2; y2_=_y2;
				validate();
			}

			int x1() { validate(); return x1_; }
			int y1() { validate(); return y1_; }
			int x2() { validate(); return x2_; }
			int y2() { validate(); return y2_; }

			void x1(int _x1) { x1_=_x1; validate(); }
			void y1(int _y1) { y1_=_y1; validate(); }
			void x2(int _x2) { x2_=_x2; validate(); }
			void y2(int _y2) { y2_=_y2; validate(); }

			int p1(int axis) { if (axis == X_AXIS) return x1(); else return y1(); }
			int p2(int axis) { if (axis == X_AXIS) return x2(); else return y2(); }
			void p1(int value, int axis) { if (axis == X_AXIS) x1_ = value; else y1_ = value; }
			void p2(int value, int axis) { if (axis == X_AXIS) x2_ = value; else y2_ = value; }

			Point center() { validate(); return Point((x2()+x1())/2,(y2()+y1())/2); }

			int width() { return x2()-x1(); }
			int height() { return y2()-y1(); }
			int dim(int axis) { if (axis == X_AXIS) return width(); else return height(); }

			string toString() { stringstream ss; 
								ss << x1_ << "," << y1_ << "," << x2_ << "," << y2_; 
								return ss.str(); }
			
			double aspect() 
			{
				validate();
				double A = (double)width()/(double)height();
				if (A > 0) A = 1 / A;
				return A;
			}

		private:
			int x1_, y1_, x2_, y2_; 

			inline void validate()
			{
				if (x1_ > x2_) swap(x1_,x2_);
				if (y1_ > y2_) swap(y1_,y2_);
			}
	};



}
