#ifndef INCLUDED_IPCU_OPERATORBOOL
#define INCLUDED_IPCU_OPERATORBOOL

#include <bsls_compilerfeatures.h>
#include <bsls_unspecifiedbool.h>

// This component defines a bunch of macros for defining 'explicit operator
// bool' for a type when the compiler supports that feature, or otherwise
// defines an analogous operator using 'bsls::UnspecifiedBool'.

#ifdef BSLS_COMPILERFEATURES_SUPPORT_OPERATOR_EXPLICIT

#define IPCU_DECLARE_OPERATOR_BOOL(CLASS) explicit operator bool() const
#define IPCU_DEFINE_OPERATOR_BOOL(CLASS) CLASS::operator bool() const
#define IPCU_RETURN_OPERATOR_BOOL(CLASS, EXPR) return (EXPR)

#else

#define IPCU_DECLARE_OPERATOR_BOOL(CLASS)                                    \
    operator BloombergLP::bsls::UnspecifiedBool<CLASS>::BoolType() const
#define IPCU_DEFINE_OPERATOR_BOOL(CLASS)                                     \
    CLASS::operator BloombergLP::bsls::UnspecifiedBool<CLASS>::BoolType() const
#define IPCU_RETURN_OPERATOR_BOOL(CLASS, EXPR)                               \
    return BloombergLP::bsls::UnspecifiedBool<CLASS>::makeValue(EXPR)

#endif

#endif
