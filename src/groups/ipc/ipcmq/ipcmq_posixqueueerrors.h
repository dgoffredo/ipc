#ifndef INCLUDED_POSIXQUEUEERRORS
#define INCLUDED_POSIXQUEUEERRORS

// Each operation in 'ipcmq::PosixQueue' returns a dedicated error code from
// a specific "category." A category is a 'struct' in which there is defined
// an 'enum Result'. So, for example, 'ipcmq::PosixQueue::open' returns an
// enumeration of type 'ipcmq::Open::Result', where 'ipcmq::Open' is an alias
// for 'ipcmq::PosixQueueTypes::Open'.
//
// Each category 'Result' has an a value 'e_SUCCESS' among its enumerated
// values, where 'e_SUCCESS' is always zero. Additionally, the last (greatest)
// value in every enumeration is always 'e_UNKNOWN'.
//
// In addition to the categories, this component defines free functions used
// to convert an error code into a technical description of the error.

#include <bsls_assert.h>

namespace BloombergLP {
namespace ipcmq {
namespace PosixQueueTypes {

// The first error category contains a normal enum.
#define DEFINE_CATEGORY_FIRST(NAME, ...)                                      \
    struct NAME {                                                             \
        enum Result { e_SUCCESS, __VA_ARGS__, e_UNKNOWN };                    \
    }

// Subsequent error categories contain 'e_SUCCESS = 0', followed by a value
// that is one greater than the largest enumerated value in the predecessor's
// 'enum'.
#define DEFINE_CATEGORY(NAME, PREDECESSOR, FIRST, ...)                        \
    struct NAME {                                                             \
        enum Result {                                                         \
            e_SUCCESS,                                                        \
            FIRST = PREDECESSOR::e_UNKNOWN + 1,                               \
            __VA_ARGS__,                                                      \
            e_UNKNOWN                                                         \
        };                                                                    \
                                                                              \
        static const int s_offset;                                            \
    }

DEFINE_CATEGORY_FIRST(Open,
                      e_PERMISSION_DENIED,
                      e_ALREADY_EXISTS,
                      e_INTERRUPTED,
                      e_NAME_TOO_LONG,
                      e_INVALID_PARAMETER,
                      e_LIMIT_REACHED,
                      e_DOES_NOT_EXIST,
                      e_NOT_ENOUGH_SPACE);

DEFINE_CATEGORY(Unlink,
                Open,
                e_PERMISSION_DENIED,
                e_INTERRUPTED,
                e_INVALID_PARAMETER,
                e_DOES_NOT_EXIST,
                e_NAME_TOO_LONG);

DEFINE_CATEGORY(Send,
                Unlink,
                e_FULL,
                e_WRONG_MODE,
                e_INTERRUPTED,
                e_BAD_PRIORITY_OR_DEADLINE,
                e_MESSAGE_TOO_LARGE,
                e_TIMED_OUT);

DEFINE_CATEGORY(Receive,
                Send,
                e_EMPTY,
                e_WRONG_MODE,
                e_INTERRUPTED,
                e_BAD_DEADLINE,
                e_TIMED_OUT,
                e_CORRUPTED_MESSAGE);

DEFINE_CATEGORY(SetNonBlocking, Receive, e_CLOSED, e_BAD_DESCRIPTOR);

DEFINE_CATEGORY(Close, SetNonBlocking, e_CLOSED, e_BAD_DESCRIPTOR);

#undef DEFINE_CATEGORY
#undef DEFINE_CATEGORY_FIRST

}  // close component namespace

// FREE FUNCTIONS

const char *description(int returnCode);
const char *description(int returnCode, const char *(*overflow)(int));
    // Return a technical description of the error indicated by the specified
    // 'returnCode'. If 'returnCode' is larger than any code known to this
    // component, then instead return the result of invoking the optionally
    // specified 'overflow' with the amount by which 'returnCode' exceeds the
    // greatest code known to this component. If 'overflow' is not specified,
    // then a default error message will be used instead. The behavior is
    // undefined unless either 'returnCode' corresponds to an code known to
    // this component, or 'overflow' returns a pointer to a valid
    // null-terminated string.

int makeError(int errorCode);
    // Return the specified 'errorCode' increased by an amount such that if it
    // were passed to 'description(int, ...)', that function's 'overflow'
    // callback would be invoked with the original value of 'errorCode'. The
    // behavior is undefined unless 'errorCode' is nonnegative.

}  // close package namespace
}  // close enterprise namespace

#endif
