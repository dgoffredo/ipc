#ifndef INCLUDED_IPCMQ_QUEUE
#define INCLUDED_IPCMQ_QUEUE

#include <ipcmq_format.h>
#include <ipcmq_posixqueue.h>
#include <ipcmq_queuereceiver.h>
#include <ipcmq_queuesender.h>

#include <bsl_string.h>

namespace BloombergLP {
namespace bslma { class Allocator; }
namespace bsls { class TimeInterval; }
namespace ipcmq {

                                // ===========
                                // class Queue
                                // ===========

class Queue : public Sender, public Receiver {
    // This class implements both the 'Sender' and 'Receiver' protocols using
    // a 'PosixQueue' object.

    // DATA
    PosixQueue               d_queue;
    QueueSender              d_sender;
    QueueReceiver            d_receiver;
    PosixQueue::Open::Result d_openResult;

  public:
    // CREATORS
    Queue(const bslstl::StringRef&       name,
          Format                         format,
          const PosixQueue::Attributes&  attributes = PosixQueue::Attributes(),
          int                            filePermissions  = 0,
          bslma::Allocator              *allocator        = 0,
          bslma::Allocator              *messageAllocator = 0);
        // Open for reading and for writing the message queue having the
        // specified 'name'. Use the specified 'format' when sending and
        // receiving messages. If the queue does not already exist, create a
        // queue having the optionally specified 'attributes' and the
        // optionally specified 'filePermissions'. On success, 'isOpen' will
        // subsequently return 'true'.

    // MANIPULATORS
    int receive(bsl::string *payload);                      // override
    int receive(bsl::string *payload, unsigned *priority);  // override
    int receive(bsl::string               *payload,
                const bsls::TimeInterval&  relativeTimeout);  // override
    int receive(bsl::string               *payload,
                const bsls::TimeInterval&  relativeTimeout,
                unsigned                  *priority);  // override
        // Assign through the specified 'payload' the content of the next
        // available message on the queue represented by this object. Assign
        // through the optionally specified 'priority' the priority of the
        // message received. Block for no longer than the optionally specified
        // 'relativeTimeout', relative to the beginning of the invocation of
        // this function. Return zero if a message is successfully received or
        // a nonzero value otherwise. If an error occurs, 'errorDescription'
        // will return a description of the error.

    int tryReceive(bsl::string *payload);                      // override
    int tryReceive(bsl::string *payload, unsigned *priority);  // override
        // Assign through the specified 'payload' the content of the next
        // available message on the queue represented by this object. Assign
        // through the optionally specified 'priority' the priority of the
        // message received. Do not block. Return zero if a message is
        // successfully received or a nonzero value otherwise. If an error
        // occurs, 'errorDescription' will return a description of the error.

    int send(const bslstl::StringRef& payload);                // override
    int send(const bslstl::StringRef& payload, int priority);  // override
    int send(const bslstl::StringRef&  payload,
             const bsls::TimeInterval& relativeTimeout);  // override
    int send(const bslstl::StringRef&  payload,
             const bsls::TimeInterval& relativeTimeout,
             int                       priority);  // override
        // Enqueue onto the queue represented by this object a message
        // consisting of the specified 'payload' and having the optionally
        // specified 'priority'. Block for no longer than the optionally
        // specified 'relativeTimeout', relative to the beginning of the
        // invocation of this function. Return zero if the message is
        // successfully sent or a nonzero value otherwise. If an error occurs,
        // 'errorDescription' will return a description of the error.

    int trySend(const bslstl::StringRef& payload);                // override
    int trySend(const bslstl::StringRef& payload, int priority);  // override
        // Enqueue onto the queue represented by this object a message
        // consisting of the specified 'payload' and having the optionally
        // specified 'priority'. Do not block. Return zero if the message is
        // successfully sent or a nonzero value otherwise. If an error occurs,
        // 'errorDescription' will return a description of the error.

    int unlink();
        // Mark for deletion the message queue opened by this object. Return
        // zero on success or a nonzero value otherwise. If an error occurs,
        // 'errorDescription' will return a description of the error.

    // ACCESSORS
    PosixQueue::Open::Result openResult() const;
        // Return the result returned when this queue was opened.

    bool isOpen() const;
        // Return whether this object represents an open message queue.

    IPCU_DECLARE_OPERATOR_BOOL(Queue);
        // Return 'isOpen()'.

    const PosixQueue& posixQueue() const;
        // Return a reference providing non-modifiable access to the
        // 'PosixQueue' instance used to implement this object.

    // CLASS METHODS
    static const char *description(int errorCode);
        // Return a pointer to a null terminated string that describes the
        // specified 'errorCode'. The behavior is undefined unless 'errorCode'
        // has the same value as the result of a previous invocation of one of
        // the methods of an instance of this class.
};

}  // close package namespace
}  // close enterprise namespace

#endif
