///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorNanoZStage.h
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

#ifndef _PRIORNANOZSTAGE_H_
#define _PRIORNANOZSTAGE_H_

#include "PriorPeripheralBase.h"

class NanoZStage : public PriorPeripheralBase<CStageBase<NanoZStage>>
{
public:
   NanoZStage();
   ~NanoZStage();

   // MMDevice API
   int Initialize();
   int Shutdown();
   void GetName(char* pszName) const;
   bool Busy();

   // Stage API
   int SetPositionUm(double pos);
   int GetPositionUm(double& pos);
   int SetPositionSteps(long steps);
   int GetPositionSteps(long& steps);
   int SetOrigin();
   int GetLimits(double& lower, double& upper);
   int Move(double velocity);
   int Stop();
   int Home();
   int IsStageSequenceable(bool& isSequenceable) const;
   bool IsContinuousFocusDrive() const { return false; }

   // Action interface
   int OnStepSize(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   int GetResolution(double& res);

   double stepSizeUm_;
   bool initialized_;
};

#endif // _PRIORNANOZSTAGE_H_
