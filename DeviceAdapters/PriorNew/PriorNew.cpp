///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorNew.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior ProScan controller adapter module API
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

#include "PriorNew.h"
#include "PriorProScanHub.h"
#include "PriorXYStage.h"
#include "PriorZStage.h"
#include "PriorNanoZStage.h"
#include "PriorShutter.h"
#include "PriorWheel.h"
#include "PriorLumen.h"
#include "PriorTTLShutter.h"
#include "ModuleInterface.h"

///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////

MODULE_API void InitializeModuleData()
{
   // Register the hub
   RegisterDevice(g_HubDeviceName, MM::HubDevice, "Prior ProScan Controller Hub");

   // Register stages
   RegisterDevice(g_XYStageDeviceName, MM::XYStageDevice, "Prior XY Stage");
   RegisterDevice(g_ZStageDeviceName, MM::StageDevice, "Prior Z Stage");
   RegisterDevice(g_NanoZStageDeviceName, MM::StageDevice, "Prior Nano Z Stage");

   // Register shutters
   RegisterDevice(g_Shutter1DeviceName, MM::ShutterDevice, "Prior Shutter 1");
   RegisterDevice(g_Shutter2DeviceName, MM::ShutterDevice, "Prior Shutter 2");
   RegisterDevice(g_Shutter3DeviceName, MM::ShutterDevice, "Prior Shutter 3");

   // Register filter wheels
   RegisterDevice(g_Wheel1DeviceName, MM::StateDevice, "Prior Filter Wheel 1");
   RegisterDevice(g_Wheel2DeviceName, MM::StateDevice, "Prior Filter Wheel 2");
   RegisterDevice(g_Wheel3DeviceName, MM::StateDevice, "Prior Filter Wheel 3");

   // Register Lumen
   RegisterDevice(g_LumenDeviceName, MM::ShutterDevice, "Prior Lumen 200Pro Lamp");

   // Register TTL shutters
   RegisterDevice(g_TTL0DeviceName, MM::ShutterDevice, "Prior TTL Shutter 0");
   RegisterDevice(g_TTL1DeviceName, MM::ShutterDevice, "Prior TTL Shutter 1");
   RegisterDevice(g_TTL2DeviceName, MM::ShutterDevice, "Prior TTL Shutter 2");
   RegisterDevice(g_TTL3DeviceName, MM::ShutterDevice, "Prior TTL Shutter 3");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == nullptr)
      return nullptr;

   std::string name(deviceName);

   // Hub
   if (name == g_HubDeviceName)
   {
      return new CPriorProScanHub();
   }

   // Stages
   else if (name == g_XYStageDeviceName)
   {
      return new CXYStage();
   }
   else if (name == g_ZStageDeviceName)
   {
      return new CZStage();
   }
   else if (name == g_NanoZStageDeviceName)
   {
      return new CNanoZStage();
   }

   // Shutters
   else if (name == g_Shutter1DeviceName)
   {
      return new CShutter(g_Shutter1DeviceName, 1);
   }
   else if (name == g_Shutter2DeviceName)
   {
      return new CShutter(g_Shutter2DeviceName, 2);
   }
   else if (name == g_Shutter3DeviceName)
   {
      return new CShutter(g_Shutter3DeviceName, 3);
   }

   // Filter wheels
   else if (name == g_Wheel1DeviceName)
   {
      return new CWheel(g_Wheel1DeviceName, 1);
   }
   else if (name == g_Wheel2DeviceName)
   {
      return new CWheel(g_Wheel2DeviceName, 2);
   }
   else if (name == g_Wheel3DeviceName)
   {
      return new CWheel(g_Wheel3DeviceName, 3);
   }

   // Lumen
   else if (name == g_LumenDeviceName)
   {
      return new CLumen();
   }

   // TTL Shutters
   else if (name == g_TTL0DeviceName)
   {
      return new CTTLShutter(g_TTL0DeviceName, 0);
   }
   else if (name == g_TTL1DeviceName)
   {
      return new CTTLShutter(g_TTL1DeviceName, 1);
   }
   else if (name == g_TTL2DeviceName)
   {
      return new CTTLShutter(g_TTL2DeviceName, 2);
   }
   else if (name == g_TTL3DeviceName)
   {
      return new CTTLShutter(g_TTL3DeviceName, 3);
   }

   return nullptr;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}
