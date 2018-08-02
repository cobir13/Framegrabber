#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <stdexcept>

// This exception is thrown when parsing input and an invalid parameter (in the parentheses) is encountered
struct bad_parameter_exception : public std::runtime_error {
	bad_parameter_exception(std::string const& message) : std::runtime_error(message) {}
	bad_parameter_exception(const char *message) : std::runtime_error(message) {}
};
// This exception is thrown on encountering a bad command
struct bad_command_exception : public std::runtime_error {
	bad_command_exception(std::string const& message) : std::runtime_error(message) {}
	bad_command_exception(const char *message) : std::runtime_error(message) {}
};

#endif //EXCEPTION_H