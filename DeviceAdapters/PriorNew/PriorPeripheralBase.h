///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorPeripheralBase.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Base template class for Prior peripheral devices
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

#ifndef _PRIORPERIPHERALBASE_H_
#define _PRIORPERIPHERALBASE_H_

#include "PriorBase.h"
#include "PriorHub.h"

//////////////////////////////////////////////////////////////////////////////
// PriorPeripheralBase - Base template for peripheral devices that communicate
//                       through a PriorHub
//////////////////////////////////////////////////////////////////////////////

template <class TDeviceBase>
class PriorPeripheralBase : public PriorBase<TDeviceBase>
{
protected:
   PriorPeripheralBase(const char* deviceName, int deviceId = 0) :
      PriorBase<TDeviceBase>(),
      hub_(nullptr),
      deviceId_(deviceId)
   {
      this->CreateProperty(MM::g_Keyword_Name, deviceName, MM::String, true);
   }

   virtual ~PriorPeripheralBase()
   {
      if (hub_ != nullptr && this->initialized_)
      {
         char label[MM::MaxStrLength];
         this->GetLabel(label);
         hub_->UnRegisterPeripheral(std::string(label));
      }
   }

   // Initialize peripheral - get hub and register with it
   // Call this from concrete device's Initialize() method
   int PeripheralInitialize()
   {
      // Get the parent hub
      MM::Hub* genericHub = this->GetParentHub();
      if (genericHub == nullptr)
         return ERR_PRIOR_HUB_NOT_FOUND;

      // Cast to PriorHub
      hub_ = dynamic_cast<PriorHub*>(genericHub);
      if (hub_ == nullptr)
         return ERR_PRIOR_HUB_NOT_FOUND;

      // Register this device with the hub
      char deviceLabel[MM::MaxStrLength];
      this->GetLabel(deviceLabel);
      std::string label(deviceLabel);
      hub_->RegisterPeripheral(label, deviceId_);

      return DEVICE_OK;
   }

protected:
   PriorHub* hub_;        // Pointer to parent hub for communication
   int deviceId_;         // Device ID (for shutters: 1-3, wheels: 1-3, TTL: 0-3)
};

#endif // _PRIORPERIPHERALBASE_H_
