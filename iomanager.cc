#include "iomanager.h"
#include <regex>



IOManager::IOManager(Framegrabber *g) {
	grabber = g;
	input = stdin;
	output = stdout;
}


IOManager::~IOManager()
{
}
