///////////////////////////////////////////////////////////////////////////////
// FILE:          DeviceThreads.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     MMDevice - Device adapter kit
//-----------------------------------------------------------------------------
// DESCRIPTION:   Cross-platform wrapper class for using threads in MMDevices
//
// AUTHOR:        Nenad Amodaj, nenad@amodaj.com 11/27/2007
// COPYRIGHT:     University of California, San Francisco, 2007
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

#pragma once

#include <thread>

#ifdef _WIN32
   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>
#else
   #include <pthread.h>
#endif

/**
 * @brief Base class for threads in MM devices.
 *
 * @attention New code should use std::thread instead.
 */
class MMDeviceThreadBase
{
public:
   MMDeviceThreadBase() {}

   virtual ~MMDeviceThreadBase()
   {
      // Detaching on destruction may not be the ideal design, but the only one
      // possible given previous behavior (which leaked the native handle).
      if (thread_.joinable())
         thread_.detach();
   }

   // Cannot copy a thread
   MMDeviceThreadBase(const MMDeviceThreadBase&) = delete;
   MMDeviceThreadBase& operator=(const MMDeviceThreadBase&) = delete;

   // We also cannot safely support move because the thread calls svc() by
   // reference.
   MMDeviceThreadBase(MMDeviceThreadBase&&) = delete;
   MMDeviceThreadBase& operator=(MMDeviceThreadBase&&) = delete;

   /**
    * @brief This function is called on the new thread.
    *
    * The return value is ignored.
    */
   virtual int svc() = 0;

   void activate()
   {
      if (thread_.joinable())
         thread_.detach();
      thread_ = std::thread([this]() { (void)svc(); });
      // Note: failure to create the thread will throw std::system_error, but
      // that only happens upon resource exhaustion (normally rare; akin to
      // std::bad_alloc). Most callers do not handle this, but terminating with
      // an uncaught exception is preferable to silently continuing (as we
      // previously did).
   }

   void wait()
   {
      // Note: joining a non-joinable thread (a programming error) will throw
      // std::system_error.
      thread_.join();
   }

private:
   std::thread thread_;
};

/**
 * @brief Critical section lock.
 */
class MMThreadLock
{
public:
   MMThreadLock()
   {
#ifdef _WIN32
      InitializeCriticalSection(&lock_);
#else
      pthread_mutexattr_t a;
      pthread_mutexattr_init(&a);
      pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
      pthread_mutex_init(&lock_, &a);
      pthread_mutexattr_destroy(&a);
#endif
   }

   ~MMThreadLock()
   {
#ifdef _WIN32
      DeleteCriticalSection(&lock_);
#else
      pthread_mutex_destroy(&lock_);
#endif
   }

   void Lock()
   {
#ifdef _WIN32
      EnterCriticalSection(&lock_);
#else
      pthread_mutex_lock(&lock_);
#endif
   }

   void Unlock()
   {
#ifdef _WIN32
      LeaveCriticalSection(&lock_);
#else
      pthread_mutex_unlock(&lock_);
#endif
   }

private:
   // Forbid copying
   MMThreadLock(const MMThreadLock&);
   MMThreadLock& operator=(const MMThreadLock&);

#ifdef _WIN32
   CRITICAL_SECTION
#else
   pthread_mutex_t
#endif
   lock_;
};

class MMThreadGuard
{
public:
   MMThreadGuard(MMThreadLock& lock) : lock_(&lock)
   {
      lock_->Lock();
   }

   MMThreadGuard(MMThreadLock* lock) : lock_(lock)
   {
      if (lock != 0)
         lock_->Lock();
   }

   bool isLocked() {return lock_ == 0 ? false : true;}

   ~MMThreadGuard()
   {
      if (lock_ != 0)
         lock_->Unlock();
   }

private:
   // Forbid copying
   MMThreadGuard(const MMThreadGuard&);
   MMThreadGuard& operator=(const MMThreadGuard&);

   MMThreadLock* lock_;
};
