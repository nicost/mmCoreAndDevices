#include "squid.h"
#include <cstdint>


const char* g_ShutterName = "LEDs";
const char* g_OnOff = "OnOff";
const char* g_Pattern = "Pattern";
const char* g_Intensity = "Intensity";
const char* g_Red = "Red";
const char* g_Green = "Green";
const char* g_Blue = "Blue";
const char* g_HasLasers = "Has Lasers";

extern const int CMD_TURN_ON_ILLUMINATION;
extern const int CMD_TURN_OFF_ILLUMINATION;
extern const int CMD_SET_ILLUMINATION_LED_MATRIX;

extern const int ILLUMINATION_SOURCE_LED_ARRAY_FULL;
extern const int ILLUMINATION_SOURCE_LED_ARRAY_LEFT_HALF;
extern const int ILLUMINATION_SOURCE_LED_ARRAY_RIGHT_HALF;
extern const int ILLUMINATION_SOURCE_LED_ARRAY_LEFTB_RIGHTR;
extern const int ILLUMINATION_SOURCE_LED_ARRAY_LOW_NA;
extern const int ILLUMINATION_SOURCE_LED_ARRAY_LEFT_DOT;
extern const int ILLUMINATION_SOURCE_LED_ARRAY_RIGHT_DOT;

const std::string ILLUMINATIONS[7] = {
   "LED-Full",
   "LED-Left_Half",
   "LED-Right_Half",
   "LED-Left-Blue_Right-Red",
   "LED-Low_NA",
   "LED-Left_Dot",
   "LED-Right_Dot"
};

// laser IDs start at 11
const std::string LASERS[5] = {
      "405nm",
      "488nm",
      "638nm",
      "561nm",
      "730nm"
};

const int illumination_source = 1; // presumably this is the lED, with lasers something else


SquidShutter::SquidShutter() :
   hub_(0),
   initialized_(false),
   hasLasers_(false),
   name_(g_ShutterName),
   pattern_(0),
   changedTime_(), 
   intensity_ (1),
   red_(255),
   green_(255),
   blue_(255),
   isOpen_(false),
   cmdNr_(0)
{
   InitializeDefaultErrorMessages();
   EnableDelay();

   SetErrorText(ERR_NO_PORT_SET, "Hub Device not found.  The Squid Hub device is needed to create this device");

   // Name
   int ret = CreateProperty(MM::g_Keyword_Name, g_ShutterName, MM::String, true);
   assert(DEVICE_OK == ret);

   // Description
   ret = CreateProperty(MM::g_Keyword_Description, "Squid Light Control", MM::String, true);
   assert(DEVICE_OK == ret);

   CPropertyAction* pAct = new CPropertyAction(this, &SquidShutter::OnHasLasers);
   ret = CreateStringProperty(g_HasLasers, g_No, false, pAct, true);
   AddAllowedValue(g_HasLasers, g_No);
   AddAllowedValue(g_HasLasers, g_Yes);

   // parent ID display
   CreateHubIDProperty();
}


SquidShutter::~SquidShutter()
{
   if (initialized_)
   {
      Shutdown();
   }
}


int SquidShutter::Shutdown()
{
   if (initialized_) {
      initialized_ = false;
   }
   return DEVICE_OK;
}


void SquidShutter::GetName(char* pszName) const
{
   CDeviceUtils::CopyLimitedString(pszName, g_ShutterName);
}


int SquidShutter::Initialize()
{
   hub_ = static_cast<SquidHub*>(GetParentHub());
   if (!hub_ || !hub_->IsPortAvailable()) {
      return ERR_NO_PORT_SET;
   }
   char hubLabel[MM::MaxStrLength];
   hub_->GetLabel(hubLabel);

   // OnOff
  // ------
   CPropertyAction* pAct = new CPropertyAction(this, &SquidShutter::OnOnOff);
   int ret = CreateProperty("OnOff", "0", MM::Integer, false, pAct);
   if (ret != DEVICE_OK)
      return ret;

   // set shutter into the off state
   //WriteToPort(0);

   std::vector<std::string> vals;
   vals.push_back("0");
   vals.push_back("1");
   ret = SetAllowedValues("OnOff", vals);
   if (ret != DEVICE_OK)
      return ret;

   // Pattern
   // ------
   pAct = new CPropertyAction(this, &SquidShutter::OnPattern);
   ret = CreateProperty(g_Pattern, ILLUMINATIONS[0].c_str(), MM::String, false, pAct);
   if (ret != DEVICE_OK)
      return ret;

   for (uint8_t i = 0; i < 7; i++)
   {
      AddAllowedValue(g_Pattern, ILLUMINATIONS[i].c_str());
   }
   if (hasLasers_) {
      for (uint8_t i = 0; i < 5; i++)
      {
         AddAllowedValue(g_Pattern, LASERS[i].c_str());
      }

   }

   // Intensity
   // ------
   pAct = new CPropertyAction(this, &SquidShutter::OnIntensity);
   ret = CreateProperty(g_Intensity, "1", MM::Integer, false, pAct);
   if (ret != DEVICE_OK)
      return ret;
   SetPropertyLimits(g_Intensity, 0, 255);

   // Red
   // ------
   pAct = new CPropertyAction(this, &SquidShutter::OnRed);
   ret = CreateProperty(g_Red, "255", MM::Integer, false, pAct);
   if (ret != DEVICE_OK)
      return ret;
   SetPropertyLimits(g_Red, 0, 255);

   // Green
   // ------
   pAct = new CPropertyAction(this, &SquidShutter::OnGreen);
   ret = CreateProperty(g_Green, "255", MM::Integer, false, pAct);
   if (ret != DEVICE_OK)
      return ret;
   SetPropertyLimits(g_Green, 0, 255);

   // Blue
   // ------
   pAct = new CPropertyAction(this, &SquidShutter::OnBlue);
   ret = CreateProperty(g_Blue, "255", MM::Integer, false, pAct);
   if (ret != DEVICE_OK)
      return ret;
   SetPropertyLimits(g_Blue, 0, 255);

   SetOpen(isOpen_);  // we can not read the state from the device, at least get it in sync with us

   //ret = UpdateStatus();
   //if (ret != DEVICE_OK)
   //   return ret;

   changedTime_ = GetCurrentMMTime();
   initialized_ = true;

   return DEVICE_OK;
}


// TODO: figure out how to get a real Busy signal
bool SquidShutter::Busy()
{
   return false;
   //return hub_->IsCommandPending(cmdNr_);
}



int SquidShutter::SetOpen(bool open)
{
   std::ostringstream os;
   os << "Request " << open;
   LogMessage(os.str().c_str(), true);

   if (open)
      return SetProperty("OnOff", "1");
   else
      return SetProperty("OnOff", "0");
}

int SquidShutter::GetOpen(bool& open)
{
   char buf[MM::MaxStrLength];
   int ret = GetProperty("OnOff", buf);
   if (ret != DEVICE_OK)
      return ret;
   long pos = atol(buf);
   pos > 0 ? open = true : open = false;

   return DEVICE_OK;
}

int SquidShutter::Fire(double /*deltaT*/)
{
   return DEVICE_UNSUPPORTED_COMMAND;
}


// action interface

int SquidShutter::OnOnOff(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      // use cached state, there is no way to query
      pProp->Set(isOpen_ ? 1l : 0l);
   }
   else if (eAct == MM::AfterSet)
   {
      long pos;
      pProp->Get(pos);
      int ret;
      const unsigned cmdSize = 8;
      unsigned char cmd[cmdSize];
      for (unsigned i = 0; i < cmdSize; i++) {
         cmd[i] = 0;
      }
      if (pos == 0)
         cmd[1] = CMD_TURN_OFF_ILLUMINATION;
      else
         cmd[1] = CMD_TURN_ON_ILLUMINATION; 

      isOpen_ = pos == 1;

      ret = hub_->SendCommand(cmd, cmdSize);
      if (ret != DEVICE_OK)
         return ret;
      changedTime_ = GetCurrentMMTime();
   }
   return DEVICE_OK;
}


int SquidShutter::OnPattern(MM::PropertyBase* pProp, MM::ActionType eAct) 
{
   if (eAct == MM::BeforeGet)
   {
      if (pattern_ < 7)
      {
         pProp->Set(ILLUMINATIONS[pattern_].c_str());
      }
      else  if (pattern_ > 10 && pattern_ < 16)// Laser
      {
         pProp->Set(LASERS[pattern_ - 11].c_str());
      }
      else {
         return DEVICE_INVALID_PROPERTY_VALUE;
      }
   }
   else if (eAct == MM::AfterSet)
   {
      std::string illumination;
      pProp->Get(illumination);
      for (uint8_t i = 0; i < 7; i++)
      {
         if (ILLUMINATIONS[i] == illumination) {
            pattern_ = i;
            return sendIllumination(pattern_, intensity_, red_, green_, blue_);
         }
      }
      for (uint8_t i = 0; i < 5; i++)
      {
         if (LASERS[i] == illumination)
         {
            pattern_ = i + 11;
            return sendIllumination(pattern_, intensity_, red_, green_, blue_);
         }
      }
   }
   return DEVICE_OK;
}


int SquidShutter::OnIntensity(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set((long) intensity_);
   }
   else if (eAct == MM::AfterSet)
   {
      long pos;
      pProp->Get(pos);
      if (pos >= 0 && pos <= 255)
      {
         intensity_ = (uint8_t)pos;
      }
      return sendIllumination(pattern_, intensity_, red_, green_, blue_);
   }
   return DEVICE_OK;
}

int SquidShutter::OnRed(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set((long) red_);
   }
   else if (eAct == MM::AfterSet)
   {
      long pos;
      pProp->Get(pos);
      if (pos >= 0 && pos <= 255)
      {
         red_ = (uint8_t) pos;
      }
      return sendIllumination(pattern_, intensity_, red_, green_, blue_);
   }
   return DEVICE_OK;
}

int SquidShutter::OnGreen(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set((long) green_);
   }
   else if (eAct == MM::AfterSet)
   {
      long pos;
      pProp->Get(pos);
      if (pos >= 0 && pos <= 255)
      {
         green_ = (uint8_t) pos;
      }
      return sendIllumination(pattern_, intensity_, red_, green_, blue_);
   }
   return DEVICE_OK;
}

int SquidShutter::OnBlue(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set((long) blue_);
   }
   else if (eAct == MM::AfterSet)
   {
      long pos;
      pProp->Get(pos);
      if (pos >= 0 && pos <= 255)
      {
         blue_ = (uint8_t) pos;
      }
      return sendIllumination(pattern_, intensity_, red_, green_, blue_);
   }
   return DEVICE_OK;
}


int SquidShutter::OnHasLasers(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      pProp->Set(hasLasers_ ? "Yes" : "No");
   }
   else if (eAct == MM::AfterSet)
   {
      std::string ans;
      pProp->Get(ans);
      hasLasers_ = ans == "Yes";
   }
   return DEVICE_OK;
}


int SquidShutter::sendIllumination(uint8_t pattern, uint8_t intensity, uint8_t red, uint8_t green, uint8_t blue)
{
   const unsigned cmdSize = 8;
   unsigned char cmd[cmdSize];
   for (unsigned i = 0; i < cmdSize; i++) {
      cmd[i] = 0;
   }
   cmd[1] = CMD_SET_ILLUMINATION_LED_MATRIX;
   cmd[2] = pattern;
   cmd[3] = (uint8_t) ((double) intensity / 255 * green);
   cmd[4] = (uint8_t) ((double) intensity / 255 * red);
   cmd[5] = (uint8_t) ((double) intensity / 255 * blue);

   int ret = hub_->SendCommand(cmd, cmdSize);
   if (ret != DEVICE_OK)
      return ret;
   changedTime_ = GetCurrentMMTime();

   return DEVICE_OK;
}