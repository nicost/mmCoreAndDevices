///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorZStage.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior Z stage implementation
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

#include "PriorZStage.h"
#include <sstream>
#include <cstdlib>

ZStage::ZStage() :
   PriorPeripheralBase<CStageBase<ZStage>>(g_ZStageDeviceName),
   stepSizeUm_(0.1),
   initialized_(false)
{
   InitializeDefaultErrorMessages();
   EnableDelay();

   // Description
   CreateProperty(MM::g_Keyword_Description, "Prior Z Stage", MM::String, true);
}

ZStage::~ZStage()
{
   Shutdown();
}

void ZStage::GetName(char* pszName) const
{
   CDeviceUtils::CopyLimitedString(pszName, g_ZStageDeviceName);
}

int ZStage::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   // Get parent hub
   RETURN_ON_MM_ERROR(PeripheralInitialize());

   // Get step size/resolution from controller
   double res;
   int ret = GetResolution(res);
   if (ret != DEVICE_OK)
      return ret;

   // Prior sometimes returns 0 as resolution, use hardcoded value
   if (res <= 0.0)
      res = 0.1;

   stepSizeUm_ = res;

   // Create properties
   CPropertyAction* pAct;

   // Step size
   pAct = new CPropertyAction(this, &ZStage::OnStepSize);
   CreateProperty("StepSize_um", CDeviceUtils::ConvertToString(stepSizeUm_),
                  MM::Float, true, pAct);

   // Max Speed
   pAct = new CPropertyAction(this, &ZStage::OnMaxSpeed);
   CreateProperty("MaxSpeed", "20", MM::Integer, false, pAct);
   SetPropertyLimits("MaxSpeed", 1, 100);

   // Acceleration
   pAct = new CPropertyAction(this, &ZStage::OnAcceleration);
   CreateProperty("Acceleration", "20", MM::Integer, false, pAct);
   SetPropertyLimits("Acceleration", 1, 100);

   initialized_ = true;
   return DEVICE_OK;
}

int ZStage::Shutdown()
{
   if (!initialized_)
      return DEVICE_OK;

   initialized_ = false;
   return DEVICE_OK;
}

bool ZStage::Busy()
{
   if (!initialized_)
      return false;

   // Query controller status
   std::string response;
   int ret = hub_->QueryCommand("$", response);
   if (ret != DEVICE_OK)
      return false;

   // Status byte: bit 2 = Z moving
   if (response.length() >= 1)
   {
      long status = atol(response.c_str());
      return ((status & 0x04) != 0);  // Check bit 2
   }

   return false;
}

//////////////////////////////////////////////////////////////////////////////
// Stage API
//////////////////////////////////////////////////////////////////////////////

int ZStage::SetPositionUm(double pos)
{
   long steps = (long)(pos / stepSizeUm_ + 0.5);
   return SetPositionSteps(steps);
}

int ZStage::GetPositionUm(double& pos)
{
   long steps;
   RETURN_ON_MM_ERROR(GetPositionSteps(steps));
   pos = steps * stepSizeUm_;
   return DEVICE_OK;
}

int ZStage::SetPositionSteps(long steps)
{
   // Get current position
   long current;
   RETURN_ON_MM_ERROR(GetPositionSteps(current));

   // Calculate relative move
   long delta = steps - current;

   std::ostringstream command;
   if (delta > 0)
      command << "U," << delta;
   else if (delta < 0)
      command << "D," << (-delta);
   else
      return DEVICE_OK;  // Already at position

   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int ZStage::GetPositionSteps(long& steps)
{
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("PZ", response));

   // Parse numeric response
   RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, steps));

   return DEVICE_OK;
}

int ZStage::SetOrigin()
{
   // Set origin to current position
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("PZ,0", response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int ZStage::GetLimits(double& lower, double& upper)
{
   // Prior doesn't report limits, return large values
   lower = -100000.0;
   upper = 100000.0;
   return DEVICE_OK;
}

int ZStage::Move(double /* velocity */)
{
   // Prior doesn't support continuous movement with velocity
   return DEVICE_UNSUPPORTED_COMMAND;
}

int ZStage::Stop()
{
   // Send halt command (K)
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("K", response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int ZStage::Home()
{
   // Send home command (SIS)
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("SIS", response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int ZStage::IsStageSequenceable(bool& isSequenceable) const
{
   // Prior doesn't support hardware sequencing
   isSequenceable = false;
   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////////////////////////////

int ZStage::GetResolution(double& res)
{
   // Query Z resolution
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("RES,Z", response));
   RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, res));

   return DEVICE_OK;
}

bool ZStage::HasCommand(const std::string& command)
{
   // Try the command and see if we get an error
   std::string response;
   int ret = hub_->QueryCommand(command, response);

   // If we get an error response, command not supported
   if (ret != DEVICE_OK || hub_->IsErrorResponse(response))
      return false;

   return true;
}

//////////////////////////////////////////////////////////////////////////////
// Action handlers
//////////////////////////////////////////////////////////////////////////////

int ZStage::OnStepSize(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(stepSizeUm_);
   }
   return DEVICE_OK;
}

int ZStage::OnMaxSpeed(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      // Query SMS
      std::string response;
      int ret = hub_->QueryCommand("SMS", response);
      if (ret == DEVICE_OK)
      {
         long value;
         if (hub_->ParseNumericResponse(response, value) == DEVICE_OK)
            pProp->Set(value);
      }
   }
   else if (eAct == MM::AfterSet)
   {
      long value;
      pProp->Get(value);

      // Clamp to valid range
      if (value < 1) value = 1;
      if (value > 100) value = 100;

      std::ostringstream command;
      command << "SMS," << value;

      std::string response;
      RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

      if (!hub_->IsSuccessResponse(response) && hub_->IsErrorResponse(response))
         return hub_->GetErrorCode(response);
   }
   return DEVICE_OK;
}

int ZStage::OnAcceleration(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      // Query SAS
      std::string response;
      int ret = hub_->QueryCommand("SAS", response);
      if (ret == DEVICE_OK)
      {
         long value;
         if (hub_->ParseNumericResponse(response, value) == DEVICE_OK)
            pProp->Set(value);
      }
   }
   else if (eAct == MM::AfterSet)
   {
      long value;
      pProp->Get(value);

      // Clamp to valid range
      if (value < 1) value = 1;
      if (value > 100) value = 100;

      std::ostringstream command;
      command << "SAS," << value;

      std::string response;
      RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

      if (!hub_->IsSuccessResponse(response) && hub_->IsErrorResponse(response))
         return hub_->GetErrorCode(response);
   }
   return DEVICE_OK;
}
