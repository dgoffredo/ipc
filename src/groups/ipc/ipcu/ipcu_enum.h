#ifndef INCLUDED_IPCU_ENUM
#define INCLUDED_IPCU_ENUM

#include <bsl_cstddef.h>
#include <bsl_iomanip.h>
#include <bsl_iostream.h>
#include <bsl_ostream.h>
#include <bsl_string.h>
#include <bsl_unordered_map.h>

#include <bslh_hash.h>

#include <bslmt_once.h>

// This component defines the following three macros, for generating enum-like
// classes:
//
//     IPCU_DEFINE_ENUM(NAME, ...VALUES...)
//
//     IPCU_DEFINE_ENUM_STRUCT(NAME, ...VALUES...)
//
//     IPCU_DEFINE_ENUM_FREE_FUNCTIONS(NAME)
//
// Each is described above its definition in this header.
//
///Example Usage
///- - - - - - -
//..
// #include <ipcu_enum.h>
//
// #include <bsl_iostream.h>
//
// IPCU_DEFINE_ENUM(Color, RED, GREEN, BLUE)
//
// int main(int argc, char *argv[])
// {
//     Color color;
//     const int rc = color.fromString(argv[1]);
//     if (rc) {
//         bsl::cerr << argv[1] << " is not a color.\n";
//         return rc;                                                 // RETURN
//     }
//
//     if (color == Color::e_BLUE) {
//         bsl::cout << "You picked the best color!\n";
//     }
//     else {
//         bsl::cout << color << "? I guess that's alright.\n";
//     }
// }
//..
//
// Note that while the enumerated values are defined in the macro without the
// leading 'e_', the generated 'enum' has values with a leading 'e_'. The
// string representations of the values, however, do not have the leading 'e_'.
//
// Also, if (for example) a 'Color' object is to be used in a 'switch', its
// 'value' must be accessed directly, since a 'switch' requires a value of
// integer type rather than of class type:
//..
// const char *dogability(Color c)
// {
//     switch (c.value) {
//       case Color::e_RED:
//         return "sees it ok";
//       case Color::e_GREEN:
//         return "hardly sees it at all";
//       default:
//         BSLS_ASSERT(c == Color::e_BLUE);
//         return "sees it very well";
//     }
// }
//..
//
// This might cause confusion, since you can use 'c.value' anywhere you can use
// 'c', but not the other way around. Still, prefer to use 'c' whenever
// possible.

// Utility macros needed to define the macros 'IPCU_DEFINE_ENUM' and
// 'IPCU_DEFINE_ENUM_STRUCT'. Variable argument lists are limited to at most
// 25 arguments.
//
#define IPCU__NUM_ARGS(X25,                                                   \
                       X24,                                                   \
                       X23,                                                   \
                       X22,                                                   \
                       X21,                                                   \
                       X20,                                                   \
                       X19,                                                   \
                       X18,                                                   \
                       X17,                                                   \
                       X16,                                                   \
                       X15,                                                   \
                       X14,                                                   \
                       X13,                                                   \
                       X12,                                                   \
                       X11,                                                   \
                       X10,                                                   \
                       X9,                                                    \
                       X8,                                                    \
                       X7,                                                    \
                       X6,                                                    \
                       X5,                                                    \
                       X4,                                                    \
                       X3,                                                    \
                       X2,                                                    \
                       X1,                                                    \
                       N,                                                     \
                       ...)                                                   \
    N

#define IPCU_NUM_ARGS(...)                                                    \
    IPCU__NUM_ARGS(__VA_ARGS__,                                               \
                   25,                                                        \
                   24,                                                        \
                   23,                                                        \
                   22,                                                        \
                   21,                                                        \
                   20,                                                        \
                   19,                                                        \
                   18,                                                        \
                   17,                                                        \
                   16,                                                        \
                   15,                                                        \
                   14,                                                        \
                   13,                                                        \
                   12,                                                        \
                   11,                                                        \
                   10,                                                        \
                   9,                                                         \
                   8,                                                         \
                   7,                                                         \
                   6,                                                         \
                   5,                                                         \
                   4,                                                         \
                   3,                                                         \
                   2,                                                         \
                   1)

#define IPCU_EXPAND(X) X
#define IPCU_FIRSTARG(X, ...) (X)
#define IPCU_RESTARGS(X, ...) (__VA_ARGS__)
#define IPCU_FOREACH(MACRO, LIST)                                             \
    IPCU_FOREACH_(IPCU_NUM_ARGS LIST, MACRO, LIST)
#define IPCU_FOREACH_(N, M, LIST) IPCU_FOREACH__(N, M, LIST)
#define IPCU_FOREACH__(N, M, LIST) IPCU_FOREACH_##N(M, LIST)
#define IPCU_FOREACH_1(H, T) H T
#define IPCU_FOREACH_2(H, T)                                                  \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_1(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_3(H, T)                                                  \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_2(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_4(H, T)                                                  \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_3(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_5(H, T)                                                  \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_4(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_6(H, T)                                                  \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_5(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_7(H, T)                                                  \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_6(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_8(H, T)                                                  \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_7(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_9(H, T)                                                  \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_8(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_10(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_9(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_11(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_10(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_12(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_11(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_13(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_12(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_14(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_13(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_15(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_14(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_16(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_15(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_17(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_16(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_18(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_17(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_19(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_18(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_20(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_19(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_21(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_20(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_22(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_21(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_23(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_22(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_24(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_23(H, IPCU_RESTARGS T)
#define IPCU_FOREACH_25(H, T)                                                 \
    IPCU_EXPAND(H IPCU_FIRSTARG T)                                            \
    , IPCU_FOREACH_24(H, IPCU_RESTARGS T)

#define IPCU_STRINGIFY(x) #x
#define IPCU_STRINGIFY_EACH(...) IPCU_FOREACH(IPCU_STRINGIFY, (__VA_ARGS__))

#define IPCU_ENUMIFY(x) e_##x
#define IPCU_ENUMIFY_EACH(...) IPCU_FOREACH(IPCU_ENUMIFY, (__VA_ARGS__))

// IPCU_DEFINE_ENUM(NAME, ...VALUES...)
//
//     - Define a 'enum'-like 'struct' with the specified 'NAME' and having
//       values derived from the specified 'VALUES'. For example, if one of the
//       'VALUES' is 'FOO', then the corresponding 'enum' value will be 'e_FOO'
//       and when printed it will display as "FOO" (without the quotes).
//
//     - This macro also defines free functions (comparisons, stream inserters)
//       for the defined 'struct'. If you wish to define only the 'struct', or
//       only the free functions (perhaps separately), use the macros
//       'IPCU_DEFINE_ENUM_STRUCT' and 'IPCU_DEFINE_ENUM_FREE_FUNCTIONS'.
//
#define IPCU_DEFINE_ENUM(NAME, ...)                                           \
    IPCU_DEFINE_ENUM_STRUCT(NAME, __VA_ARGS__)                                \
    IPCU_DEFINE_ENUM_FREE_FUNCTIONS(NAME)

// IPCU_DEFINE_ENUM_STRUCT(NAME, ...VALUES...)
//
//     - The stringified names will be exactly as appearing in the macro.
//
//     - The enum values will be prefixed with 'e_', e.g. 'FOO' --> 'e_FOO'.
//
#define IPCU_DEFINE_ENUM_STRUCT(NAME, ...)                                    \
    struct NAME {                                                             \
        enum Value { IPCU_ENUMIFY_EACH(__VA_ARGS__) };                        \
                                                                              \
        Value value;                                                          \
                                                                              \
        static const unsigned NUM_VALUES = IPCU_NUM_ARGS(__VA_ARGS__);        \
                                                                              \
        static const ::BloombergLP::bslstl::StringRef (&names())[NUM_VALUES]  \
        {                                                                     \
            typedef ::BloombergLP::bslstl::StringRef StringRef;               \
                                                                              \
            static const StringRef(*dataPtr)[NUM_VALUES] = 0;                 \
            BSLMT_ONCE_DO                                                     \
            {                                                                 \
                static const StringRef data[] = {                             \
                    IPCU_STRINGIFY_EACH(__VA_ARGS__)};                        \
                dataPtr = &data;                                              \
            }                                                                 \
            return *dataPtr;                                                  \
        }                                                                     \
                                                                              \
        NAME()                                                                \
        : value()                                                             \
        {                                                                     \
        }                                                                     \
                                                                              \
        NAME(Value enumValue)                                                 \
        : value(enumValue)                                                    \
        {                                                                     \
        }                                                                     \
                                                                              \
        NAME& operator =(Value newValue)                                      \
        {                                                                     \
            value = newValue;                                                 \
            return *this;                                                     \
        }                                                                     \
                                                                              \
        ::BloombergLP::bslstl::StringRef toString() const                     \
        {                                                                     \
            return names()[value];                                            \
        }                                                                     \
                                                                              \
        void print(::bsl::ostream& stream) const                              \
        {                                                                     \
            stream << names()[value];                                         \
        }                                                                     \
                                                                              \
        int fromString(const ::BloombergLP::bslstl::StringRef& str)           \
        {                                                                     \
            typedef bsl::unordered_map< ::BloombergLP::bslstl::StringRef,     \
                                        Value>                                \
                Map;                                                          \
                                                                              \
            static Map *mapPtr = 0;                                           \
            BSLMT_ONCE_DO                                                     \
            {                                                                 \
                static Map mapInstance;                                       \
                for (size_t i = 0; i < NUM_VALUES; ++i)                       \
                    mapInstance.emplace(names()[i], Value(i));                \
                mapPtr = &mapInstance;                                        \
            }                                                                 \
            const Map& map = *mapPtr;                                         \
                                                                              \
            const Map::const_iterator found = map.find(str);                  \
            if (found == map.end())                                           \
                return -1;                                                    \
                                                                              \
            value = found->second;                                            \
            return 0;                                                         \
        }                                                                     \
    };

// IPCU_DEFINE_ENUM_FREE_FUNCTIONS(NAME)
//
//     - Define 'inline' the non-member functions for the 'enum' 'struct' with
//       the specified fully qualified 'NAME'. The underlying 'struct' must
//       have been defined previously using the 'IPCU_DEFINE_ENUM_STRUCT'
//       macro.
//
#define IPCU_DEFINE_ENUM_FREE_FUNCTIONS(NAME)                                 \
    inline                                                                    \
    bool operator==(const NAME& lhs, const NAME& rhs)                         \
    {                                                                         \
        return lhs.value == rhs.value;                                        \
    }                                                                         \
                                                                              \
    inline                                                                    \
    bool operator!=(const NAME& lhs, const NAME& rhs)                         \
    {                                                                         \
        return !(lhs == rhs);                                                 \
    }                                                                         \
                                                                              \
    inline                                                                    \
    bool operator==(const NAME& lhs, const NAME::Value& rhs)                  \
    {                                                                         \
        return lhs.value == rhs;                                              \
    }                                                                         \
                                                                              \
    inline                                                                    \
    bool operator!=(const NAME& lhs, const NAME::Value& rhs)                  \
    {                                                                         \
        return !(lhs == rhs);                                                 \
    }                                                                         \
                                                                              \
    inline                                                                    \
    bool operator==(const NAME::Value& lhs, const NAME& rhs)                  \
    {                                                                         \
        return lhs == rhs.value;                                              \
    }                                                                         \
                                                                              \
    inline                                                                    \
    bool operator!=(const NAME::Value& lhs, const NAME& rhs)                  \
    {                                                                         \
        return !(lhs == rhs);                                                 \
    }                                                                         \
                                                                              \
    inline                                                                    \
    bsl::ostream& operator<<(bsl::ostream& stream, const NAME& rhs)           \
    {                                                                         \
        rhs.print(stream);                                                    \
        return stream;                                                        \
    }                                                                         \
                                                                              \
    inline                                                                    \
    bsl::ostream& operator<<(bsl::ostream& stream, const NAME::Value& rhs)    \
    {                                                                         \
        NAME(rhs).print(stream);                                              \
        return stream;                                                        \
    }                                                                         \
                                                                              \
    template <typename ALGORITHM>                                             \
    void hashAppend(ALGORITHM& algo, const NAME& value)                       \
    {                                                                         \
        using ::BloombergLP::bslh::hashAppend;                                \
        hashAppend(algo, value.value);                                        \
    }

#endif
