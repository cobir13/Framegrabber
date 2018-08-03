//
//  ebus.h
//  Framegrabber
//
//  Created by Thomas Malthouse on 8/2/18.
//  Copyright © 2018 Thomas Malthouse. All rights reserved.
//

#ifndef ebus_h
#define ebus_h

#ifdef __APPLE__
#include <eBUS/PvDeviceGEV.h>
#include <eBUS/PvGenParameterArray.h>
#include <eBUS/PvStreamGEV.h>
#include <eBUS/PvBuffer.h>
#include <eBUS/PvResult.h>
#include <eBUS/PvSystem.h>
#include <eBUS/PvStreamInfo.h>
#include <eBus/PvVersion.h>
#else
#include <PvDevice.h>
#include <PvGenParameterArray.h>
#include <PvStream.h>
#include <PvBuffer.h>
#include <PvResult.h>
#include <PvSystem.h>
#include <PvStreamInfo.h>
#include <PvVersion.h
#endif

// This code was written for eBUS SDK v3.x, and we need to keep supporting it.
// This is a little hacky, but originally eBUS only supported GEV devices (no USB3)
// so the old PvDevice is very similar to today's PvDeviceGEV
#if VERSION_MAJOR < 5
typedef PvDevice PvDeviceGEV;
typedef PvStream PvStreamGEV;
typedef PvDeviceInfo PvDeviceInfoGEV;
#endif


#endif /* ebus_h */
