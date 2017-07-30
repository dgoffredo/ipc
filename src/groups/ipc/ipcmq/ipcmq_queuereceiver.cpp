
#include <ipcmq_queuereceiver.h>
#include <ipcu_algoutil.h>

#include <bdlt_currenttime.h>

#include <bsls_assert.h>
#include <bsls_timeinterval.h>

namespace BloombergLP {
namespace ipcmq {

// CREATORS
QueueReceiver::QueueReceiver(const bslstl::StringRef&       name,
                             Format                         format,
                             const PosixQueue::Attributes&  attributes,
                             int                            permissions,
                             bslma::Allocator              *allocator)
: d_queue(allocator)
, d_decoder(FormatUtil::decoder(format))
{
    d_queue.createInPlace<PosixQueue>(allocator);

    using namespace PosixQueueTypes;
    const CreateMode createMode(permissions ? OpenOrCreate(permissions)
                                            : OpenOrCreate());
    d_openResult = posixQueue().open(name, ReadOnly(), createMode, attributes);
}

QueueReceiver::QueueReceiver(PosixQueue *queue, Format format)
: d_queue(queue)
, d_decoder(FormatUtil::decoder(format))
{
    BSLS_ASSERT(queue);
}

// MANIPULATORS
int QueueReceiver::receive(bsl::string *payload)
{
    return receive(payload, 0);
}

int QueueReceiver::receive(bsl::string               *payload,
                           const bsls::TimeInterval&  relativeTimeout)
{
    return receive(payload, relativeTimeout, 0);
}

int QueueReceiver::tryReceive(bsl::string *payload)
{
    return tryReceive(payload, 0);
}

int QueueReceiver::receive(bsl::string *payload, unsigned *priority)
{
    using namespace PosixQueueTypes;
    BSLS_ASSERT(payload);

    // This 'receive' is blocking (and without a timeout)
    if (const SetNonBlocking::Result rc = posixQueue().setNonBlocking(false)) {
        return rc;                                                    // RETURN
    }

    // Receive a message from the queue.
    if (const Receive::Result rc = posixQueue().receive(payload, priority)) {
        return rc;                                                    // RETURN
    }

    // Decode the message in place.
    return d_decoder(payload);
}

int QueueReceiver::receive(bsl::string               *payload,
                           const bsls::TimeInterval&  relativeTimeout,
                           unsigned                  *priority)
{
    using namespace PosixQueueTypes;
    BSLS_ASSERT(payload);

    // This 'receive' is blocking (though with a timeout)
    if (const SetNonBlocking::Result rc = posixQueue().setNonBlocking(false)) {
        return rc;                                                    // RETURN
    }

    // Receive a message from the queue.
    if (const Receive::Result rc = posixQueue().receive(
            payload, bdlt::CurrentTime::now() + relativeTimeout, priority)) {
        return rc;                                                    // RETURN
    }

    // Decode the message in place.
    return d_decoder(payload);
}

int QueueReceiver::tryReceive(bsl::string *payload, unsigned *priority)
{
    using namespace PosixQueueTypes;
    BSLS_ASSERT(payload);

    // 'tryReceive' is non-blocking
    if (const SetNonBlocking::Result rc = posixQueue().setNonBlocking(true)) {
        return rc;                                                    // RETURN
    }

    // Receive a message from the queue.
    if (const Receive::Result rc = posixQueue().receive(payload, priority)) {
        return rc;                                                    // RETURN
    }

    // Decode the message in place.
    return d_decoder(payload);
}

int QueueReceiver::unlink()
{
    return PosixQueue::unlink(posixQueue().name());
}

PosixQueue& QueueReceiver::posixQueue()
{
    return const_cast<PosixQueue&>(
        static_cast<const QueueReceiver&>(*this).posixQueue());
}

// ACCESSORS
bool QueueReceiver::isOpen() const
{
    return posixQueue().isOpen();
}

IPCU_DEFINE_OPERATOR_BOOL(QueueReceiver)
{
    IPCU_RETURN_OPERATOR_BOOL(QueueReceiver, isOpen());
}

const PosixQueue& QueueReceiver::posixQueue() const
{
    return *d_queue.applyRaw(ipcu::AlgoUtil::GetPtr<const PosixQueue>());
}

PosixQueue::Open::Result QueueReceiver::openResult() const
{
    BSLS_ASSERT(d_queue.is<PosixQueue>());

    return d_openResult;
}

// CLASS METHODS
const char *QueueReceiver::description(int errorCode)
{
    return FormatUtil::description(errorCode);
}

}  // close package namespace
}  // close enterprise namespace
