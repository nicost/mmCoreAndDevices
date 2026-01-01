///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorProScanHub.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior ProScan controller hub implementation
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

#include "PriorProScanHub.h"
#include "PriorXYStage.h"
#include "PriorZStage.h"
#include "PriorNanoZStage.h"
#include "PriorShutter.h"
#include "PriorWheel.h"
#include "PriorLumen.h"
#include "PriorTTLShutter.h"
#include "ModuleInterface.h"
#include <sstream>

CPriorProScanHub::CPriorProScanHub() :
   PriorHub()
{
   InitializeDefaultErrorMessages();

   // Pre-initialization property: Port
   CPropertyAction* pAct = new CPropertyAction(this, &CPriorProScanHub::OnPort);
   CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);

   // Description
   CreateProperty(MM::g_Keyword_Description,
                  "Prior ProScan controller hub",
                  MM::String, true);
}

CPriorProScanHub::~CPriorProScanHub()
{
   Shutdown();
}

void CPriorProScanHub::GetName(char* pName) const
{
   CDeviceUtils::CopyLimitedString(pName, g_HubDeviceName);
}

bool CPriorProScanHub::Busy()
{
   return false;
}

//////////////////////////////////////////////////////////////////////////////
// Initialization and Shutdown
//////////////////////////////////////////////////////////////////////////////

int CPriorProScanHub::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   // Wait for controller to initialize.  Only needed for Arduino, make this a pre-init property, or remove completely after debugging
   CDeviceUtils::SleepMs(2500);

   // Set controller to standard mode (disable compatibility mode)
   RETURN_ON_MM_ERROR(SetStandardMode());

   // Get controller information
   RETURN_ON_MM_ERROR(GetControllerInfo());

   initialized_ = true;
   return DEVICE_OK;
}

int CPriorProScanHub::Shutdown()
{
   if (!initialized_)
      return DEVICE_OK;

   initialized_ = false;
   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Device Detection
//////////////////////////////////////////////////////////////////////////////

bool CPriorProScanHub::SupportsDeviceDetection()
{
   return true;
}

MM::DeviceDetectionStatus CPriorProScanHub::DetectDevice()
{
   if (initialized_)
      return MM::CanCommunicate;

   // Try to communicate with controller
   std::string response;

   // Set standard mode
   int ret = QueryCommand("COMP 0", response);
   if (ret != DEVICE_OK)
      return MM::CanNotCommunicate;

   // Verify with status command
   ret = QueryCommand("$", response);
   if (ret == DEVICE_OK && response.length() >= 1)
      return MM::CanCommunicate;

   return MM::CanNotCommunicate;
}

int CPriorProScanHub::DetectInstalledDevices()
{
   if (DetectDevice() != MM::CanCommunicate)
      return DEVICE_COMM_HUB_MISSING;

   ClearInstalledDevices();

   // Probe for devices, these will call AddInstalledDevice internally
   DetectXYStage();
   DetectZStage();
   DetectShutters();
   DetectWheels();
   // Not implmenet yet: DetectNanoZStage(); DetectLumen(); DetectTTLShutters();

   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Device Detection Helpers
//////////////////////////////////////////////////////////////////////////////

int CPriorProScanHub::DetectXYStage()
{
   std::string response;

   // Try to query X position
   int ret = QueryCommand("PX", response);
   if (ret != DEVICE_OK)
      return ERR_PRIOR_DEVICE_NOT_PRESENT;

   // Verify response is numeric
   char* endPtr;
   strtol(response.c_str(), &endPtr, 10);
   if (endPtr == response.c_str())
      return ERR_PRIOR_DEVICE_NOT_PRESENT;

   AddInstalledDevice(new XYStage());

   return DEVICE_OK;
}

int CPriorProScanHub::DetectZStage()
{
   std::string response;

   // Try to query Z position
   int ret = QueryCommand("PZ", response);
   if (ret != DEVICE_OK)
      return ERR_PRIOR_DEVICE_NOT_PRESENT;

   // Verify response is numeric
   char* endPtr;
   strtol(response.c_str(), &endPtr, 10);
   if (endPtr == response.c_str())
      return ERR_PRIOR_DEVICE_NOT_PRESENT;

   AddInstalledDevice(new ZStage());

   return DEVICE_OK;
}

int CPriorProScanHub::DetectNanoZStage()
{
   // Nano Z stage detection would require specific commands
   // For now, we'll skip automatic detection
   return ERR_PRIOR_DEVICE_NOT_PRESENT;
}

int CPriorProScanHub::DetectShutters()
{
   // Probe shutters 1-3
   const char* shutterNames[] = {g_Shutter1DeviceName, g_Shutter2DeviceName, g_Shutter3DeviceName};

   for (int id = 1; id <= 3; ++id)
   {
      std::ostringstream cmd;
      cmd << "8," << id;

      std::string response;
      int ret = QueryCommand(cmd.str(), response);

      if (ret == DEVICE_OK && !IsErrorResponse(response))
      {
         // Shutter detected - add to installed devices
         AddInstalledDevice(new Shutter(shutterNames[id - 1], id));
      }
   }

   return DEVICE_OK;
}

int CPriorProScanHub::DetectWheels()
{
   // Probe filter wheels 1-3
   const char* wheelNames[] = {g_Wheel1DeviceName, g_Wheel2DeviceName, g_Wheel3DeviceName};

   for (int id = 1; id <= 3; ++id)
   {
      std::ostringstream cmd;
      cmd << "7," << id << ",F";

      std::string response;
      int ret = QueryCommand(cmd.str(), response);

      if (ret == DEVICE_OK && !IsErrorResponse(response))
      {
         // Filter wheel detected - add to installed devices
         AddInstalledDevice(new Wheel(wheelNames[id - 1], id));
      }
   }

   return DEVICE_OK;
}

int CPriorProScanHub::DetectLumen()
{
   // Lumen detection would require specific commands
   // For now, we'll skip automatic detection
   return ERR_PRIOR_DEVICE_NOT_PRESENT;
}

int CPriorProScanHub::DetectTTLShutters()
{
   // TTL shutter detection would require specific commands
   // For now, we'll skip automatic detection
   return ERR_PRIOR_DEVICE_NOT_PRESENT;
}

//////////////////////////////////////////////////////////////////////////////
// Controller Communication
//////////////////////////////////////////////////////////////////////////////

int CPriorProScanHub::SetStandardMode()
{
   std::string response;
   int ret = QueryCommand("COMP 0", response);
   if (ret != DEVICE_OK)
      return ret;

   // Response should be "0"
   if (response != "0")
      return ERR_PRIOR_COMMAND_FAILED;

   return DEVICE_OK;
}

int CPriorProScanHub::GetControllerInfo()
{
   // Could query firmware version, model, etc.
   // For now, just verify communication works
   std::string response;
   return QueryCommand("$", response);
}

//////////////////////////////////////////////////////////////////////////////
// Property Handlers
//////////////////////////////////////////////////////////////////////////////

int CPriorProScanHub::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(port_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      if (initialized_)
      {
         // Port cannot be changed after initialization
         pProp->Set(port_.c_str());
         return ERR_PRIOR_PORT_CHANGE_FORBIDDEN;
      }
      pProp->Get(port_);
   }

   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// PriorHub virtual method implementations
//////////////////////////////////////////////////////////////////////////////

int CPriorProScanHub::PurgeComPortImpl()
{
   return PurgeComPort(port_.c_str());
}

int CPriorProScanHub::SendSerialCommandImpl(const std::string& command)
{
   return SendSerialCommand(port_.c_str(), command.c_str(), "");
}

int CPriorProScanHub::GetSerialAnswerImpl(std::string& answer)
{
   return GetSerialAnswer(port_.c_str(), "\r", answer);
}
