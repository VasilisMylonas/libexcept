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
        throw(EINVAL);
        assert(false);
    }
    catch (int error)
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
    catch (int error)
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

int main()
{
    test_throw();
    test_no_throw();

    throw(EINPROGRESS);

    return 0;
}
