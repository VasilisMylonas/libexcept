//
// The MIT License (MIT)
//
// Copyright (c) 2022 Vasilis Mylonas
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//

#include "except.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

static void __libexcept_handle_signal(int signal)
{
    switch (signal)
    {
    case SIGILL:
        throw(EILSEQ);
    case SIGFPE:
        throw(EDOM);
    case SIGSEGV:
        throw(EFAULT);
    default:
        break;
    }
}

jmp_buf** __libexcept_current_context()
{
    static thread_local jmp_buf* buffer;
    return &buffer;
}

void __libexcept_enable_sigcatch()
{
    signal(SIGILL, __libexcept_handle_signal);
    signal(SIGFPE, __libexcept_handle_signal);
    signal(SIGSEGV, __libexcept_handle_signal);
}

void __libexcept_disable_sigcatch()
{
    signal(SIGILL, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
}

void __libexcept_throw(int exception, const char* file, int line)
{
    if (libexcept_on_throw != NULL)
    {
        libexcept_on_throw(exception, file, line);
    }

    if (*__libexcept_current_context() != NULL)
    {
        longjmp(**__libexcept_current_context(), exception);
    }

    __libexcept_unhandled(exception);
}

void __libexcept_unhandled(int exception)
{
    if (libexcept_on_unhandled != NULL)
    {
        libexcept_on_unhandled(exception);
    }
    else
    {
        fputs("Unhandled exception\n", stderr);
    }
}

void __libexcept_unexpected(int exception)
{
    if (libexcept_on_unexpected != NULL)
    {
        libexcept_on_unexpected(exception);
    }
    else
    {
        fputs("Unexpected exception\n", stderr);
    }
}

void (*libexcept_on_throw)(int, const char*, int);
void (*libexcept_on_unhandled)(int);
void (*libexcept_on_unexpected)(int);
