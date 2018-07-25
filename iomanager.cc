#include "iomanager.h"
#include <regex>
#include <vector>
#include <string>
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

		if (base == "STREAM") {
			printf("Caught stream\n");
			new_app(appname, args);
		}
		else if (base == "WORD") {
			write_word(appname, args);
		}
		else if (base == "KILL") {
			kill_app(appname);
		}
		else {
			error(base, "Unknown procedure");
		}
	}
	else if (std::regex_match(input, matches, stub_command)) {
		std::string command = matches[1].str();
		info("IOPARSER", command);
		if (command == std::string("QUIT")) {
			info("IOMANAGER", "Quitting");
			return false;
		}
		else {
			error(command, "Unknown stub command");
		}
	}
	else {
		error("IOMANAGER", "Parse error");
	}
}

bool IOManager::ManageInput()
{
	static char combuf[1024];
	while (fgets(combuf, 1024, input)) {
		if (combuf[0] == '\n') {
			break;
		}

		if (!RunCommand(std::string(combuf))) {
			return false;
		}
	}
	return true;
}

bool IOManager::write_word(std::string word, std::string val_str) {
	int val = std::stoi(val_str);

	if (word == std::string("WAX")) {
		grabber->words.wax = val;
		grabber->words.WAXY_updated = true;
	}
	else if (word == std::string("WAY")) {
		grabber->words.way = val;
		grabber->words.WAXY_updated = true;
	}
	else if (word == std::string("TINT")) {
		grabber->words.tint = val;
		grabber->words.tint_updated = true;
	}
	else {
		error("IOMANAGER", "Invalid serial word");
		return false;
	}
	success(word, "Wrote word");
	return true;
}

bool IOManager::new_app(std::string appname, std::string argstring) {
	FramegrabberApp *newapp = NULL;
	std::vector<std::string> args = split(argstring, std::string(","));

	if (appname == "FOCUSER") {
		newapp = new Focuser(grabber, args);
	}
	else if (appname == "FULLFRAME") {
		newapp = new FullFrame(grabber, args);
	}
	else if (appname == "WINDOW") {
		newapp = new Window(grabber, args);
	}
	
	if (newapp == NULL) {
		error("IOMANAGER", "Unknown app");
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

void IOManager::info(std::string subject, std::string message) {
	fprintf(output, "INFO %s(%s);\n", subject.c_str(), message.c_str());
}

void IOManager::error(std::string subject, std::string message) {
	fprintf(output, "ERROR %s(%s);\n", subject.c_str(), message.c_str());
}

void IOManager::success(std::string subject, std::string message) {
	fprintf(output, "SUCCESS %s(%s);\n", subject.c_str(), message.c_str());
}

void IOManager::fatal(std::string message) {
	fprintf(output, "FATAL (%s);\n", message.c_str());
	throw std::exception("Fatal exception");
}

void IOManager::ready() {
	fprintf(output, "READY;\n");
}

void IOManager::done() {
	fprintf(output, "DONE;\n");
}

