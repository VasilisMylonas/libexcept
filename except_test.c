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
        throw(int, EINVAL);
        assert(false);
    }
    catch (int, e)
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
    catch (int, e)
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

void test_signal()
{
    libexcept_enable_sigcatch();

    bool error_caught = false;
    try
    {
        int y = 0;
        int x = 0 / y;
    }
    catch (arithmetic_error_t, e)
    {
        error_caught = true;
    }

    assert(error_caught);

    libexcept_disable_sigcatch();
}

int main()
{
    test_throw();
    test_no_throw();
    test_signal();

    return 0;
}
