#include "iomanager.h"
#include <regex>
#include <vector>
#include <string>
#include <zmq.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include "exceptions.h"
#include "focuser.h"
#include "fullframe_framegrabber_app.h"
#include "window.h"
#include "query.h"


static const char *helpmsg = "\n"\
"Framegrabber help:\n"\
"----------------------------\n"\
"Commands are of the form:\n"\
"<CMD> <subject>(<arg(s)>);\n"\
"---or---\n"\
"<CMD>;\n\n"\
"Valid commands are:\n"\
"STREAM <appname>(<args>);\n"\
"\tStarts the named app\n"\
"SETWORD <wordname>(<val>);\n"\
"\tWrites the named serial word\n"\
"GETWORD <wordname();\n"\
"\tReturns the named serial word\n"\
"KILLAPP <appname>();\n"\
"\tKills the named app (no saving)\n"\
"QUIT;\n"\
"\tKills the server\n"\
"HELP;\n"\
"\tDisplays this message\n"\
"SYNC;\n"\
"\tForce-updates the log file\n"\
"----------------------------\n"\
"See the manual for details\n";


// Splits up a string into parts based on a character. Similar to python .split
std::vector<std::string> split(const std::string& str, const char *delims) {
	std::vector<std::string> parts;
	static char strbuf[1024];
	strcpy_s(strbuf, 1024, str.c_str());
	char *ptbuf;
	ptbuf = strtok(strbuf, delims);
	while (ptbuf != NULL) {
		parts.push_back(ptbuf);
		ptbuf = strtok(NULL, delims);
	}
	return parts;
}


IOManager::IOManager(Framegrabber *g) {
	grabber = g;
	input = stdin;
	output = stdout;
	logfile = fopen(grabber->config.communications.logfile.c_str(), "w");
	printf("Log output will be directed to %s\n", grabber->config.communications.logfile.c_str());
	
	printf("Starting server...\n");
	printf(helpmsg);
	printf("\nThis server cannot be interacted with via stdin.\n"\
		"A ZeroMQ client is required.\n"\
		"The fgterm.py program located in the root directory\n"\
		"of this project provides a simple terminal.\n"\
		"Please wait 5-10s for the stream to stabilize.\n");

	zmq_context = zmq_ctx_new();
	zmq_responder = zmq_socket(zmq_context, ZMQ_REP);
	char connectbuf[32];
	snprintf(connectbuf, 32, "tcp://%s:%d", grabber->config.communications.ip_addr.c_str(),
		grabber->config.communications.port);
	if (zmq_bind(zmq_responder, connectbuf)) {
		throw std::runtime_error("Could not bind ZeroMQ socket!");
	}

	printf("Server ready\n");

	full_command = std::regex("\\s*(\\w*)\\s*(\\w*)\\s*\\(([^)]*)\\);\\s*");
	stub_command = std::regex("\\s*(\\w*);\\s*");
	extract_fp_pattern = std::regex("\\s*\"([^\"]*)\"\\s*");
}


IOManager::~IOManager()
{
}

bool IOManager::RunCommand(std::string input) {
	// Log the input string we're passed
	info(__FUNCTION__, input);

	//Try to match the input against the patterns we have
	std::smatch matches;
	//First, the full command pattern of <1> <2> (<3>);
	if (std::regex_match(input, matches, full_command)) {
		std::string base = matches[1].str();
		std::string appname = matches[2].str();
		std::string args = matches[3].str();

		// Then send it out to the appropriate function to do the work
		if (base == "STREAM") {
			new_app(appname, args);
		}
		else if (base == "SETWORD") {
			write_word(appname, args);
		}
		else if (base == "GETWORD") {
			read_word(appname);
		}
		else if (base == "KILLAPP") {
			kill_app(appname);
		}
		// Otherwise, return an error
		else {
			error(base, "Unknown procedure");
		}
	}
	//Check if it matches the pattern <1>;
	else if (std::regex_match(input, matches, stub_command)) {
		std::string command = matches[1].str();
		info(__FUNCTION__, command);
		if (command == "HEARTBEAT") {
			success("HEARTBEAT", "");
		}
		else if (command == std::string("QUIT")) {
			info(__FUNCTION__, "Quitting");
			fatal("Exiting upon request");
		}
		else if (command == "HELP") {
			success("HELP", helpmsg);
		} 
		else if (command == "SYNC") {
			fflush(logfile);
			success("SYNC", "Updated log");
		}
		else {
			error(command, "Unknown stub command");
		}
	}
	// Otherwise, we dont know what to do and log an error
	else {
		error(__FUNCTION__, "Parse error");
	}
	return true;
}



bool IOManager::ManageInput() {
	//The buffer to store the input (static for the tiny tiny performance improvement)
	static char combuf[1024];

	//Handle input messages until there are none left
	while (true) {
		//Reset errno in case someone else set it (led to a nasty crash a while back)
		errno = 0;
		// Receive a message, and put it in the buffer. Don't block if there is no message to receive.
		int msglen = zmq_recv(zmq_responder, combuf, 1023, ZMQ_DONTWAIT);
		//-1 is the error return
		if (-1 == msglen) {
			int e = errno;
			// If the error is just EAGAIN, it meant there was no input.
			// We just go ahead and continue in that case.
			if (e == EAGAIN || e == 0) {
				break;
			}
			// Otherwise, something's gone seriously wrong with the socket. Go ahead and quit
			else {
				warning(std::to_string(e), strerror(e));
				fatal("Socket recv error");
			}
		}
		// Because we get in a raw (binary) string, we need to add the zero terminator ourselves
		else {
			combuf[msglen] = '\0';
		}

		//Try to run our new string as a command
		if (!RunCommand(std::string(combuf))) {
			return false;
		}
	}
	return true;
}

// Update the serial_words structure, and flag it as updated
bool IOManager::write_word(std::string word, std::string val_str) {
	int val;
	try {
		val = std::stoi(val_str);
	}
	catch (std::overflow_error) {
		error(__FUNCTION__, "Overflow error");
		return false;
	}

	if (word == std::string("wax")) {
		if (val >= 320 || val < 0) {
			error(word, "OutOfRange: Wax must be 0 <= WAX < 320");
			return false;
		}
		grabber->words.set_wax(val);
	}
	else if (word == std::string("way")) {
		if (val >= 256 || val < 0) {
			error(word, "OutOfRange: Way must be 0 <= WAY < 256");
			return false;
		}
		grabber->words.set_way(val);
	}
	else if (word == std::string("tint")) {
		if (!grabber->words.valid_tint(val)) {
			error(word, "Invalid tint. See manual for valid options");
			return false;
		}
		grabber->words.set_tint(val);
	}
	else {
		error(__FUNCTION__, "Invalid serial word");
		return false;
	}
	success(word, "Wrote word");
	return true;
}

bool IOManager::read_word(std::string word) {
	static char wordbuf[32];
	if (word == "wax" || word == "way" || word == "waxy") {
		sprintf_s(wordbuf, 32, "[%d,%d]", grabber->words.get_wax(), grabber->words.get_way());
	}
	else if (word == "tint") {
		sprintf_s(wordbuf, 32, "%d", grabber->words.get_tint());
	}
	else {
		sprintf_s(wordbuf, 32, "Unknown word %s", word.c_str());
		error(__FUNCTION__, wordbuf);
		return false;
	}
	success(word, wordbuf);
	return true;
}

bool IOManager::new_app(std::string appname, std::string argstring) {
	if (grabber->apps.size() > (uint32_t)grabber->config.fg_config.maxapps) {
		error(__FUNCTION__, "Could not add app. config.configuration.max_apps may need to be increased.");
		return false;
	}

	FramegrabberApp *newapp = NULL;
	std::vector<std::string> args = split(argstring, ",");

	try {
		if (appname == "focuser") {
			newapp = new Focuser(grabber, args);
		}
		else if (appname == "fullframe") {
			newapp = new FullFrame(grabber, args);
		}
		else if (appname == "window") {
			newapp = new Window(grabber, args);
		}
		else if (appname == "query") {
			newapp = new PixelQuery(grabber, args);
		}
	}
	catch (bad_parameter_exception) {
		error(__FUNCTION__, "Parameter error");
		return false;
	}

	
	if (newapp == NULL) {
		error(__FUNCTION__, "Unknown app");
		return false;
	}
	
	grabber->apps.push_back(newapp);
	if (newapp->reportsuccess) {
		grabber->iomanager->success(newapp->getname(), std::to_string(newapp->id));
	}
	return true;
	
}

void IOManager::kill_app(std::string appid_str) {
	uint16_t appid;
	try {
		appid = std::stoi(appid_str);
		if (appid > grabber->config.fg_config.maxapps) {
			throw std::out_of_range("AppID out of range");
		}
	} catch (std::invalid_argument) {
		error(__FUNCTION__, "Bad AppID [Did you pass the name instead of the ID?]");
		return;
	}
	catch (std::out_of_range) {
		error(__FUNCTION__, "AppID out of range");
		return;
	}


	printf("%d\n", appid);
	
	auto app = grabber->apps.begin();
	while (app != grabber->apps.end()) {
		if ((*app)->id == appid) {
			info("Killing", (*app)->getname());
			delete *app;
			grabber->apps.erase(app++);
			success(std::to_string(appid), "Killed");
			return;
		}
		else {
			app++;
		}
	}
	error(__FUNCTION__, "App not found");
}

void IOManager::log(const char * msg) {
	fprintf(logfile, "%s %s;\n", timestr(), msg);
#ifdef _DEBUG
	fflush(logfile);
#endif
}

// NOFATAL: Because this function is called in fatal(),
// it may not throw a fatal error.
void IOManager::log(const char * type, const char * msg) {
	fprintf(logfile, "%s %s (\"%s\");\n", timestr(), type, msg);
#ifdef _DEBUG
	fflush(logfile);
#endif
}

void IOManager::log(const char *type, const char *header, const char *msg) {
	fprintf(logfile, "%s %s %s (\"%s\");\n", timestr(), type, header, msg);
#ifdef _DEBUG
	fflush(logfile);
#endif
}

char * IOManager::timestr()
{
	static char timebuf[128];
	time_t curtime = time(NULL);
	struct tm *tstruct = localtime(&curtime);
	strftime(timebuf, 128, "%F %T", tstruct);
	return timebuf;
}

static char msgbuf[1024];
static char timebuf[128];
void IOManager::info(std::string subject, std::string message) {
	log("INFO", subject.c_str(), message.c_str());
}

void IOManager::warning(std::string subject, std::string message) {
	log("WARNING", subject.c_str(), message.c_str());
}

void IOManager::error(std::string subject, std::string message){ 
	log("ERROR", subject.c_str(), message.c_str());
	sprintf(msgbuf, "ERROR %s (%s);\n", subject.c_str(), message.c_str());
	zmq_send(zmq_responder, msgbuf, strlen(msgbuf), 0);
}

void IOManager::success(std::string subject, std::string message) {
	log("SUCCESS", subject.c_str(), message.c_str());
	sprintf(msgbuf, "SUCCESS %s (%s);\n", subject.c_str(), message.c_str());
	zmq_send(zmq_responder, msgbuf, strlen(msgbuf), 0);
}

void IOManager::fatal(std::string message) {
	printf("FATAL ERROR. SEE LOG.\n");
	log("FATAL", message.c_str());
	// We want to disconnect, but this may throw exceptions if we never connected
	try {
		grabber->Disconnect();
	}
	catch (std::exception) {}
	// Priority is closing the log, so we get info
	done();
	fclose(logfile);

	exit(-1);
	throw std::exception("Fatal exception");
}

void IOManager::ready() {
	log("READY");
}

// NOFATAL: Because this function is called in fatal(),
// it may not throw a fatal error.
void IOManager::done() {
	log("DONE");
}

std::string IOManager::extract_fp(std::string rawpath) {
	// Try to extract the filepath from the quotation marks it comes wrapped in
	std::smatch matches;
	if (std::regex_match(rawpath, matches, extract_fp_pattern)) {
		std::string fname = matches[1].str();
		//Check if the file is valid by opening it for writing
		FILE *testfile = fopen(fname.c_str(), "w");
		// If it's openable, we're good. Close the file and return it
		if (testfile) {
			fclose(testfile);
			return fname;
		}
		else {
			// Throw a warning into the log
			warning(__FUNCTION__, "Invalid filename");
			return std::string();
		}
	}
	// Throw a warning. This should only occur if the fpath isn't wrapped in quote marks
	else {
		warning(__FUNCTION__, "Could not parse FP");
		return std::string();
	}
}

