#ifndef INCLUDED_IPCU_ENUM
#define INCLUDED_IPCU_ENUM

#include <bdlat_enumfunctions.h>

#include <bsl_cstddef.h>
#include <bsl_iomanip.h>
#include <bsl_ios.h>
#include <bsl_iostream.h>
#include <bsl_ostream.h>
#include <bsl_string.h>
#include <bsl_unordered_map.h>

#include <bslh_hash.h>

#include <bslma_default.h>

#include <bslmt_once.h>

#include <bsls_assert.h>

// This component defines a macro for generating enum-like classes:
//
//     IPCU_DEFINE_ENUM(NAME, ...VALUES...);
//
///Example Usage
///- - - - - - -
//..
// #include <ipcu_enum.h>
//
// #include <bsl_iostream.h>
//
// IPCU_DEFINE_ENUM(Color, RED, GREEN, BLUE);
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
// The generated class implicitly converts into the underlying 'enum', so a
// variable of the class type can be used in contexts where the underlying
// integral type is needed, such as in a switch statement:
//..
// const char *dogability(Color c)
// {
//     switch (c) {
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

namespace BloombergLP {
namespace ipcu {

template <typename ENUM>
class EnumBase {
    // 'EnumBase' is used as a base class for all classes defined using
    // 'IPCU_DEFINE_ENUM'.  It allows the 'bdlat_enumfunctions' to be used on
    // instances of the generated type.
};

template <typename ENUM>
int bdlat_enumFromInt(EnumBase<ENUM> *result, int number);
    // Load into the specified 'result' the enumerator matching the
    // specified 'number'.  Return 0 on success, and a non-zero value
    // with no effect on 'result' if 'number' does not match any
    // enumerator.

template <typename ENUM>
int bdlat_enumFromString(EnumBase<ENUM> *result,
                         const char     *string,
                         int             stringLength);
    // Load into the specified 'result' the enumerator matching the
    // specified 'string' of the specified 'stringLength'.  Return 0 on
    // success, and a non-zero value with no effect on 'result' if
    // 'string' and 'stringLength' do not match any enumerator.

template <typename ENUM>
void bdlat_enumToInt(int *result, const EnumBase<ENUM>& value);
    // Return the integer representation exactly matching the
    // enumerator name corresponding to the specified enumeration
    // 'value'.

template <typename ENUM>
void bdlat_enumToString(bsl::string *result, const EnumBase<ENUM>& value);
    // Return the string representation exactly matching the enumerator
    // name corresponding to the specified enumeration 'value'.

// Below are the definitions of the function templates declared above.
//
template <typename ENUM>
int bdlat_enumFromInt(EnumBase<ENUM> *result, int number)
{
    if (number < 0 || number >= ENUM::k_NUM_VALUES) {
        return -1;  // failure, no op
    }

    BSLS_ASSERT(result);
    
    static_cast<ENUM&>(*result) = ENUM::Value(number);

    return 0;  // success
}

template <typename ENUM>
int bdlat_enumFromString(EnumBase<ENUM>  *result,
                         const char      *string,
                         int              stringLength)
{
    BSLS_ASSERT(result);

    return static_cast<ENUM&>(*result).fromString(
        bslstl::StringRef(string, stringLength));
}

template <typename ENUM>
void bdlat_enumToInt(int *result, const EnumBase<ENUM>& value)
{
    BSLS_ASSERT(result);

    *result = int(static_cast<const ENUM&>(value));
}

template <typename ENUM>
void bdlat_enumToString(bsl::string *result, const EnumBase<ENUM>& value)
{
    BSLS_ASSERT(result);

    *result = static_cast<const ENUM&>(value).toString();
}

}  // close package namespace

// Define the 'IsEnumeration' trait for all classes derived from 'EnumBase'.
namespace bdlat_EnumFunctions {

template <typename ENUM>
struct IsEnumeration<ipcu::EnumBase<ENUM> > {
    enum { VALUE = 1 };
};

}  // close traits namespace
}  // close enterprise namespace

// Utility macros needed to define the 'IPCU_DEFINE_ENUM' macro. Variable
// argument lists are limited to at most 25 arguments.
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

// IPCU_DEFINE_ENUM(NAME, ...VALUES...);
//
//     - Define a 'enum'-like 'class' with the specified 'NAME' and having
//       values derived from the specified 'VALUES'. For example, if one of the
//       'VALUES' is 'FOO', then the corresponding 'enum' value will be 'e_FOO'
//       and when printed it will display as "FOO" (without the quotes).
//
#define IPCU_DEFINE_ENUM(NAME, ...)                                           \
    class NAME : public ::BloombergLP::ipcu::EnumBase<NAME> {                 \
      public:                                                                 \
        enum Value { IPCU_ENUMIFY_EACH(__VA_ARGS__) };                        \
                                                                              \
        static const unsigned k_NUM_VALUES = IPCU_NUM_ARGS(__VA_ARGS__);        \
                                                                              \
        static const ::BloombergLP::bslstl::StringRef (&names())[k_NUM_VALUES]  \
        {                                                                     \
            typedef ::BloombergLP::bslstl::StringRef StringRef;               \
                                                                              \
            static const StringRef(*dataPtr)[k_NUM_VALUES] = 0;                 \
            BSLMT_ONCE_DO                                                     \
            {                                                                 \
                static const StringRef data[] = {                             \
                    IPCU_STRINGIFY_EACH(__VA_ARGS__)};                        \
                dataPtr = &data;                                              \
            }                                                                 \
            return *dataPtr;                                                  \
        }                                                                     \
                                                                              \
      private:                                                                \
        Value value;                                                          \
                                                                              \
      public:                                                                 \
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
        operator Value() const                                                \
        {                                                                     \
            return value;                                                     \
        }                                                                     \
                                                                              \
        ::BloombergLP::bslstl::StringRef toString() const                     \
        {                                                                     \
            return names()[value];                                            \
        }                                                                     \
                                                                              \
        friend bsl::ostream& operator <<(bsl::ostream& stream,                \
                                         const NAME&   toPrint)               \
        {                                                                     \
            return stream << names()[toPrint.value];                          \
        }                                                                     \
                                                                              \
        int fromString(const ::BloombergLP::bslstl::StringRef& str)           \
        {                                                                     \
            using namespace ::BloombergLP;                                    \
            typedef bsl::unordered_map<bslstl::StringRef, Value> Map;         \
                                                                              \
            static Map *mapPtr = 0;                                           \
            BSLMT_ONCE_DO                                                     \
            {                                                                 \
                static Map mapInstance(bslma::Default::globalAllocator());    \
                for (size_t i = 0; i < k_NUM_VALUES; ++i)                       \
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
                                                                              \
        friend bsl::istream& operator >>(bsl::istream& stream, NAME& result)  \
        {                                                                     \
            bsl::string word;                                                 \
            stream >> word;                                                   \
            if (!stream) {                                                    \
                /* unable to read a word */                                   \
                return stream;                                                \
            }                                                                 \
                                                                              \
            if (result.fromString(word)) {                                    \
                /* The word is not a valid stringified enum value. */         \
                stream.setstate(bsl::ios::failbit);                           \
            }                                                                 \
                                                                              \
            return stream;                                                    \
        }                                                                     \
    }

#endif
