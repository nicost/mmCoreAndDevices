///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorNew.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior ProScan controller adapter using Hub pattern
//
// AUTHOR:        Nico Stuurman, 2025
//
// COPYRIGHT:     University of California, San Francisco, 2025
//
// LICENSE:       This file is distributed under the BSD license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.

#ifndef _PRIORNEW_H_
#define _PRIORNEW_H_

#include "MMDevice.h"
#include "DeviceBase.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>

//////////////////////////////////////////////////////////////////////////////
// Error codes
//////////////////////////////////////////////////////////////////////////////

#define ERR_PRIOR_OFFSET                 11000

// Communication errors
#define ERR_PRIOR_UNRECOGNIZED_ANSWER    (ERR_PRIOR_OFFSET + 1)
#define ERR_PRIOR_INVALID_RESPONSE       (ERR_PRIOR_OFFSET + 2)
#define ERR_PRIOR_TIMEOUT                (ERR_PRIOR_OFFSET + 3)
#define ERR_PRIOR_COMMAND_FAILED         (ERR_PRIOR_OFFSET + 4)
#define ERR_PRIOR_SERIAL_COMMAND_FAILED  (ERR_PRIOR_OFFSET + 5)

// Configuration errors
#define ERR_PRIOR_PORT_CHANGE_FORBIDDEN  (ERR_PRIOR_OFFSET + 10)
#define ERR_PRIOR_INVALID_DEVICE_ID      (ERR_PRIOR_OFFSET + 11)
#define ERR_PRIOR_HUB_NOT_FOUND          (ERR_PRIOR_OFFSET + 12)
#define ERR_PRIOR_INVALID_PROPERTY       (ERR_PRIOR_OFFSET + 13)

// Device-specific errors
#define ERR_PRIOR_POSITION_OUT_OF_RANGE  (ERR_PRIOR_OFFSET + 20)
#define ERR_PRIOR_MOVEMENT_FAILED        (ERR_PRIOR_OFFSET + 21)
#define ERR_PRIOR_DEVICE_NOT_PRESENT     (ERR_PRIOR_OFFSET + 22)
#define ERR_PRIOR_WHEEL_POSITION_INVALID (ERR_PRIOR_OFFSET + 23)
#define ERR_PRIOR_SHUTTER_FAILED         (ERR_PRIOR_OFFSET + 24)

// Controller error codes (offset from controller error numbers)
// When controller returns "E,N", the error code is ERR_PRIOR_CONTROLLER_OFFSET + N
#define ERR_PRIOR_CONTROLLER_OFFSET      (ERR_PRIOR_OFFSET + 100)

//////////////////////////////////////////////////////////////////////////////
// Device names
//////////////////////////////////////////////////////////////////////////////

// Hub
#define g_HubDeviceName                  "PriorProScanHub"

// Stages
#define g_XYStageDeviceName              "XYStage"
#define g_ZStageDeviceName               "ZStage"
#define g_NanoZStageDeviceName           "NanoScanZ"

// Shutters
#define g_Shutter1DeviceName             "Shutter-1"
#define g_Shutter2DeviceName             "Shutter-2"
#define g_Shutter3DeviceName             "Shutter-3"

// Filter Wheels
#define g_Wheel1DeviceName               "Wheel-1"
#define g_Wheel2DeviceName               "Wheel-2"
#define g_Wheel3DeviceName               "Wheel-3"

// Lumen
#define g_LumenDeviceName                "Lumen"

// TTL Shutters
#define g_TTL0DeviceName                 "TTL-0"
#define g_TTL1DeviceName                 "TTL-1"
#define g_TTL2DeviceName                 "TTL-2"
#define g_TTL3DeviceName                 "TTL-3"

//////////////////////////////////////////////////////////////////////////////
// Property names
//////////////////////////////////////////////////////////////////////////////

#define g_PropName_SerialCommand         "SerialCommand"
#define g_PropName_SerialResponse        "SerialResponse"
#define g_PropName_FirmwareVersion       "FirmwareVersion"
#define g_PropName_ControllerModel       "ControllerModel"

//////////////////////////////////////////////////////////////////////////////
// Utility macros
//////////////////////////////////////////////////////////////////////////////

#define RETURN_ON_MM_ERROR(result) do { \
   int return_value = (result); \
   if (return_value != DEVICE_OK) { \
      return return_value; \
   } \
} while (0)

#endif // _PRIORNEW_H_
