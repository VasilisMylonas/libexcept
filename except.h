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

#ifndef EXCEPT_H
#define EXCEPT_H

/**
 * @file except.h
 * @author Vasilis Mylonas <vasilismylonas@protonmail.com>
 * @brief setjmp/longjmp exception implementation for C.
 * @version 0.1
 * @date 2022-04-09
 * @copyright Copyright (c) 2022 Vasilis Mylonas
 *
 * libexcept provides a simple exception implementation in pure C. It does this by providing macros
 * that use setjmp and longjmp in a way that simulates try/catch/finally blocks.
 *
 * Safety rules:
 * - Only use rethrow() inside catch blocks.
 * - Do not use break, continue, return, goto or other ways to exit try/catch/finally blocks.
 * - Do not use anything beginning with __libexcept.
 *
 * TODO: documentation
 */

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdnoreturn.h>

/**
 * @defgroup keywords Exception handling keywords.
 *
 * try: Begins a code block from which exceptions are expected to be thrown.
 * catch: Follows a try block and only executes when an exception is thrown.
 * finally: Follows a try block and is always executed. This is useful for cleaning up resources.
 *
 * TODO: catch_any
 *
 * catch/finally blocks are only required to come after a try block but not in a specific order. In
 * particular all of the following are valid:
 *
 * @code
 *
 * try { ... }
 * catch (int error) { ... }
 * finally { ... }
 *
 * try { ... }
 * finally { ... }
 * catch (int error) { ... }
 *
 * try { ... }
 * catch (int error) { ... }
 *
 * try { ... }
 * finally { ... }
 *
 * @endcode
 *
 * throw: Throws an exception. Execution of the current function immediately halts.
 * rethrow: Re-throws an exception caught in a catch block. This will preserve the original
 *          exception object.
 *
 *
 * If any of these keyword macros interfere with other symbol names you may choose to prevent their
 * definition. This can be done by defining the LIBEXCEPT_NO_KEYWORDS macro. The same constructs can
 * be used under the following names:
 *
 * __LIBEXCEPT_TRY
 * __LIBEXCEPT_CATCH
 * __LIBEXCEPT_CATCH_ANY
 * __LIBEXCEPT_FINALLY
 * __LIBEXCEPT_THROW
 * __LIBEXCEPT_RETHROW
 *
 * @{
 */

#ifndef LIBEXCEPT_NO_KEYWORDS
#define try __LIBEXCEPT_TRY
#define catch __LIBEXCEPT_CATCH
#define catch_any __LIBEXCEPT_CATCH_ANY
#define finally   __LIBEXCEPT_FINALLY
#define throw __LIBEXCEPT_THROW
#define rethrow __LIBEXCEPT_RETHROW
#endif

/**
 * @}
 */

/**
 * The maximum size of an object allowed to be thrown.
 */
#define LIBEXCEPT_MAX_THROWABLE_SIZE 128

/**
 * @defgroup event_hooks Event hooks.
 *
 * libexcept provides hooks for some events. They are global to the program and not thread-safe to
 * set or unset. They are intended to be set just after entering main.
 *
 * The default implementations just print a simple message to stderr. To restore the default
 * implementations just set these back to NULL.
 *
 * Ideally these functions should just perform some logging or set a flag and return. If any of
 * these throw an exception then the default unexpected handler is called and the program is
 * terminated. If these function never return to their callers then the behavior is undefined. It is
 * up to the programmer to ensure correct use.
 *
 * @{
 */

/**
 * Called whenever an exception is thrown.
 *
 * @param exception The exception that was thrown.
 */
extern void (*libexcept_on_throw)(void* exception);

/**
 * Called whenever an exception is never caught.
 *
 * @param exception The exception that was thrown.
 */
extern void (*libexcept_on_unhandled)(void* exception);

/**
 * Called whenever an exception is thrown from a catch or finally clause or from any of the user
 * defined event handlers.
 *
 * @param exception The exception that was thrown.
 */
extern void (*libexcept_on_unexpected)(void* exception);

/**
 * @}
 */

#ifdef LIBEXCEPT_SIGNAL_AWARE
// TODO: Documentation

/**
 * Enables transforming of signals to exceptions.
 */
void libexcept_enable_sigcatch();

/**
 * Disables transforming of signals to exceptions.
 */
void libexcept_disable_sigcatch();

typedef struct
{
    const char* message;
    void* pc;
} arithmetic_error_t;

typedef struct
{
    const char* message;
    void* pc;
} illegal_instruction_error_t;

typedef struct
{
    const char* message;
    void* pc;
} stack_corruption_error_t;

typedef struct
{
    const char* message;
    void* address;
} access_violation_t;

typedef struct
{
    const char* message;
    void* address;
} misaligned_access_error_t;

#endif

#ifdef LIBEXCEPT_SIGNAL_AWARE
#define __LIBEXCEPT_JMP_BUF        jmp_buf
#define __LIBEXCEPT_SETJMP(buffer) sigsetjmp(buffer, 1)
#define __LIBEXCEPT_LONGJMP        siglongjmp
#else
#define __LIBEXCEPT_JMP_BUF        sigjmp_buf
#define __LIBEXCEPT_SETJMP(buffer) setjmp(buffer)
#define __LIBEXCEPT_LONGJMP        longjmp
#endif

#define __LIBEXCEPT_STAGE_TRY        0
#define __LIBEXCEPT_STAGE_CATCH      1
#define __LIBEXCEPT_STAGE_FINALLY    2
#define __LIBEXCEPT_STAGE_PROPAGATE  3
#define __LIBEXCEPT_STAGE_UNEXPECTED -1
#define __LIBEXCEPT_UNIQUE(var)      __LIBEXCEPT_CONCAT(__libexcept_##var, __LINE__)
#define __LIBEXCEPT_CONCAT(a, b)     __LIBEXCEPT_CONCAT_(a, b)
#define __LIBEXCEPT_CONCAT_(a, b)    a##b
#define __LIBEXCEPT_THROW(T, object)                                                               \
    __libexcept_throw(#T, sizeof(T), (T[1]){object});                                              \
    static_assert(sizeof(T) <= LIBEXCEPT_MAX_THROWABLE_SIZE,                                       \
                  "Throwable object size exceeds the maximum supported by libexcept")
#define __LIBEXCEPT_RETHROW() break

#define __LIBEXCEPT_TRY                                                                            \
    __LIBEXCEPT_JMP_BUF __LIBEXCEPT_UNIQUE(local_buffer);                                          \
    __LIBEXCEPT_JMP_BUF* __LIBEXCEPT_UNIQUE(old_buffer) = *__libexcept_current_context();          \
    *__libexcept_current_context() = &__LIBEXCEPT_UNIQUE(local_buffer);                            \
    for (int __libexcept_stage = 0,                                                                \
             __libexcept_error = __LIBEXCEPT_SETJMP(__LIBEXCEPT_UNIQUE(local_buffer));             \
         __libexcept_stage < 4;                                                                    \
         __libexcept_stage++)                                                                      \
        if (__libexcept_stage == __LIBEXCEPT_STAGE_PROPAGATE)                                      \
        {                                                                                          \
            *__libexcept_current_context() = __LIBEXCEPT_UNIQUE(old_buffer);                       \
            if (__libexcept_error != 0)                                                            \
            {                                                                                      \
                __libexcept_throw(NULL, 0, NULL);                                                  \
            }                                                                                      \
        }                                                                                          \
        else if (__libexcept_stage == __LIBEXCEPT_STAGE_UNEXPECTED)                                \
        {                                                                                          \
            __libexcept_unexpected();                                                              \
        }                                                                                          \
        else if (__libexcept_stage == __LIBEXCEPT_STAGE_TRY && __libexcept_error == 0)

#define __LIBEXCEPT_CATCH(T, var)                                                                  \
    else if (__libexcept_stage == __LIBEXCEPT_STAGE_CATCH && __libexcept_error != 0 &&             \
             __libexcept_personality(#T))                                                          \
        __LIBEXCEPT_UNEXPECTED_LOOP(                                                               \
            __LIBEXCEPT_STAGE_CATCH) for (T var = *(T*)__libexcept_current_exception();            \
                                          __libexcept_error != 0;                                  \
                                          __libexcept_error = 0)

#define __LIBEXCEPT_CATCH_ANY                                                                      \
    else if (__libexcept_stage == __LIBEXCEPT_STAGE_CATCH && __libexcept_error != 0)               \
        __LIBEXCEPT_UNEXPECTED_LOOP(__LIBEXCEPT_STAGE_CATCH) for (; __libexcept_error != 0;        \
                                                                  __libexcept_error = 0)

#define __LIBEXCEPT_FINALLY                                                                        \
    else if (__libexcept_stage == __LIBEXCEPT_STAGE_FINALLY)                                       \
        __LIBEXCEPT_UNEXPECTED_LOOP(__LIBEXCEPT_STAGE_FINALLY)

#define __LIBEXCEPT_UNEXPECTED_LOOP(stage)                                                         \
    for (__libexcept_stage = __LIBEXCEPT_STAGE_UNEXPECTED; __libexcept_stage != stage;             \
         __libexcept_stage = stage)

__LIBEXCEPT_JMP_BUF** __libexcept_current_context();
noreturn void __libexcept_throw(const char*, size_t, void*);
noreturn void __libexcept_unexpected();
noreturn void __libexcept_unhandled();
int __libexcept_personality(const char*);
void* __libexcept_current_exception();

#endif // EXCEPT_H
