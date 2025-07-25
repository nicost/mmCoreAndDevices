///////////////////////////////////////////////////////////////////////////////
// FILE:          ASIClocked.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   ASI clocked device adapter (filter slider, turret)
//
// COPYRIGHT:     Applied Scientific Instrumentation, Eugene OR
//
// LICENSE:       This file is distributed under the BSD license.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//
// AUTHOR:        Jon Daniels (jon@asiimaging.com) 09/2013
//
// BASED ON:      ASIStage.h and others
//

#ifndef ASICLOCKED_H
#define ASICLOCKED_H

#include "ASIPeripheralBase.h"
#include "MMDevice.h"
#include "DeviceBase.h"

class CClocked : public ASIPeripheralBase<CStateDeviceBase, CClocked>
{
public:
   CClocked(const char* name);
   ~CClocked() { }

   // Generic device API
   int Initialize();
   bool Busy();

   // State device API
   unsigned long GetNumberOfPositions() const { return numPositions_; }

   // action interface
   int OnState                (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnLabel                (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSaveCardSettings     (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnRefreshProperties    (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnJoystickSelect       (MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   unsigned int numPositions_;
   unsigned int curPosition_;

   int OnSaveJoystickSettings();

protected:
   std::string axisLetter_;
};

class CFSlider : public CClocked
{
public:
   CFSlider(const char* name);

   int Initialize();
};

class CTurret : public CClocked
{
public:
   CTurret(const char* name);

   int Initialize();
};

class CPortSwitch : public CClocked
{
public:
   CPortSwitch(const char* name);

   int Initialize();
};

#endif // ASICLOCKED_H
