#include "framegrabber.h"
#include <PvSystem.h>
#include <vector>
#include <stdexcept>
#include <stdio.h>
#include "iomanager.h"

#define BUFFER_COUNT (16)

Framegrabber::Framegrabber() {
	iomanager = new IOManager(this);
	width = 320;
	height = 256;
}

Framegrabber::~Framegrabber() {
	delete iomanager;
}

PvDeviceInfo *Framegrabber::select_sole_device() {
	iomanager->info("CONNECTION", "Looking for devices");
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
		iomanager->fatal("No (or more than one) devices found.");
		return NULL;
	}
	char msgbuf[1024];
	sprintf(msgbuf, "Found device %s", (PvDeviceInfo*)devices[0]->GetMACAddress().GetAscii());
	iomanager->info("CONNECTION", msgbuf);
	return (PvDeviceInfo*)devices[0];
}

void Framegrabber::data_loop() {
	printf("In dl\n");
	PvBuffer *buffer = NULL;
	PvResult opResult;
	
	PvResult result = stream.RetrieveBuffer(&buffer, &opResult, 1000);
	if (result.IsOK() && opResult.IsOK()) {
		if (buffer->GetPayloadType() == PvPayloadTypeImage) {
			printf("Got img\n");
			PvImage *img = buffer->GetImage();
			width = img->GetWidth();
			height = img->GetHeight();
			uint16_t *vdata = (uint16_t*)(img->GetDataPointer());

			for (FramegrabberApp *app : apps) {
				printf("setting data for %s\n", app->id);
				app->set_frame(vdata);
			}
		}
		stream.QueueBuffer(buffer);
	}
	else {
		printf("%s, %s\n", result.GetCodeString().GetAscii(), opResult.GetCodeString().GetAscii());
		throw std::runtime_error("Error retrieving buffer");
	}
}

bool Framegrabber::Connect() {
	PvDeviceInfo *dev_info = select_sole_device();

	if (dev_info == nullptr) {
		iomanager->fatal("No device found");
		return false;
	}

	char msgbuf[1024];
	sprintf(msgbuf, "Connecting to device %s", dev_info->GetMACAddress().GetAscii());
	iomanager->info("CONNECTION", msgbuf);
	if (device.Connect(dev_info).IsFailure()) {
		sprintf(msgbuf, "Unable to connect to %s", dev_info->GetMACAddress().GetAscii());
		iomanager->fatal(msgbuf);
		return false;
	}
	else {
		sprintf(msgbuf, "Connected to %s", dev_info->GetMACAddress().GetAscii());
		iomanager->success("CONNECTION", msgbuf);
	}

	params = device.GetGenParameters();
	TLLocked = dynamic_cast<PvGenInteger *>(params->Get("TLParamsLocked"));
	payload_size = dynamic_cast<PvGenInteger *>(params->Get("PayloadSize"));
	start = dynamic_cast<PvGenCommand *>(params->Get("AcquisitionStart"));
	stop = dynamic_cast<PvGenCommand *>(params->Get("AcquisitionStop"));

	device.NegotiatePacketSize();

	iomanager->info("STREAM", "Opening stream to device");
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

	iomanager->ready();

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