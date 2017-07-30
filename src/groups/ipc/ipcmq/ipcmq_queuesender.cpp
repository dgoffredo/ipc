
#include <ipcmq_queuesender.h>
#include <ipcu_algoutil.h>

#include <bdlma_localsequentialallocator.h>

#include <bdlt_currenttime.h>

namespace BloombergLP {
namespace ipcmq {
namespace {

// 8192 was chosen because it's the observed maximum message size on Linux, and
// is no more than a small multiple of the page size elsewhere.
typedef bdlma::LocalSequentialAllocator<8192> LocalAllocator;

}  // close unnamed namespace

// CREATORS
QueueSender::QueueSender(const bslstl::StringRef&       name,
                         Format                         format,
                         const PosixQueue::Attributes&  attributes,
                         int                            permissions,
                         bslma::Allocator              *allocator,
                         bslma::Allocator              *messageAllocator)
: d_queue(allocator)
, d_encoder(FormatUtil::encoder(format))
, d_messageAllocator(messageAllocator)
{
    d_queue.createInPlace<PosixQueue>(allocator);

    using namespace PosixQueueTypes;

    const CreateMode createMode(permissions ? OpenOrCreate(permissions)
                                            : OpenOrCreate());
    d_openResult =
        posixQueue().open(name, WriteOnly(), createMode, attributes);
}

QueueSender::QueueSender(PosixQueue       *queue,
                         Format            format,
                         bslma::Allocator *messageAllocator)
: d_queue(queue)
, d_encoder(FormatUtil::encoder(format))
, d_messageAllocator(messageAllocator)
{
    BSLS_ASSERT(queue);
}

// MANIPULATORS
int QueueSender::send(const bslstl::StringRef& payload, int priority)
{
    using namespace PosixQueueTypes;

    // This flavor of 'send' blocks (and has no timeout).
    if (const SetNonBlocking::Result rc = posixQueue().setNonBlocking(false)) {
        return rc;                                                    // RETURN
    }

    bslstl::StringRef encodedMessage = payload;
    LocalAllocator    allocator(d_messageAllocator);
    bsl::string       messageBuffer(&allocator);
    if (const int rc = d_encoder(
            posixQueue().maxMessageSize(), &encodedMessage, &messageBuffer)) {
        return rc;                                                    // RETURN
    }

    return posixQueue().send(encodedMessage, priority);
}

int QueueSender::send(const bslstl::StringRef&  payload,
                      const bsls::TimeInterval& relativeTimeout,
                      int                       priority)
{
    using namespace PosixQueueTypes;

    // This flavor of 'send' blocks (even though it has a timeout).
    if (const SetNonBlocking::Result rc = posixQueue().setNonBlocking(false)) {
        return rc;                                                    // RETURN
    }

    bslstl::StringRef encodedMessage = payload;
    LocalAllocator    allocator(d_messageAllocator);
    bsl::string       messageBuffer(&allocator);
    if (const int rc = d_encoder(
            posixQueue().maxMessageSize(), &encodedMessage, &messageBuffer)) {
        return rc;                                                    // RETURN
    }

    return posixQueue().send(
        encodedMessage, bdlt::CurrentTime::now() + relativeTimeout, priority);
}

int QueueSender::trySend(const bslstl::StringRef& payload, int priority)
{
    using namespace PosixQueueTypes;

    // 'trySend' does not block.
    if (const SetNonBlocking::Result rc = posixQueue().setNonBlocking(true)) {
        return rc;                                                    // RETURN
    }

    bslstl::StringRef encodedMessage = payload;
    LocalAllocator    allocator(d_messageAllocator);
    bsl::string       messageBuffer(&allocator);
    if (const int rc = d_encoder(
            posixQueue().maxMessageSize(), &encodedMessage, &messageBuffer)) {
        return rc;                                                    // RETURN
    }

    return posixQueue().send(encodedMessage, priority);
}

int QueueSender::unlink()
{
    return PosixQueue::unlink(posixQueue().name());
}

PosixQueue& QueueSender::posixQueue()
{
    return const_cast<PosixQueue&>(
        static_cast<const QueueSender&>(*this).posixQueue());
}

// ACCESSORS
PosixQueue::Open::Result QueueSender::openResult() const
{
    BSLS_ASSERT(d_queue.is<PosixQueue>());

    return d_openResult;
}

bool QueueSender::isOpen() const
{
    return posixQueue().isOpen();
}

IPCU_DEFINE_OPERATOR_BOOL(QueueSender)
{
    IPCU_RETURN_OPERATOR_BOOL(QueueSender, isOpen());
}

const PosixQueue& QueueSender::posixQueue() const
{
    return *d_queue.applyRaw(ipcu::AlgoUtil::GetPtr<const PosixQueue>());
}

// CLASS METHODS
const char *QueueSender::description(int errorCode)
{
    return FormatUtil::description(errorCode);
}

}  // close package namespace
}  // close enterprise namespace
