///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorTTLShutter.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Prior TTL shutter implementation (TTL 0-3)
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

#ifndef _PRIORTTLSHUTTER_H_
#define _PRIORTTLSHUTTER_H_

#include "PriorPeripheralBase.h"

class CTTLShutter : public PriorPeripheralBase<CShutterBase, CTTLShutter>
{
public:
   CTTLShutter(const char* name, int id);
   ~CTTLShutter();

   // MMDevice API
   int Initialize();
   int Shutdown();
   void GetName(char* pszName) const;
   bool Busy();

   // Shutter API
   int SetOpen(bool open = true);
   int GetOpen(bool& open);
   int Fire(double deltaT);

private:
   std::string name_;
   int ttlId_;  // 0, 1, 2, or 3
   bool initialized_;
};

#endif // _PRIORTTLSHUTTER_H_
