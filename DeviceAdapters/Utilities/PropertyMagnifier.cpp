
///////////////////////////////////////////////////////////////////////////////
// FILE:          PropertyMagnifier.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Various 'Meta-Devices' that add to or combine functionality of 
//                physcial devices.
//
// AUTHOR:        Nico Stuurman, nico@cmp.ucsf.edu, 11/07/2008
//                Nico Stuurman, nstuurman@altoslabs.com, 4/22/2022
//                Nico Stuurman, nico.stuurman@ucsf.edu, 10/22/2024
// COPYRIGHT:     University of California, San Francisco, 2008
//                2015-2016, Open Imaging, Inc.
//                Altos Labs, 2022
//                University of California, San Francisco, 2024
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
//

#ifdef _WIN32
// Prevent windows.h from defining min and max macros,
// which clash with std::min and std::max.
#define NOMINMAX
#endif


#include "Utilities.h"

extern const char* g_DeviceNamePropertyMagnifier;
extern const char* g_Device;
extern const char* g_Property;
extern const char* g_Magnification;
extern const char* g_SetMagnification;
extern const char* g_NrMagnifications;



PropertyMagnifier::PropertyMagnifier() :
   propertyDevice_(""),
   propertyName_(""),
   nrMagnifications_(2),
   magnifications_(10),
   initialized_(false)
{
   // Name                                                                   
   CreateProperty(MM::g_Keyword_Name, g_DeviceNamePropertyMagnifier, MM::String, true);

   // Description                                                            
   CreateProperty(MM::g_Keyword_Description, "Set magnification associated with Property values", MM::String, true);

   CPropertyAction* pAct = new CPropertyAction(this, &PropertyMagnifier::OnNrMagnifications);
   CreateIntegerProperty(g_NrMagnifications, nrMagnifications_, false, pAct, true);
   SetPropertyLimits(g_NrMagnifications, 1, 10);

   magnifications_.clear();

}

PropertyMagnifier::~PropertyMagnifier()
{
   if (initialized_)
      Shutdown();
}

int PropertyMagnifier::Shutdown() 
{
   initialized_ = false; 
   return DEVICE_OK;
}

int PropertyMagnifier::Initialize()
{

   CPropertyAction* pAct = new CPropertyAction(this, &PropertyMagnifier::OnDevice);
   int ret = CreateStringProperty(g_Device, "", false, pAct);
   if (ret != DEVICE_OK)
      return ret;

   pAct = new CPropertyAction(this, &PropertyMagnifier::OnProperty);
   ret = CreateStringProperty(g_Property, "", false, pAct);
   if (ret != DEVICE_OK)
      return ret;

   pAct = new CPropertyAction(this, &PropertyMagnifier::OnMagnification);
   ret = CreateStringProperty(g_Magnification, "", true, pAct);
   if (ret != DEVICE_OK)
      return ret;

   for (int i = 0; i < nrMagnifications_; i++)
   {
      CPropertyActionEx* pActEx = new CPropertyActionEx(this, &PropertyMagnifier::OnSetMagnification, i);
      std::ostringstream os;
      os << g_SetMagnification << '-' << i;
      std::string propName = os.str();
      ret = CreateFloatProperty(propName.c_str(), 1.0, false, pActEx);
      if (ret != DEVICE_OK)
         return ret;
   }

   return DEVICE_OK;
}

  
void PropertyMagnifier::GetName(char* pszName) const
{
   CDeviceUtils::CopyLimitedString(pszName, g_DeviceNamePropertyMagnifier);
}


bool PropertyMagnifier::Busy()
{
   if (propertyDevice_ != "")
   {
      MM::Device* device = (MM::Device*)GetDevice(propertyDevice_.c_str());
      if (device != 0)
         return device->Busy();
   }
   return false;
}


int PropertyMagnifier::OnNrMagnifications(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(nrMagnifications_);
   }
   else if (eAct == MM::AfterSet)
   {
      pProp->Get(nrMagnifications_);
   }
   return DEVICE_OK;
}


int PropertyMagnifier::OnDevice(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(propertyDevice_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      std::string deviceName;
      pProp->Get(deviceName);
      MM::Device* device = (MM::Device*)GetDevice(deviceName.c_str());
      if (device != 0 && propertyDevice_ != deviceName)
      {
         propertyDevice_ = deviceName;
         // new device, so our magnification values are no longer valid.
         magnifications_.clear();
         ClearAllowedValues(g_Property);
         int numProps = device->GetNumberOfProperties();
         for (int i = 0; i < numProps; i++) {
            char name[MM::MaxStrLength];
            device->GetPropertyName(i, name);
            AddAllowedValue(g_Property, name);
         }
      }
   }
   return DEVICE_OK;
}



int PropertyMagnifier::OnProperty(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(propertyName_.c_str());
   }
   else if (eAct == MM::AfterSet)
   {
      std::string propertyName;
      pProp->Get(propertyName);
      // We could check if the property exists, but then we always will need to first set the device,
      // then the property, which makes things more difficult.
      if (propertyName_ != propertyName) 
      {
         propertyName_ = propertyName;
         // new property, so our magnification values are no longer valid.
         magnifications_.clear();
      }
   }
   return DEVICE_OK;
}



int PropertyMagnifier::OnMagnification(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(GetMagnification());
   }

   return DEVICE_OK;
}


int PropertyMagnifier::OnSetMagnification(MM::PropertyBase* pProp, MM::ActionType eAct, long i)
{
   if (eAct == MM::BeforeGet)
   {
      if (i < magnifications_.size())
         pProp->Set(magnifications_[i]);
      else
         pProp->Set("");
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(magnifications_[i]);
   }
   return DEVICE_OK;
}


double PropertyMagnifier::GetMagnification()
{
   MM::Device* device = (MM::Device*)GetDevice(propertyDevice_.c_str());
   if (device != 0)
   {
      if (device->HasProperty(propertyName_.c_str())) {
         char buf[MM::MaxStrLength];
         int ret = device->GetProperty(propertyName_.c_str(), buf);
         if (ret != DEVICE_OK)
            return ret;
         int val = std::atol(buf);
         if (val < magnifications_.size())
            return magnifications_[val];
      }
   }
   return 1.0;
}


