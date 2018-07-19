#include "framegrabber.h"
#include <PvSystem.h>
#include <vector>

PvDeviceInfo *Framegrabber::select_sole_device() {
	PvSystem *sys = new PvSystem();
	sys->Find();

	std::vector<const PvDeviceInfo*> devices;
	auto interfacecount = sys->GetInterfaceCount();
	for (unsigned i = 0; i < interfacecount; i++) {
		auto iface = sys->GetInterface(i);
		auto devcount = iface->GetDeviceCount();
		for (unsigned j = 0; j < devcount; j++) {
			auto device = iface->GetDeviceInfo(j);
			devices.push_back(device);
		}
	}
	if (devices.size() != 1) {
		printf("Devices found: %d\n", devices.size());
		printf("No (or more than one) devices found.\n");
		return NULL;
	}
	return (PvDeviceInfo*)devices[0];
}