/////////////////////////////////////////////////////////////////////////////
// Purpose:   Check for overflowing the stack during deep recursion
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

/*

// Usage
void deepRecursion(int n)
{
    StackMonitor::checkStack();  // Check before going deeper

    // ... recursive work ...
    if (n > 0)
    {
        deepRecursion(n - 1);
    }
}

*/

#include <cstddef>

class StackMonitor
{
public:
    static constexpr auto getSafetyMargin() -> size_t
    {
        return static_cast<size_t>(64 * 1024);  // NOLINT (cppcheck-suppress)
    }  // 64 KB buffer
    static constexpr auto getWindowsDefaultStackSize() -> size_t
    {
        // In wxUiEditor, the stack size defaults to 8MB on Windows.
        return static_cast<size_t>(8 * 1024 * 1024);  // NOLINT (cppcheck-suppress)
    }  // Windows default 1MB

    static auto hasSufficientStack() -> bool { return getStackRemaining() > getSafetyMargin(); }

    static void checkStack()
    {
        if (!hasSufficientStack())
        {
            throw std::runtime_error("Insufficient stack space");
        }
    }

    // These two functions are available for all platforms, but implementation differs.

    // static auto getStackRemaining() -> size_t
    // static auto getStackSize() -> size_t

#if defined(_WIN32)
    #include <windows.h>

    static auto getStackRemaining() -> size_t
    {
        // GetCurrentThreadStackLimits returns the actual stack limits for the current thread
        // stack_high = lowest address (guard page, stack limit)
        // stack_low = highest address (bottom of allocated stack)
        ULONG_PTR stack_high;
        ULONG_PTR stack_low;
        GetCurrentThreadStackLimits(&stack_high, &stack_low);

        char dummy = 0;
        // Stack grows downward (high to low addresses)
        // Distance from current position to the guard page (limit)
        ptrdiff_t remaining = (ULONG_PTR) &dummy - stack_high;
        return static_cast<size_t>(remaining);
    }

    static auto getStackSize() -> size_t { return getWindowsDefaultStackSize(); }

#elif defined(__linux__)
    #include <pthread.h>

    static auto getStackRemaining() -> size_t
    {
        pthread_attr_t attr;
        void* stackaddr;
        size_t stacksize;

        pthread_getattr_np(pthread_self(), &attr);
        pthread_attr_getstack(&attr, &stackaddr, &stacksize);
        pthread_attr_destroy(&attr);

        char dummy;
        // stackaddr is the base (lowest address), distance to current position is remaining
        ptrdiff_t remaining = (char*) &dummy - (char*) stackaddr;
        return static_cast<size_t>(remaining);
    }

    static auto getStackSize() -> size_t
    {
        pthread_attr_t attr;
        size_t stacksize;
        pthread_getattr_np(pthread_self(), &attr);
        pthread_attr_getstacksize(&attr, &stacksize);
        pthread_attr_destroy(&attr);
        return stacksize;
    }

#elif defined(__APPLE__)
    #include <pthread.h>

    static auto getStackRemaining() -> size_t
    {
        char dummy;
        ptrdiff_t remaining = (char*) pthread_get_stackaddr_np(pthread_self()) - &dummy;
        return static_cast<size_t>(remaining);
    }

    static auto getStackSize() -> size_t { return pthread_get_stacksize_np(pthread_self()); }

#else
    // Fallback - very conservative
    static auto getStackRemaining() -> size_t { return 100 * 1024; }
    static auto getStackSize() -> size_t { return 512 * 1024; }
#endif
};
