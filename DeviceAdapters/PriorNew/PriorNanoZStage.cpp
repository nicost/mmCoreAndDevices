///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorNanoZStage.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior Nano Z stage implementation
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

#include "PriorNanoZStage.h"
#include <sstream>
#include <cstdlib>

NanoZStage::NanoZStage() :
   PriorPeripheralBase<CStageBase<NanoZStage>>(g_NanoZStageDeviceName),
   stepSizeUm_(0.001),  // Nano stage has finer resolution
   initialized_(false)
{
   InitializeDefaultErrorMessages();
   EnableDelay();

   // Description
   CreateProperty(MM::g_Keyword_Description, "Prior Nano Z Stage", MM::String, true);
}

NanoZStage::~NanoZStage()
{
   Shutdown();
}

void NanoZStage::GetName(char* pszName) const
{
   CDeviceUtils::CopyLimitedString(pszName, g_NanoZStageDeviceName);
}

int NanoZStage::Initialize()
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
      res = 0.001;

   stepSizeUm_ = res;

   // Create properties
   CPropertyAction* pAct;

   // Step size
   pAct = new CPropertyAction(this, &NanoZStage::OnStepSize);
   CreateProperty("StepSize_um", CDeviceUtils::ConvertToString(stepSizeUm_),
                  MM::Float, true, pAct);

   initialized_ = true;
   return DEVICE_OK;
}

int NanoZStage::Shutdown()
{
   if (!initialized_)
      return DEVICE_OK;

   initialized_ = false;
   return DEVICE_OK;
}

bool NanoZStage::Busy()
{
   if (!initialized_)
      return false;

   // Query controller status
   std::string response;
   int ret = hub_->QueryCommand("$", response);
   if (ret != DEVICE_OK)
      return false;

   // Status byte: bit 3 = Nano Z moving
   if (response.length() >= 1)
   {
      long status = atol(response.c_str());
      return ((status & 0x08) != 0);  // Check bit 3
   }

   return false;
}

//////////////////////////////////////////////////////////////////////////////
// Stage API
//////////////////////////////////////////////////////////////////////////////

int NanoZStage::SetPositionUm(double pos)
{
   long steps = (long)(pos / stepSizeUm_ + 0.5);
   return SetPositionSteps(steps);
}

int NanoZStage::GetPositionUm(double& pos)
{
   long steps;
   RETURN_ON_MM_ERROR(GetPositionSteps(steps));
   pos = steps * stepSizeUm_;
   return DEVICE_OK;
}

int NanoZStage::SetPositionSteps(long steps)
{
   std::ostringstream command;
   command << "VZ," << steps;

   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int NanoZStage::GetPositionSteps(long& steps)
{
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("VZ", response));

   // Parse numeric response
   RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, steps));

   return DEVICE_OK;
}

int NanoZStage::SetOrigin()
{
   // Set origin to current position
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("VZ,0", response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int NanoZStage::GetLimits(double& lower, double& upper)
{
   // Prior doesn't report limits, return typical nano stage range
   lower = -100.0;  // Nano stages typically have smaller range
   upper = 100.0;
   return DEVICE_OK;
}

int NanoZStage::Move(double /* velocity */)
{
   // Prior doesn't support continuous movement with velocity
   return DEVICE_UNSUPPORTED_COMMAND;
}

int NanoZStage::Stop()
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

int NanoZStage::Home()
{
   // Nano Z stage may not support homing
   return DEVICE_UNSUPPORTED_COMMAND;
}

int NanoZStage::IsStageSequenceable(bool& isSequenceable) const
{
   // Prior doesn't support hardware sequencing
   isSequenceable = false;
   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////////////////////////////

int NanoZStage::GetResolution(double& res)
{
   // Query Nano Z resolution
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("RES,F", response));
   RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, res));

   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Action handlers
//////////////////////////////////////////////////////////////////////////////

int NanoZStage::OnStepSize(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(stepSizeUm_);
   }
   return DEVICE_OK;
}
