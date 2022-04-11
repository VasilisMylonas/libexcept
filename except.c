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

void __libexcept_throw_exception(int exception, const char* file, int line)
{
    if (libexcept_on_throw != NULL)
    {
        libexcept_on_throw(exception, file, line);
    }

    if (*__libexcept_current_context() != NULL)
    {
        longjmp(**__libexcept_current_context(), exception);
    }

    if (libexcept_on_unhandled_exception != NULL)
    {
        libexcept_on_unhandled_exception(exception);
    }
    else
    {
        fputs("Unhandled exception\n", stderr);
    }
}

void (*libexcept_on_throw)(int exception, const char* file, int line);
void (*libexcept_on_unhandled_exception)(int exception);
