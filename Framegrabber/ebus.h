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
#define EBUS_VERSION_5
#else
#include <PvDevice.h>
#include <PvGenParameterArray.h>
#include <PvStream.h>
#include <PvBuffer.h>
#include <PvResult.h>
#include <PvSystem.h>
#include <PvStreamInfo.h>
#define EBUS_VERSION_3
#endif

//This code was written for eBUS SDK v3.x, and we need to keep supporting it.
#ifdef EBUS_VERSION_3
typedef PvDevice PvDeviceGEV;
typedef PvStream PvStreamGEV;
typedef PvDeviceInfo PvDeviceInfoGEV;
#endif


#endif /* ebus_h */
