#ifndef INCLUDED_IPCMQ_RECEIVER
#define INCLUDED_IPCMQ_RECEIVER

#include <bsl_string.h>

namespace BloombergLP {
namespace bsls { class TimeInterval; }
namespace ipcmq {

                                // ==============
                                // class Receiver
                                // ==============
class Receiver {
    // This class defines a protocol for receiving messages from a message
    // queue.

  public:
    // CREATORS
    virtual ~Receiver();

    // MANIPULATORS
    virtual int receive(bsl::string *payload)                     = 0;
    virtual int receive(bsl::string *payload, unsigned *priority) = 0;
    virtual int receive(bsl::string               *payload,
                        const bsls::TimeInterval&  relativeTimeout) = 0;
    virtual int receive(bsl::string               *payload,
                        const bsls::TimeInterval&  relativeTimeout,
                        unsigned                  *priority) = 0;
        // Assign through the specified 'payload' the content of the next
        // available message on the queue represented by this object. Assign
        // through the optionally specified 'priority' the priority of the
        // message received. Block for no longer than the optionally specified
        // 'relativeTimeout', relative to the beginning of the invocation of
        // this function. Return zero if a message is successfully received or
        // a nonzero value otherwise.

    virtual int tryReceive(bsl::string *payload)                     = 0;
    virtual int tryReceive(bsl::string *payload, unsigned *priority) = 0;
        // Assign through the specified 'payload' the content of the next
        // available message on the queue represented by this object. Assign
        // through the optionally specified 'priority' the priority of the
        // message received. Do not block. Return zero if a message is
        // successfully received or a nonzero value otherwise.
};

}  // close package namespace
}  // close enterprise namespace

#endif
