
#include <ipcmq_posixqueueerrors.h>

namespace BloombergLP {
namespace ipcmq {
namespace PosixQueueTypes {

// The wording of most of the error descriptions below are derived from the
// following document:
//
//     The Open Group Base Specifications Issue 7
//     IEEE Std 1003.1-2008, 2016 Edition
//     Copyright © 2001-2016 The IEEE and The Open Group
//
// accessed online via http://pubs.opengroup.org/onlinepubs/ in July of 2017.

// Each set of descriptions for a "category" (e.g. Open, Close) begins with a
// generic message for the 'e_SUCCESS' return code and ends with a generic
// message for the 'e_UNKNOWN' return code.
#define CATEGORY(...) __VA_ARGS__, UNKNOWN_MESSAGE

namespace {

const char SUCCESS_MESSAGE[] = "success";

const char UNKNOWN_MESSAGE[] =
    "An error occured that this library did not anticipate.";

typedef Close LastCategory;

const int MAX_RETURN_CODE = LastCategory::e_UNKNOWN;

const char *defaultOverflowError(int)
{
    return "The error code is not known to this component.";
}

}  // close unnamed namespace

const char *const k_ERROR_DESCRIPTIONS[] = {
    SUCCESS_MESSAGE,

    // ----
    // Open
    // ----
    CATEGORY(
        // e_PERMISSION_DENIED
        "The message queue exists and the permissions specified by oflag are "
        "denied, or the message queue does not exist and permission to create "
        "the message queue is denied.",
        // e_ALREADY_EXISTS
        "O_CREAT and O_EXCL are set and the named message queue already "
        "exists.",
        // e_INTERRUPTED
        "The mq_open() function was interrupted by a signal.",
        // e_NAME_TOO_LONG
        "The length of the name argument exceeds {PATH_MAX} or a pathname "
        "component is longer than {NAME_MAX}.",
        // e_INVALID_PARAMETER
        "Either the mq_open() function is not supported for the given name, "
        "or O_CREAT was specified in oflag, the value of attr is not NULL, "
        "and either mq_maxmsg or mq_msgsize was less than or equal to zero or "
        "greater than allowed by the system.",
        // e_LIMIT_REACHED
        "Too many message queue descriptors or file descriptors are currently "
        "in use by this process or by the system as a whole.",
        // e_DOES_NOT_EXIST
        "O_CREAT is not set and the named message queue does not exist.",
        // e_NOT_ENOUGH_SPACE
        "There is insufficient space for the creation of the new message "
        "queue."),

    // ------
    // Unlink
    // ------
    CATEGORY(
        // e_PERMISSION_DENIED
        "Permission is denied to unlink the named message queue.",
        // e_INTERRUPTED
        "The call to mq_unlink() blocked waiting for all references to the "
        "named "
        "message queue to be closed and a signal interrupted the call.",
        // e_INVALID_PARAMETER
        "The specified queue name is not a valid name.",
        // e_DOES_NOT_EXIST
        "The named message queue does not exist.",
        // e_NAME_TOO_LONG
        "The length of the name argument exceeds {PATH_MAX} or a pathname "
        "component is longer than {NAME_MAX}."),

    // ----
    // Send
    // ----
    CATEGORY(
        // e_FULL
        "The O_NONBLOCK flag is set in the message queue description "
        "associated with mqdes, and the specified message queue is full.",
        // e_WRONG_MODE
        "The mqdes argument to mq_open is not a valid message queue "
        "descriptor open for writing.",
        // e_INTERRUPTED
        "A signal interrupted the call to mq_send() or mq_timedsend().",
        // e_BAD_PRIORITY_OR_DEADLINE
        "The value of msg_prio was outside the valid range, or the process or "
        "thread would have blocked, and the abstime parameter specified a "
        "nanoseconds field value less than zero or greater than or equal to "
        "1000 million.",
        // e_MESSAGE_TOO_LARGE
        "The specified message length, msg_len, exceeds the message size "
        "attribute of the message queue.",
        // e_TIMED_OUT
        "The O_NONBLOCK flag was not set when the message queue was opened, "
        "but the timeout expired before the message could be added to the "
        "queue."),

    // -------
    // Receive
    // -------
    CATEGORY(
        // e_EMPTY
        "O_NONBLOCK was set in the message description associated with mqdes, "
        "and the specified message queue is empty.",
        // e_WRONG_MODE
        "The mqdes argument to mq_receive or mq_timedreceive is not a valid "
        "message queue descriptor open for reading.",
        // e_INTERRUPTED
        "The mq_receive() or mq_timedreceive() operation was interrupted by a "
        "signal.",
        // e_BAD_DEADLINE
        "The process or thread would have blocked, and the abstime parameter "
        "specified a nanoseconds field value less than zero or greater than "
        "or equal to 1000 million.",
        // e_TIMED_OUT
        "The O_NONBLOCK flag was not set when the message queue was opened, "
        "but no message arrived on the queue before the specified timeout "
        "expired.",
        // e_CORRUPTED_MESSAGE
        "The implementation has detected a data corruption problem with the "
        "message."),

    // --------------
    // SetNonBlocking
    // --------------
    CATEGORY(
        // e_CLOSED
        "This PosixQueue object is closed, so there is nothing to set.",
        // e_BAD_DESCRIPTOR
        "The mqdes argument is not a valid message queue descriptor."),

    // -----
    // Close
    // -----
    CATEGORY(
        // e_CLOSED
        "This PosixQueue object is already closed.",
        // e_BAD_DESCRIPTOR
        "The mqdes argument is not a valid message queue descriptor.")};

#undef CATEGORY

}  // close component namespace

using namespace PosixQueueTypes;

const char *description(int errorCode)
{
    return description(errorCode, &defaultOverflowError);
}

const char *description(int errorCode, const char *(*overflow)(int))
{
    const char *errorDescription = 0;

    if (errorCode <= MAX_RETURN_CODE) {
        // It's one of the error messages defined here.
        errorDescription = k_ERROR_DESCRIPTIONS[errorCode];
    }
    else {
        // It's too large. Defer to the 'overflow' callback.
        errorDescription = overflow(errorCode - (MAX_RETURN_CODE + 1));
    }

    BSLS_ASSERT(errorDescription);
    return errorDescription;
}

int makeError(int errorCode)
{
    BSLS_ASSERT(errorCode >= 0);

    return (MAX_RETURN_CODE + 1) + errorCode;
}

}  // close package namespace
}  // close enterprise namespace
