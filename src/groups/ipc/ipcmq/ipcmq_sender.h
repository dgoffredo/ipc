#ifndef INCLUDED_IPCMQ_SENDER
#define INCLUDED_IPCMQ_SENDER

#include <bsl_string.h>

namespace BloombergLP {
namespace bsls { class TimeInterval; }
namespace ipcmq {

                                // ============
                                // class Sender
                                // ============
class Sender {
    // This class defines a protocol for sending messages to a message queue.

  public:
    // CREATORS
    virtual ~Sender();

    // MANIPULATORS
    virtual int send(const bslstl::StringRef& payload, int priority) = 0;
    virtual int send(const bslstl::StringRef& payload)               = 0;
    virtual int send(const bslstl::StringRef&  payload,
                     const bsls::TimeInterval& relativeTimeout,
                     int                       priority) = 0;
    virtual int send(const bslstl::StringRef&  payload,
                     const bsls::TimeInterval& relativeTimeout) = 0;
        // Enqueue onto the queue represented by this object a message
        // consisting of the specified 'payload' and having the optionally
        // specified 'priority'. Block for no longer than the optionally
        // specified 'relativeTimeout', relative to the beginning of the
        // invocation of this function. Return zero if the message is
        // successfully sent or a nonzero value otherwise.

    virtual int trySend(const bslstl::StringRef& payload)               = 0;
    virtual int trySend(const bslstl::StringRef& payload, int priority) = 0;
        // Enqueue onto the queue represented by this object a message
        // consisting of the specified 'payload' and having the optionally
        // specified 'priority'. Do not block. Return zero if the message is
        // successfully sent or a nonzero value otherwise.
};

}  // close package namespace
}  // close enterprise namespace

#endif
