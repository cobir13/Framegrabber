#ifndef SERIALWORDS_H
#define SERIALWORDS_H

#include <unordered_map>
#include <stdint.h>
#include "ebus.h"

class Framegrabber;

#define MAX_TINT_VAL (256)

class SerialWords {
public:
	SerialWords();
	SerialWords(Framegrabber *fg);
	~SerialWords() {}

	uint8_t get_wax() { return wax; }
	void set_wax(uint8_t wax_a) {
		wax = wax_a;
		waxy_updated = true;
	}

	uint8_t get_way() { return way; }
	void set_way(uint8_t way_a) {
		way = way_a;
		waxy_updated = true;
	}

	int get_tint() { return tint; }
	void set_tint(int tint_a) {
		tint = tint_a;
		tint_updated = true;
	}

	bool write_words(PvDeviceGEV *dev);


	bool valid_tint(int tint);

private:
	Framegrabber *grabber;

	uint8_t wax;
	uint8_t way;
	int tint;
	bool waxy_updated;
	bool tint_updated;

	uint32_t serial_register;

	std::unordered_map<int, uint32_t> tint_table;

	inline uint32_t get_wax_pkt(uint8_t wax);
	inline uint32_t get_way_pkt(uint8_t way);

	bool write_waxy(PvDeviceGEV *dev);

	bool write_tint(PvDeviceGEV *dev);
	
};

#endif //SERIALWORDS_H
