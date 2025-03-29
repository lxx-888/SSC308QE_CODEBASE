/* SigmaStar trade secret */
/* Copyright (c) [2019~2022] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef _CPP_SUITE_RTOS_H_
#define _CPP_SUITE_RTOS_H_

#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <initializer_list>
#include <limits>
#include <limits.h>
#include <float.h>
#include <cwchar>
#include <type_traits>
#include <string>
#include <cstring>
#include <debug/string>
#include <system_error>
#include <locale>
#include <memory>
#include <stdexcept>
#include <list>
#include <any>
#include <vector>
#include <tuple>
#include <functional>
#include <utility>
#include <sstream>
#include <iterator>
#include <array>
#include <bitset>
#include <deque>
#include <forward_list>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <numeric>
#include <random>
#include <valarray>
#include <regex>
#include <atomic>
#include <future>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <exception>
#include <chrono>

#ifdef __cplusplus
extern "C"
{
#endif
#include "time.h"

    // int cpp_test_suite(unsigned int nCaseNum);
    // typedef unsigned int u32;
    extern void MsSleep(u32 ms);
#ifdef __cplusplus
}
#endif

typedef void (*_TEST_FUNC_PTR)(void);

typedef struct
{
    _TEST_FUNC_PTR pFnTest;
    char*          pTestDesc;
} CppTestItem_t, *pCppTestItem;
#ifdef stderr
#define _VERIFY_PRINT(S, F, L, P, C) __builtin_fprintf(stderr, S, F, L, P, C)
#else
#define _VERIFY_PRINT(S, F, L, P, C) __builtin_printf(S, F, L, P, C)
#endif

#define PRINT(x)    cout << #x << ": " << x << endl
#define TESTHEAD(x) cout << x << endl

#define VERIFY(fn)                                                                                              \
    do                                                                                                          \
    {                                                                                                           \
        if (!(fn))                                                                                              \
        {                                                                                                       \
            _VERIFY_PRINT("%s:%d: %s: Assertion '%s' failed.\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, #fn); \
            __builtin_abort();                                                                                  \
        }                                                                                                       \
    } while (false)

namespace __gnu_test
{
// For use in 22_locale/time_get and time_put.
std::tm test_tm(int sec, int min, int hour, int mday, int mon, int year, int wday, int yday, int isdst)
{
    static std::tm tmp;
    tmp.tm_sec   = sec;
    tmp.tm_min   = min;
    tmp.tm_hour  = hour;
    tmp.tm_mday  = mday;
    tmp.tm_mon   = mon;
    tmp.tm_year  = year;
    tmp.tm_wday  = wday;
    tmp.tm_yday  = yday;
    tmp.tm_isdst = isdst;
    return tmp;
}
} // namespace __gnu_test

//=============================================================================
// testsuite_api.h
//=============================================================================
namespace __gnu_test
{
// Checks for virtual public derivation in exception classes.
// See:
// http://www.boost.org/more/error_handling.html
struct bad_non_virtual : virtual public std::exception
{
};

template <typename Exception, bool DefaultCons>
struct diamond_derivation_base;

template <typename Exception>
struct diamond_derivation_base<Exception, true>
{
    struct diamond_derivation_error : bad_non_virtual, Exception
    {
        diamond_derivation_error() : bad_non_virtual(), Exception() {}
    };
};

template <typename Exception>
struct diamond_derivation_base<Exception, false>
{
    struct diamond_derivation_error : bad_non_virtual, Exception
    {
        diamond_derivation_error() : bad_non_virtual(), Exception("construct diamond") {}
    };
};

template <typename Exception, bool DefaultCons>
struct diamond_derivation : diamond_derivation_base<Exception, DefaultCons>
{
    typedef diamond_derivation_base<Exception, DefaultCons> base_type;
    typedef typename base_type::diamond_derivation_error    error_type;

    // NB: In the libstdc++-v3 testsuite, all the standard exception
    // classes (+ a couple of extensions) are checked:  since they
    // all derive *non* virtually from std::exception, the expected
    // behavior is ambiguity.
    static void test()
    {
        try
        {
            throw error_type();
        }
        catch (std::exception const&)
        {
            VERIFY(false);
        }
        catch (...)
        {
            VERIFY(true);
        }
    }
};

// Testing type requirements for template arguments.
struct NonDefaultConstructible
{
    NonDefaultConstructible(int) {}
    NonDefaultConstructible(const NonDefaultConstructible&) {}

#if __cplusplus >= 201103L
    // For std::iota.
    NonDefaultConstructible& operator++()
    {
        return *this;
    }
#endif
};

// See: 20.1.1 Template argument requirements.
inline bool operator==(const NonDefaultConstructible&, const NonDefaultConstructible&)
{
    return false;
}

inline bool operator<(const NonDefaultConstructible&, const NonDefaultConstructible&)
{
    return false;
}

// For 23 unordered_* requirements.
struct NonDefaultConstructible_hash
{
    std::size_t operator()(NonDefaultConstructible) const
    {
        return 1;
    }
};

// For 26 numeric algorithms requirements, need addable,
// subtractable, multiplicable.
inline NonDefaultConstructible operator+(const NonDefaultConstructible& lhs, const NonDefaultConstructible& rhs)
{
    return NonDefaultConstructible(1);
}

inline NonDefaultConstructible operator-(const NonDefaultConstructible& lhs, const NonDefaultConstructible& rhs)
{
    return NonDefaultConstructible(1);
}

inline NonDefaultConstructible operator*(const NonDefaultConstructible& lhs, const NonDefaultConstructible& rhs)
{
    return NonDefaultConstructible(1);
}

// Like unary_function, but takes no argument. (ie, void).
// Used for generator template parameter.
template <typename _Result>
struct void_function
{
    typedef _Result result_type;

    result_type operator()() const
    {
        return result_type();
    }
};

template <>
struct void_function<NonDefaultConstructible>
{
    typedef NonDefaultConstructible result_type;

    result_type operator()() const
    {
        return result_type(2);
    }
};

// For std::addressof, etc.
struct OverloadedAddressAux
{
};

struct OverloadedAddress
{
    OverloadedAddressAux operator&() const
    {
        return OverloadedAddressAux();
    }
};

inline bool operator<(const OverloadedAddress&, const OverloadedAddress&)
{
    return false;
}

inline bool operator==(const OverloadedAddress&, const OverloadedAddress&)
{
    return false;
}

struct OverloadedAddress_hash
{
    std::size_t operator()(const OverloadedAddress&) const
    {
        return 1;
    }
};

#if __cplusplus >= 201103L
struct NonCopyConstructible
{
    NonCopyConstructible() : num(-1) {}

    NonCopyConstructible(NonCopyConstructible&& other) : num(other.num)
    {
        other.num = 0;
    }

    NonCopyConstructible(const NonCopyConstructible&) = delete;

    operator int()
    {
        return num;
    }

   private:
    int num;
};
#endif

} // namespace __gnu_test


#endif