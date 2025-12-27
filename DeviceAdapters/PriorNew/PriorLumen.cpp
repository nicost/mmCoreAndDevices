///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorLumen.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior Lumen 200Pro lamp implementation
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

#include "PriorLumen.h"
#include <sstream>

CLumen::CLumen() :
   PriorPeripheralBase<CShutterBase, CLumen>(g_LumenDeviceName),
   initialized_(false)
{
   InitializeDefaultErrorMessages();
   EnableDelay();

   // Description
   CreateProperty(MM::g_Keyword_Description, "Prior Lumen 200Pro Lamp", MM::String, true);
}

CLumen::~CLumen()
{
   Shutdown();
}

void CLumen::GetName(char* pszName) const
{
   CDeviceUtils::CopyLimitedString(pszName, g_LumenDeviceName);
}

int CLumen::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   // Get parent hub
   RETURN_ON_MM_ERROR(PeripheralInitialize());

   // Create properties
   CPropertyAction* pAct;

   // State property
   pAct = new CPropertyAction(this, &CLumen::OnState);
   CreateProperty(MM::g_Keyword_State, "0", MM::Integer, false, pAct);
   AddAllowedValue(MM::g_Keyword_State, "0");  // Closed
   AddAllowedValue(MM::g_Keyword_State, "1");  // Open

   // Intensity property (0-255)
   pAct = new CPropertyAction(this, &CLumen::OnIntensity);
   CreateProperty("Intensity", "255", MM::Integer, false, pAct);
   SetPropertyLimits("Intensity", 0, 255);

   initialized_ = true;
   return DEVICE_OK;
}

int CLumen::Shutdown()
{
   if (!initialized_)
      return DEVICE_OK;

   initialized_ = false;
   return DEVICE_OK;
}

bool CLumen::Busy()
{
   // Lumen operations are instantaneous
   return false;
}

//////////////////////////////////////////////////////////////////////////////
// Shutter API
//////////////////////////////////////////////////////////////////////////////

int CLumen::SetOpen(bool open)
{
   std::ostringstream command;
   command << "LM," << (open ? "1" : "0");  // 1=on, 0=off

   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int CLumen::GetOpen(bool& open)
{
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("LM", response));

   // Parse numeric response (1=on, 0=off)
   long state;
   RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, state));

   open = (state == 1);
   return DEVICE_OK;
}

int CLumen::Fire(double deltaT)
{
   // Open lamp
   RETURN_ON_MM_ERROR(SetOpen(true));

   // Wait for specified time
   CDeviceUtils::SleepMs((long)(deltaT + 0.5));

   // Close lamp
   RETURN_ON_MM_ERROR(SetOpen(false));

   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Action handlers
//////////////////////////////////////////////////////////////////////////////

int CLumen::OnIntensity(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      // Query current intensity
      std::string response;
      RETURN_ON_MM_ERROR(hub_->QueryCommand("LI", response));

      // Parse numeric response
      long intensity;
      RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, intensity));

      pProp->Set(intensity);
   }
   else if (eAct == MM::AfterSet)
   {
      long intensity;
      pProp->Get(intensity);

      // Clamp to valid range
      if (intensity < 0) intensity = 0;
      if (intensity > 255) intensity = 255;

      // Set intensity
      std::ostringstream command;
      command << "LI," << intensity;

      std::string response;
      RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

      if (!hub_->IsSuccessResponse(response) && hub_->IsErrorResponse(response))
         return hub_->GetErrorCode(response);
   }
   return DEVICE_OK;
}
