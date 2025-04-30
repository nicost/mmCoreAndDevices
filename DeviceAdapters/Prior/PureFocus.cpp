///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorPureFocus.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Device adapter for Prior PureFocus Autofocus System
//                
//                
// AUTHOR:        Your Name
//
// COPYRIGHT:     Your Institution, 2025
//
// LICENSE:       This file is distributed under the BSD license.
//

#include "PureFocus.h"
#include <cstdio>
#include <string>
#include <math.h>
#include "ModuleInterface.h"
#include "DeviceUtils.h"
#include <sstream>

using namespace std;

// External names used by the rest of the system
// to load particular device from the "PriorPureFocus.dll" library
const char* g_PureFocusDevice = "Prior PureFocus";

// Device name variables
const char* g_PureFocusDeviceName = "PureFocus";
const char* g_PureFocusDeviceDescription = "Prior Scientific PureFocus Autofocus System";

///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////

MODULE_API void InitializeModuleData()
{
   RegisterDevice(g_PureFocusDevice, MM::AutoFocusDevice, g_PureFocusDeviceDescription);
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == 0)
      return 0;

   if (strcmp(deviceName, g_PureFocusDevice) == 0)
   {
      return new CPureFocus();
   }

   return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// CPureFocus implementation
// ~~~~~~~~~~~~~~~~~~~~~~~

CPureFocus::CPureFocus() :
   initialized_(false),
   name_(g_PureFocusDeviceName),
   port_("Undefined"),
   locked_(false),
   busy_(false),
   answerTimeoutMs_(1000),
   statusMode_(0)
{
   InitializeDefaultErrorMessages();

   // create pre-initialization properties
   // ------------------------------------

   // Port
   CPropertyAction* pAct = new CPropertyAction(this, &CPureFocus::OnPort);
   CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);
}

CPureFocus::~CPureFocus()
{
   Shutdown();
}

void CPureFocus::GetName(char* Name) const
{
   CDeviceUtils::CopyLimitedString(Name, name_.c_str());
}

int CPureFocus::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   // Set property list
   // -----------------

   // Name
   int ret = CreateProperty(MM::g_Keyword_Name, name_.c_str(), MM::String, true);
   if (DEVICE_OK != ret)
      return ret;

   // Description
   ret = CreateProperty(MM::g_Keyword_Description, g_PureFocusDeviceDescription, MM::String, true);
   if (DEVICE_OK != ret)
      return ret;

   // Initialize the device
   ret = SendCommand("?"); // Query device
   if (ret != DEVICE_OK)
      return ret;

   // Check if we're talking to the right device
   string resp;
   ret = GetResponse(resp);
   if (ret != DEVICE_OK)
      return ret;

   if (resp.substr(0, 3) != "PFS") // Adjust based on actual device response
   {
      return ERR_DEVICE_NOT_FOUND;
   }

   // Status
   CPropertyAction* pAct = new CPropertyAction(this, &CPureFocus::OnStatus);
   ret = CreateProperty("Status", "Unknown", MM::String, true, pAct);
   if (ret != DEVICE_OK)
      return ret;

   // Lock
   pAct = new CPropertyAction(this, &CPureFocus::OnLock);
   ret = CreateProperty("Lock", "Unlocked", MM::String, false, pAct);
   if (ret != DEVICE_OK)
      return ret;

   AddAllowedValue("Lock", "Locked");
   AddAllowedValue("Lock", "Unlocked");

   // Focus score
   pAct = new CPropertyAction(this, &CPureFocus::OnFocusScore);
   ret = CreateProperty("Focus Score", "0", MM::Integer, true, pAct);
   if (ret != DEVICE_OK)
      return ret;

   // Offset
   pAct = new CPropertyAction(this, &CPureFocus::OnOffset);
   ret = CreateProperty("Offset", "0", MM::Integer, false, pAct);
   if (ret != DEVICE_OK)
      return ret;

   initialized_ = true;
   return DEVICE_OK;
}

int CPureFocus::Shutdown()
{
   if (initialized_)
   {
      initialized_ = false;
   }
   return DEVICE_OK;
}

bool CPureFocus::Busy()
{
   return busy_;
}

int CPureFocus::SetContinuousFocusing(bool state)
{
   if (state == locked_)
      return DEVICE_OK;

   int ret;
   if (state)
   {
      ret = SendCommand("LK"); // Enable focus lock
   }
   else
   {
      ret = SendCommand("UL"); // Disable focus lock
   }

   if (ret != DEVICE_OK)
      return ret;

   locked_ = state;
   return DEVICE_OK;
}

int CPureFocus::GetContinuousFocusing(bool& state)
{
   state = locked_;
   return DEVICE_OK;
}

int CPureFocus::FullFocus()
{
   if (locked_)
      return ERR_AUTOFOCUS_LOCKED;

   int ret = SendCommand("AF"); // Start full focus operation
   if (ret != DEVICE_OK)
      return ret;

   // Wait for the auto-focus operation to complete
   // In a real implementation, you might need to poll the device status
   // until it reports that it is no longer busy
   CDeviceUtils::SleepMs(5000); // Sleep for 5 seconds as placeholder

   return DEVICE_OK;
}

int CPureFocus::IncrementalFocus()
{
   if (locked_)
      return ERR_AUTOFOCUS_LOCKED;

   int ret = SendCommand("IF"); // Start incremental focus operation
   if (ret != DEVICE_OK)
      return ret;

   // Wait for the incremental focus operation to complete
   CDeviceUtils::SleepMs(2000); // Sleep for 2 seconds as placeholder

   return DEVICE_OK;
}

int CPureFocus::GetOffset(double& offset)
{
   int ret = SendCommand("GO"); // Get current offset
   if (ret != DEVICE_OK)
      return ret;

   string resp;
   ret = GetResponse(resp);
   if (ret != DEVICE_OK)
      return ret;

   // Extract offset value from response
   // This assumes the response format is something like "OFFSET:123"
   size_t colonPos = resp.find(":");
   if (colonPos != string::npos)
   {
      string offsetStr = resp.substr(colonPos + 1);
      offset = atof(offsetStr.c_str());
   }
   else
   {
      return ERR_UNEXPECTED_RESPONSE;
   }

   return DEVICE_OK;
}

int CPureFocus::SetOffset(double offset)
{
   ostringstream cmd;
   cmd << "SO " << offset; // Set offset command with parameter
   int ret = SendCommand(cmd.str().c_str());
   if (ret != DEVICE_OK)
      return ret;

   return DEVICE_OK;
}

int CPureFocus::GetFocusScore(double& score)
{
   int ret = SendCommand("GFS"); // Get focus score
   if (ret != DEVICE_OK)
      return ret;

   string resp;
   ret = GetResponse(resp);
   if (ret != DEVICE_OK)
      return ret;

   // Extract focus score from response
   // Format assumption: "SCORE:456"
   size_t colonPos = resp.find(":");
   if (colonPos != string::npos)
   {
      string scoreStr = resp.substr(colonPos + 1);
      score = atof(scoreStr.c_str());
   }
   else
   {
      return ERR_UNEXPECTED_RESPONSE;
   }

   return DEVICE_OK;
}

int CPureFocus::ResetDevice(void)
{
   int ret = SendCommand("RST"); // Reset device
   if (ret != DEVICE_OK)
      return ret;

   // Allow time for the device to reset
   CDeviceUtils::SleepMs(5000);

   return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Action handlers
///////////////////////////////////////////////////////////////////////////////

int CPureFocus::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(port_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      if (initialized_)
      {
         // revert
         pProp->Set(port_.c_str());
         return ERR_PORT_CHANGE_FORBIDDEN;
      }

      pProp->Get(port_);
   }

   return DEVICE_OK;
}

int CPureFocus::OnStatus(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      int ret = SendCommand("GS"); // Get status
      if (ret != DEVICE_OK)
         return ret;

      string resp;
      ret = GetResponse(resp);
      if (ret != DEVICE_OK)
         return ret;

      pProp->Set(resp.c_str());
   }

   return DEVICE_OK;
}

int CPureFocus::OnLock(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      if (locked_)
         pProp->Set("Locked");
      else
         pProp->Set("Unlocked");
   }
   else if (eAct == MM::AfterSet)
   {
      string state;
      pProp->Get(state);

      bool lockState = (state == "Locked");
      int ret = SetContinuousFocusing(lockState);
      if (ret != DEVICE_OK)
         return ret;
   }

   return DEVICE_OK;
}

int CPureFocus::OnFocusScore(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      double score;
      int ret = GetFocusScore(score);
      if (ret != DEVICE_OK)
         return ret;

      pProp->Set(score);
   }

   return DEVICE_OK;
}

int CPureFocus::OnOffset(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      double offset;
      int ret = GetOffset(offset);
      if (ret != DEVICE_OK)
         return ret;

      pProp->Set(offset);
   }
   else if (eAct == MM::AfterSet)
   {
      double offset;
      pProp->Get(offset);

      int ret = SetOffset(offset);
      if (ret != DEVICE_OK)
         return ret;
   }

   return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Communication methods
///////////////////////////////////////////////////////////////////////////////

int CPureFocus::SendCommand(const string& cmd)
{
   string cmdWithTerminator = cmd + "\r\n"; // Add line ending
   int ret = WriteToComPort(port_.c_str(), (unsigned char*)cmdWithTerminator.c_str(), (unsigned int)cmdWithTerminator.length());
   if (ret != DEVICE_OK)
      return ret;

   // Sleep for a short period to allow device to process command
   CDeviceUtils::SleepMs(10);

   return DEVICE_OK;
}

int CPureFocus::GetResponse(string& resp)
{
   // Read from serial port until terminator
   const int bufSize = 256;
   char buf[bufSize];
   unsigned int charsRead = 0;

   MM::MMTime startTime = GetCurrentMMTime();
   unsigned char c = '\0';
   do {
      if (ReadFromComPort(port_.c_str(), &c, 1, charsRead) != DEVICE_OK)
         return ERR_COMMUNICATION;

      if (charsRead > 0)
      {
         buf[resp.length()] = c;
         resp += c;
      }

      if (GetCurrentMMTime() - startTime > MM::MMTime(answerTimeoutMs_ * 1000))
         return ERR_RESPONSE_TIMEOUT;

   } while (c != '\n' && resp.length() < (unsigned)bufSize);

   // Remove any carriage returns and line feeds
   resp = RemoveLineEndings(resp);

   return DEVICE_OK;
}

string CPureFocus::RemoveLineEndings(string input)
{
   string output = input;
   output.erase(remove(output.begin(), output.end(), '\r'), output.end());
   output.erase(remove(output.begin(), output.end(), '\n'), output.end());
   return output;
}