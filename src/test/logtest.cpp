#include <iostream>

#include <boost/format.hpp>

#include "amalgamate/Log.hpp"
using namespace std;

LOG_INIT;

int main(int ac, char* av[])
{
	cout << "LogTestApp. -- written by Wilston Oreo." << endl;

	cout << boost::format("%1% %2% %3% %2% %1% \n") % "11" % "22" % "333"; // 'simple' style. 

	//amalgamate::Log log = amalgamate::Log::instance();
//	amalgamate::Logger::Log().Log("Test",1);

//	amalgamate::Log::init();

	amalgamate::LOG->level(2);
	amalgamate::LOG_MSG << 4;
	amalgamate::LOG_MSG_(1) << "TestHallo" << "Ja";
	amalgamate::LOG_MSG_(2);
	amalgamate::LOG_MSG_(3);
	amalgamate::LOG_MSG_(4);
//	amalgamate::LOG_ERR << "Test%d%d" % 3 % 3;
//	amalgamate::LOG_WRN << "Test%s%d" % "Test" % 3;


	amalgamate::LOG->level(0);
//	amalgamate::LOG_ERR("Message LEVEL 3");
//	amalgamate::LOG_ERR("ERROR");

//	amalgamate::LOG_ERR(boost::str(boost::format("Test,%d") % 3));

	return 0;
}

