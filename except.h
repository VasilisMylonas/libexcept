/**
 * @file except.h
 * @author Vasilis Mylonas <vasilismylonas@protonmail.com>
 * @brief setjmp/longjmp exception implementation for C.
 * @version 0.1
 * @date 2022-04-09
 *
 * @copyright Copyright (c) 2022 Vasilis Mylonas
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef EXCEPT_H
#define EXCEPT_H

#include <errno.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdlib.h>

#define throw __LIBEXCEPT_THROW
#define try __LIBEXCEPT_HANDLER __LIBEXCEPT_TRY
#define catch __LIBEXCEPT_CATCH
#define finally __LIBEXCEPT_FINALLY
#define rethrow __LIBEXCEPT_RETHROW

extern void (*libexcept_on_throw)(int exception, const char* file, int line);
extern void (*libexcept_on_unhandled_exception)(int exception);

#define __LIBEXCEPT_STAGE_TRY       0
#define __LIBEXCEPT_STAGE_CATCH     1
#define __LIBEXCEPT_STAGE_FINALLY   2
#define __LIBEXCEPT_STAGE_PROPAGATE 3
#define __LIBEXCEPT_UNIQUE(var)     __LIBEXCEPT_CONCAT(__libexcept_##var, __LINE__)
#define __LIBEXCEPT_CONCAT(a, b)    __LIBEXCEPT_CONCAT_(a, b)
#define __LIBEXCEPT_CONCAT_(a, b)   a##b

#define __LIBEXCEPT_THROW(error) (__libexcept_throw_exception(error, __FILE__, __LINE__), abort())
#define __LIBEXCEPT_RETHROW()    break

#define __LIBEXCEPT_HANDLER                                                                        \
    jmp_buf __LIBEXCEPT_UNIQUE(local_buffer);                                                      \
    jmp_buf* __LIBEXCEPT_UNIQUE(old_buffer) = *__libexcept_current_context();                      \
    *__libexcept_current_context() = &__LIBEXCEPT_UNIQUE(local_buffer);                            \
    for (int __libexcept_stage = 0, __libexcept_error = setjmp(__LIBEXCEPT_UNIQUE(local_buffer));  \
         __libexcept_stage < 4;                                                                    \
         __libexcept_stage++)                                                                      \
        if (__libexcept_stage == __LIBEXCEPT_STAGE_PROPAGATE)                                      \
        {                                                                                          \
            *__libexcept_current_context() = __LIBEXCEPT_UNIQUE(old_buffer);                       \
            if (__libexcept_error != 0)                                                            \
            {                                                                                      \
                __LIBEXCEPT_THROW(__libexcept_error);                                              \
            }                                                                                      \
        }

#define __LIBEXCEPT_TRY                                                                            \
    else if (__libexcept_stage == __LIBEXCEPT_STAGE_TRY && __libexcept_error == 0)

#define __LIBEXCEPT_CATCH(decl)                                                                    \
    else if (__libexcept_stage == __LIBEXCEPT_STAGE_CATCH &&                                       \
             __libexcept_error != 0) for (decl = __libexcept_error; __libexcept_error != 0;        \
                                          __libexcept_error = 0)

#define __LIBEXCEPT_FINALLY else if (__libexcept_stage == __LIBEXCEPT_STAGE_FINALLY)

jmp_buf** __libexcept_current_context();
void __libexcept_enable_sigcatch();
void __libexcept_disable_sigcatch();
void __libexcept_throw_exception(int, const char*, int);

#endif // EXCEPT_H
