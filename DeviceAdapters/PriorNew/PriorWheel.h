///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorWheel.h
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

#ifndef _PRIORWHEEL_H_
#define _PRIORWHEEL_H_

#include "PriorPeripheralBase.h"

class CWheel : public PriorPeripheralBase<CStateDeviceBase<CWheel>>
{
public:
   CWheel(const char* name, int id);
   ~CWheel();

   // MMDevice API
   int Initialize();
   int Shutdown();
   void GetName(char* pszName) const;
   bool Busy();

   // State API
   unsigned long GetNumberOfPositions() const { return numPositions_; }

   // Action interface
   int OnState(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnNumPositions(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   int GetNumberOfPositions();

   std::string name_;
   int wheelId_;  // 1, 2, or 3
   unsigned long numPositions_;
   bool initialized_;
};

#endif // _PRIORWHEEL_H_
