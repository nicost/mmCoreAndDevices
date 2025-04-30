///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorPureFocus.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Device adapter for Prior PureFocus Autofocus System
//                
//                
// AUTHOR:        Your Name
//
// COPYRIGHT:     Your Institution, 2025
//
// LICENSE:       This file is distributed under the BSD license.
//

#ifndef _PRIORPUREFOCUS_H_
#define _PRIORPUREFOCUS_H_

#include "DeviceBase.h"
#include "MMDevice.h"
#include <string>

//////////////////////////////////////////////////////////////////////////////
// Error codes
//
#define ERR_PORT_CHANGE_FORBIDDEN 101
#define ERR_DEVICE_NOT_FOUND      102
#define ERR_UNEXPECTED_RESPONSE   103
#define ERR_RESPONSE_TIMEOUT      104
#define ERR_COMMUNICATION         105
#define ERR_AUTOFOCUS_LOCKED      106

class CPureFocus : public CAutoFocusBase<CPureFocus>
{
public:
   CPureFocus();
   ~CPureFocus();

   // Device API
   bool Busy();
   void GetName(char* pszName) const;
   int Initialize();
   int Shutdown();

   // AutoFocus API
   virtual int SetContinuousFocusing(bool state);
   virtual int GetContinuousFocusing(bool& state);
   virtual bool IsContinuousFocusLocked();
   virtual int FullFocus();
   virtual int IncrementalFocus();
   virtual int GetLastFocusScore(double& score);
   virtual int GetCurrentFocusScore(double& score);
   virtual int GetOffset(double& offset);
   virtual int SetOffset(double offset);
   virtual int GetFocusScore(double& score);
   virtual int ResetDevice(void);

   // Property Action Handlers
   int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnStatus(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnLock(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnFocusScore(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnOffset(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   bool initialized_;
   std::string name_;
   std::string port_;
   bool locked_;
   bool busy_;
   long answerTimeoutMs_;
   int statusMode_;

   // Communication methods
   int SendCommand(const std::string& cmd);
   int GetResponse(std::string& resp);
   std::string RemoveLineEndings(std::string input);
};

#endif // _PRIORPUREFOCUS_H_#pragma once
