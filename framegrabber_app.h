#ifndef FRAMEGRABBER_APP_H
#define FRAMEGRABBER_APP_H
#include <stdint.h>
#include <stdexcept>

class FramegrabberApp {
public:
	bool done;
	virtual bool set_frame(uint16_t *data) = 0;
	virtual bool save() = 0;
	virtual void update();
};

struct BadFormatStringException : public std::runtime_error {
	BadFormatStringException(std::string const& message)
        : std::runtime_error(message)
    {}
};

#endif //FRAMEGRABBER_APP_H