/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// Borrowed from Chromium's src/base/threading/thread_checker_impl.cc.

#include "webrtc/base/thread_checker_impl.h"

#include "webrtc/base/checks.h"
#include "webrtc/base/thread.h"

#if defined(WEBRTC_LINUX)
#include <sys/syscall.h>
#endif

namespace rtc {

PlatformThreadId CurrentThreadId() {
  PlatformThreadId ret;
#if defined(WEBRTC_WIN)
  ret = GetCurrentThreadId();
#elif defined(WEBRTC_POSIX)
#if defined(WEBRTC_MAC) || defined(WEBRTC_IOS)
  ret = pthread_mach_thread_np(pthread_self());
#elif defined(WEBRTC_LINUX)
  ret =  syscall(__NR_gettid);
#elif defined(WEBRTC_ANDROID)
  ret = gettid();
#else
  // Default implementation for nacl and solaris.
  ret = reinterpret_cast<pid_t>(pthread_self());
#endif
#endif  // defined(WEBRTC_POSIX)
  DCHECK(ret);
  return ret;
}

PlatformThreadRef CurrentThreadRef() {
#if defined(WEBRTC_WIN)
  return GetCurrentThreadId();
#elif defined(WEBRTC_POSIX)
  return pthread_self();
#endif
}

bool IsThreadRefEqual(const PlatformThreadRef& a, const PlatformThreadRef& b) {
#if defined(WEBRTC_WIN)
	if (a != b)
	{
		printf("valid_thread_ error \n");
	}
  return a == b;
#elif defined(WEBRTC_POSIX)
  return pthread_equal(a, b);
#endif
}

ThreadCheckerImpl::ThreadCheckerImpl() : valid_thread_(CurrentThreadRef()) {
#ifdef _WIN32
	rtc::Thread *curr_thread = rtc::Thread::Current();
	int curr_threadId = -1;
	if (curr_thread)
	{
		curr_threadId = curr_thread->GetId();
		if (curr_threadId != (int)valid_thread_)
		{
			valid_thread_ = (PlatformThreadRef)curr_threadId;
		}
	}
#elif defined(WEBRTC_LINUX)
    
#if defined(WEBRTC_ANDROID)
    rtc::Thread *curr_thread = rtc::Thread::Current();
    
    if (curr_thread)
    {
        PlatformThreadRef curr_threadId = curr_thread->GetPThread();
        if (curr_threadId != valid_thread_)
        {
            valid_thread_ = (PlatformThreadRef)curr_threadId;
        }
    }
#else
#endif
    
#else
    rtc::Thread *curr_thread = rtc::Thread::Current();
    
    if (curr_thread)
    {
        PlatformThreadRef curr_threadId = curr_thread->GetPThread();
        if (curr_threadId != valid_thread_)
        {
            valid_thread_ = (PlatformThreadRef)curr_threadId;
        }
    }
#endif
}

ThreadCheckerImpl::~ThreadCheckerImpl() {
}

bool ThreadCheckerImpl::CalledOnValidThread() const {
  
#ifdef _WIN32
	PlatformThreadRef current_thread = CurrentThreadRef();
  rtc::Thread *curr_thread = rtc::Thread::Current();
  int curr_threadId = -1;
  if (curr_thread)
  {
	  curr_threadId = curr_thread->GetId();
	  if (curr_threadId != (int)current_thread)
	  {
		  current_thread = (PlatformThreadRef)curr_threadId;
	  }
  }
   // const PlatformThreadRef current_thread = CurrentThreadRef();
#elif defined(WEBRTC_LINUX)
    
#if defined(WEBRTC_ANDROID)
    PlatformThreadRef current_thread = CurrentThreadRef();
    
    rtc::Thread *curr_thread = rtc::Thread::Current();
    
    if (curr_thread)
    {
        PlatformThreadRef curr_threadId = curr_thread->GetPThread();
        if (curr_threadId != current_thread)
        {
            //printf("curr_thread = %x, valid_thread_= %x",*(int *)&curr_thread, *(int *)&valid_thread_);
            current_thread = (PlatformThreadRef)curr_threadId;
        }
    }
#else
    const PlatformThreadRef current_thread = CurrentThreadRef();
    
#endif

#else
  PlatformThreadRef current_thread = CurrentThreadRef();
    
    rtc::Thread *curr_thread = rtc::Thread::Current();
    
    if (curr_thread)
    {
        PlatformThreadRef curr_threadId = curr_thread->GetPThread();
        if (curr_threadId != current_thread)
        {
            //printf("curr_thread = %x, valid_thread_= %x",*(int *)&curr_thread, *(int *)&valid_thread_);
            current_thread = (PlatformThreadRef)curr_threadId;
        }
    }
#endif

  CritScope scoped_lock(&lock_);
  if (!valid_thread_)  // Set if previously detached.
    valid_thread_ = current_thread;
  return IsThreadRefEqual(valid_thread_, current_thread);
}

void ThreadCheckerImpl::DetachFromThread() {
  CritScope scoped_lock(&lock_);
  valid_thread_ = 0;
}

}  // namespace rtc
