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
#include "st_common_dualos_sample_rtos.h"
#include "cpp_suite_sample_rtos.h"

#define MODULE_NAME "CPP_SUITE"

static void _test_cpp_support(void);
static void _test_cpp_diagnostics(void);
static void _test_cpp_util(void);
static void _test_cpp_string(void);
static void _test_cpp_locale(void);
static void _test_cpp_containers(void);
static void _test_cpp_iterators(void);
static void _test_cpp_algorithms(void);
static void _test_cpp_numerics(void);
static void _test_cpp_io(void);
static void _test_cpp_regex(void);
static void _test_cpp_atomics(void);
static void _test_cpp_threads(void);
static void _test_cpp_mutex(void);

static const CppTestItem_t aCppsTestItemTbl[] = {
    {_test_cpp_support, (char*)"test support function"},
    {_test_cpp_diagnostics, (char*)"test diagnostics function"},
    {_test_cpp_util, (char*)"test util function"},
    {_test_cpp_string, (char*)"test string function"},
    {_test_cpp_locale, (char*)"test locale function"},
    {_test_cpp_containers, (char*)"test containers function"},
    {_test_cpp_iterators, (char*)"test iterators function"},
    {_test_cpp_algorithms, (char*)"test algorithms function"},
    {_test_cpp_numerics, (char*)"test numerics function"},
    {_test_cpp_io, (char*)"test io function"},
    {_test_cpp_regex, (char*)"test regex function"},
    {_test_cpp_atomics, (char*)"test atomics function"},
    {_test_cpp_threads, (char*)"test threads function"},
    {_test_cpp_mutex, (char*)"test mutex function"},
    {NULL, (char*)"test all function"},
};

#define CPP_TEST_ITEM_NUM (sizeof(aCppsTestItemTbl) / sizeof(CppTestItem_t))

//=============================================================================
// initializer_list\range_access.cc
//=============================================================================
void itializer_list_range_access_test01()
{
    std::begin({1, 2, 3});
    std::end({1, 2, 3});
}

//=============================================================================
// numeric_limits\infinity.cc
//=============================================================================
template <typename T>
void numeric_limits_infinity_test_infinity()
{
    bool test;

    if (std::numeric_limits<T>::has_infinity)
    {
        T inf = std::numeric_limits<T>::infinity();
        test  = (inf + inf == inf);
    }
    else
        test = true;

    VERIFY(test);
}

//=============================================================================
// numeric_limits\is_signed.cc
//=============================================================================
#ifdef __CHAR_UNSIGNED__
#define char_is_signed false
#else
#define char_is_signed true
#endif

void numeric_limits_is_signed_test_sign()
{
    VERIFY(std::numeric_limits<char>::is_signed == char_is_signed);
    VERIFY(std::numeric_limits<signed char>::is_signed == true);
    VERIFY(std::numeric_limits<unsigned char>::is_signed == false);
    VERIFY(std::numeric_limits<short>::is_signed == true);
    VERIFY(std::numeric_limits<unsigned short>::is_signed == false);
    VERIFY(std::numeric_limits<int>::is_signed == true);
    VERIFY(std::numeric_limits<unsigned>::is_signed == false);
    VERIFY(std::numeric_limits<long>::is_signed == true);
    VERIFY(std::numeric_limits<unsigned long>::is_signed == false);
    VERIFY(std::numeric_limits<float>::is_signed == true);
    VERIFY(std::numeric_limits<double>::is_signed == true);
    VERIFY(std::numeric_limits<long double>::is_signed == true);
}

//=============================================================================
// numeric_limits\lowest.cc
//=============================================================================
template <typename T>
void lowest_do_test(std::true_type)
{
    T limits_min = std::numeric_limits<T>::min();
    VERIFY(std::numeric_limits<T>::lowest() == limits_min);
}

template <typename T>
void lowest_do_test(std::false_type)
{
    T limits_max = std::numeric_limits<T>::max();
    VERIFY(std::numeric_limits<T>::lowest() == -limits_max);
}

template <typename Tp>
void lowest_do_test()
{
    lowest_do_test<Tp>(typename std::is_integral<Tp>::type());
}

void numeric_limits_lowest_test01()
{
    lowest_do_test<char>();
    lowest_do_test<signed char>();
    lowest_do_test<unsigned char>();
#ifdef _GLIBCXX_USE_WCHAR_T
    lowest_do_test<wchar_t>();
#endif
#ifdef _GLIBCXX_USE_CHAR8_T
    lowest_do_test<char8_t>();
#endif
    lowest_do_test<char16_t>();
    lowest_do_test<char32_t>();

    lowest_do_test<short>();
    lowest_do_test<unsigned short>();

    lowest_do_test<int>();
    lowest_do_test<unsigned int>();

    lowest_do_test<long>();
    lowest_do_test<unsigned long>();

    lowest_do_test<long long>();
    lowest_do_test<unsigned long long>();

    // GNU Extensions.
#ifdef _GLIBCXX_USE_INT128
    lowest_do_test<__int128>();
    lowest_do_test<unsigned __int128>();
#endif

    lowest_do_test<float>();
    lowest_do_test<double>();
    lowest_do_test<long double>();
}

//=============================================================================
// numeric_limits\min_max.cc
//=============================================================================
template <typename T>
struct extrema
{
    static T min;
    static T max;
};

#define DEFINE_EXTREMA(T, m, M) \
    template <>                 \
    T extrema<T>::min = m;      \
    template <>                 \
    T extrema<T>::max = M

DEFINE_EXTREMA(char, CHAR_MIN, CHAR_MAX);
DEFINE_EXTREMA(signed char, SCHAR_MIN, SCHAR_MAX);
DEFINE_EXTREMA(unsigned char, 0, UCHAR_MAX);
DEFINE_EXTREMA(short, SHRT_MIN, SHRT_MAX);
DEFINE_EXTREMA(unsigned short, 0, USHRT_MAX);
DEFINE_EXTREMA(int, INT_MIN, INT_MAX);
DEFINE_EXTREMA(unsigned, 0U, UINT_MAX);
DEFINE_EXTREMA(long, LONG_MIN, LONG_MAX);
DEFINE_EXTREMA(unsigned long, 0UL, ULONG_MAX);

#if _GLIBCXX_USE_WCHAR_T
DEFINE_EXTREMA(wchar_t, WCHAR_MIN, WCHAR_MAX);
#endif //_GLIBCXX_USE_WCHAR_T

DEFINE_EXTREMA(float, FLT_MIN, FLT_MAX);
DEFINE_EXTREMA(double, DBL_MIN, DBL_MAX);
DEFINE_EXTREMA(long double, LDBL_MIN, LDBL_MAX);

#undef DEFINE_EXTREMA

template <typename T>
void numeric_limits_min_max_test_extrema()
{
    T limits_min  = std::numeric_limits<T>::min();
    T limits_max  = std::numeric_limits<T>::max();
    T extrema_min = extrema<T>::min;
    T extrema_max = extrema<T>::max;
    VERIFY(extrema_min == limits_min);
    VERIFY(extrema_max == limits_max);
}

//=============================================================================
// numeric_limits\quiet_NaN.cc
//=============================================================================
template <typename T>
void numeric_limits_quiet_NaN_test_qnan()
{
    bool test;

    if (std::numeric_limits<T>::has_quiet_NaN)
    {
        T nan = std::numeric_limits<T>::quiet_NaN();
        test  = (nan != nan);
    }
    else
        test = true;

    VERIFY(test);
}

//=============================================================================
// 19_diagnostics
//=============================================================================

//=============================================================================
// error_category\generic_category.cc
//=============================================================================
void error_category_generic_category_test01()
{
    const char* name = std::generic_category().name();
    VERIFY(name == std::string("generic"));
}

void error_category_generic_category_test02()
{
    const std::error_category& cat = std::generic_category();
    std::error_condition       cond;

    cond = cat.default_error_condition(EBADF);
    VERIFY(cond.value() == EBADF);
    VERIFY(cond == std::errc::bad_file_descriptor);
    VERIFY(cond.category() == std::generic_category());
    cond = cat.default_error_condition(EACCES);
    VERIFY(cond.value() == EACCES);
    VERIFY(cond == std::errc::permission_denied);
    VERIFY(cond.category() == std::generic_category());

    // PR libstdc++/60555
    VERIFY(std::error_code(EBADF, cat) == std::errc::bad_file_descriptor);
    VERIFY(std::error_code(EACCES, cat) == std::errc::permission_denied);
}

void error_category_generic_category_test03()
{
    // set "C" locale to get expected message
    auto loc = std::locale::global(std::locale::classic());

    std::string msg = std::generic_category().message(EBADF);
    VERIFY(msg.find("file") != std::string::npos);

    std::locale::global(loc);
}

//=============================================================================
// error_category\system_category.cc
//=============================================================================
void error_category_system_category_test01()
{
    const char* name = std::system_category().name();
    VERIFY(name == std::string("system"));
}

void error_category_system_category_test02()
{
    const std::error_category& cat = std::system_category();
    std::error_condition       cond;

    // As of 2011, ISO C only defines EDOM, EILSEQ and ERANGE:
    cond = cat.default_error_condition(EDOM);
    VERIFY(cond.value() == EDOM);
    VERIFY(cond == std::errc::argument_out_of_domain);
    VERIFY(cond.category() == std::generic_category());
    cond = cat.default_error_condition(EILSEQ);
    VERIFY(cond.value() == EILSEQ);
    VERIFY(cond == std::errc::illegal_byte_sequence);
    VERIFY(cond.category() == std::generic_category());
    cond = cat.default_error_condition(ERANGE);
    VERIFY(cond.value() == ERANGE);
    VERIFY(cond == std::errc::result_out_of_range);
    VERIFY(cond.category() == std::generic_category());

    // EBADF and EACCES are defined on all targets,
    // according to config/os/*/error_constants.h
    cond = cat.default_error_condition(EBADF);
    VERIFY(cond.value() == EBADF);
    VERIFY(cond == std::errc::bad_file_descriptor);
    VERIFY(cond.category() == std::generic_category());
    cond = cat.default_error_condition(EACCES);
    VERIFY(cond.value() == EACCES);
    VERIFY(cond == std::errc::permission_denied);
    VERIFY(cond.category() == std::generic_category());

    // All POSIX errno values are positive:
    cond = cat.default_error_condition(-1);
    VERIFY(cond.value() == -1);
    VERIFY(cond.category() == cat);
    cond = cat.default_error_condition(-99);
    VERIFY(cond.value() == -99);
    VERIFY(cond.category() == cat);

    // PR libstdc++/60555
    VERIFY(std::error_code(EDOM, cat) == std::errc::argument_out_of_domain);
    VERIFY(std::error_code(EILSEQ, cat) == std::errc::illegal_byte_sequence);
    VERIFY(std::error_code(ERANGE, cat) == std::errc::result_out_of_range);
    VERIFY(std::error_code(EBADF, cat) == std::errc::bad_file_descriptor);
    VERIFY(std::error_code(EACCES, cat) == std::errc::permission_denied);

    // As shown at https://gcc.gnu.org/ml/libstdc++/2018-08/msg00018.html
    // these two error codes might have the same value on AIX, but we still
    // expect both to be matched by system_category and so use generic_category:
#ifdef EEXIST
    cond = cat.default_error_condition(EEXIST);
    VERIFY(cond.value() == EEXIST);
    VERIFY(cond == std::errc::file_exists);
    VERIFY(cond.category() == std::generic_category());
    VERIFY(std::error_code(EEXIST, cat) == std::errc::file_exists);
#endif
#ifdef ENOTEMPTY
    cond = cat.default_error_condition(ENOTEMPTY);
    VERIFY(cond.value() == ENOTEMPTY);
    VERIFY(cond == std::errc::directory_not_empty);
    VERIFY(cond.category() == std::generic_category());
    VERIFY(std::error_code(ENOTEMPTY, cat) == std::errc::directory_not_empty);
#endif
}

void error_category_system_category_test03()
{
    // set "C" locale to get expected message
    auto loc = std::locale::global(std::locale::classic());

    std::string msg = std::system_category().message(EBADF);
    VERIFY(msg.find("file") != std::string::npos);

    std::locale::global(loc);
}

//=============================================================================
// 20_util
//=============================================================================

//=============================================================================
// addressof\1.cc
//=============================================================================
void f1(int) {}

void addressof_1_test01()
{
    using namespace __gnu_test;

    OverloadedAddress* ao1 = new OverloadedAddress();
    OverloadedAddress& o1  = *ao1;

    VERIFY(std::addressof(o1) == ao1);

    const OverloadedAddress* ao2 = new OverloadedAddress();
    const OverloadedAddress& o2  = *ao2;

    VERIFY(std::addressof(o2) == ao2);

    VERIFY(std::addressof(f1) == &f1);

    delete ao1;
    delete ao2;
}

//=============================================================================
// align\1.cc
//=============================================================================
void align_1_test01()
{
    size_t       space      = 100;
    void*        ptr        = new char[space];
    char* const  orig_ptr   = static_cast<char*>(ptr);
    char*        old_ptr    = orig_ptr;
    const size_t orig_space = space;
    size_t       old_space  = space;
    const size_t alignment  = 16;
    const size_t size       = 10;
    while (void* const r = std::align(alignment, size, ptr, space))
    {
        VERIFY(r == ptr);
        uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
        VERIFY(p % alignment == 0);
        char* const x = static_cast<char*>(ptr);
        VERIFY(x - old_ptr == (int)(old_space - space));
        VERIFY((void*)x < (void*)(orig_ptr + orig_space));
        VERIFY((void*)(x + size) < (void*)(orig_ptr + orig_space));
        ptr       = x + size;
        old_ptr   = x;
        old_space = space;
        space -= size;
    }
    delete[] orig_ptr;
}

//=============================================================================
// align\2.cc
//=============================================================================
void align_2_test01()
{
    int   i     = 0;
    void* ptr   = &i;
    auto  space = sizeof(i);
    auto  p2    = std::align(alignof(int), space, ptr, space);
    VERIFY(ptr == &i);
    VERIFY(p2 == &i);
    VERIFY(space == sizeof(i));
}

//=============================================================================
// allocator\10378.cc
//=============================================================================
class Bob
{
   public:
    static void* operator new(size_t sz)
    {
        return std::malloc(sz);
    }
};

// libstdc++/10378
void allocator_10378_test01()
{
    using namespace std;

    list<Bob> uniset;
    uniset.push_back(Bob());
}

//=============================================================================
// allocator\14176.cc
//=============================================================================
// libstdc++/14176
void allocator_14176_test02()
{
    unsigned int        len = 0;
    std::allocator<int> a;
    int*                p = a.allocate(len);
    a.deallocate(p, len);
}

//=============================================================================
// allocator\overaligned.cc
//=============================================================================
constexpr std::size_t align = alignof(std::max_align_t) * 4;

struct X
{
    alignas(align) char c;
};

void allocator_overaligned_test01()
{
    std::allocator<X> a;
    X*                p1 = a.allocate(1);
    VERIFY((reinterpret_cast<std::uintptr_t>(p1) % align) == 0);
    a.deallocate(p1, 1);
    X* p2 = a.allocate(20);
    VERIFY((reinterpret_cast<std::uintptr_t>(p2) % align) == 0);
    a.deallocate(p2, 20);
}

//=============================================================================
// allocator\void.cc
//=============================================================================
template class std::allocator<void>;

void allocator_void_test01()
{
    int i;
    using alloc_type = std::allocator<void>;
    alloc_type a;
    std::allocator_traits<alloc_type>::construct(a, &i, 42);
    VERIFY(i == 42);
    std::allocator_traits<alloc_type>::destroy(a, &i);
}

//=============================================================================
// bind\all_bound.cc
//=============================================================================
// Operations on empty function<> objects
void bind_all_bound_test01()
{
    VERIFY(std::bind(std::plus<int>(), 3, 5)() == 8);
    VERIFY(std::bind(std::minus<int>(), 3, 5)() == -2);
    VERIFY(std::bind<int>(std::plus<int>(), 3, 5)() == 8);
    VERIFY(std::bind<int>(std::minus<int>(), 3, 5)() == -2);
}

//=============================================================================
// bind\move.cc
//=============================================================================
// PR libstdc++/45924

struct f
{
    f() : i(0) {}
    f(f&& r) : i(1)
    {
        r.i = -1;
    }
    f(const f&) = delete;
    int operator()()
    {
        return i;
    }
    int i;
};

void bind_move_test01()
{
    auto b = std::bind(f());
    VERIFY(b() == 1);
    auto bc(std::move(b));
    VERIFY(bc() == 1);
    VERIFY(b() == -1);
}

void bind_move_test02()
{
    auto b = std::bind<int>(f());
    VERIFY(b() == 1);
    auto bc(std::move(b));
    VERIFY(bc() == 1);
    VERIFY(b() == -1);
}

//=============================================================================
// tuple\48476.cc
//=============================================================================
template <typename T>
typename std::decay<T>::type copy(T&& x)
{
    return std::forward<T>(x);
}

// libstdc++/48476
void tuple_48476_test01()
{
    std::shared_ptr<int> p(new int()), q, r;

    std::tuple<std::shared_ptr<int>&, int> t0(p, 23), t1(q, 0);
    t1 = copy(t0); // shall be equivalent to
                   // q = p; std::get<1>(t1) = std::get<1>(t0);
    VERIFY(q == p);

    std::tuple<std::shared_ptr<int>&, char> t2(r, 0);
    t2 = copy(t1); // shall be equivalent to
                   // r = q; std::get<1>(t2) = std::get<1>(t1);
    VERIFY(r == q);
}

//=============================================================================
// 21_strings
//=============================================================================

//=============================================================================
// basic_string\dr2268.cc
//=============================================================================
void basic_string_dr2268_test01()
{
    // PR libstdc++/84087

    std::string s0 = "string";
    std::string s;
    s.append(s0, 2);
    VERIFY(s == "ring");
    s.assign(s0, 3);
    VERIFY(s == "ing");
    s.insert(2, s0, 4);
    VERIFY(s == "inngg");
    s.replace(2, 3, s0, 2);
    VERIFY(s == "inring");
    VERIFY(s.compare(2, 4, s0, 2) == 0);
}

//=============================================================================
// basic_string\init-list.cc
//=============================================================================
void basic_string_init_list_test01(void)
{
    std::string s1 = {'a', 'b', 'c'};
    VERIFY(s1 == "abc");

    s1 = {'d', 'e', 'f'};
    VERIFY(s1 == "def");

    s1 += {'g', 'h', 'i'};
    VERIFY(s1 == "defghi");

    s1.append({'j', 'k', 'l'});
    VERIFY(s1 == "defghijkl");

    s1.assign({'m', 'n', 'o'});
    VERIFY(s1 == "mno");

    // There aren't actually overloads of insert and replace taking size_type
    // and initializer_list, but test the usage anyway.
    s1.insert(2, {'p', 'q', 'r'});
    VERIFY(s1 == "mnpqro");

    s1.replace(2, 3, {'s', 't', 'u'});
    VERIFY(s1 == "mnstuo");

    std::string::iterator i1, i2;

    i1 = s1.begin() + 2;
    s1.insert(i1, {'v', 'w', 'x'});
    VERIFY(s1 == "mnvwxstuo");

    i1 = s1.begin() + 2;
    i2 = i1 + 6;
    s1.replace(i1, i2, {'y', 'z'});
    VERIFY(s1 == "mnyzo");
}

//=============================================================================
// c_strings\char\1.cc
//=============================================================================
void c_strings_char_1_test01()
{
    char        c        = 'a';
    const char  cc       = 'b';
    char*       c1       = &c;
    const char* cc1      = &cc;
    const char* ccarray1 = "san francisco roof garden inspectors";
    const char* ccarray2 = "san francisco sunny-day park inspectors";
    char        carray[50];
    std::strcpy(carray, ccarray1);
    void*       v  = carray;
    const void* cv = ccarray1;

    // const char* strchr(const char* s, int c);
    // char* strchr(char* s, int c);
    cc1 = std::strchr(ccarray1, 'c');
    c1  = std::strchr(carray, 'c');

    // const char* strpbrk(const char* s1, const char* s2);
    // char* strpbrk(char* s1, const char* s2);
    cc1 = std::strpbrk(ccarray1, ccarray2);
    c1  = std::strpbrk(carray, ccarray2);

    // const char* strrchr(const char* s, int c);
    // char* strrchr(char* s, int c);
    cc1 = std::strrchr(ccarray1, 'c');
    c1  = std::strrchr(carray, 'c');

    // const char* strstr(const char* s1, const char* s2);
    // char* strstr(char* s1, const char* s2);
    cc1 = std::strstr(ccarray1, ccarray2);
    c1  = std::strstr(carray, carray);

    // const void* memchr(const void* s, int c, size_t n);
    // void* memchr(      void* s, int c, size_t n);
    cv = std::memchr(cv, 'a', 3);
    v  = std::memchr(v, 'a', 3);

    cc1 = cc1; // Suppress unused warnings.
    c1  = c1;
}

//=============================================================================
// c_strings\wchar_t\1.cc
//=============================================================================
void c_strings_wchar_t_1_test01()
{
    wchar_t        c        = L'a';
    const wchar_t  cc       = L'b';
    wchar_t*       c1       = &c;
    const wchar_t* cc1      = &cc;
    const wchar_t* ccarray1 = L"san francisco roof garden inspectors";
    const wchar_t* ccarray2 = L"san francisco sunny-day park inspectors";
    wchar_t        carray[50];
    std::wcscpy(carray, ccarray1);

    // const wchar_t* wcschr(const wchar_t* s, wchar_t c);
    // wchar_t* wcschr(wchar_t* s, wchar_t c);
    cc1 = std::wcschr(ccarray1, L'c');
    c1  = std::wcschr(carray, L'c');

    // const char* wcspbrk(const wchar_t* s1, const wchar_t* s2);
    // char* wcspbrk(wchar_t* s1, const wchar_t* s2);
    cc1 = std::wcspbrk(ccarray1, ccarray2);
    c1  = std::wcspbrk(carray, ccarray2);

    // const wchar_t* strrchr(const wchar_t* s, wchar_t c);
    // wchar_t* strrchr(wchar_t* s, wchar_t c);
    cc1 = std::wcsrchr(ccarray1, L'c');
    c1  = std::wcsrchr(carray, L'c');

    // const wchar_t* strstr(const wchar_t* s1, const wchar_t* s2);
    // wchar_t* strstr(wchar_t* s1, const wchar_t* s2);
    cc1 = std::wcsstr(ccarray1, ccarray2);
    c1  = std::wcsstr(carray, carray);

    // const wchar_t* wmemchr(const wchar_t* s, wchar_t c, size_t n);
    // wchar_t* wmemchr(      wchar_t* s, wchar_t c, size_t n);
    cc1 = std::wmemchr(ccarray1, L'a', 3);
    c1  = std::wmemchr(carray, L'a', 3);

    cc1 = cc1; // Suppress unused warnings.
    c1  = c1;
}

//=============================================================================
// 22_locale
//=============================================================================

//=============================================================================
// time_get\get\char\1.cc
//=============================================================================

void time_get_get_char_1_test01()
{
    using namespace std;

    locale loc_c = locale::classic();

    istringstream iss;
    iss.imbue(loc_c);
    const time_get<char>&             tget = use_facet<time_get<char>>(iss.getloc());
    typedef istreambuf_iterator<char> iter;
    const iter                        end;

    tm                time;
    ios_base::iostate err = ios_base::badbit;

    // check regular operations with format string
    TESTHEAD("regular operations");
    iss.str("d 2014-04-14 01:09:35");
    string format = "d %Y-%m-%d %H:%M:%S";
    auto   ret    = tget.get(iter(iss), end, iss, err, &time, format.data(), format.data() + format.size());
    PRINT(err);
    VERIFY(err == ios_base::eofbit);
    VERIFY(ret == end);
    PRINT(time.tm_year);
    VERIFY(time.tm_year == 114);
    PRINT(time.tm_mon);
    VERIFY(time.tm_mon == 3);
    PRINT(time.tm_mday);
    VERIFY(time.tm_mday == 14);
    PRINT(time.tm_hour);
    VERIFY(time.tm_hour == 1);
    PRINT(time.tm_min);
    VERIFY(time.tm_min == 9);
    PRINT(time.tm_sec);
    VERIFY(time.tm_sec == 35);

    TESTHEAD("check eof");
    iss.str("2020  ");
    format = "%Y";
    ret    = tget.get(iter(iss), end, iss, err, &time, format.data(), format.data() + format.size());
    VERIFY(err != ios_base::eofbit);
    VERIFY(time.tm_year == 120);
    VERIFY(ret != end);

    TESTHEAD("check broken format");
    iss.str("2014-04-14 01:09:35");
    format = "%";
    ret    = tget.get(iter(iss), end, iss, err, &time, format.data(), format.data() + format.size());
    VERIFY(err == ios_base::failbit);

    TESTHEAD("check single format letter version");
    iss.str("2020");
    ret = tget.get(iter(iss), end, iss, err, &time, 'Y');
    VERIFY(err == ios_base::eofbit);
    VERIFY(time.tm_year == 120);
    VERIFY(ret == end);

    TESTHEAD("check skipping of space");
    iss.str("2010    07 01");
    format = "%Y %m %d";
    ret    = tget.get(iter(iss), end, iss, err, &time, format.data(), format.data() + format.size());
    VERIFY(err == ios_base::eofbit);
    VERIFY(time.tm_year == 110);
    VERIFY(time.tm_mon == 6);
    VERIFY(time.tm_mday == 1);
    VERIFY(ret == end);

    TESTHEAD("check mismatch");
    iss.str("year: 1970");
    format = "jahr: %Y";
    ret    = tget.get(iter(iss), end, iss, err, &time, format.data(), format.data() + format.size());
    VERIFY(err == ios_base::failbit);
    VERIFY(ret == iter(iss));

    TESTHEAD("check case insensitive match");
    iss.str("yEaR: 1980");
    format = "YeAR: %Y";
    ret    = tget.get(iter(iss), end, iss, err, &time, format.data(), format.data() + format.size());
    VERIFY(err == ios_base::eofbit);
    VERIFY(ret == end);
    VERIFY(time.tm_year == 80);
}

//=============================================================================
// time_put\put\char\1.cc
//=============================================================================
void time_put_put_char_1_test01()
{
    using namespace std;
    // typedef ostreambuf_iterator<char> iterator_type;

    // create "C" time objects
    const tm time1 = __gnu_test::test_tm(0, 0, 12, 4, 3, 71, 0, 93, 0);

    // basic construction
    locale loc_c = locale::classic();

    // create an ostream-derived object, cache the time_put facet
    const string  empty;
    ostringstream oss;
    oss.imbue(loc_c);
    const time_put<char>& tim_put = use_facet<time_put<char>>(oss.getloc());

    // 1
    // iter_type
    // put(iter_type s, ios_base& str, char_type fill, const tm* t,
    //	 char format, char modifier = 0) const;
    oss.str(empty);
    tim_put.put(oss.rdbuf(), oss, '*', &time1, 'a');
    string result1 = oss.str();
    VERIFY(result1 == "Sun");

    oss.str(empty);
    tim_put.put(oss.rdbuf(), oss, '*', &time1, 'x');
    string result21 = oss.str(); // "04/04/71"
    VERIFY(result21 == "04/04/71");

    oss.str(empty);
    tim_put.put(oss.rdbuf(), oss, '*', &time1, 'X');
    string result22 = oss.str(); // "12:00:00"
    VERIFY(result22 == "12:00:00");

    oss.str(empty);
    tim_put.put(oss.rdbuf(), oss, '*', &time1, 'x', 'E');
    string result31 = oss.str(); // "04/04/71"
    VERIFY(result31 == "04/04/71");

    oss.str(empty);
    tim_put.put(oss.rdbuf(), oss, '*', &time1, 'X', 'E');
    string result32 = oss.str(); // "12:00:00"
    VERIFY(result32 == "12:00:00");
}

//=============================================================================
// 23_containers
//=============================================================================

//=============================================================================
// array\range_access.cc
//=============================================================================
void array_range_access_test01()
{
    std::array<int, 3> a{{1, 2, 3}};
    std::begin(a);
    std::end(a);
}

//=============================================================================
// bitset\to_string\1.cc
//=============================================================================
void bitset_to_string_1_test01()
{
    using namespace std;

    bitset<5> b5;
    string    s0 = b5.to_string<char, char_traits<char>, allocator<char>>();
    VERIFY(s0 == "00000");

    // DR 434. bitset::to_string() hard to use.
    b5.set(0);
    string s1 = b5.to_string<char, char_traits<char>>();
    VERIFY(s1 == "00001");

    b5.set(2);
    string s2 = b5.to_string<char>();
    VERIFY(s2 == "00101");

    b5.set(4);
    string s3 = b5.to_string();
    VERIFY(s3 == "10101");
}

//=============================================================================
// deque\range_access.cc
//=============================================================================
void deque_range_access_test01()
{
    std::deque<int> d{1, 2, 3};
    std::begin(d);
    std::end(d);
}

//=============================================================================
// forward_list\range_access.cc
//=============================================================================
void forward_list_range_access_test01()
{
    std::forward_list<int> fl{1, 2, 3};
    std::begin(fl);
    std::end(fl);
}

//=============================================================================
// list\range_access.cc
//=============================================================================
void list_range_access_test01()
{
    std::list<int> l{1, 2, 3};
    std::begin(l);
    std::end(l);
}

//=============================================================================
// map\range_access.cc
//=============================================================================
void map_range_access_test01()
{
    std::map<int, double> m{{1, 1.0}, {2, 2.0}, {3, 3.0}};
    std::begin(m);
    std::end(m);
}

//=============================================================================
// multimap\range_access.cc
//=============================================================================
void multimap_range_access_test01()
{
    std::multimap<int, double> mm{{1, 1.0}, {2, 2.0}, {3, 3.0}};
    std::begin(mm);
    std::end(mm);
}

//=============================================================================
// multiset\range_access.cc
//=============================================================================
void multiset_range_access_test01()
{
    std::multiset<int> ms{1, 2, 3};
    std::begin(ms);
    std::end(ms);
}

//=============================================================================
// priority_queue\67085.cc
//=============================================================================
unsigned count;

struct CopyCounter : std::less<int>
{
    CopyCounter() = default;
    CopyCounter(const CopyCounter&)
    {
        ++count;
    }
    CopyCounter(CopyCounter&&) = default;
};

void priority_queue_67085_test01()
{
    int i;
    count = 0;
    std::priority_queue<int, std::vector<int>, CopyCounter> q{&i, &i};
    VERIFY(count == 2);
    q.push(1);
    VERIFY(count == 3);
}

//=============================================================================
// set\range_access.cc
//=============================================================================
void set_range_access_test01()
{
    std::set<int> s{1, 2, 3};
    std::begin(s);
    std::end(s);
}

//=============================================================================
// vector\range_access.cc
//=============================================================================
void vector_range_access_test01()
{
    std::vector<double> v{1.0, 2.0, 3.0};
    std::begin(v);
    std::end(v);

    std::vector<bool> vb{true, false, true};
    std::begin(vb);
    std::end(vb);
}

//=============================================================================
// 24_iterators
//=============================================================================

//=============================================================================
// back_insert_iterator\2.cc
//=============================================================================
void back_insert_iterator_2_test02()
{
    typedef std::back_insert_iterator<std::list<int>> iterator_type;
    std::list<int>                                    li;
    iterator_type                                     it = std::back_inserter(li);
    it                                                   = it; // Suppress unused warning.
}

//=============================================================================
// front_insert_iterator\2.cc
//=============================================================================
void front_insert_iterator_2_test02()
{
    typedef std::front_insert_iterator<std::list<int>> iterator_type;
    std::list<int>                                     li;
    iterator_type                                      it = std::front_inserter(li);
    it                                                    = it; // Suppress unused warning.
}

//=============================================================================
// insert_iterator\2.cc
//=============================================================================
void insert_iterator_2_test02()
{
    typedef std::insert_iterator<std::list<int>> iterator_type;

    std::list<int>           li;
    std::list<int>::iterator liit = li.begin();
    iterator_type            it01(li, liit);
    iterator_type            it02 = std::inserter(li, liit);
    it01                          = it01; // Suppress unused warnings.
    it02                          = it02;
}

//=============================================================================
// istreambuf_iterator\2.cc
//=============================================================================
void istreambuf_iterator_2_test02(void)
{
    typedef std::istreambuf_iterator<char> cistreambuf_iter;
    const char                             slit01[] = "playa hermosa, liberia, guanacaste";
    std::string                            str01(slit01);
    std::istringstream                     istrs00(str01);
    std::istringstream                     istrs01(str01);

    // ctor sanity checks
    cistreambuf_iter istrb_it01(istrs00);
    cistreambuf_iter istrb_eos;
    VERIFY(istrb_it01 != istrb_eos);

    std::string tmp(istrb_it01, istrb_eos);
    VERIFY(tmp == str01);

    VERIFY(istrb_it01 == istrb_eos);

    cistreambuf_iter istrb_it03(0);
    cistreambuf_iter istrb_it04;
    VERIFY(istrb_it03 == istrb_it04);

    cistreambuf_iter istrb_it05(istrs01);
    cistreambuf_iter istrb_it06(istrs01.rdbuf());
    VERIFY(istrb_it05 == istrb_it06);

    // bool equal(istreambuf_iter& b)
    cistreambuf_iter istrb_it07(0);
    cistreambuf_iter istrb_it08;
    VERIFY(istrb_it07.equal(istrb_it08));
    cistreambuf_iter istrb_it09(0);
    cistreambuf_iter istrb_it10;
    VERIFY(istrb_it10.equal(istrb_it09));

    cistreambuf_iter istrb_it11(istrs01);
    cistreambuf_iter istrb_it12(istrs01.rdbuf());
    VERIFY(istrb_it11.equal(istrb_it12));
    cistreambuf_iter istrb_it13(istrs01);
    cistreambuf_iter istrb_it14(istrs01.rdbuf());
    VERIFY(istrb_it14.equal(istrb_it13));

    cistreambuf_iter istrb_it15(istrs01);
    cistreambuf_iter istrb_it16;
    VERIFY(!(istrb_it15.equal(istrb_it16)));
    cistreambuf_iter istrb_it17(istrs01);
    cistreambuf_iter istrb_it18;
    VERIFY(!(istrb_it18.equal(istrb_it17)));

    // bool operator==(const istreambuf_iterator&a, const istreambuf_iterator& b)
    // bool operator!=(const istreambuf_iterator&a, const istreambuf_iterator& b)
    cistreambuf_iter istrb_it19(0);
    cistreambuf_iter istrb_it20;
    VERIFY(istrb_it19 == istrb_it20);

    cistreambuf_iter istrb_it21(istrs01);
    cistreambuf_iter istrb_it22(istrs01.rdbuf());
    VERIFY(istrb_it22 == istrb_it21);

    cistreambuf_iter istrb_it23(istrs01);
    cistreambuf_iter istrb_it24;
    VERIFY(istrb_it23 != istrb_it24);

    cistreambuf_iter istrb_it25(0);
    cistreambuf_iter istrb_it26(istrs01.rdbuf());
    VERIFY(istrb_it25 != istrb_it26);

    // charT operator*() const
    // istreambuf_iterator& operator++();
    // istreambuf_iterator& operator++(int);
    cistreambuf_iter istrb_it27(istrs01.rdbuf());
    char             c;
    for (std::size_t i = 0; i < sizeof(slit01) - 2; ++i)
    {
        c = *istrb_it27++;
        VERIFY(c == slit01[i]);
    }

    std::istringstream istrs02(str01);
    cistreambuf_iter   istrb_it28(istrs02);
    for (std::size_t i = 0; i < sizeof(slit01) - 2;)
    {
        c = *++istrb_it28;
        VERIFY(c == slit01[++i]);
    }
}

//=============================================================================
// istream_iterator\2.cc
//=============================================================================
void istream_iterator_2_test02()
{
    using namespace std;

    string st("R.Rorty");

    string re_01, re_02, re_03;
    re_02 = ",H.Putnam";
    re_03 = "D.Dennett,xxx,H.Putnam";

    stringbuf              sb_01(st);
    istream                is_01(&sb_01);
    istream_iterator<char> inb_01(is_01);
    istream_iterator<char> ine_01;
    re_01.assign(inb_01, ine_01);
    VERIFY(re_01 == "R.Rorty");

    stringbuf              sb_02(st);
    istream                is_02(&sb_02);
    istream_iterator<char> inb_02(is_02);
    istream_iterator<char> ine_02;
    re_02.insert(re_02.begin(), inb_02, ine_02);
    VERIFY(re_02 == "R.Rorty,H.Putnam");

    stringbuf              sb_03(st);
    istream                is_03(&sb_03);
    istream_iterator<char> inb_03(is_03);
    istream_iterator<char> ine_03;
    re_03.replace(re_03.begin() + 10, re_03.begin() + 13, inb_03, ine_03);
    VERIFY(re_03 == "D.Dennett,R.Rorty,H.Putnam");
}

//=============================================================================
// move_iterator\dr2061.cc
//=============================================================================
void move_iterator_dr2061_test01()
{
    int a[] = {1, 2, 3, 4};
    std::make_move_iterator(a + 4);
    std::make_move_iterator(a);
}

//=============================================================================
// normal_iterator\58403.cc
//=============================================================================
struct A
{
    static constexpr std::iterator_traits<std::string::iterator>::difference_type a = 1;
};

void normal_iterator_58403_main()
{
    std::string s  = "foo";
    auto        it = s.begin();
    it += A::a;
}

//=============================================================================
// operations\next.cc
//=============================================================================
void operations_next_test01()
{
    std::vector<int>           c(3);
    std::vector<int>::iterator i = c.begin(), j;

    j = std::next(i, 3);
    VERIFY(i == c.begin());
    VERIFY(j == c.end());
}

void operations_next_test02()
{
    std::list<int>           c(3);
    std::list<int>::iterator i = c.begin(), j;

    j = std::next(i, 3);
    VERIFY(i == c.begin());
    VERIFY(j == c.end());
}

//=============================================================================
// operations\prev.cc
//=============================================================================
void operations_prev_test01()
{
    std::vector<int>           c(3);
    std::vector<int>::iterator i = c.end(), j;

    j = std::prev(i, 3);
    VERIFY(i == c.end());
    VERIFY(j == c.begin());
}

void operations_prev_test02()
{
    std::list<int>           c(3);
    std::list<int>::iterator i = c.end(), j;

    j = std::prev(i, 3);
    VERIFY(i == c.end());
    VERIFY(j == c.begin());
}

//=============================================================================
// ostreambuf_iterator\2.cc
//=============================================================================
void ostreambuf_iterator_2_test02(void)
{
    typedef std::ostreambuf_iterator<char> costreambuf_iter;
    // typedef costreambuf_iter::streambuf_type cstreambuf_type;
    const char         slit01[] = "playa hermosa, liberia, guanacaste";
    const char         slit02[] = "bodega bay, lost coast, california";
    std::string        str01(slit01);
    std::string        str02(slit02);
    std::string        tmp;
    std::stringbuf     strbuf01;
    std::stringbuf     strbuf02(str01);
    std::ostringstream ostrs00(str01);
    std::ostringstream ostrs01(str01);

    // ctor sanity checks
    costreambuf_iter ostrb_it01(ostrs00);
    VERIFY(!ostrb_it01.failed());
    ostrb_it01++;
    ++ostrb_it01;
    VERIFY(!ostrb_it01.failed());
    ostrb_it01 = 'a';
    VERIFY(!ostrb_it01.failed());
    *ostrb_it01;
    VERIFY(!ostrb_it01.failed());

    costreambuf_iter ostrb_it02(0);
    VERIFY(ostrb_it02.failed());
    ostrb_it02++;
    ++ostrb_it02;
    VERIFY(ostrb_it02.failed());
    *ostrb_it02;
    VERIFY(ostrb_it02.failed());
    ostrb_it02 = 'a';
    VERIFY(ostrb_it02.failed());

    // charT operator*() const
    // ostreambuf_iterator& operator++();
    // ostreambuf_iterator& operator++(int);
    costreambuf_iter ostrb_it27(ostrs01);
    VERIFY(!ostrb_it27.failed());
    int j = str02.size();
    for (int i = 0; i < j; ++i)
        ostrb_it27 = str02[i];
    VERIFY(!ostrb_it27.failed());
    tmp = ostrs01.str();
    VERIFY(tmp != str01);
    VERIFY(tmp == str02);

    costreambuf_iter ostrb_it28(ostrs00);
    VERIFY(!ostrb_it28.failed());
    j = ostrs00.str().size();
    for (int i = 0; i < j + 2; ++i)
        ostrb_it28 = 'b';
    VERIFY(!ostrb_it28.failed());
    tmp = ostrs00.str();
    VERIFY(tmp != str01);
    VERIFY(tmp != str02);
}

//=============================================================================
// random_access_iterator\26020.cc
//=============================================================================
void random_access_iterator_26020_test01()
{
    using namespace std;

    list<int> ll;
    ll.push_back(1);

    list<int>::iterator it(ll.begin());

    advance(it, 0.5);

    VERIFY(it == ll.begin());
}

//=============================================================================
// reverse_iterator\2.cc
//=============================================================================
void reverse_iterator_2_test02()
{
    typedef std::reverse_iterator<int*> iterator_type;
    iterator_type                       it01;
    iterator_type                       it02;

    // Sanity check non-member operators and functions can be instantiated.
    it01 == it02;
    it01 != it02;
    it01 < it02;
    it01 <= it02;
    it01 > it02;
    it01 >= it02;
    it01 - it02;
    5 + it02;
}

//=============================================================================
// 25_algorithms
//=============================================================================

//=============================================================================
// fill\1.cc
//=============================================================================
class num
{
    int stored;

   public:
    num(int init = 0) : stored(init) {}

    operator int() const
    {
        return stored;
    }
};

// fill
void fill_1_test01()
{
    using namespace std;

    const int val = 1;

    const int                  V[] = {val, val, val, val, val, val, val};
    const list<int>::size_type N   = sizeof(V) / sizeof(int);

    list<int> coll(N);
    fill(coll.begin(), coll.end(), val);
    VERIFY(equal(coll.begin(), coll.end(), V));

    list<num> coll2(N);
    fill(coll2.begin(), coll2.end(), val);
    VERIFY(equal(coll2.begin(), coll2.end(), V));
}

//=============================================================================
// is_heap\1.cc
//=============================================================================
int       is_heap_A[] = {9, 8, 6, 7, 7, 5, 5, 3, 6, 4, 1, 2, 3, 4};
int       is_heap_B[] = {1, 3, 2, 4, 4, 6, 3, 5, 5, 7, 7, 6, 8, 9};
const int is_heap_N   = sizeof(A) / sizeof(int);

void is_heap_1_test01()
{
    for (int i = 0; i <= is_heap_N; ++i)
    {
        VERIFY(std::is_heap(is_heap_A, is_heap_A + i));
        VERIFY(std::is_heap(is_heap_A, is_heap_A + i, std::less<int>()));
        VERIFY(std::is_heap(is_heap_B, is_heap_B + i, std::greater<int>()));
        VERIFY((i < 2) || !std::is_heap(is_heap_B, is_heap_B + i));
    }
}

//=============================================================================
// is_sorted\1.cc
//=============================================================================
int       is_sorted_A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int       is_sorted_B[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
const int is_sorted_N   = sizeof(A) / sizeof(int);

void is_sorted_1_test01()
{
    for (int i = 0; i <= is_sorted_N; ++i)
    {
        VERIFY(std::is_sorted(is_sorted_A, is_sorted_A + i));
        VERIFY(std::is_sorted(is_sorted_A, is_sorted_A + i, std::less<int>()));
        VERIFY(std::is_sorted(is_sorted_B, is_sorted_B + i, std::greater<int>()));
        VERIFY((i < 2) || !std::is_sorted(is_sorted_B, is_sorted_B + i));
    }
}

//=============================================================================
// minmax\1.cc
//=============================================================================
void minmax_1_test01()
{
    std::pair<const int, const int> z = std::minmax(1, 2);
    std::pair<const int, const int> w = std::minmax(4, 3);
    VERIFY(z.first == 1);
    VERIFY(z.second == 2);
    VERIFY(w.first == 3);
    VERIFY(w.second == 4);

    std::pair<const int, const int> zc = std::minmax(1, 2, std::greater<int>());
    std::pair<const int, const int> wc = std::minmax(4, 3, std::greater<int>());
    VERIFY(zc.first == 2);
    VERIFY(zc.second == 1);
    VERIFY(wc.first == 4);
    VERIFY(wc.second == 3);
}

//=============================================================================
// search_n\58358.cc
//=============================================================================
void search_n_58358_test01()
{
    std::vector<int> a{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    int              count = 0;
    std::search_n(a.begin(), a.end(), 10, 1,
                  [&count](int t, int u)
                  {
                      ++count;
                      return t == u;
                  });
    VERIFY(count <= 11);
}

//=============================================================================
// sort\1.cc
//=============================================================================
const int sort_A[]  = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
const int sort_B[]  = {10, 20, 1, 11, 2, 12, 3, 13, 4, 14, 5, 15, 6, 16, 7, 17, 8, 18, 9, 19};
const int sort_C[]  = {20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
const int sort_N    = sizeof(A) / sizeof(int);
const int sort_logN = 3; // ln(N) rounded up
const int sort_P    = 7;

// comparison predicate for stable_sort: order by rightmost digit
struct CompLast
{
    bool operator()(const int x, const int y)
    {
        return x % 10 < y % 10;
    }
};

// This functor has the equivalent functionality of std::geater<>,
// but there is no dependency on <functional> and it also tracks the
// number of invocations since creation.
class Gt
{
   public:
    static int count()
    {
        return itsCount;
    }
    static void reset()
    {
        itsCount = 0;
    }

    bool operator()(const int& x, const int& y)
    {
        ++itsCount;
        return x > y;
    }

   private:
    static int itsCount;
};

int Gt::itsCount = 0;

// 25.3.1.1 sort()
void sort_1_test01()
{
    int s1[sort_N];
    std::copy(sort_B, sort_B + sort_N, s1);
    VERIFY(std::equal(s1, s1 + sort_N, sort_B));

    std::sort(s1, s1 + sort_N);
    VERIFY(std::equal(s1, s1 + sort_N, sort_A));

    Gt gt;
    gt.reset();
    std::sort(s1, s1 + sort_N, gt);
    VERIFY(std::equal(s1, s1 + sort_N, sort_C));
}

//=============================================================================
// 26_numerics
//=============================================================================

//=============================================================================
// accumulate\1.cc
//=============================================================================
int       accumulate_A[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const int accumulate_NA  = sizeof(accumulate_A) / sizeof(int);

void accumulate_1_test01()
{
    int res = std::accumulate(accumulate_A, accumulate_A + accumulate_NA, 11);
    VERIFY(res == 66);
}

bool      accumulate_B[] = {true, false, true, true, false, true, false, true, true, false};
const int accumulate_NB  = sizeof(accumulate_B) / sizeof(bool);

void accumulate_1_test02()
{
    int res = std::accumulate(accumulate_B, accumulate_B + accumulate_NB, 100);
    VERIFY(res == 106);
}

//=============================================================================
// random\minstd_rand.cc
//=============================================================================
void random_minstd_rand_test01()
{
    std::minstd_rand a;
    a.discard(9999);

    VERIFY(a() == 399268537);
}

//=============================================================================
// slice\1.cc
//=============================================================================
bool construction(std::size_t start, std::size_t size, std::size_t stride)
{
    std::slice s(start, size, stride);
    return s.start() == start && s.size() == size && s.stride() == stride;
}

bool copy(std::size_t start, std::size_t size, std::size_t stride)
{
    std::slice s(start, size, stride);
    std::slice t = s;
    return t.start() == start && t.size() == size && t.stride() == stride;
}

bool assignment(std::size_t start, std::size_t size, std::size_t stride)
{
    std::slice s(start, size, stride);
    std::slice t;
    t = s;
    return t.start() == start && t.size() == size && t.stride() == stride;
}

int slice_1_main()
{
    std::srand(20020717);
    using std::rand;
    VERIFY(construction(rand(), rand(), rand()));

    VERIFY(copy(rand(), rand(), rand()));

    VERIFY(assignment(rand(), rand(), rand()));

    return 0;
}

//=============================================================================
// valarray\range_access.cc
//=============================================================================
void valarray_range_access_test01()
{
    std::valarray<double> va{1.0, 2.0, 3.0};
    std::begin(va);
    std::end(va);
    const auto& cva = va;
    std::begin(cva);
    std::end(cva);
}

//=============================================================================
// 27_io
//=============================================================================

//=============================================================================
// basic_ostream\endl\char\1.cc
//=============================================================================
void basic_ostream_endl_char_1_test01(void)
{
    const std::string str01(" santa barbara ");
    std::string       str04;
    std::string       str05;

    std::ostringstream oss01(str01);
    std::ostringstream oss02;
    // typedef std::ostringstream::traits_type traits_type;

    // template<_CharT, _Traits>
    //  basic_ostream<_CharT, _Traits>& ends(basic_ostream<_Char, _Traits>& os)
    oss01 << std::endl;
    str04 = oss01.str();
    VERIFY(str04.size() == str01.size());

    oss02 << std::endl;
    str05 = oss02.str();
    VERIFY(str05.size() == 1);
}

//=============================================================================
// basic_ostream\endl\wchar_t\1.cc
//=============================================================================
void basic_ostream_endl_wchar_t_1_test01(void)
{
    const std::wstring str01(L" santa barbara ");
    std::wstring       str04;
    std::wstring       str05;

    std::wostringstream oss01(str01);
    std::wostringstream oss02;
    // typedef std::wostringstream::traits_type traits_type;

    // template<_CharT, _Traits>
    //  basic_ostream<_CharT, _Traits>& ends(basic_ostream<_Char, _Traits>& os)
    oss01 << std::endl;
    str04 = oss01.str();
    VERIFY(str04.size() == str01.size());

    oss02 << std::endl;
    str05 = oss02.str();
    VERIFY(str05.size() == 1);
}

//=============================================================================
// ios_base\state\1.cc
//=============================================================================
void ios_base_state_1_test02()
{
    const std::string strue("true");
    const std::string sfalse("false");
    std::string       str01;
    std::string       str02;

    std::locale        loc_c = std::locale::classic();
    std::ostringstream ostr01;
    ostr01.imbue(loc_c);
    ostr01.flags(std::ios_base::boolalpha);

    ostr01 << true;
    str02 = ostr01.str();
    VERIFY(str02 == strue);

    ostr01.str(str01);
    ostr01 << false;
    str02 = ostr01.str();
    VERIFY(str02 == sfalse);
}

//=============================================================================
// objects\char\1.cc
//=============================================================================
// Make sure all the standard streams are defined.
void objects_char_1_test01()
{
    char                          array1[20];
    typedef std::ios::traits_type ctraits_type;
    ctraits_type::int_type        i = 15;
    ctraits_type::copy(array1, "testing istream", i);
    array1[i] = '\0';
    std::cout << "testing cout" << std::endl;
    std::cerr << "testing cerr" << std::endl;
    VERIFY(std::cerr.flags() & std::ios_base::unitbuf);
    std::clog << "testing clog" << std::endl;
    // std::cin >> array1; // requires somebody to type something in.
    VERIFY(std::cin.tie() == &std::cout);
}

//=============================================================================
// objects\wchar_t\1.cc
//=============================================================================
// Make sure all the standard streams are defined.
void objects_wchar_t_1_test01()
{
    wchar_t                        array2[20];
    typedef std::wios::traits_type wtraits_type;
    wtraits_type::int_type         wi = 15;
    wtraits_type::copy(array2, L"testing istream", wi);
    std::wcout << L"testing wcout" << std::endl;
    std::wcerr << L"testing wcerr" << std::endl;
    VERIFY(std::wcerr.flags() & std::ios_base::unitbuf);
    std::wclog << L"testing wclog" << std::endl;
    // std::wcin >> array2; // requires somebody to type something in.
    VERIFY(std::wcin.tie() == &std::wcout);
}

//=============================================================================
// 28_regex
//=============================================================================

//=============================================================================
// basic_regex\85098.cc
//=============================================================================
void f(const std::regex_constants::syntax_option_type&) {}

void basic_regex_85098_test01()
{
    f(std::regex::icase);
    f(std::regex::nosubs);
    f(std::regex::optimize);
    f(std::regex::collate);
    f(std::regex::ECMAScript);
    f(std::regex::basic);
    f(std::regex::extended);
    f(std::regex::awk);
    f(std::regex::grep);
    f(std::regex::egrep);
    // f(std::regex::multiline);
}

//=============================================================================
// init-list.cc
//=============================================================================
void init_list_test01(void)
{
    using namespace std;
    regex  r = {'a', 'b', 'c'};
    cmatch res;
    VERIFY(regex_match("abc", res, r));
    VERIFY(!regex_match("ab", res, r));

    r = {'d', 'e', 'f'};
    VERIFY(regex_match("def", res, r));
    VERIFY(!regex_match("abc", res, r));
}

//=============================================================================
// sub_match\cast_char.cc
//=============================================================================
void sub_match_cast_char_main()
{
    typedef char                          value_type;
    typedef std::basic_string<value_type> string_type;
    typedef std::sub_match<value_type*>   sub_match_type;
    value_type                            test_data[] = "cabbage";

    sub_match_type sm;
    sm.first   = std::begin(test_data);
    sm.second  = std::end(test_data) - 1;
    sm.matched = true;

    string_type sm_string = sm;

    VERIFY(sm_string == string_type(test_data));
}

//=============================================================================
// 29_atomics
//=============================================================================

//=============================================================================
// atomic\65913.cc
//=============================================================================
// PR libstdc++/65913

void atomic_65913_test01()
{
    struct Int
    {
        int i;
    };
    VERIFY(std::atomic<Int>{}.is_lock_free());
    VERIFY(std::atomic<int>{}.is_lock_free());
    VERIFY(std::atomic<int*>{}.is_lock_free());
}

//=============================================================================
// atomic\nonmembers.cc
//=============================================================================
void atomic_nonmembers_test01()
{
    struct X
    {
    };
    struct Y
    {
    };
    // Primary template
    volatile std::atomic<X> v;
    std::atomic<Y>          a;
    const std::memory_order mo = std::memory_order_seq_cst;
    X                       x;
    Y                       y;
    auto                    r1 = atomic_is_lock_free(&v);
    static_assert(std::is_same<decltype(r1), bool>::value, "");
    auto r2 = atomic_is_lock_free(&a);
    static_assert(std::is_same<decltype(r2), bool>::value, "");
    atomic_init(&v, x);
    atomic_init(&a, y);
    atomic_store(&v, x);
    atomic_store(&a, y);
    atomic_store_explicit(&v, x, mo);
    atomic_store_explicit(&a, y, mo);
    auto r3 = atomic_load(&v);
    static_assert(std::is_same<decltype(r3), X>::value, "");
    auto r4 = atomic_load(&a);
    static_assert(std::is_same<decltype(r4), Y>::value, "");
    auto r5 = atomic_load_explicit(&v, mo);
    static_assert(std::is_same<decltype(r5), X>::value, "");
    auto r6 = atomic_load_explicit(&a, mo);
    static_assert(std::is_same<decltype(r6), Y>::value, "");
    auto r7 = atomic_exchange(&v, x);
    static_assert(std::is_same<decltype(r7), X>::value, "");
    auto r8 = atomic_exchange(&a, y);
    static_assert(std::is_same<decltype(r8), Y>::value, "");
    auto r9 = atomic_exchange_explicit(&v, x, mo);
    static_assert(std::is_same<decltype(r9), X>::value, "");
    auto r10 = atomic_exchange_explicit(&a, y, mo);
    static_assert(std::is_same<decltype(r10), Y>::value, "");
    auto r11 = atomic_compare_exchange_weak(&v, &x, x);
    static_assert(std::is_same<decltype(r11), bool>::value, "");
    auto r12 = atomic_compare_exchange_weak(&a, &y, y);
    static_assert(std::is_same<decltype(r12), bool>::value, "");
    auto r13 = atomic_compare_exchange_strong(&v, &x, x);
    static_assert(std::is_same<decltype(r13), bool>::value, "");
    auto r14 = atomic_compare_exchange_strong(&a, &y, y);
    static_assert(std::is_same<decltype(r14), bool>::value, "");
    auto r15 = atomic_compare_exchange_weak_explicit(&v, &x, x, mo, mo);
    static_assert(std::is_same<decltype(r15), bool>::value, "");
    auto r16 = atomic_compare_exchange_weak_explicit(&a, &y, y, mo, mo);
    static_assert(std::is_same<decltype(r16), bool>::value, "");
    auto r17 = atomic_compare_exchange_strong_explicit(&v, &x, x, mo, mo);
    static_assert(std::is_same<decltype(r17), bool>::value, "");
    auto r18 = atomic_compare_exchange_strong_explicit(&a, &y, y, mo, mo);
    static_assert(std::is_same<decltype(r18), bool>::value, "");
}

void atomic_nonmembers_test02()
{
    // Specialization for bool
    volatile std::atomic<bool> v;
    std::atomic<bool>          a;
    const std::memory_order    mo = std::memory_order_seq_cst;
    bool                       b  = false;
    auto                       r1 = atomic_is_lock_free(&v);
    static_assert(std::is_same<decltype(r1), bool>::value, "");
    auto r2 = atomic_is_lock_free(&a);
    static_assert(std::is_same<decltype(r2), bool>::value, "");
    atomic_init(&v, b);
    atomic_init(&a, b);
    atomic_store(&v, b);
    atomic_store(&a, b);
    atomic_store_explicit(&v, b, mo);
    atomic_store_explicit(&a, b, mo);
    auto r3 = atomic_load(&v);
    static_assert(std::is_same<decltype(r3), bool>::value, "");
    auto r4 = atomic_load(&a);
    static_assert(std::is_same<decltype(r4), bool>::value, "");
    auto r5 = atomic_load_explicit(&v, mo);
    static_assert(std::is_same<decltype(r5), bool>::value, "");
    auto r6 = atomic_load_explicit(&a, mo);
    static_assert(std::is_same<decltype(r6), bool>::value, "");
    auto r7 = atomic_exchange(&v, b);
    static_assert(std::is_same<decltype(r7), bool>::value, "");
    auto r8 = atomic_exchange(&a, b);
    static_assert(std::is_same<decltype(r8), bool>::value, "");
    auto r9 = atomic_exchange_explicit(&v, b, mo);
    static_assert(std::is_same<decltype(r9), bool>::value, "");
    auto r10 = atomic_exchange_explicit(&a, b, mo);
    static_assert(std::is_same<decltype(r10), bool>::value, "");
    auto r11 = atomic_compare_exchange_weak(&v, &b, b);
    static_assert(std::is_same<decltype(r11), bool>::value, "");
    auto r12 = atomic_compare_exchange_weak(&a, &b, b);
    static_assert(std::is_same<decltype(r12), bool>::value, "");
    auto r13 = atomic_compare_exchange_strong(&v, &b, b);
    static_assert(std::is_same<decltype(r13), bool>::value, "");
    auto r14 = atomic_compare_exchange_strong(&a, &b, b);
    static_assert(std::is_same<decltype(r14), bool>::value, "");
    auto r15 = atomic_compare_exchange_weak_explicit(&v, &b, b, mo, mo);
    static_assert(std::is_same<decltype(r15), bool>::value, "");
    auto r16 = atomic_compare_exchange_weak_explicit(&a, &b, b, mo, mo);
    static_assert(std::is_same<decltype(r16), bool>::value, "");
    auto r17 = atomic_compare_exchange_strong_explicit(&v, &b, b, mo, mo);
    static_assert(std::is_same<decltype(r17), bool>::value, "");
    auto r18 = atomic_compare_exchange_strong_explicit(&a, &b, b, mo, mo);
    static_assert(std::is_same<decltype(r18), bool>::value, "");
}

void atomic_nonmembers_test03()
{
    // Partial specialization for pointers
    volatile std::atomic<int*> v;
    std::atomic<long*>         a;
    const std::memory_order    mo = std::memory_order_seq_cst;
    int*                       i  = nullptr;
    long*                      l  = nullptr;
    auto                       r1 = atomic_is_lock_free(&v);
    static_assert(std::is_same<decltype(r1), bool>::value, "");
    auto r2 = atomic_is_lock_free(&a);
    static_assert(std::is_same<decltype(r2), bool>::value, "");
    atomic_init(&v, i);
    atomic_init(&a, l);
    atomic_store(&v, i);
    atomic_store(&a, l);
    atomic_store_explicit(&v, i, mo);
    atomic_store_explicit(&a, l, mo);
    auto r3 = atomic_load(&v);
    static_assert(std::is_same<decltype(r3), int*>::value, "");
    auto r4 = atomic_load(&a);
    static_assert(std::is_same<decltype(r4), long*>::value, "");
    auto r5 = atomic_load_explicit(&v, mo);
    static_assert(std::is_same<decltype(r5), int*>::value, "");
    auto r6 = atomic_load_explicit(&a, mo);
    static_assert(std::is_same<decltype(r6), long*>::value, "");
    auto r7 = atomic_exchange(&v, i);
    static_assert(std::is_same<decltype(r7), int*>::value, "");
    auto r8 = atomic_exchange(&a, l);
    static_assert(std::is_same<decltype(r8), long*>::value, "");
    auto r9 = atomic_exchange_explicit(&v, i, mo);
    static_assert(std::is_same<decltype(r9), int*>::value, "");
    auto r10 = atomic_exchange_explicit(&a, l, mo);
    static_assert(std::is_same<decltype(r10), long*>::value, "");
    auto r11 = atomic_compare_exchange_weak(&v, &i, i);
    static_assert(std::is_same<decltype(r11), bool>::value, "");
    auto r12 = atomic_compare_exchange_weak(&a, &l, l);
    static_assert(std::is_same<decltype(r12), bool>::value, "");
    auto r13 = atomic_compare_exchange_strong(&v, &i, i);
    static_assert(std::is_same<decltype(r13), bool>::value, "");
    auto r14 = atomic_compare_exchange_strong(&a, &l, l);
    static_assert(std::is_same<decltype(r14), bool>::value, "");
    auto r15 = atomic_compare_exchange_weak_explicit(&v, &i, i, mo, mo);
    static_assert(std::is_same<decltype(r15), bool>::value, "");
    auto r16 = atomic_compare_exchange_weak_explicit(&a, &l, l, mo, mo);
    static_assert(std::is_same<decltype(r16), bool>::value, "");
    auto r17 = atomic_compare_exchange_strong_explicit(&v, &i, i, mo, mo);
    static_assert(std::is_same<decltype(r17), bool>::value, "");
    auto r18 = atomic_compare_exchange_strong_explicit(&a, &l, l, mo, mo);
    static_assert(std::is_same<decltype(r18), bool>::value, "");

    auto r19 = atomic_fetch_add(&v, 1);
    static_assert(std::is_same<decltype(r19), int*>::value, "");
    auto r20 = atomic_fetch_add(&a, 1);
    static_assert(std::is_same<decltype(r20), long*>::value, "");
    auto r21 = atomic_fetch_add_explicit(&v, 1, mo);
    static_assert(std::is_same<decltype(r21), int*>::value, "");
    auto r22 = atomic_fetch_add_explicit(&a, 1, mo);
    static_assert(std::is_same<decltype(r22), long*>::value, "");
    auto r23 = atomic_fetch_sub(&v, 1);
    static_assert(std::is_same<decltype(r23), int*>::value, "");
    auto r24 = atomic_fetch_sub(&a, 1);
    static_assert(std::is_same<decltype(r24), long*>::value, "");
    auto r25 = atomic_fetch_sub_explicit(&v, 1, mo);
    static_assert(std::is_same<decltype(r25), int*>::value, "");
    auto r26 = atomic_fetch_sub_explicit(&a, 1, mo);
    static_assert(std::is_same<decltype(r26), long*>::value, "");
}

void atomic_nonmembers_test04()
{
    struct base
    {
    };
    struct derived : base
    {
    };
    // Partial specialization for pointers
    volatile std::atomic<base*> v;
    std::atomic<base*>          a;
    const std::memory_order     mo = std::memory_order_seq_cst;
    // Repeat tests with arguments of type different to value_type.
    derived* const p = nullptr;
    base*          b = nullptr;
    atomic_init(&v, p);
    atomic_init(&a, p);
    atomic_store(&v, p);
    atomic_store(&a, p);
    atomic_store_explicit(&v, p, mo);
    atomic_store_explicit(&a, p, mo);
    atomic_exchange(&v, p);
    atomic_exchange(&a, p);
    atomic_exchange_explicit(&v, p, mo);
    atomic_exchange_explicit(&a, p, mo);
    atomic_compare_exchange_weak(&v, &b, p);
    atomic_compare_exchange_weak(&a, &b, p);
    atomic_compare_exchange_strong(&v, &b, p);
    atomic_compare_exchange_strong(&a, &b, p);
    atomic_compare_exchange_weak_explicit(&v, &b, p, mo, mo);
    atomic_compare_exchange_weak_explicit(&a, &b, p, mo, mo);
    atomic_compare_exchange_strong_explicit(&v, &b, p, mo, mo);
    atomic_compare_exchange_strong_explicit(&a, &b, p, mo, mo);
}

//=============================================================================
// atomic_flag\test_and_set\explicit.cc
//=============================================================================
int atomic_flag_test_and_set_explicit_main()
{
    using namespace std;
    atomic_flag af = ATOMIC_FLAG_INIT;

    if (!af.test_and_set(memory_order_acquire))
        af.clear(memory_order_release);

    return 0;
}

//=============================================================================
// atomic_flag\test_and_set\implicit.cc
//=============================================================================
int atomic_flag_test_and_set_implicit_main()
{
    using namespace std;
    atomic_flag af = ATOMIC_FLAG_INIT;

    if (!af.test_and_set())
        af.clear();

    return 0;
}

//=============================================================================
// atomic_integral\nonmembers.cc
//=============================================================================
void atomic_integral_nonmembers_test01()
{
    volatile std::atomic<int> v;
    std::atomic<long>         a;
    const std::memory_order   mo = std::memory_order_seq_cst;
    int                       i  = 0;
    long                      l  = 0;
    auto                      r1 = atomic_is_lock_free(&v);
    static_assert(std::is_same<decltype(r1), bool>::value, "");
    auto r2 = atomic_is_lock_free(&a);
    static_assert(std::is_same<decltype(r2), bool>::value, "");
    atomic_init(&v, i);
    atomic_init(&a, l);
    atomic_store(&v, i);
    atomic_store(&a, l);
    atomic_store_explicit(&v, i, mo);
    atomic_store_explicit(&a, l, mo);
    auto r3 = atomic_load(&v);
    static_assert(std::is_same<decltype(r3), int>::value, "");
    auto r4 = atomic_load(&a);
    static_assert(std::is_same<decltype(r4), long>::value, "");
    auto r5 = atomic_load_explicit(&v, mo);
    static_assert(std::is_same<decltype(r5), int>::value, "");
    auto r6 = atomic_load_explicit(&a, mo);
    static_assert(std::is_same<decltype(r6), long>::value, "");
    auto r7 = atomic_exchange(&v, i);
    static_assert(std::is_same<decltype(r7), int>::value, "");
    auto r8 = atomic_exchange(&a, l);
    static_assert(std::is_same<decltype(r8), long>::value, "");
    auto r9 = atomic_exchange_explicit(&v, i, mo);
    static_assert(std::is_same<decltype(r9), int>::value, "");
    auto r10 = atomic_exchange_explicit(&a, l, mo);
    static_assert(std::is_same<decltype(r10), long>::value, "");
    auto r11 = atomic_compare_exchange_weak(&v, &i, i);
    static_assert(std::is_same<decltype(r11), bool>::value, "");
    auto r12 = atomic_compare_exchange_weak(&a, &l, l);
    static_assert(std::is_same<decltype(r12), bool>::value, "");
    auto r13 = atomic_compare_exchange_strong(&v, &i, i);
    static_assert(std::is_same<decltype(r13), bool>::value, "");
    auto r14 = atomic_compare_exchange_strong(&a, &l, l);
    static_assert(std::is_same<decltype(r14), bool>::value, "");
    auto r15 = atomic_compare_exchange_weak_explicit(&v, &i, i, mo, mo);
    static_assert(std::is_same<decltype(r15), bool>::value, "");
    auto r16 = atomic_compare_exchange_weak_explicit(&a, &l, l, mo, mo);
    static_assert(std::is_same<decltype(r16), bool>::value, "");
    auto r17 = atomic_compare_exchange_strong_explicit(&v, &i, i, mo, mo);
    static_assert(std::is_same<decltype(r17), bool>::value, "");
    auto r18 = atomic_compare_exchange_strong_explicit(&a, &l, l, mo, mo);
    static_assert(std::is_same<decltype(r18), bool>::value, "");

    auto r19 = atomic_fetch_add(&v, i);
    static_assert(std::is_same<decltype(r19), int>::value, "");
    auto r20 = atomic_fetch_add(&a, l);
    static_assert(std::is_same<decltype(r20), long>::value, "");
    auto r21 = atomic_fetch_add_explicit(&v, i, mo);
    static_assert(std::is_same<decltype(r21), int>::value, "");
    auto r22 = atomic_fetch_add_explicit(&a, l, mo);
    static_assert(std::is_same<decltype(r22), long>::value, "");
    auto r23 = atomic_fetch_sub(&v, i);
    static_assert(std::is_same<decltype(r23), int>::value, "");
    auto r24 = atomic_fetch_sub(&a, l);
    static_assert(std::is_same<decltype(r24), long>::value, "");
    auto r25 = atomic_fetch_sub_explicit(&v, i, mo);
    static_assert(std::is_same<decltype(r25), int>::value, "");
    auto r26 = atomic_fetch_sub_explicit(&a, l, mo);
    static_assert(std::is_same<decltype(r26), long>::value, "");
    auto r27 = atomic_fetch_and(&v, i);
    static_assert(std::is_same<decltype(r27), int>::value, "");
    auto r28 = atomic_fetch_and(&a, l);
    static_assert(std::is_same<decltype(r28), long>::value, "");
    auto r29 = atomic_fetch_and_explicit(&v, i, mo);
    static_assert(std::is_same<decltype(r29), int>::value, "");
    auto r30 = atomic_fetch_and_explicit(&a, l, mo);
    static_assert(std::is_same<decltype(r30), long>::value, "");
    auto r31 = atomic_fetch_or(&v, i);
    static_assert(std::is_same<decltype(r31), int>::value, "");
    auto r32 = atomic_fetch_or(&a, l);
    static_assert(std::is_same<decltype(r32), long>::value, "");
    auto r33 = atomic_fetch_or_explicit(&v, i, mo);
    static_assert(std::is_same<decltype(r33), int>::value, "");
    auto r34 = atomic_fetch_or_explicit(&a, l, mo);
    static_assert(std::is_same<decltype(r34), long>::value, "");
    auto r35 = atomic_fetch_xor(&v, i);
    static_assert(std::is_same<decltype(r35), int>::value, "");
    auto r36 = atomic_fetch_xor(&a, l);
    static_assert(std::is_same<decltype(r36), long>::value, "");
    auto r37 = atomic_fetch_xor_explicit(&v, i, mo);
    static_assert(std::is_same<decltype(r37), int>::value, "");
    auto r38 = atomic_fetch_xor_explicit(&a, l, mo);
    static_assert(std::is_same<decltype(r38), long>::value, "");
}

void atomic_integral_nonmembers_test02()
{
    volatile std::atomic<long> v;
    std::atomic<long>          a;
    std::memory_order          mo = std::memory_order_seq_cst;
    // Repeat tests with arguments of type different to value_type.
    const int i = 0;
    long      l = 0;
    atomic_init(&v, i);
    atomic_init(&a, i);
    atomic_store(&v, i);
    atomic_store(&a, i);
    atomic_store_explicit(&v, i, mo);
    atomic_store_explicit(&a, i, mo);
    atomic_exchange(&v, i);
    atomic_exchange(&a, i);
    atomic_exchange_explicit(&v, i, mo);
    atomic_exchange_explicit(&a, i, mo);
    atomic_compare_exchange_weak(&v, &l, i);
    atomic_compare_exchange_weak(&a, &l, i);
    atomic_compare_exchange_strong(&v, &l, i);
    atomic_compare_exchange_strong(&a, &l, i);
    atomic_compare_exchange_weak_explicit(&v, &l, i, mo, mo);
    atomic_compare_exchange_weak_explicit(&a, &l, i, mo, mo);
    atomic_compare_exchange_strong_explicit(&v, &l, i, mo, mo);
    atomic_compare_exchange_strong_explicit(&a, &l, i, mo, mo);
    atomic_fetch_add(&v, i);
    atomic_fetch_add(&a, i);
    atomic_fetch_add_explicit(&v, i, mo);
    atomic_fetch_add_explicit(&a, i, mo);
    atomic_fetch_sub(&v, i);
    atomic_fetch_sub(&a, i);
    atomic_fetch_sub_explicit(&v, i, mo);
    atomic_fetch_sub_explicit(&a, i, mo);
    atomic_fetch_and(&v, i);
    atomic_fetch_and(&a, i);
    atomic_fetch_and_explicit(&v, i, mo);
    atomic_fetch_and_explicit(&a, i, mo);
    atomic_fetch_or(&v, i);
    atomic_fetch_or(&a, i);
    atomic_fetch_or_explicit(&v, i, mo);
    atomic_fetch_or_explicit(&a, i, mo);
    atomic_fetch_xor(&v, i);
    atomic_fetch_xor(&a, i);
    atomic_fetch_xor_explicit(&v, i, mo);
    atomic_fetch_xor_explicit(&a, i, mo);
}

//=============================================================================
// 30_threads
//=============================================================================

//=============================================================================
// lock\2.cc
//=============================================================================
#if 1 // Not support now
void locker(std::mutex& m1, std::mutex& m2, std::mutex& m3)
{
    typedef std::unique_lock<std::mutex> lock_type;

    lock_type l1(m1, std::defer_lock);
    lock_type l2(m2, std::defer_lock);
    lock_type l3(m3, std::defer_lock);
    std::lock(l1, l2, l3);
    VERIFY(l1.owns_lock());
    VERIFY(l2.owns_lock());
    VERIFY(l3.owns_lock());
}

void lock_2_test01()
{
    std::mutex  m1, m2, m3;
    std::thread t1(locker, std::ref(m1), std::ref(m2), std::ref(m3));
    std::thread t2(locker, std::ref(m3), std::ref(m2), std::ref(m1));
    t1.join();
    t2.join();
}
#endif
//=============================================================================
// mutex\unlock\2.cc
//=============================================================================
#if 1 // Not support now
using mutex_type = std::mutex;

mutex_type m;

void mutex_f()
{
    std::lock_guard<mutex_type> l(m);
}

void mutex_unlock_2_main()
{
    std::thread t1(mutex_f);
    std::thread t2(mutex_f);
    t1.join();
    t2.join();
}
#endif
#define TEST_THREAD_NUM (10)
#define TEST_LOOP_COUNT (100000)
std::atomic_long atomic_l;
void             thread_test01(int number)
{
    std::cout << "thread " << std::this_thread::get_id() << ": num " << number << std::endl;

    for (int i = 0; i < TEST_LOOP_COUNT; i++)
    {
        atomic_l++;
    }
}

void thread_test_main_01(int is_joinable)
{
    int         thread_num;
    std::thread thread_handle[TEST_THREAD_NUM];

    atomic_l = 0;

    for (thread_num = 0; thread_num < TEST_THREAD_NUM; thread_num++)
    {
        thread_handle[thread_num] = std::thread(thread_test01, thread_num);
        if (!is_joinable)
        {
            std::cout << "thread " << thread_num << " detach!" << std::endl;
            thread_handle[thread_num].detach();
        }
    }

    for (thread_num = 0; thread_num < TEST_THREAD_NUM; thread_num++)
    {
        if (thread_handle[thread_num].joinable())
        {
            thread_handle[thread_num].join();
            std::cout << "thread " << thread_num << " exited!(join)" << std::endl;
        }
    }

    if (atomic_l != TEST_THREAD_NUM * TEST_LOOP_COUNT)
        std::cout << "thread_test_main_2 fail, atomic_l = " << atomic_l << std::endl;
}

//=============================================================================
// this_thread\57060.cc
//=============================================================================
void this_thread_57060_test01()
{
    VERIFY(std::this_thread::get_id() != std::thread::id());
}

#define TEST_MUTEX_CASE_NUM_MAX (4)
#define TEST_MUTEX_THREAD_NUM   (5)
#define TEST_MUTEX_LOOP_COUNT   (100)

unsigned int          mutex_test_count;
std::mutex*           mtx_ptr  = NULL;
std::recursive_mutex* rmtx_ptr = NULL;
std::timed_mutex*     tmtx_ptr = NULL;
void                  mutex_thread_test01(int thread_num, int test_case)
{
    unsigned int temp_count;

    for (int i = 0; i < TEST_MUTEX_LOOP_COUNT; i++)
    {
        switch (test_case)
        {
            case 1:
                mtx_ptr->lock();
                break;
            case 2:
                rmtx_ptr->lock();
                rmtx_ptr->lock();
                break;
            case 3:
                if (!tmtx_ptr->try_lock_for(std::chrono::milliseconds(1000)))
                {
                    std::cout << "thread " << thread_num << " get timed mutex fail!(timeout 1000ms)" << std::endl;
                    return;
                }
                break;
            default:
                break;
        }

        temp_count = mutex_test_count;

        MsSleep(1);

        mutex_test_count = ++temp_count;

        switch (test_case)
        {
            case 1:
                mtx_ptr->unlock();
                break;
            case 2:
                rmtx_ptr->unlock();
                rmtx_ptr->unlock();
                break;
            case 3:
                // tmtx_ptr->unlock();
                break;
            default:
                break;
        }
    }
}

void mutex_test_main_01(int mutex_test_case)
{
    int         thread_num;
    std::thread thread_handle[TEST_MUTEX_THREAD_NUM];

    if (mutex_test_case > TEST_MUTEX_CASE_NUM_MAX)
        return;

    std::cout << "mutex_test_main_01, test case = " << mutex_test_case << std::endl;

    switch (mutex_test_case)
    {
        case 0:
            std::cout << "[no mutex]" << std::endl;
            break;
        case 1:
            if (mtx_ptr == NULL)
            {
                mtx_ptr = new std::mutex();
            }
            std::cout << "[basic mutex]" << std::endl;
            break;
        case 2:
            if (rmtx_ptr == NULL)
            {
                rmtx_ptr = new std::recursive_mutex();
            }
            std::cout << "[recursive mutex]" << std::endl;
            break;
        case 3:
            if (tmtx_ptr == NULL)
            {
                tmtx_ptr = new std::timed_mutex();
            }
            std::cout << "[timed mutex]" << std::endl;
            tmtx_ptr->lock();
            break;
        default:
            break;
    }

    mutex_test_count = 0;

    for (thread_num = 0; thread_num < TEST_MUTEX_THREAD_NUM; thread_num++)
    {
        thread_handle[thread_num] = std::thread(mutex_thread_test01, thread_num, mutex_test_case);
    }

    for (thread_num = 0; thread_num < TEST_MUTEX_THREAD_NUM; thread_num++)
    {
        if (thread_handle[thread_num].joinable())
        {
            thread_handle[thread_num].join();
            std::cout << "thread " << thread_num << " exited!(join)" << std::endl;
        }
    }

    switch (mutex_test_case)
    {
        case 1:
            delete mtx_ptr;
            mtx_ptr = NULL;
            break;
        case 2:
            delete rmtx_ptr;
            rmtx_ptr = NULL;
            break;
        case 3:
            tmtx_ptr->unlock();
            delete tmtx_ptr;
            tmtx_ptr = NULL;
            break;
        default:
            break;
    }

    std::cout << "mutex_test_main_01 result: mutex_test_count = " << mutex_test_count
              << "(total count = " << TEST_MUTEX_THREAD_NUM * TEST_MUTEX_LOOP_COUNT << ")" << std::endl;
}

static void _test_cpp_support(void)
{
    itializer_list_range_access_test01();

    numeric_limits_infinity_test_infinity<float>();
    numeric_limits_infinity_test_infinity<double>();
    numeric_limits_infinity_test_infinity<long double>();

    numeric_limits_is_signed_test_sign();

    numeric_limits_lowest_test01();

    numeric_limits_min_max_test_extrema<char>();
    numeric_limits_min_max_test_extrema<signed char>();
    numeric_limits_min_max_test_extrema<unsigned char>();

    numeric_limits_min_max_test_extrema<short>();
    numeric_limits_min_max_test_extrema<unsigned short>();

    numeric_limits_min_max_test_extrema<int>();
    numeric_limits_min_max_test_extrema<unsigned>();

    numeric_limits_min_max_test_extrema<long>();
    numeric_limits_min_max_test_extrema<unsigned long>();

    numeric_limits_min_max_test_extrema<float>();
    numeric_limits_min_max_test_extrema<double>();
    numeric_limits_min_max_test_extrema<long double>();

    numeric_limits_quiet_NaN_test_qnan<float>();
    numeric_limits_quiet_NaN_test_qnan<double>();
    numeric_limits_quiet_NaN_test_qnan<long double>();
}

static void _test_cpp_diagnostics(void)
{
    error_category_generic_category_test01();
    error_category_generic_category_test02();
    error_category_generic_category_test03();

    error_category_system_category_test01();
    error_category_system_category_test02();
    error_category_system_category_test03();
}

static void _test_cpp_util(void)
{
    addressof_1_test01();

    align_1_test01();
    align_2_test01();

    allocator_10378_test01();
    allocator_14176_test02();
    allocator_overaligned_test01();
    allocator_void_test01();

    bind_all_bound_test01();
    bind_move_test01();
    bind_move_test02();

    tuple_48476_test01();
}

static void _test_cpp_string(void)
{
    basic_string_dr2268_test01();

    basic_string_init_list_test01();

    c_strings_char_1_test01();
    c_strings_wchar_t_1_test01();
}

static void _test_cpp_locale(void)
{
    time_get_get_char_1_test01();

    time_put_put_char_1_test01();
}

static void _test_cpp_containers(void)
{
    array_range_access_test01();

    bitset_to_string_1_test01();

    deque_range_access_test01();

    forward_list_range_access_test01();

    list_range_access_test01();

    map_range_access_test01();

    multimap_range_access_test01();

    multiset_range_access_test01();

    priority_queue_67085_test01();

    set_range_access_test01();

    vector_range_access_test01();
}
static void _test_cpp_iterators(void)
{
    back_insert_iterator_2_test02();

    front_insert_iterator_2_test02();

    insert_iterator_2_test02();

    istreambuf_iterator_2_test02();

    istream_iterator_2_test02();

    move_iterator_dr2061_test01();

    normal_iterator_58403_main();

    operations_next_test01();
    operations_next_test02();
    operations_prev_test01();
    operations_prev_test02();

    ostreambuf_iterator_2_test02();

    random_access_iterator_26020_test01();

    reverse_iterator_2_test02();
}

static void _test_cpp_algorithms(void)
{
    fill_1_test01();

    is_heap_1_test01();

    is_sorted_1_test01();

    minmax_1_test01();

    search_n_58358_test01();

    sort_1_test01();
}

static void _test_cpp_numerics(void)
{
    accumulate_1_test01();
    accumulate_1_test02();

    random_minstd_rand_test01();

    slice_1_main();

    valarray_range_access_test01();
}

static void _test_cpp_io(void)
{
    basic_ostream_endl_char_1_test01();
    basic_ostream_endl_wchar_t_1_test01();

    ios_base_state_1_test02();

    objects_char_1_test01();
    objects_wchar_t_1_test01();
}

static void _test_cpp_regex(void)
{
    basic_regex_85098_test01();

    init_list_test01();

    sub_match_cast_char_main();
}

static void _test_cpp_atomics(void)
{
    atomic_65913_test01();
    atomic_nonmembers_test01();
    atomic_nonmembers_test02();
    atomic_nonmembers_test03();
    atomic_nonmembers_test04();

    atomic_flag_test_and_set_explicit_main();
    atomic_flag_test_and_set_implicit_main();

    atomic_integral_nonmembers_test01();
    atomic_integral_nonmembers_test02();
}

static void _test_cpp_threads(void)
{
    lock_2_test01();

    mutex_unlock_2_main();

    // Not support detach now
    // thread_test_main_01(0);

    thread_test_main_01(1);
}

static void _test_cpp_mutex(void)
{
    for (int test_case = 0; test_case < TEST_MUTEX_CASE_NUM_MAX; test_case++)
    {
        mutex_test_main_01(test_case);
    }
}

void _cpp_show_test_menu(void)
{
    unsigned int i;

    printf("cam_os_wrapper test menu: \n");

    for (i = 0; i < CPP_TEST_ITEM_NUM; i++)
    {
        printf("\t%2d) %s\r\n", i, aCppsTestItemTbl[i].pTestDesc);
    }
}

int cpp_test_suite(unsigned int nCaseNum)
{
    unsigned int i;

    if (nCaseNum < CPP_TEST_ITEM_NUM)
    {
        if (nCaseNum == (CPP_TEST_ITEM_NUM - 1))
        {
            for (i = 0; i < CPP_TEST_ITEM_NUM; i++)
            {
                if (aCppsTestItemTbl[i].pFnTest)
                {
                    printf("===============================================\r\n");
                    printf("%s\r\n", aCppsTestItemTbl[i].pTestDesc);
                    printf("===============================================\r\n");
                    aCppsTestItemTbl[i].pFnTest();
                    printf("\r\n");
                }
            }
        }
        else
        {
            if (aCppsTestItemTbl[nCaseNum].pFnTest)
            {
                printf("===============================================\r\n");
                printf("%s\r\n", aCppsTestItemTbl[nCaseNum].pTestDesc);
                printf("===============================================\r\n");
                aCppsTestItemTbl[nCaseNum].pFnTest();
            }
        }
    }
    else
    {
        _cpp_show_test_menu();
        return -1;
    }

    std::cout << "===============================================" << std::endl;
    std::cout << "All tests are finished!" << std::endl;
    std::cout << "===============================================" << std::endl;

    return 0;
}

int ST_CPP_SUITE_Main(int menu_argc, char** menu_argv)
{
    cpp_test_suite(CPP_TEST_ITEM_NUM - 1);
    return 0;
}

DUALOS_SAMPLE_MODULE_AUTO_INITCALLBACK(MODULE_NAME, ST_CPP_SUITE_Main)