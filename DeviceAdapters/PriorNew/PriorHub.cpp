///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorHub.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Generic hub base class for Prior controllers
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

#include "PriorHub.h"
#include "ModuleInterface.h"
#include <sstream>
#include <cstdlib>

PriorHub::PriorHub() :
   port_("Undefined"),
   lastCommand_(""),
   lastResponse_(""),
   initialized_(false)
{
}

PriorHub::~PriorHub()
{
}

//////////////////////////////////////////////////////////////////////////////
// Serial Communication
//////////////////////////////////////////////////////////////////////////////

int PriorHub::QueryCommand(const std::string& command, std::string& response)
{
   std::lock_guard<std::mutex> lock(serialLock_);

   // Clear any stale data in the port
   int ret = PurgeComPortImpl();
   if (ret != DEVICE_OK)
      return ret;

   // Send command with \r terminator
   std::string fullCommand = command + "\r";
   ret = SendSerialCommandImpl(fullCommand);
   if (ret != DEVICE_OK)
   {
      lastCommand_ = command;
      lastResponse_ = "";
      return ERR_PRIOR_SERIAL_COMMAND_FAILED;
   }

   // Get response (terminated by \r)
   ret = GetSerialAnswerImpl(response);
   if (ret != DEVICE_OK)
   {
      lastCommand_ = command;
      lastResponse_ = "";
      return ERR_PRIOR_TIMEOUT;
   }

   // Store for debugging
   lastCommand_ = command;
   lastResponse_ = response;

   return DEVICE_OK;
}

int PriorHub::QueryCommandVerify(const std::string& command,
                                 const std::string& expectedPrefix,
                                 std::string& response)
{
   int ret = QueryCommand(command, response);
   if (ret != DEVICE_OK)
      return ret;

   // Check if response starts with expected prefix
   if (response.length() < expectedPrefix.length() ||
       response.substr(0, expectedPrefix.length()) != expectedPrefix)
   {
      return ERR_PRIOR_UNRECOGNIZED_ANSWER;
   }

   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Response Parsing
//////////////////////////////////////////////////////////////////////////////

bool PriorHub::IsSuccessResponse(const std::string& response)
{
   return (response.length() >= 1 && response[0] == 'R');
}

bool PriorHub::IsErrorResponse(const std::string& response)
{
   return (response.length() >= 2 && response.substr(0, 2) == "E,");
}

int PriorHub::GetErrorCode(const std::string& response)
{
   if (!IsErrorResponse(response))
      return ERR_PRIOR_UNRECOGNIZED_ANSWER;

   // Parse error number from "E,N" format
   if (response.length() > 2)
   {
      int errorNum = atoi(response.substr(2).c_str());
      return ERR_PRIOR_CONTROLLER_OFFSET + errorNum;
   }

   return ERR_PRIOR_INVALID_RESPONSE;
}

int PriorHub::ParseNumericResponse(const std::string& response, long& value)
{
   if (response.empty())
      return ERR_PRIOR_INVALID_RESPONSE;

   // Check if this is an error response
   if (IsErrorResponse(response))
      return GetErrorCode(response);

   // Parse the numeric value
   char* endPtr;
   value = strtol(response.c_str(), &endPtr, 10);

   // Check if parsing succeeded
   if (endPtr == response.c_str())
      return ERR_PRIOR_INVALID_RESPONSE;

   return DEVICE_OK;
}

int PriorHub::ParseNumericResponse(const std::string& response, double& value)
{
   if (response.empty())
      return ERR_PRIOR_INVALID_RESPONSE;

   // Check if this is an error response
   if (IsErrorResponse(response))
      return GetErrorCode(response);

   // Parse the numeric value
   char* endPtr;
   value = strtod(response.c_str(), &endPtr);

   // Check if parsing succeeded
   if (endPtr == response.c_str())
      return ERR_PRIOR_INVALID_RESPONSE;

   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Device Registration
//////////////////////////////////////////////////////////////////////////////

void PriorHub::RegisterPeripheral(const std::string& deviceLabel, int deviceId)
{
   deviceMap_[deviceLabel] = deviceId;
}

void PriorHub::UnRegisterPeripheral(const std::string& deviceLabel)
{
   deviceMap_.erase(deviceLabel);
}

bool PriorHub::IsDeviceRegistered(const std::string& deviceLabel)
{
   return (deviceMap_.find(deviceLabel) != deviceMap_.end());
}
