///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorWheel.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior filter wheel implementation (wheels 1-3)
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

#include "PriorWheel.h"
#include <sstream>

Wheel::Wheel(const char* name, int id) :
   PriorPeripheralBase<CStateDeviceBase<Wheel>>(name),
   name_(name),
   wheelId_(id),
   numPositions_(6),  // Default to 6 positions
   initialized_(false)
{
   InitializeDefaultErrorMessages();
   EnableDelay();

   // Description
   std::ostringstream desc;
   desc << "Prior Filter Wheel " << id;
   CreateProperty(MM::g_Keyword_Description, desc.str().c_str(), MM::String, true);
}

Wheel::~Wheel()
{
   Shutdown();
}

void Wheel::GetName(char* pszName) const
{
   CDeviceUtils::CopyLimitedString(pszName, name_.c_str());
}

int Wheel::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   // Get parent hub
   deviceId_ = wheelId_;
   RETURN_ON_MM_ERROR(PeripheralInitialize());

   // Get number of positions from controller
   int ret = GetNumberOfPositions();
   if (ret != DEVICE_OK || numPositions_ == 0)
      numPositions_ = 6;  // Default if query fails

   // Create properties
   CPropertyAction* pAct;

   // Number of positions
   pAct = new CPropertyAction(this, &Wheel::OnNumPositions);
   CreateProperty("Number of Positions", CDeviceUtils::ConvertToString((long)numPositions_),
                  MM::Integer, true, pAct);

   // State (position)
   pAct = new CPropertyAction(this, &Wheel::OnState);
   CreateProperty(MM::g_Keyword_State, "0", MM::Integer, false, pAct);
   SetPropertyLimits(MM::g_Keyword_State, 0, numPositions_ - 1);

   // Label
   pAct = new CPropertyAction(this, &CStateBase::OnLabel);
   CreateProperty(MM::g_Keyword_Label, "", MM::String, false, pAct);

   // Create labels for each position
   for (unsigned long i = 0; i < numPositions_; i++)
   {
      std::ostringstream label;
      label << "Position-" << (i + 1);
      SetPositionLabel(i, label.str().c_str());
   }

   initialized_ = true;
   return DEVICE_OK;
}

int Wheel::Shutdown()
{
   if (!initialized_)
      return DEVICE_OK;

   initialized_ = false;
   return DEVICE_OK;
}

bool Wheel::Busy()
{
   if (!initialized_)
      return false;

   // Query controller status for wheel movement
   std::string response;
   int ret = hub_->QueryCommand("$", response);
   if (ret != DEVICE_OK)
      return false;

   // Status byte: bits 4-6 indicate wheel movement
   if (response.length() >= 1)
   {
      long status = atol(response.c_str());
      // Check appropriate bit for this wheel (simplified - actual bit depends on wheel ID)
      return ((status & (0x10 << (wheelId_ - 1))) != 0);
   }

   return false;
}

//////////////////////////////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////////////////////////////

int Wheel::GetNumberOfPositions()
{
   std::ostringstream command;
   command << "FPW " << wheelId_;

   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

   // Parse numeric response
   long num;
   RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, num));

   numPositions_ = (unsigned long)num;
   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Action handlers
//////////////////////////////////////////////////////////////////////////////

int Wheel::OnState(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      // Query current position
      std::ostringstream command;
      command << "7," << wheelId_ << ",F";

      std::string response;
      RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

      // Parse numeric response (1-based position)
      long pos;
      RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, pos));

      // Convert to 0-based
      pProp->Set(pos - 1);
   }
   else if (eAct == MM::AfterSet)
   {
      long pos;
      pProp->Get(pos);

      // Convert to 1-based position
      std::ostringstream command;
      command << "7," << wheelId_ << "," << (pos + 1);

      std::string response;
      RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

      if (!hub_->IsSuccessResponse(response) && hub_->IsErrorResponse(response))
         return hub_->GetErrorCode(response);
   }
   return DEVICE_OK;
}

int Wheel::OnNumPositions(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set((long)numPositions_);
   }
   return DEVICE_OK;
}
