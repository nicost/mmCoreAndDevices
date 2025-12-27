///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorHub.h
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

#ifndef _PRIORHUB_H_
#define _PRIORHUB_H_

#include "PriorNew.h"
#include "DeviceBase.h"
#include <mutex>

//////////////////////////////////////////////////////////////////////////////
// PriorHub - Generic hub base class for serial communication
//////////////////////////////////////////////////////////////////////////////

class PriorHub
{
public:
   PriorHub();
   virtual ~PriorHub();

   //////////////////////////////////////////////////////////////////////////////
   // Thread-safe serial communication
   //////////////////////////////////////////////////////////////////////////////

   // Clear the serial port buffer
   int QueryCommand(const std::string& command, std::string& response);

   // Send command and verify response starts with expected prefix
   int QueryCommandVerify(const std::string& command,
                         const std::string& expectedPrefix,
                         std::string& response);

protected:
   // Virtual methods that derived class must implement to access MM DeviceBase functions
   virtual int PurgeComPortImpl() = 0;
   virtual int SendSerialCommandImpl(const std::string& command) = 0;
   virtual int GetSerialAnswerImpl(std::string& answer) = 0;

   //////////////////////////////////////////////////////////////////////////////
   // Response parsing utilities (public for use by peripheral devices)
   //////////////////////////////////////////////////////////////////////////////

   // Check if response indicates success (starts with "R")
   bool IsSuccessResponse(const std::string& response);

   // Check if response indicates error (starts with "E,")
   bool IsErrorResponse(const std::string& response);

   // Get error code from error response "E,N"
   int GetErrorCode(const std::string& response);

   // Parse numeric response (position value, status, etc.)
   int ParseNumericResponse(const std::string& response, long& value);
   int ParseNumericResponse(const std::string& response, double& value);

   // Get last command and response (for debugging)
   std::string LastCommand() const { return lastCommand_; }
   std::string LastResponse() const { return lastResponse_; }

   //////////////////////////////////////////////////////////////////////////////
   // Device registration
   //////////////////////////////////////////////////////////////////////////////

   // Register a peripheral device with the hub
   void RegisterPeripheral(const std::string& deviceLabel, int deviceId);

   // Unregister a peripheral device
   void UnRegisterPeripheral(const std::string& deviceLabel);

   // Check if a device is registered
   bool IsDeviceRegistered(const std::string& deviceLabel);

protected:
   std::mutex serialLock_;                      // Thread safety for serial port
   std::string port_;                           // Serial port name
   std::string lastCommand_;                    // Last command sent (for debugging)
   std::string lastResponse_;                   // Last response received (for debugging)
   std::map<std::string, int> deviceMap_;      // deviceLabel -> deviceId mapping
   bool initialized_;
};

#endif // _PRIORHUB_H_
