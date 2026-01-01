///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorProScanHub.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior ProScan controller hub implementation
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

#ifndef _PRIORPROSCANHUB_H_
#define _PRIORPROSCANHUB_H_

#include "PriorHub.h"

//////////////////////////////////////////////////////////////////////////////
// CPriorProScanHub - ProScan controller-specific hub
//////////////////////////////////////////////////////////////////////////////

class CPriorProScanHub : public HubBase<CPriorProScanHub>, public PriorHub
{
public:
   CPriorProScanHub();
   ~CPriorProScanHub();

   // MM::Device API
   int Initialize();
   int Shutdown();
   void GetName(char* pName) const;
   bool Busy();

   // MM::Hub API
   bool SupportsDeviceDetection();
   MM::DeviceDetectionStatus DetectDevice();
   int DetectInstalledDevices();

   // Property handlers
   int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);

protected:
   // Implement PriorHub virtual methods for serial communication
   virtual int PurgeComPortImpl();
   virtual int SendSerialCommandImpl(const std::string& command);
   virtual int GetSerialAnswerImpl(std::string& answer);

private:
   // Device detection helpers
   int DetectXYStage();
   int DetectZStage();
   int DetectNanoZStage();
   int DetectShutters();      // Detect shutters 1-3
   int DetectWheels();        // Detect filter wheels 1-3
   int DetectLumen();
   int DetectTTLShutters();   // Detect TTL 0-3

   // Controller information
   int GetControllerInfo();
   int SetStandardMode();     // Set COMP 0 mode
};

#endif // _PRIORPROSCANHUB_H_
