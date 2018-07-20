#include "framegrabber.h"
#include <PvSystem.h>
#include <vector>
#include <stdexcept>
#include <stdio.h>

#define BUFFER_COUNT (16)

PvDeviceInfo *Framegrabber::select_sole_device() {
	printf("Looking for devices\n");
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
	printf("Found device %s\n", (PvDeviceInfo*)devices[0]->GetMACAddress().GetAscii());
	return (PvDeviceInfo*)devices[0];
}

void Framegrabber::data_loop() {
	PvBuffer *buffer = NULL;
	PvResult opResult;
	
	PvResult result = stream.RetrieveBuffer(&buffer, &opResult, 100);
	if (result.IsOK() && opResult.IsOK()) {
		if (buffer->GetPayloadType() == PvPayloadTypeImage) {
			PvImage *img = buffer->GetImage();
			width = img->GetWidth();
			height = img->GetHeight();
			uint16_t *vdata = (uint16_t*)(img->GetDataPointer());

			for (FramegrabberApp *app : apps) {
				app->set_frame(vdata);
			}
		}
	}
	else {
		throw std::runtime_error("Error retrieving buffer");
	}
}

bool Framegrabber::Connect() {
	PvDeviceInfo *dev_info = select_sole_device();

	if (dev_info == nullptr) {
		printf("No device found\n");
		return false;
	}

	printf("Connecting to %s\n", dev_info->GetMACAddress().GetAscii());
	if (device.Connect(dev_info).IsFailure()) {
		printf("Unable to connect to %s\n", dev_info->GetMACAddress().GetAscii());
		return false;
	}
	else {
		printf("Connected to %s\n", dev_info->GetMACAddress().GetAscii());
	}

	params = device.GetGenParameters();
	TLLocked = dynamic_cast<PvGenInteger *>(params->Get("TLParamsLocked"));
	payload_size = dynamic_cast<PvGenInteger *>(params->Get("PayloadSize"));
	start = dynamic_cast<PvGenCommand *>(params->Get("AcquisitionStart"));
	stop = dynamic_cast<PvGenCommand *>(params->Get("AcquisitionStop"));

	device.NegotiatePacketSize();

	printf("Opening stream to device\n");
	stream.Open(dev_info->GetIPAddress());

	device.SetStreamDestination(stream.GetLocalIPAddress(), stream.GetLocalPort());

	PvInt64 size = 0;
	payload_size->GetValue(size);

	uint32_t buffer_count = (stream.GetQueuedBufferMaximum() < BUFFER_COUNT) ?
		stream.GetQueuedBufferMaximum() :
		BUFFER_COUNT;
	
	buffers = new PvBuffer[buffer_count];
	for (uint32_t i = 0; i < buffer_count; i++) {
		buffers[i].Alloc(static_cast<PvUInt32>(size));
		stream.QueueBuffer(&buffers[i]);
	}

	if (TLLocked != nullptr) {
		TLLocked->SetValue(1);
	}

	dynamic_cast<PvGenCommand *>(params->Get("GevTimestampControlReset"))->Execute();
	start->Execute();

	printf("Ready for input\n");

	return true;
}

bool Framegrabber::Disconnect() {
	stop->Execute();
	if (TLLocked != nullptr) {
		TLLocked->SetValue(0);
	}

	stream.AbortQueuedBuffers();
	while (stream.GetQueuedBufferCount() > 0) {
		PvBuffer *buf = nullptr;
		PvResult opResult;
		stream.RetrieveBuffer(&buf, &opResult);
	}
	delete[]buffers;
	stream.Close();
	device.Disconnect();
	return true;
}