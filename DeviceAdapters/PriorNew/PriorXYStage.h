///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorXYStage.h
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

#ifndef _PRIORXYSTAGE_H_
#define _PRIORXYSTAGE_H_

#include "PriorPeripheralBase.h"

class CXYStage : public PriorPeripheralBase<CXYStageBase<CXYStage>>
{
public:
   CXYStage();
   ~CXYStage();

   // MMDevice API
   int Initialize();
   int Shutdown();
   void GetName(char* pszName) const;
   bool Busy();

   // XYStage API
   int SetPositionSteps(long x, long y);
   int GetPositionSteps(long& x, long& y);
   int SetRelativePositionSteps(long x, long y);
   int Home();
   int Stop();
   int SetOrigin();
   int GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax);
   int GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax);
   double GetStepSizeXUm() { return stepSizeXUm_; }
   double GetStepSizeYUm() { return stepSizeYUm_; }
   int IsXYStageSequenceable(bool& isSequenceable) const;

   // Action interface
   int OnStepSizeX(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnStepSizeY(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMaxSpeed(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAcceleration(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSCurve(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   int GetPositionStepsSingle(char axis, long& steps);
   int GetResolution(double& resX, double& resY);
   bool HasCommand(const std::string& command);

   double stepSizeXUm_;
   double stepSizeYUm_;
   bool initialized_;
};

#endif // _PRIORXYSTAGE_H_
