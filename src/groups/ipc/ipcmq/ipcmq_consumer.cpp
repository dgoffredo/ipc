
#include <ipcmq_consumer.h>

#include <bael_log.h>

#include <bdlf_memfn.h>

#include <bslma_stdallocator.h>

#include <bsls_timeinterval.h>

namespace BloombergLP {
namespace ipcmq {
namespace {

const char k_LOG_CATEGORY[] = "IPCMQ.CONSUMER";

// Poll at most once every 100 milliseconds.
const bsls::TimeInterval k_TIMEOUT(0, 100 * 1000 * 1000);

}  // close unnamed namespace

// CREATORS
Consumer::Consumer(const bslstl::StringRef&       name,
                   Format                         format,
                   const MessageCallback&         callback,
                   const PosixQueue::Attributes&  attributes,
                   int                            filePermissions,
                   bslma::Allocator              *allocator)
: d_shuttingDown(false)
, d_messageBuffer(allocator)
, d_receiver(name, format, attributes, filePermissions, allocator)
, d_callback(bsl::allocator_arg_t(),
             bsl::allocator<MessageCallback>(allocator),
             callback)
, d_thread(bslmt::ThreadUtil::invalidHandle())
{
    BAEL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

    const int rc = bslmt::ThreadUtil::create(
                  &d_thread, bdlf::MemFnUtil::memFn(&Consumer::consume, this));
    if (rc) {
        BAEL_LOG_ERROR << "Unable to start consumer thread for consumer of "
                          "the message queue "
                       << name << BAEL_LOG_END;
    }
}

Consumer::~Consumer()
{
    BAEL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

    if (d_thread == bslmt::ThreadUtil::invalidHandle()) {
        // Thread never started. Nothing to join.
        return;                                                       // RETURN
    }

    d_shuttingDown = true;
    const int rc   = bslmt::ThreadUtil::join(d_thread);
    if (rc) {
        BAEL_LOG_ERROR << "Unable to join consumer thread. On a POSIX system, "
                          "this most likely means that a deadlock was "
                          "detected. The unjoined thread might access data "
                          "members of this object after it is destroyed, so "
                          "the program might now be in a bad state. "
                          "bslmt::ThreadUtil::join returned rc="
                       << rc << BAEL_LOG_END;
    }
}

// MANIPULATORS
void Consumer::consume()
{
    BAEL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

    while (!d_shuttingDown.load()) {
        unsigned  priority;
        const int rc =
            d_receiver.receive(&d_messageBuffer, k_TIMEOUT, &priority);
        if (rc == 0) {
            d_callback(&d_messageBuffer, priority);
        }
        else if (rc != int(PosixQueue::Receive::e_TIMED_OUT)) {
            // Note that the condition above assumes that
            // 'QueueReceiver::receive' returns a 'PosixQueue::Receive::Result'
            // cast to an 'int', which we are not allowed to assume based on
            // the contract. But it does, so we will.
            BAEL_LOG_ERROR << "Unable to receive message from message queue: "
                           << d_receiver.description(rc) << BAEL_LOG_END;
        }
    }
}

}  // close package namespace
}  // close enterprise namespace
