#ifndef INCLUDED_IPCMQ_CONSUMER
#define INCLUDED_IPCMQ_CONSUMER

#include <ipcmq_posixqueue.h>
#include <ipcmq_queuereceiver.h>

#include <bsl_functional.h>
#include <bsl_string.h>

#include <bslmt_threadutil.h>

#include <bsls_atomic.h>

namespace BloombergLP {
namespace bslma { class Allocator; }
namespace ipcmq {

class Consumer {
    // This class manages a thread that receives messages from a message queue,
    // invoking a callback function with each message received.

  public:
    // PUBLIC TYPES
    typedef bsl::function<void(bsl::string *, unsigned)> MessageCallback;

  private:
    // DATA
    bsls::AtomicInt           d_shuttingDown;
    bsl::string               d_messageBuffer;
    QueueReceiver             d_receiver;
    MessageCallback           d_callback;  // message, priority
    bslmt::ThreadUtil::Handle d_thread;

  public:
    // CREATORS
    Consumer(
        const bslstl::StringRef&       name,
        Format                         format,
        const MessageCallback&         callback,
        const PosixQueue::Attributes&  attributes = PosixQueue::Attributes(),
        int                            filePermissions = 0,
        bslma::Allocator              *allocator       = 0);
        // Create a 'Consumer' object that receives from the message queue with
        // the specified 'name' in the specified 'format', invoking the
        // specified 'callback' with every message received and its priority.
        // Optionally specify 'attributes' and 'filePermissions', which will be
        // used when creating the queue if the queue does not already exist.
        // This object will begin consuming messages immediately.

    ~Consumer();
        // Send a "stop" notification to the thread managed by this object and
        // wait for it to finish. Then destroy this object.

    // ATTRIBUTES
    bool isOpen() const;
        // Return whether the queue consumed by this object is open.

  private:
    // PRIVATE MANIPULATORS
    void consume();
};

}  // close package namespace
}  // close enterprise namespace

#endif
