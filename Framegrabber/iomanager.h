#ifndef IOMANAGER_H
#define IOMANAGER_H
#include <stdio.h>
#include <regex>
#include "framegrabber.h"

class Framegrabber;

class IOManager {
	const char *delims = "$";
public:
	std::vector<std::string> split_arglist(std::string & argstring);
	IOManager(Framegrabber *grabber);
	~IOManager();
	bool RunCommand(std::string input);
	bool ManageInput();

	void info(std::string subject, std::string message);
	void warning(std::string subject, std::string message);
	void error(std::string subject, std::string message);
	void success(std::string subject, std::string message);
	void fatal(std::string message);
	void ready();
	void done();

	std::string extract_fp(std::string);

private:
	FILE *input;
	FILE *output;
	FILE *logfile;
	void *zmq_context;
	void *zmq_responder;
	Framegrabber *grabber;

	bool blocked;

	std::regex full_command;
	std::regex stub_command;
	std::regex extract_fp_pattern;
	std::regex match_argument;

	bool write_word(std::string word, std::string val);
	bool read_word(std::string word);
	bool new_app(std::string word, std::string argstring);
	void kill_app(std::string appid);
	void update_app(std::string appid_str, std::string argstring);

	void log(const char *msg);
	void log(const char *type, const char *msg);
	void log(const char *type, const char *header, const char *msg);
	char *timestr();


};


#endif // IOMANAGER_H

