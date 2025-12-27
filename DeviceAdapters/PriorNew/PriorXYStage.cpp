///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorXYStage.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior XY stage implementation
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

#include "PriorXYStage.h"
#include <sstream>
#include <cstdlib>

CXYStage::CXYStage() :
   PriorPeripheralBase<CXYStageBase, CXYStage>(g_XYStageDeviceName),
   stepSizeXUm_(0.1),
   stepSizeYUm_(0.1),
   initialized_(false)
{
   InitializeDefaultErrorMessages();
   EnableDelay();

   // Description
   CreateProperty(MM::g_Keyword_Description, "Prior XY Stage", MM::String, true);
}

CXYStage::~CXYStage()
{
   Shutdown();
}

void CXYStage::GetName(char* pszName) const
{
   CDeviceUtils::CopyLimitedString(pszName, g_XYStageDeviceName);
}

int CXYStage::Initialize()
{
   if (initialized_)
      return DEVICE_OK;

   // Get parent hub
   RETURN_ON_MM_ERROR(PeripheralInitialize());

   // Get step size/resolution from controller
   double resX, resY;
   int ret = GetResolution(resX, resY);
   if (ret != DEVICE_OK)
      return ret;

   // Prior sometimes returns 0 as resolution, use hardcoded value
   if (resX <= 0.0 || resY <= 0.0)
   {
      resX = 0.1;
      resY = 0.1;
   }

   stepSizeXUm_ = resX;
   stepSizeYUm_ = resY;

   // Create properties
   CPropertyAction* pAct;

   // Step size X
   pAct = new CPropertyAction(this, &CXYStage::OnStepSizeX);
   CreateProperty("StepSizeX_um", CDeviceUtils::ConvertToString(stepSizeXUm_),
                  MM::Float, true, pAct);

   // Step size Y
   pAct = new CPropertyAction(this, &CXYStage::OnStepSizeY);
   CreateProperty("StepSizeY_um", CDeviceUtils::ConvertToString(stepSizeYUm_),
                  MM::Float, true, pAct);

   // Max Speed
   pAct = new CPropertyAction(this, &CXYStage::OnMaxSpeed);
   CreateProperty("MaxSpeed", "20", MM::Integer, false, pAct);
   SetPropertyLimits("MaxSpeed", 1, 100);

   // Acceleration
   pAct = new CPropertyAction(this, &CXYStage::OnAcceleration);
   CreateProperty("Acceleration", "20", MM::Integer, false, pAct);
   SetPropertyLimits("Acceleration", 1, 100);

   // SCurve (if supported)
   if (HasCommand("SCS"))
   {
      pAct = new CPropertyAction(this, &CXYStage::OnSCurve);
      CreateProperty("SCurve", "20", MM::Integer, false, pAct);
      SetPropertyLimits("SCurve", 1, 100);
   }

   initialized_ = true;
   return DEVICE_OK;
}

int CXYStage::Shutdown()
{
   if (!initialized_)
      return DEVICE_OK;

   initialized_ = false;
   return DEVICE_OK;
}

bool CXYStage::Busy()
{
   if (!initialized_)
      return false;

   // Query controller status
   std::string response;
   int ret = hub_->QueryCommand("$", response);
   if (ret != DEVICE_OK)
      return false;

   // Status byte: bit 0 = X moving, bit 1 = Y moving
   if (response.length() >= 1)
   {
      long status = atol(response.c_str());
      return ((status & 0x03) != 0);  // Check bits 0 and 1
   }

   return false;
}

//////////////////////////////////////////////////////////////////////////////
// XYStage API
//////////////////////////////////////////////////////////////////////////////

int CXYStage::SetPositionSteps(long x, long y)
{
   std::ostringstream command;
   command << "G," << x << "," << y;

   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int CXYStage::SetRelativePositionSteps(long x, long y)
{
   std::ostringstream command;
   command << "GR," << x << "," << y;

   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int CXYStage::GetPositionSteps(long& x, long& y)
{
   RETURN_ON_MM_ERROR(GetPositionStepsSingle('X', x));
   RETURN_ON_MM_ERROR(GetPositionStepsSingle('Y', y));
   return DEVICE_OK;
}

int CXYStage::Home()
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

int CXYStage::Stop()
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

int CXYStage::SetOrigin()
{
   // Set origin to current position
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("PS,0,0", response));

   if (hub_->IsSuccessResponse(response))
      return DEVICE_OK;

   if (hub_->IsErrorResponse(response))
      return hub_->GetErrorCode(response);

   return ERR_PRIOR_UNRECOGNIZED_ANSWER;
}

int CXYStage::GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax)
{
   // Prior doesn't report limits, return large values
   xMin = -100000.0;
   xMax = 100000.0;
   yMin = -100000.0;
   yMax = 100000.0;
   return DEVICE_OK;
}

int CXYStage::GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax)
{
   // Prior doesn't report limits, return large values
   xMin = -1000000;
   xMax = 1000000;
   yMin = -1000000;
   yMax = 1000000;
   return DEVICE_OK;
}

int CXYStage::IsXYStageSequenceable(bool& isSequenceable) const
{
   // Prior doesn't support hardware sequencing
   isSequenceable = false;
   return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////////////////////////////

int CXYStage::GetPositionStepsSingle(char axis, long& steps)
{
   std::string command = "P";
   command += axis;

   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand(command, response));

   // Parse numeric response
   RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, steps));

   return DEVICE_OK;
}

int CXYStage::GetResolution(double& resX, double& resY)
{
   // Query X resolution
   std::string response;
   RETURN_ON_MM_ERROR(hub_->QueryCommand("RES,X", response));
   RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, resX));

   // Query Y resolution
   RETURN_ON_MM_ERROR(hub_->QueryCommand("RES,Y", response));
   RETURN_ON_MM_ERROR(hub_->ParseNumericResponse(response, resY));

   return DEVICE_OK;
}

bool CXYStage::HasCommand(const std::string& command)
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

int CXYStage::OnStepSizeX(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(stepSizeXUm_);
   }
   return DEVICE_OK;
}

int CXYStage::OnStepSizeY(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(stepSizeYUm_);
   }
   return DEVICE_OK;
}

int CXYStage::OnMaxSpeed(MM::PropertyBase* pProp, MM::ActionType eAct)
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

int CXYStage::OnAcceleration(MM::PropertyBase* pProp, MM::ActionType eAct)
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

int CXYStage::OnSCurve(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      // Query SCS
      std::string response;
      int ret = hub_->QueryCommand("SCS", response);
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
      command << "SCS," << value;

      std::string response;
      RETURN_ON_MM_ERROR(hub_->QueryCommand(command.str(), response));

      if (!hub_->IsSuccessResponse(response) && hub_->IsErrorResponse(response))
         return hub_->GetErrorCode(response);
   }
   return DEVICE_OK;
}
