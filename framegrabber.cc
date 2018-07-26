#include "framegrabber.h"
#include <PvSystem.h>
#include <PvStreamInfo.h>
#include <vector>
#include <stdexcept>
#include <stdio.h>
#include "iomanager.h"

#define BUFFER_COUNT (120)

Framegrabber::Framegrabber() {
	iomanager = new IOManager(this);
	width = 320;
	height = 256;
}

Framegrabber::~Framegrabber() {
	delete iomanager;
}

PvDeviceInfo *Framegrabber::select_sole_device() {
	iomanager->info(__FUNCTION__, "Looking for devices");
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
	iomanager->info(__FUNCTION__, msgbuf);
	return (PvDeviceInfo*)devices[0];
}

void Framegrabber::data_loop() {
	PvStreamInfo info(&stream);
	iomanager->info(__FUNCTION__, info.GetErrors().GetAscii());
	iomanager->info(__FUNCTION__, info.GetWarnings(false).GetAscii());
	iomanager->info(__FUNCTION__, info.GetStatistics(60).GetAscii());
	iomanager->info(__FUNCTION__, std::to_string(stream.GetQueuedBufferCount()));
	PvBuffer *buffer = NULL;
	PvResult opResult;
	static int i = 0;
	PvResult result = stream.RetrieveBuffer(&buffer, &opResult, 1000);
	if (result.IsOK() && opResult.IsOK()) {
		printf("i=%d\n", i++);
		if (buffer->GetPayloadType() == PvPayloadTypeImage) {
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
		iomanager->error(__FUNCTION__, "Could not retrieve buffer");
		char errstr[64];
		sprintf_s(errstr, 64, "%s, %s", result.GetCodeString().GetAscii(), opResult.GetCodeString().GetAscii());
		iomanager->error(__FUNCTION__, errstr);
		PvStreamInfo info(&stream);
		iomanager->info(__FUNCTION__, info.GetErrors().GetAscii());
		iomanager->info(__FUNCTION__, info.GetWarnings(false).GetAscii());
		iomanager->info(__FUNCTION__, info.GetStatistics(60).GetAscii());
		iomanager->info(__FUNCTION__, std::to_string(stream.GetQueuedBufferCount()));
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
	sprintf(msgbuf, "Connecting to device %s at %s", dev_info->GetMACAddress().GetAscii(),
		dev_info->GetIPAddress().GetAscii());
	iomanager->info(__FUNCTION__, msgbuf);
	if (!device.Connect(dev_info).IsOK()) {
		sprintf(msgbuf, "Unable to connect to %s", dev_info->GetMACAddress().GetAscii());
		iomanager->fatal(msgbuf);
		return false;
	}
	else {
		sprintf(msgbuf, "Connected to %s", dev_info->GetMACAddress().GetAscii());
		iomanager->info(__FUNCTION__, msgbuf);
	}

	params = device.GetGenParameters();
	TLLocked = dynamic_cast<PvGenInteger *>(params->Get("TLParamsLocked"));
	payload_size = dynamic_cast<PvGenInteger *>(params->Get("PayloadSize"));
	start = dynamic_cast<PvGenCommand *>(params->Get("AcquisitionStart"));
	stop = dynamic_cast<PvGenCommand *>(params->Get("AcquisitionStop"));

	device.NegotiatePacketSize();

	iomanager->info(__FUNCTION__, "Opening stream to device");
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
	}

	for (uint32_t i = 0; i < buffer_count; i++) {
		stream.QueueBuffer(buffers + i);
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
	iomanager->info(__FUNCTION__, "Transmission ended");
	if (TLLocked != nullptr) {
		TLLocked->SetValue(0);
	}

	iomanager->info(__FUNCTION__, "Aborting buffers");
	stream.AbortQueuedBuffers();
	while (stream.GetQueuedBufferCount() > 0) {
		PvBuffer *buf = nullptr;
		PvResult opResult;
		stream.RetrieveBuffer(&buf, &opResult);
	}
	delete[]buffers;
	stream.Close();
	iomanager->info(__FUNCTION__, "Stream closed");
	device.Disconnect();
	iomanager->info(__FUNCTION__, "Disconnected");
	iomanager->done();
	return true;
}

#ifdef FOO
int main(int argc, char **argv) {
	Framegrabber f;
	f.Connect();
	for (unsigned i = 0; 1; i++) {
		printf("%d\n", i);
		try { f.data_loop(); }
		catch (std::runtime_error &e) { f.iomanager->fatal("it ded rip"); }
	}
	f.Disconnect();
	printf("success\n");
}
#endif