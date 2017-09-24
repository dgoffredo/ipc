#ifndef INCLUDED_IPCU_ALGOUTIL
#define INCLUDED_IPCU_ALGOUTIL

#include <ball_log.h>

#include <bsl_limits.h>

#include <bsls_assert.h>

namespace BloombergLP {
namespace ipcu {

                                // ===============
                                // struct AlgoUtil
                                // ===============

struct AlgoUtil {
    // This 'struct' provides a namespace for functions implementing algorithms
    // used in the implementation of components within this package group.

    // TYPES
    template <typename T>
    struct GetPtr;
        // Create a unary function-like object that returns an argument of type
        // 'T*' unchanged, asserting that it is nonzero, and returns the
        // address of any argument of type 'T'.

    // CLASS METHODS
    template <typename NUMBER, typename PREDICATE>
    static NUMBER findMaxIf(NUMBER startingValue, PREDICATE notTooLarge);
        // Return the greatest non-negative 'NUMBER' greater than or equal to
        // the specified 'startingValue' for which the specified 'notTooLarge'
        // predicate returns a true value. The behavior is undefined unless all
        // of the following hold:
        //: o 'notTooLarge(startingValue)' is true
        //: o 'startingValue' is non-negative
        //: o if 'notTooLarge(a)' is true for some 'a', then 'notTooLarge(b)'
        //    is true for every 'b >= a'.
        // Note that this function does not allow the specification of an
        // epsilon, and so is not efficient for floating point types.

    template <typename NUMBER>
    static NUMBER twice(NUMBER number);
        // Return two times the specified 'number', or return
        // 'bsl::numeric_limits<NUMBER>::max()' if doubling 'number' would
        // overflow.

    template <typename NUMBER>
    static NUMBER midpoint(NUMBER lesser, NUMBER greater);
        // Return the value lying halfway between the specified 'lesser' and
        // the specified 'greater'. The behavior is undefined unless
        // 'lesser <= greater'.

  private:
    // CLASS DATA
    static const char *const k_LOG_CATEGORY;
};

// ============================================================================
//                          INLINE DEFINITIONS
// ============================================================================

                            // -----------------------
                            // struct AlgoUtil::GetPtr
                            // -----------------------

template <typename T>
struct AlgoUtil::GetPtr {
    typedef T *ResultType;

    ResultType operator()(T& ref) const { return &ref; }
    ResultType operator()(T *ptr) const
    {
        BSLS_ASSERT(ptr);
        return ptr;
    }
};

                                // ---------------
                                // struct AlgoUtil
                                // ---------------

template <typename NUMBER>
NUMBER AlgoUtil::twice(NUMBER x)
{
    const NUMBER maxValue = bsl::numeric_limits<NUMBER>::max();
    if (NUMBER(maxValue - x) >= x) {
        return NUMBER(2) * x;
    }
    else {
        return maxValue;
    }
}

template <typename NUMBER>
NUMBER AlgoUtil::midpoint(NUMBER a, NUMBER b)
{
    return a + NUMBER(b - a) / NUMBER(2);
}

template <typename NUMBER, typename PREDICATE>
NUMBER AlgoUtil::findMaxIf(NUMBER startingValue, PREDICATE notTooLarge)
{
    BALL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

    BSLS_ASSERT(startingValue >= NUMBER(0));
    BSLS_ASSERT(notTooLarge(startingValue));

    // A couple of corner cases:
    // - If 'startingValue' is the type's max, then return it.
    // - If 'startingValue' is zero, increment it rather than double it.
    const NUMBER maxValue = bsl::numeric_limits<NUMBER>::max();
    if (startingValue == maxValue) {
        return startingValue;                                         // RETURN
    }

    // 'highest' will rise geometrically to meet 'ceiling' at 'current'.
    NUMBER current = startingValue
                         ? twice(startingValue)
                         : startingValue + NUMBER(1);  // the value to check
    for (NUMBER highest = startingValue,  // highest working value so far
         ceiling        = startingValue;  // lowest non-working value so far
         current != highest;) {
        BALL_LOG_TRACE << "findMaxIf iterating with highest=" << highest
                       << " ceiling=" << ceiling << " current=" << current
                       << BALL_LOG_END;

        if (current == highest) {
            // Either we hit a ceiling or dropped back down to where we were.
            break;
        }

        if (notTooLarge(current)) {
            // 'current' is not too large. Go up either towards the max by
            // doubling or towards the ceiling half way.
            highest = current;
            current = ceiling <= current ? twice(current)
                                         : midpoint(current, ceiling);
        }
        else {
            // 'current' is too large. Scale back half way towards 'highest'.
            BSLS_ASSERT_SAFE(current > highest);
            ceiling = current;
            current = midpoint(highest, current);
        }
    }

    return current;
}

}  // close package namespace
}  // close enterprise namespace

#endif
