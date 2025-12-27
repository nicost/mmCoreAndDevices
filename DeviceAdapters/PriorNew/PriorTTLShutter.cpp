///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorTTLShutter.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior TTL shutter implementation (TTL 0-3)
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

#include "PriorTTLShutter.h"
#include <sstream>

CTTLShutter::CTTLShutter(const char* name, int id) :
   PriorPeripheralBase<CShutterBase, CTTLShutter>(name),
   name_(name),
   ttlId_(id),
   initialized_(false)
{
   InitializeDefaultErrorMessages();
   EnableDelay();

   // Description
   std::ostringstream desc;
   desc << "Prior TTL Shutter " << id;
   CreateProperty(MM::g_Keyword_Description, desc.str().c_str(), MM::String, true);
}

CTTLShutter::~CTTLShutter()
{
   Shutdown();
}

void CTTLShutter::GetName(char* pszName) const
{
   CDeviceUtils::CopyLimitedString(pszName, name_.c_str());
}

int CTTLShutter::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   // Get parent hub
   deviceId_ = ttlId_;
   RETURN_ON_MM_ERROR(PeripheralInitialize());

   // Create state property
   CPropertyAction* pAct = new CPropertyAction(this, &CTTLShutter::OnState);
   CreateProperty(MM::g_Keyword_State, "0", MM::Integer, false, pAct);
   AddAllowedValue(MM::g_Keyword_State, "0");  // Closed
   AddAllowedValue(MM::g_Keyword_State, "1");  // Open

   initialized_ = true;
   return DEVICE_OK;
}

int CTTLShutter::Shutdown()
{
   if (!initialized_)
      return DEVICE_OK;

   initialized_ = false;
   return DEVICE_OK;
}

bool CTTLShutter::Busy()
{
   // TTL shutters are not busy (instantaneous operation)
   return false;
}

//////////////////////////////////////////////////////////////////////////////
// Shutter API
//////////////////////////////////////////////////////////////////////////////

int CTTLShutter::SetOpen(bool open)
{
   std::ostringstream command;
   command << "TTL," << ttlId_ << "," << (open ? "1" : "0");  // 1=high/open, 0=low/closed

   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int CTTLShutter::GetOpen(bool& open)
{
   std::ostringstream command;
   command << "TTL," << ttlId_;

   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

   // Parse numeric response (1=high/open, 0=low/closed)
   long state;
   RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, state));

   open = (state == 1);
   return DEVICE_OK;
}

int CTTLShutter::Fire(double deltaT)
{
   // Open shutter
   RETURN_ON_MM_ERROR(SetOpen(true));

   // Wait for specified time
   CDeviceUtils::SleepMs((long)(deltaT + 0.5));

   // Close shutter
   RETURN_ON_MM_ERROR(SetOpen(false));

   return DEVICE_OK;
}
