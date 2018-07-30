#include "serialwords.h"
#include "framegrabber.h"
#include "iomanager.h"



SerialWords::SerialWords() {

}

SerialWords::SerialWords(Framegrabber *fg) {
	this->grabber = fg;
	tint_table.insert({
		{ 13, 0x00001D04 },
		{ 61, 0x00001D13 },
		{ 97, 0x00001D13 },
		{ 112, 0x00001D23 },
		{ 157, 0x00001D31 },
		{ 202, 0x00001D3F }
	});
	wax = 0;
	way = 0;
	tint = 202;
	waxy_updated = false;
	tint_updated = false;

	serial_register = 0x0000E91C;
}





bool SerialWords::write_words(PvDevice * dev) {
	bool status = true;

	if (waxy_updated) {
		status = write_waxy(dev);
		waxy_updated = false;
	}

	if (tint_updated) {
		status = write_tint(dev);
		tint_updated = false;
	}


	return status;
}

bool SerialWords::valid_tint(int tint) {
	if (tint_table.end() == tint_table.find(tint)) {
		return false;
	}
	return true;
}

inline uint32_t SerialWords::get_wax_pkt(uint8_t wax) {
	// This formula was calculated by repeatedly observing the packets sent for various values of WAX
	uint32_t pkt_base = 192 + (wax / 4);
	// Bitwise OR -- the data packet is of the form 0x00001FXX, where XX is the 1-byte data word
	uint32_t pkt = 0x00001F00 | pkt_base;
	return pkt;
}

inline uint32_t SerialWords::get_way_pkt(uint8_t way) {
	// This formula was calculated by repeatedly observing the packets sent for various values of WAY
	uint32_t pkt_base = way / 2;
	// Bitwise OR -- the data packet is of the form 0x00001FXX, where XX is the 1-byte data word
	uint32_t pkt = 0x00001F00 | pkt_base;
	return pkt;
}

bool SerialWords::write_waxy(PvDevice * dev)
{
	uint32_t regs[][2] = {
		{ 0x00001E1E, 0x00001F14 },
		{ 0x00001E01, get_wax_pkt(wax) },
		{ 0x00001E02, get_way_pkt(way) },
		{ 0x00001E03, 0x00001F12 },
		{ 0x00001E04, 0x00001F94 },
		{ 0x00001E05, 0x00001F00 },
		{ 0x00001E1F, 0x00001F20 },
		{ 0x00001E00, 0x00001F01 },
	};

	size_t regs_len = sizeof(regs) / sizeof(uint32_t[2]);

	for (unsigned i = 0; i<regs_len; i++) {
		PvResult dest_status = dev->WriteRegister(serial_register, regs[i][0]).IsFailure();
		PvResult word_status = dev->WriteRegister(serial_register, regs[i][1]).IsFailure();
		if (!dest_status.IsOK() || !word_status.IsOK()) {
			char msgbuf[512];
			sprintf_s(msgbuf, 512, "Could not write registers %x, %x", regs[i][0], regs[i][1]);
			grabber->iomanager->warning(__FUNCTION__, msgbuf);
			grabber->iomanager->info(__FUNCTION__, dest_status.GetCodeString().GetAscii());
			grabber->iomanager->info(__FUNCTION__, word_status.GetCodeString().GetAscii());
			return false;
		}
	}
	return true;
}

bool SerialWords::write_tint(PvDevice * dev)
{
	std::unordered_map<int, uint32_t>::const_iterator r = tint_table.find(tint);
	if (r == tint_table.end()) {
		return false;
	}
	uint32_t tint_pkt = r->second;

	uint32_t tint_regs[][2] = {
		{ 0x00001C04, tint_pkt },
		{ 0x00001C05, 0x00001D00 },
	};

	size_t tint_regs_len = sizeof(tint_regs) / sizeof(uint32_t[2]);

	for (unsigned i = 0; i < tint_regs_len; i++) {
		if (dev->WriteRegister(serial_register, tint_regs[i][0]).IsFailure() ||
			dev->WriteRegister(serial_register, tint_regs[i][1]).IsFailure()) {
			printf("Could not write registers %x, %x\n", tint_regs[i][0], tint_regs[i][1]);
			return false;
		}
	}
	return true;
}
