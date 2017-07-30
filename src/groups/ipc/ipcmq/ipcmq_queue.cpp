
#include <ipcmq_queue.h>

namespace BloombergLP {
namespace ipcmq {

Queue::Queue(const bslstl::StringRef&       name,
             Format                         format,
             const PosixQueue::Attributes&  attributes,
             int                            permissions,
             bslma::Allocator              *allocator,
             bslma::Allocator              *messageAllocator)
: d_queue(allocator)
, d_sender(&d_queue, format, messageAllocator)
, d_receiver(&d_queue, format)
{
    using namespace PosixQueueTypes;

    const CreateMode createMode(permissions ? OpenOrCreate(permissions)
                                            : OpenOrCreate());
    d_openResult = d_queue.open(name, ReadWrite(), createMode, attributes);
}

// MANIPULATORS
int Queue::receive(bsl::string *payload)
{
    return d_receiver.receive(payload);
}

int Queue::receive(bsl::string *payload, unsigned *priority)
{
    return d_receiver.receive(payload, priority);
}

int Queue::receive(bsl::string               *payload,
                   const bsls::TimeInterval&  relativeTimeout)
{
    return d_receiver.receive(payload, relativeTimeout);
}

int Queue::receive(bsl::string               *payload,
                   const bsls::TimeInterval&  relativeTimeout,
                   unsigned                  *priority)
{
    return d_receiver.receive(payload, relativeTimeout, priority);
}

int Queue::tryReceive(bsl::string *payload)
{
    return d_receiver.tryReceive(payload);
}

int Queue::tryReceive(bsl::string *payload, unsigned *priority)
{
    return d_receiver.tryReceive(payload, priority);
}

int Queue::send(const bslstl::StringRef& payload)
{
    return d_sender.send(payload);
}

int Queue::send(const bslstl::StringRef& payload, int priority)
{
    return d_sender.send(payload, priority);
}

int Queue::send(const bslstl::StringRef&  payload,
                const bsls::TimeInterval& relativeTimeout)
{
    return d_sender.send(payload, relativeTimeout);
}

int Queue::send(const bslstl::StringRef&  payload,
                const bsls::TimeInterval& relativeTimeout,
                int                       priority)
{
    return d_sender.send(payload, relativeTimeout, priority);
}

int Queue::trySend(const bslstl::StringRef& payload)
{
    return d_sender.trySend(payload);
}

int Queue::trySend(const bslstl::StringRef& payload, int priority)
{
    return d_sender.trySend(payload, priority);
}

int Queue::unlink()
{
    using namespace PosixQueueTypes;

    return PosixQueue::unlink(d_queue.name());
}

// ACCESSORS
PosixQueue::Open::Result Queue::openResult() const
{
    return d_openResult;
}

bool Queue::isOpen() const
{
    return d_queue.isOpen();
}

IPCU_DEFINE_OPERATOR_BOOL(Queue)
{
    IPCU_RETURN_OPERATOR_BOOL(Queue, isOpen());
}

const PosixQueue& Queue::posixQueue() const
{
    return d_queue;
}

// CLASS METHODS
const char *Queue::description(int errorCode)
{
    return FormatUtil::description(errorCode);
}

}  // close package namespace
}  // close enterprise namespace
