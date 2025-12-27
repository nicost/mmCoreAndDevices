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
#include "ModuleInterface.h"
#include <sstream>

// Forward declarations for device classes (will be defined later)
class CXYStage;
class CZStage;
class CNanoZStage;
class CShutter;
class CWheel;
class CLumen;
class CTTLShutter;

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

   // Set controller to standard mode (disable compatibility mode)
   RETURN_ON_MM_ERROR(SetStandardMode());

   // Get controller information
   RETURN_ON_MM_ERROR(GetControllerInfo());

   // Create post-initialization properties

   // Serial command (for debugging)
   CPropertyAction* pAct = new CPropertyAction(this, &CPriorProScanHub::OnSerialCommand);
   CreateProperty(g_PropName_SerialCommand, "", MM::String, false, pAct);

   // Serial response (read-only, for debugging)
   pAct = new CPropertyAction(this, &CPriorProScanHub::OnSerialResponse);
   CreateProperty(g_PropName_SerialResponse, "", MM::String, true, pAct);

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

   // Probe for devices
   DetectXYStage();
   DetectZStage();
   DetectNanoZStage();
   DetectShutters();
   DetectWheels();
   DetectLumen();
   DetectTTLShutters();

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

   // XY Stage detected - will be created when we implement CXYStage
   // MM::Device* pDev = ::CreateDevice(g_XYStageDeviceName);
   // if (pDev)
   //    AddInstalledDevice(pDev);

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

   // Z Stage detected
   // Will add device creation when CZStage is implemented

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
   for (int id = 1; id <= 3; ++id)
   {
      std::ostringstream cmd;
      cmd << "8," << id;

      std::string response;
      int ret = QueryCommand(cmd.str(), response);

      if (ret == DEVICE_OK && !IsErrorResponse(response))
      {
         // Shutter detected
         // Will add device creation when CShutter is implemented
      }
   }

   return DEVICE_OK;
}

int CPriorProScanHub::DetectWheels()
{
   // Probe filter wheels 1-3
   for (int id = 1; id <= 3; ++id)
   {
      std::ostringstream cmd;
      cmd << "7," << id;

      std::string response;
      int ret = QueryCommand(cmd.str(), response);

      if (ret == DEVICE_OK && !IsErrorResponse(response))
      {
         // Filter wheel detected
         // Will add device creation when CWheel is implemented
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
   return DEVICE_OK;
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

int CPriorProScanHub::OnSerialCommand(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(lastCommand_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      // User entered a command - execute it
      std::string command;
      pProp->Get(command);

      if (!command.empty())
      {
         std::string response;
         int ret = QueryCommand(command, response);
         if (ret != DEVICE_OK)
            return ret;

         // Update response property
         SetProperty(g_PropName_SerialResponse, response.c_str());
      }
   }

   return DEVICE_OK;
}

int CPriorProScanHub::OnSerialResponse(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(lastResponse_.c_str());
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
