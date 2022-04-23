/*
  This file is part of libexcept (https://github.com/VasilisMylonas/libexcept).

  The MIT License (MIT)

  Copyright (c) 2022 Vasilis Mylonas

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
 */

#include "except.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LIBEXCEPT_THREAD_AWARE
#include <threads.h>
static thread_local char current_exception[LIBEXCEPT_MAX_THROWABLE_SIZE];
static thread_local const char* current_id;
#else
static char current_exception[LIBEXCEPT_MAX_THROWABLE_SIZE];
static const char* current_id;
#endif

#ifdef LIBEXCEPT_SIGNAL_AWARE
#include <signal.h>

static void __libexcept_handle_signal(int signal, siginfo_t* info, void* context)
{
    (void)context;

    switch (signal)
    {
    case SIGFPE: {
        arithmetic_error_t error = {
            .pc = info->si_addr,
        };

        switch (info->si_code)
        {
        case FPE_INTDIV:
            error.message = "Integer division by zero.";
            break;
        case FPE_INTOVF:
            error.message = "Integer overflow.";
            break;
        case FPE_FLTDIV:
            error.message = "Floating point division by zero.";
            break;
        case FPE_FLTOVF:
            error.message = "Floating point overflow.";
            break;
        case FPE_FLTUND:
            error.message = "Floating point underflow.";
            break;
        case FPE_FLTRES:
            error.message = "Floating point inexact result.";
            break;
        case FPE_FLTINV:
            error.message = "Invalid floating point operation.";
            break;
        case FPE_FLTSUB:
            error.message = "Subscript out of range.";
            break;
        default:
            error.message = "Unknown arithmetic exception.";
            break;
        }
        throw(arithmetic_error_t, error);
    }
    case SIGBUS:
        if (info->si_code == BUS_ADRALN)
        {
            misaligned_access_error_t error = {
                .message = "Invalid address alignment.",
                .address = info->si_addr,
            };
            throw(misaligned_access_error_t, error);
        }
    case SIGSEGV: {
        access_violation_t error = {
            .message = "Access violation.",
            .address = info->si_addr,
        };
        throw(access_violation_t, error);
    }
    case SIGILL: {
        if (info->si_code == ILL_BADSTK)
        {
            stack_corruption_error_t error = {
                .message = "Internal stack error.",
                .pc = info->si_addr,
            };
            throw(stack_corruption_error_t, error);
        }
        else
        {
            illegal_instruction_error_t error = {
                .message = "Illegal instruction.",
                .pc = info->si_addr,
            };
            throw(illegal_instruction_error_t, error);
        }
    }

    default:
        abort();
    }
}

void libexcept_enable_sigcatch()
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = __libexcept_handle_signal;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
}

void libexcept_disable_sigcatch()
{
    signal(SIGILL, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
}
#endif

jmp_buf** __libexcept_current_context()
{
#ifdef LIBEXCEPT_THREAD_AWARE
    static thread_local __LIBEXCEPT_JMP_BUF* buffer;
#else
    static jmp_buf* buffer;
#endif
    return &buffer;
}

void __libexcept_throw(const char* id, size_t exception_size, void* exception)
{
    // id is NULL only when rethrowing
    if (id != NULL)
    {
        memcpy(current_exception, exception, exception_size);
        current_id = id;
    }

    // Call the user defined handler if possible.
    if (libexcept_on_throw != NULL)
    {
        // Exceptions are not expected to be thrown.
        __LIBEXCEPT_TRY
        {
            libexcept_on_throw(current_exception);
        }
        __LIBEXCEPT_CATCH_ANY
        {
            __libexcept_unexpected();
        }
    }

    // If this is NULL then we have reached the end of the chain.
    if (*__libexcept_current_context() != NULL)
    {
        __LIBEXCEPT_LONGJMP(**__libexcept_current_context(), 1);
    }

    __libexcept_unhandled();
}

void __libexcept_unhandled()
{
    // Call the user provided handler if possible.
    if (libexcept_on_unhandled != NULL)
    {
        // Exceptions are not expected to be thrown.
        __LIBEXCEPT_TRY
        {
            libexcept_on_unhandled(__libexcept_current_exception());
        }
        __LIBEXCEPT_CATCH_ANY
        {
            __libexcept_unexpected();
        }
    }
    else
    {
        fputs("Unhandled exception\n", stderr);
    }

#ifdef LIBEXCEPT_THREAD_AWARE
    thrd_exit(EXIT_FAILURE);
#else
    abort();
#endif
}

void __libexcept_unexpected()
{
    // Call the user provided handler if possible.
    if (libexcept_on_unexpected != NULL)
    {
        __LIBEXCEPT_TRY
        {
            libexcept_on_unexpected(current_exception);
        }
        __LIBEXCEPT_CATCH_ANY
        {
            // If an exception occurs, reset the handler and try again.
            libexcept_on_unexpected = NULL;
            __libexcept_unexpected();
        }
    }
    else
    {
        fputs("Unexpected exception\n", stderr);
    }

#ifdef LIBEXCEPT_THREAD_AWARE
    thrd_exit(EXIT_FAILURE);
#else
    abort();
#endif
}

void* __libexcept_current_exception()
{
    return current_exception;
}

int __libexcept_personality(const char* id)
{
    return strcmp(current_id, id) == 0;
}

void (*libexcept_on_throw)(void*);
void (*libexcept_on_unhandled)(void*);
void (*libexcept_on_unexpected)(void*);
