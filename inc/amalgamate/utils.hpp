#pragma once 
#include <iostream>
#include <fstream>
#include <sstream>
#include <Magick++.h>
#include "tbd/log.h"
#include <boost/foreach.hpp>


using namespace Magick;
using namespace std;

namespace amalgamate 
{
#define X_AXIS 0
#define Y_AXIS 1

#define INF 10e16

#define FOR_2D(x,y,size) for ((y) = 0; (y) < (size); (y)++) \
	for ((x) = 0; (x) < (size); (x)++) 

#define FOR_1D(n) for (size_t i = 0; i < (n); i++) 

#define INVALID_MATCH -1.0


	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;


#define CLIP_u16(i) (i) > 65535 ? 65535 : ((i) < 0) ? 0 : (i) 
#define CLIP_u8(i) (i) > 255 ? 255 : ((i) < 0) ? 0 : (i) 
#define BLEND_u16(a,b,c) CLIP_u16(((a)/16*(c)/16 + (b)/16*(65535-(c))/16)/256)

	class Writer {
		public:
			Writer(string filename) { os_.open(filename.c_str(), ios::binary | ios::out); }

			Writer& operator<< (int i) { os_.write(reinterpret_cast<char*>(&i),sizeof(i)); return *this; }
			Writer& operator<< (size_t i) { os_.write(reinterpret_cast<char*>(&i),sizeof(i)); return *this; }
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
			Reader&  operator>> (size_t& i) { is_.read(reinterpret_cast<char*>(&i),sizeof(size_t)); return *this; }
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
		Point(double _x, double _y) { set(_x,_y); }

		void set(double _x , double _y) { x = _x; y = _y; };

		double dist(const Point& p) { double dx = x-p.x, dy = y-p.y; return dx*dx + dy*dy; }

		friend Writer& operator<<(Writer& w, Point& p)
		{
			w << p.x; w << p.y;
			return w;
		}

		friend Reader& operator>>(Reader& r, Point& p)
		{
			r >> p.x; r >> p.y;
			return r;
		}

		string toString() { stringstream ss; ss << x << "|" << y; return ss.str(); }

		double x,y;
	};

	class Rect {
		public:
			Rect() { set(0,0,0,0); }
			Rect(double _x1, double _y1, double _x2, double _y2) { set(_x1,_y1,_x2,_y2); }

			void set(double _x1, double _y1, double _x2, double _y2)
			{
				x1_=_x1; y1_=_y1; x2_=_x2; y2_=_y2;
				validate();
			}

			double x1() { return x1_; }
			double y1() { return y1_; }
			double x2() { return x2_; }
			double y2() { return y2_; }

			void x1(double _x1) { x1_=_x1; validate(); }
			void y1(double _y1) { y1_=_y1; validate(); }
			void x2(double _x2) { x2_=_x2; validate(); }
			void y2(double _y2) { y2_=_y2; validate(); }

			double p1(double axis) { if (axis == X_AXIS) return x1(); else return y1(); }
			double p2(double axis) { if (axis == X_AXIS) return x2(); else return y2(); }
			void p1(double value, double axis) { if (axis == X_AXIS) x1_ = value; else y1_ = value; }
			void p2(double value, double axis) { if (axis == X_AXIS) x2_ = value; else y2_ = value; }

			Point center() { validate(); return Point((x2()+x1())/2,(y2()+y1())/2); }

			void move(double x, double y) { x1_+=x; x2_+=x; y1_+=y; y2_+=y; }
			void scale(double sx, double sy) { x1_ *= sx; y1_ *= sy; x2_ *= sx; y2_ *= sy; }

			double width()  { return x2()-x1(); }
			double height() { return y2()-y1(); }

			string toString() { stringstream ss; 
				ss << x1_ << "," << y1_ << "," << x2_ << "," << y2_; 
				return ss.str(); }

				double aspect() 
				{
					validate();
					double A = (double)width()/(double)height();
					if (A < 1.0) A = 1 / A;
					return A;
				}

		private:
				double x1_, y1_, x2_, y2_; 

				inline void validate()
				{
					if (x1_ > x2_) swap(x1_,x2_);
					if (y1_ > y2_) swap(y1_,y2_);
				}
	};



}
