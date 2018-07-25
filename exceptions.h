#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <stdexcept>

struct bad_parameter_exception : public std::runtime_error {
	bad_parameter_exception(std::string const& message) : std::runtime_error(message) {}
	bad_parameter_exception(const char *message) : std::runtime_error(message) {}
};

struct bad_command_exception : public std::runtime_error {
	bad_command_exception(std::string const& message) : std::runtime_error(message) {}
	bad_command_exception(const char *message) : std::runtime_error(message) {}
};

#endif //EXCEPTION_H