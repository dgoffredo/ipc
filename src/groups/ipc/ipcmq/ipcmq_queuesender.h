#ifndef INCLUDED_IPCMQ_QUEUESENDER
#define INCLUDED_IPCMQ_QUEUESENDER

#include <ipcmq_format.h>
#include <ipcmq_formatutil.h>
#include <ipcu_operatorbool.h>
#include <ipcmq_posixqueue.h>
#include <ipcmq_sender.h>

#include <bdeut_variant.h>

#include <bsl_string.h>

namespace BloombergLP {
namespace bslma { class Allocator; }
namespace ipcmq {

                            // =================
                            // class QueueSender
                            // =================

class QueueSender : public Sender {
    // This class implements the 'Sender' protocol using a 'PosixQueue' object.

    // DATA
    bdeut_Variant2<PosixQueue, PosixQueue *>  d_queue;
    FormatUtil::Encoder                       d_encoder;
    bslma::Allocator                         *d_messageAllocator;
    PosixQueue::Open::Result                  d_openResult;

  public:
    // CREATORS
    QueueSender(
        const bslstl::StringRef&       name,
        Format                         format,
        const PosixQueue::Attributes&  attributes = PosixQueue::Attributes(),
        int                            filePermissions  = 0,
        bslma::Allocator              *allocator        = 0,
        bslma::Allocator              *messageAllocator = 0);
        // Open for writing the message queue having the specified 'name'. Use
        // the specified 'format' when sending messages. If the queue does not
        // already exist, create a queue having the optionally specified
        // 'attributes' and the optionally specified 'filePermissions'. On
        // success, 'isOpen' will subsequently return 'true'.  On failure,
        // 'isOpen' will subsequently return 'false'.

    QueueSender(PosixQueue       *queue,
                Format            format,
                bslma::Allocator *messageAllocator = 0);
        // Create a 'QueueSender' object that can send messages to the
        // specified 'queue'. Use the specified 'format' when sending messages.

    // MANIPULATORS
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
        // successfully sent or a nonzero value otherwise.

    int trySend(const bslstl::StringRef& payload);                // override
    int trySend(const bslstl::StringRef& payload, int priority);  // override
        // Enqueue onto the queue represented by this object a message
        // consisting of the specified 'payload' and having the optionally
        // specified 'priority'. Do not block. Return zero if the message is
        // successfully sent or a nonzero value otherwise.

    int unlink();
        // Mark for deletion the message queue opened by this object. Return
        // zero on success or a nonzero value otherwise.

    // ACCESSORS
    PosixQueue::Open::Result openResult() const;
        // Return the result of having opened this queue. The behavior is
        // undefined unless this object owns its 'PosixQueue'.

    bool isOpen() const;
        // Return whether this object represents an open message queue.

    IPCU_DECLARE_OPERATOR_BOOL(QueueSender);
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

  private:
    // PRIVATE MANIPULATORS
    PosixQueue& posixQueue();
        // Return a reference providing modifiable access to the 'PosixQueue'
        // instance used to implement this object.
};

// ============================================================================
//                 INLINE DEFINITIONS
// ============================================================================

                            // -----------------
                            // class QueueSender
                            // -----------------

inline
int QueueSender::send(const bslstl::StringRef& payload)
{
    return send(payload, 0);
}

inline
int QueueSender::send(const bslstl::StringRef&  payload,
                      const bsls::TimeInterval& relativeTimeout)
{
    return send(payload, relativeTimeout, 0);
}

inline
int QueueSender::trySend(const bslstl::StringRef& payload)
{
    return trySend(payload, 0);
}

}  // close package namespace
}  // close enterprise namespace

#endif
