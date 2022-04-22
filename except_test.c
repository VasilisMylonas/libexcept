#include <assert.h>
#include <stdbool.h>

#include "except.h"

void test_throw()
{
    bool exec_try = false;
    bool exec_catch = false;
    bool exec_finally = false;

    try
    {
        exec_try = true;
        throw_value(int, EINVAL);
        assert(false);
    }
    catch (int)
    {
        exec_catch = true;
    }
    finally
    {
        exec_finally = true;
    }

    assert(exec_try);
    assert(exec_catch);
    assert(exec_finally);
}

void test_no_throw()
{
    bool exec_try = false;
    bool exec_catch = false;
    bool exec_finally = false;

    try
    {
        exec_try = true;
    }
    catch (int)
    {
        exec_catch = true;
    }
    finally
    {
        exec_finally = true;
    }

    assert(exec_try);
    assert(!exec_catch);
    assert(exec_finally);
}

typedef struct
{
    const char* name;
} libexcept_error_t;

int main()
{
    test_throw();
    test_no_throw();

    try
    {
        libexcept_error_t error;
        throw(libexcept_error_t, error);
    }
    catch (libexcept_error_t)
    {
    }

    return 0;
}
