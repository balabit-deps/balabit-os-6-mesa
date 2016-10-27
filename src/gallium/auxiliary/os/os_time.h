/**************************************************************************
 *
 * Copyright 2008-2010 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * OS independent time-manipulation functions.
 * 
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#ifndef _OS_TIME_H_
#define _OS_TIME_H_


#include "pipe/p_config.h"

#if defined(PIPE_OS_UNIX)
#  include <unistd.h> /* usleep */
#endif

#include "pipe/p_compiler.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Get the current time in nanoseconds from an unknown base.
 */
int64_t
os_time_get_nano(void);


/*
 * Get the current time in microseconds from an unknown base.
 */
static inline int64_t
os_time_get(void)
{
   return os_time_get_nano() / 1000;
}


/*
 * Sleep.
 */
#if defined(PIPE_OS_UNIX)
#define os_time_sleep(_usecs) usleep(_usecs)
#else
void
os_time_sleep(int64_t usecs);
#endif


/*
 * Helper function for detecting time outs, taking in account overflow.
 *
 * Returns true if the current time has elapsed beyond the specified interval.
 */
static inline boolean
os_time_timeout(int64_t start,
                int64_t end,
                int64_t curr)
{
   if (start <= end)
      return !(start <= curr && curr < end);
   else
      return !((start <= curr) || (curr < end));
}


/**
 * Convert a relative timeout in nanoseconds into an absolute timeout,
 * in other words, it returns current time + timeout.
 * os_time_get_nano() must be monotonic.
 * PIPE_TIMEOUT_INFINITE is passed through unchanged. If the calculation
 * overflows, PIPE_TIMEOUT_INFINITE is returned.
 */
int64_t
os_time_get_absolute_timeout(uint64_t timeout);


/**
 * Wait until the variable at the given memory location is zero.
 *
 * \param var           variable
 * \param timeout       timeout in ns, can be anything from 0 (no wait) to
 *                      PIPE_TIME_INFINITE (wait forever)
 * \return     true if the variable is zero
 */
bool
os_wait_until_zero(volatile int *var, uint64_t timeout);


/**
 * Wait until the variable at the given memory location is zero.
 * The timeout is the absolute time when the waiting should stop. If it is
 * less than or equal to the current time, it only returns the status and
 * doesn't wait. PIPE_TIME_INFINITE waits forever. This requires that
 * os_time_get_nano is monotonic.
 *
 * \param var       variable
 * \param timeout   the time in ns when the waiting should stop
 * \return     true if the variable is zero
 */
bool
os_wait_until_zero_abs_timeout(volatile int *var, int64_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* _OS_TIME_H_ */
