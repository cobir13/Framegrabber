#ifndef IOMANAGER_H
#define IOMANAGER_H
#include <stdio.h>
#include <regex>
#include "framegrabber.h"


class IOManager {
public:
	IOManager(Framegrabber *grabber);
	~IOManager();
	bool RunCommand(std::string input);
	bool ManageInput();

	void info(std::string subject, std::string message);
	void error(std::string subject, std::string message);
	void success(std::string subject, std::string message);
	void fatal(std::string message);
	void ready();
	void done();

private:
	FILE *input;
	FILE *output;
	Framegrabber *grabber;

	std::regex full_command;
	std::regex stub_command;

	bool write_word(std::string word, std::string val);
	bool new_app(std::string word, std::string argstring);
	void kill_app(std::string appid);


};


#endif // IOMANAGER_H

