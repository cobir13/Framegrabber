#ifndef IOMANAGER_H
#define IOMANAGER_H
#include <stdio.h>
#include "framegrabber.h"

class IOManager{
public:
	IOManager(Framegrabber *grabber);
	~IOManager();
private:
	FILE *input;
	FILE *output;
	Framegrabber *grabber;

};

#endif // IOMANAGER_H

