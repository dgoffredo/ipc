#ifndef INCLUDED_IPCMQ_QUEUERECEIVER
#define INCLUDED_IPCMQ_QUEUERECEIVER

#include <ipcmq_format.h>
#include <ipcmq_formatutil.h>
#include <ipcu_operatorbool.h>
#include <ipcmq_posixqueue.h>
#include <ipcmq_receiver.h>

#include <bdeut_variant.h>

#include <bsl_string.h>

namespace BloombergLP {
namespace bslma { class Allocator; }
namespace bsls { class TimeInterval; }
namespace ipcmq {

                            // ===================
                            // class QueueReceiver
                            // ===================

class QueueReceiver : public Receiver {
    // This class implements the 'Receiver' protocol using a 'PosixQueue'
    // object.

    // DATA
    bdeut_Variant2<PosixQueue, PosixQueue *> d_queue;
    FormatUtil::Decoder                      d_decoder;
    PosixQueue::Open::Result                 d_openResult;

  public:
    // CREATORS
    QueueReceiver(
        const bslstl::StringRef&       name,
        Format                         format,
        const PosixQueue::Attributes&  attributes = PosixQueue::Attributes(),
        int                            filePermissions = 0,
        bslma::Allocator              *allocator       = 0);
        // Open for reading the message queue having the specified 'name'. Use
        // the specified 'format' when receiving messages. If the queue does
        // not already exist, create a queue having the optionally specified
        // 'attributes' and the optionally specified 'filePermissions'. On
        // success, 'isOpen' will subsequently return 'true'. On failure,
        // 'isOpen' will subsequently return 'false' and 'openResult' will
        // return the error code.

    QueueReceiver(PosixQueue *queue, Format format);
        // Create a 'QueueReceiver' object that can receive messages from the
        // specified 'queue'. Use the specified 'format' when receiving
        // messages.

    // MANIPULATORS
    int receive(bsl::string *payload);
    int receive(bsl::string *payload, unsigned *priority);
    int receive(bsl::string               *payload,
                const bsls::TimeInterval&  relativeTimeout);
    int receive(bsl::string               *payload,
                const bsls::TimeInterval&  relativeTimeout,
                unsigned                  *priority);
        // Assign through the specified 'payload' the content of the next
        // available message on the queue represented by this object. Assign
        // through the optionally specified 'priority' the priority of the
        // message received. Block for no longer than the optionally specified
        // 'relativeTimeout', relative to the beginning of the invocation of
        // this function. Return zero if a message is successfully received or
        // a nonzero value otherwise.

    int tryReceive(bsl::string *payload);
    int tryReceive(bsl::string *payload, unsigned *priority);
        // Assign through the specified 'payload' the content of the next
        // available message on the queue represented by this object. Assign
        // through the optionally specified 'priority' the priority of the
        // message received. Do not block. Return zero if a message is
        // successfully received or a nonzero value otherwise.

    int unlink();
        // Mark for deletion the message queue opened by this object. Return
        // zero on success or a nonzero value otherwise.

    // ACCESSORS
    PosixQueue::Open::Result openResult() const;
        // Return the result of having opened this queue. The behavior is
        // undefined unless this object owns its 'PosixQueue'.

    bool isOpen() const;
        // Return whether this object represents an open message queue.

    IPCU_DECLARE_OPERATOR_BOOL(QueueReceiver);
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

}  // close package namespace
}  // close enterprise namespace

#endif
