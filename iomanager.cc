#include "iomanager.h"
#include <regex>
#include <vector>
#include <string>
#include <zmq.h>
#include <errno.h>
#include <time.h>
#include "exceptions.h"
#include "focuser.h"
#include "fullframe_framegrabber_app.h"
#include "window.h"


std::vector<std::string> split(const std::string& str, const std::string& delim) {
	std::vector<std::string> parts;
	size_t start, end = 0;
	while (end < str.size()) {
		start = end;
		while (start < str.size() && (delim.find(str[start]) != std::string::npos)) {
			start++;  // skip initial whitespace
		}
		end = start;
		while (end < str.size() && (delim.find(str[end]) == std::string::npos)) {
			end++; // skip to end of word
		}
		if (end - start != 0) {  // just ignore zero-length strings.
			parts.push_back(std::string(str, start, end - start));
		}
	}
	return parts;
}


IOManager::IOManager(Framegrabber *g) {
	grabber = g;
	input = stdin;
	output = stdout;
	logfile = fopen("C:/Users/Keck Project/Documents/Framegrabber/framegrabber.log", "w");

	zmq_context = zmq_ctx_new();
	zmq_responder = zmq_socket(zmq_context, ZMQ_REP);
	if (zmq_bind(zmq_responder, "tcp://*:5555")) {
		throw std::runtime_error("Could not bind ZeroMQ socket!");
	}

	full_command = std::regex("\\s*(\\w*)\\s*(\\w*)\\s*\\(([^)]*)\\);\\s*");
	stub_command = std::regex("\\s*(\\w*);\\s*");
}


IOManager::~IOManager()
{
}

bool IOManager::RunCommand(std::string input) {
	std::smatch matches;
	if (std::regex_match(input, matches, full_command)) {
		std::string base = matches[1].str();
		std::string appname = matches[2].str();
		std::string args = matches[3].str();

		if (base == "stream") {
			printf("Caught stream\n");
			new_app(appname, args);
		}
		else if (base == "word") {
			write_word(appname, args);
		}
		else if (base == "getword") {
			read_word(appname);
		}
		else if (base == "kill") {
			kill_app(appname);
		}
		else {
			error(base, "Unknown procedure");
		}
	}
	else if (std::regex_match(input, matches, stub_command)) {
		std::string command = matches[1].str();
		info(__FUNCTION__, command);
		if (command == std::string("quit")) {
			info(__FUNCTION__, "Quitting");
			return false;
		}
		else {
			error(command, "Unknown stub command");
		}
	}
	else {
		error(__FUNCTION__, "Parse error");
	}
}

static char combuf[1024];
bool IOManager::ManageInput()
{
	while (true) {
		if (-1 == zmq_recv(zmq_responder, combuf, 1024, ZMQ_DONTWAIT)) {
			int e = errno;
			if (e == EAGAIN || e == 0) {
				break;
			}
			else {
				printf("%s\n", strerror(e));
				throw std::runtime_error("Error receiving from socket");
			}
			// reset for the next go round
			errno = 0;
		}
		printf("%c\n", combuf[0]);
		if (!RunCommand(std::string(combuf))) {
			return false;
		}
	}
	return true;
}

bool IOManager::write_word(std::string word, std::string val_str) {
	int val = std::stoi(val_str);

	if (word == std::string("wax")) {
		grabber->words.wax = val;
		grabber->words.WAXY_updated = true;
	}
	else if (word == std::string("way")) {
		grabber->words.way = val;
		grabber->words.WAXY_updated = true;
	}
	else if (word == std::string("tint")) {
		grabber->words.tint = val;
		grabber->words.tint_updated = true;
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
		sprintf_s(wordbuf, 32, "(%s,%s)", grabber->words.wax, grabber->words.way);
	}
	else if (word == "tint") {
		sprintf_s(wordbuf, 32, "(%s)", grabber->words.tint);
	}
	else {
		sprintf_s(wordbuf, 32, "Unknown word %s", word.c_str());
		error(__FUNCTION__, wordbuf);
		return false;
	}
	success(__FUNCTION__, wordbuf);
	return true;
}

bool IOManager::new_app(std::string appname, std::string argstring) {
	FramegrabberApp *newapp = NULL;
	std::vector<std::string> args = split(argstring, std::string(","));

	if (appname == "focuser") {
		newapp = new Focuser(grabber, args);
	}
	else if (appname == "fullframe") {
		newapp = new FullFrame(grabber, args);
	}
	else if (appname == "window") {
		newapp = new Window(grabber, args);
	}
	
	if (newapp == NULL) {
		error(__FUNCTION__, "Unknown app");
		return false;
	}
	else {
		grabber->apps.push_back(newapp);
		success(newapp->getname(), "Initialized!");
		return true;
	}
}

void IOManager::kill_app(std::string appid_str) {
	uint16_t appid;
	try {
		appid = std::stoi(appid_str);
	} catch (std::invalid_argument &iarg) {
		throw bad_parameter_exception(iarg.what());
	}

	auto toremove = [appid](FramegrabberApp *other) { return other->id == appid; };
	grabber->apps.remove_if(toremove);
	success(appid_str, "If this app existed, it no longer does.");
}

void IOManager::log(const char * msg) {
	fprintf(logfile, "%s %s;\n", timestr(), msg);
}

void IOManager::log(const char * type, const char * msg) {
	fprintf(logfile, "%s %s(\"%s\");\n", timestr(), type, msg);
}

void IOManager::log(const char *type, const char *header, const char *msg) {
	fprintf(logfile, "%s %s %s(\"%s\");\n", timestr(), type, header, msg);
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
	log("FATAL", message.c_str());
	grabber->Disconnect();
	fclose(logfile);
	throw std::exception("Fatal exception");
}

void IOManager::ready() {
	log("READY");
}

void IOManager::done() {
	log("DONE");
}

