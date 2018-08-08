#include "framegrabber.h"
#include <vector>
#include <stdexcept>
#include <stdio.h>
#include "iomanager.h"
#include "cpptoml.h"


Framegrabber::Framegrabber() {
	printf("Loading configuration...\n");
	load_config("C:/Users/Keck Project/Documents/Framegrabber/framegrabber.cfg");
	iomanager = new IOManager(this);
	words = SerialWords(this);
	auto &fgconfig = config.fg_config;

	width = fgconfig.img_w;
	height = fgconfig.img_h;
}

Framegrabber::~Framegrabber() {
	delete iomanager;
}

PvDeviceInfoGEV *Framegrabber::select_sole_device() {
	iomanager->info(__FUNCTION__, "Looking for devices");
	PvSystem *sys = new PvSystem();
	sys->Find();

	std::vector<const PvDeviceInfoGEV*> devices;
	auto interfacecount = sys->GetInterfaceCount();
	for (unsigned i = 0; i < interfacecount; i++) {
		auto iface = sys->GetInterface(i);
		auto devcount = iface->GetDeviceCount();
		for (unsigned j = 0; j < devcount; j++) {
			auto device = iface->GetDeviceInfo(j);
			devices.push_back(dynamic_cast<const PvDeviceInfoGEV*>(device));
		}
	}
	if (devices.size() != 1) {
		iomanager->fatal("No (or more than one) devices found.");
		return NULL;
	}
	char msgbuf[1024];
	sprintf(msgbuf, "Found device %s", ((PvDeviceInfoGEV*)devices[0])->GetMACAddress().GetAscii());
	iomanager->info(__FUNCTION__, msgbuf);
	return (PvDeviceInfoGEV*)devices[0];
}

void Framegrabber::stream_info() {
	PvStreamInfo info(&stream);
	iomanager->info(__FUNCTION__, info.GetErrors().GetAscii());
	iomanager->info(__FUNCTION__, info.GetWarnings(false).GetAscii());
	iomanager->info(__FUNCTION__, info.GetStatistics(config.window.fps).GetAscii());
	iomanager->info(__FUNCTION__, std::to_string(stream.GetQueuedBufferCount()));
}

void Framegrabber::load_config(const char *configfile)
{
	std::vector<std::string> defaults_used;
	std::shared_ptr<cpptoml::table> config_table;
	remove("crash.txt");
	try {
		config_table = cpptoml::parse_file(configfile);
	}
	catch (cpptoml::parse_exception &e) {
		FILE *crash = fopen("crash.txt", "w");
		fprintf(crash, "Error parsing config: %s\n", e.what());
		fclose(crash);
		throw std::runtime_error("Error loading configuration");
	}
	
	config.communications.ip_addr = config_table->get_qualified_as<std::string>("communications.ip").value_or("*");
	config.communications.port = config_table->get_qualified_as<int>("communications.port").value_or(5555);
	config.communications.logfile = config_table->get_qualified_as<std::string>("communications.logfile").value_or("fg.log");

	config.fg_config.img_h = config_table->get_qualified_as<int>("configuration.img_height").value_or(16);
	config.fg_config.img_w = config_table->get_qualified_as<int>("configuration.img_width").value_or(64);
	config.fg_config.maxapps = config_table->get_qualified_as<int>("configuration.max_apps").value_or(512);
	config.fg_config.bufcount = config_table->get_qualified_as<int>("configuration.buffer_count").value_or(64);
  config.fg_config.font = config_table->get_qualified_as<std::string>("configuration.font").value_or("");

	config.window.fps = config_table->get_qualified_as<int>("window.fps").value_or(50);
	config.window.scaling = config_table->get_qualified_as<int>("window.scaling").value_or(10);
	config.window.text_height = config_table->get_qualified_as<int>("window.text_height").value_or(20);
  
  config.focusergraph.fps = config_table->get_qualified_as<int>("focusergraph.fps").value_or(20);
  config.focusergraph.height = config_table->get_qualified_as<int>("focusergraph.height").value_or(600);
  config.focusergraph.width = config_table->get_qualified_as<int>("focusergraph.width").value_or(600);
  config.focusergraph.font_size = config_table->get_qualified_as<int>("focusergraph.font_size").value_or(16);
}

void Framegrabber::data_loop() {
	words.write_words(&device);

	PvBuffer *buffer = NULL;
	PvResult opResult;

	PvResult result = stream.RetrieveBuffer(&buffer, &opResult, 1000);
	if (result.IsOK()) {
		if (opResult.IsOK()) {
			if (buffer->GetPayloadType() == PvPayloadTypeImage) {
				PvImage *img = buffer->GetImage();
				width = img->GetWidth();
				height = img->GetHeight();
				uint16_t *vdata = (uint16_t*)(img->GetDataPointer());

				for (FramegrabberApp *app : apps) {
					app->set_frame(vdata);
				}
			}
			else {
				iomanager->warning(__FUNCTION__, "Buffer payload type not image.");
			}
		}
		else {
			iomanager->warning(__FUNCTION__, "opResult error. Stream info following.");
			//stream_info();
		}
		stream.QueueBuffer(buffer);
	}
	else {
		iomanager->error(__FUNCTION__, "Could not retrieve buffer");
		char errstr[64];
		snprintf(errstr, 64, "%s, %s", result.GetCodeString().GetAscii(), opResult.GetCodeString().GetAscii());
		iomanager->error(__FUNCTION__, errstr);
		PvStreamInfo info(&stream);
		iomanager->info(__FUNCTION__, info.GetErrors().GetAscii());
		iomanager->info(__FUNCTION__, info.GetWarnings(false).GetAscii());
		iomanager->info(__FUNCTION__, info.GetStatistics(60).GetAscii());
		iomanager->info(__FUNCTION__, std::to_string(stream.GetQueuedBufferCount()));
		throw std::runtime_error("Error retrieving buffer");
	}
}

std::pair<int, int> Framegrabber::small_to_large(int x, int y)
{
	int wax = words.get_wax();
	int way = words.get_way();
	return std::pair<int, int>(x + (wax/4)*8, y + way*2 + 1);
}

bool Framegrabber::Connect() {
	PvDeviceInfoGEV *dev_info = select_sole_device();

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

#if VERSION_MAJOR < 5
  params = device.GetGenParameters();
#else
  params = device.GetParameters();
#endif
	TLLocked = dynamic_cast<PvGenInteger *>(params->Get("TLParamsLocked"));
	payload_size = dynamic_cast<PvGenInteger *>(params->Get("PayloadSize"));
	start = dynamic_cast<PvGenCommand *>(params->Get("AcquisitionStart"));
	stop = dynamic_cast<PvGenCommand *>(params->Get("AcquisitionStop"));

	device.NegotiatePacketSize();

	iomanager->info(__FUNCTION__, "Opening stream to device");
	stream.Open(dev_info->GetIPAddress());

	device.SetStreamDestination(stream.GetLocalIPAddress(), stream.GetLocalPort());

	int64_t size = 0;
	payload_size->GetValue(size);

	auto &cfg_bufcount = config.fg_config.bufcount;
	uint32_t buffer_count = (stream.GetQueuedBufferMaximum() < (uint32_t)cfg_bufcount) ?
		stream.GetQueuedBufferMaximum() :
		cfg_bufcount;
	
	buffers = new PvBuffer[buffer_count];
	for (uint32_t i = 0; i < buffer_count; i++) {
		buffers[i].Alloc(static_cast<uint32_t>(size));
	}

	for (uint32_t i = 0; i < buffer_count; i++) {
		stream.QueueBuffer(buffers + i);
	}

	if (TLLocked != nullptr) {
		TLLocked->SetValue(1);
	}

	try {
		dynamic_cast<PvGenCommand *>(params->Get("GevTimestampControlReset"))->Execute();
		start->Execute();
	}
	catch (std::runtime_error) {
		iomanager->fatal("Could not start stream. Check that power to the camera head is turned on");
	}

	iomanager->ready();

	return true;
}

// NOFATAL: Because this function is called in fatal(),
// it may not throw a fatal error.
bool Framegrabber::Disconnect() {
	if (stop) {
		stop->Execute();
	}
	else {
		iomanager->warning(__FUNCTION__, "stop is NULL. Exiting early.");
		return false;
	}
	
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
	return true;
}
