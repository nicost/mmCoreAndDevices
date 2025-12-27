///////////////////////////////////////////////////////////////////////////////
// FILE:          PriorBase.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Base template class for Prior devices
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

#ifndef _PRIORBASE_H_
#define _PRIORBASE_H_

#include "PriorNew.h"

//////////////////////////////////////////////////////////////////////////////
// PriorBase - Base template class for all Prior devices
//////////////////////////////////////////////////////////////////////////////

template <template <typename> class TDeviceBase, class UConcreteDevice>
class PriorBase : public TDeviceBase<UConcreteDevice>
{
protected:
   PriorBase() :
      initialized_(false),
      firmwareVersion_(""),
      firmwareDate_("")
   {
      InitializePriorErrorMessages();
   }

   virtual ~PriorBase() {}

   void InitializePriorErrorMessages()
   {
      // Communication errors
      this->SetErrorText(ERR_PRIOR_UNRECOGNIZED_ANSWER,
         "Unrecognized response from Prior controller");
      this->SetErrorText(ERR_PRIOR_INVALID_RESPONSE,
         "Invalid response format from controller");
      this->SetErrorText(ERR_PRIOR_TIMEOUT,
         "Communication timeout with Prior controller");
      this->SetErrorText(ERR_PRIOR_COMMAND_FAILED,
         "Command execution failed");
      this->SetErrorText(ERR_PRIOR_SERIAL_COMMAND_FAILED,
         "Serial command transmission failed");

      // Configuration errors
      this->SetErrorText(ERR_PRIOR_PORT_CHANGE_FORBIDDEN,
         "Cannot change serial port after initialization");
      this->SetErrorText(ERR_PRIOR_INVALID_DEVICE_ID,
         "Invalid device ID specified");
      this->SetErrorText(ERR_PRIOR_HUB_NOT_FOUND,
         "PriorNew hub device not found - create hub first");
      this->SetErrorText(ERR_PRIOR_INVALID_PROPERTY,
         "Invalid property value");

      // Device-specific errors
      this->SetErrorText(ERR_PRIOR_POSITION_OUT_OF_RANGE,
         "Requested position is out of range");
      this->SetErrorText(ERR_PRIOR_MOVEMENT_FAILED,
         "Stage movement failed");
      this->SetErrorText(ERR_PRIOR_DEVICE_NOT_PRESENT,
         "Device not detected on controller");
      this->SetErrorText(ERR_PRIOR_WHEEL_POSITION_INVALID,
         "Invalid filter wheel position");
      this->SetErrorText(ERR_PRIOR_SHUTTER_FAILED,
         "Shutter operation failed");
   }

protected:
   bool initialized_;
   std::string firmwareVersion_;
   std::string firmwareDate_;
};

#endif // _PRIORBASE_H_
