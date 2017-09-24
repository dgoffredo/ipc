
#include <ipcmq_posixqueue.h>
#include <ipcu_algoutil.h>

#include <ball_log.h>

#include <bdlb_arrayutil.h>

#include <bdlb_guid.h>
#include <bdlb_guidutil.h>

#include <bslma_default.h>

#include <bslmt_once.h>

#include <bsls_assert.h>
#include <bsls_timeinterval.h>

#include <errno.h>     // error codes
#include <fcntl.h>     // file open constants
#include <limits.h>    // _POSIX_*_MAX
#include <mqueue.h>    // mq_*
#include <string.h>    // strerror
#include <sys/stat.h>  // file mode constants

namespace BloombergLP {
namespace ipcmq {
namespace {

using namespace PosixQueueTypes;

const char k_LOG_CATEGORY[] = "IPCMQ.POSIXQUEUE";

bsl::string systemError(int errorNumber)
    // Return the system-dependent description of the specified 'errorNumber',
    // or return an empty string if an error occurs.
{
    // The guaranteed thread safe 'strerror_r' would be better, but its
    // signature various among platforms.
    return strerror(errorNumber);
}

#define LOG_UNEXPECTED(ERROR_NUMBER)                                          \
    {                                                                         \
        BALL_LOG_SET_CATEGORY(k_LOG_CATEGORY);                                \
        const int eRrOrNuMbEr = (ERROR_NUMBER);                               \
        BALL_LOG_WARN << "Unexpected error number " #ERROR_NUMBER "="         \
                      << eRrOrNuMbEr << " which is the system error "         \
                      << systemError(eRrOrNuMbEr) << BALL_LOG_END;            \
    }

template <typename OPERATION>
typename OPERATION::Result convertBasicError(int errorNumber)
    // Operations like 'mq_close', 'mq_getattr', and 'mq_setattr' fail only
    // if the descriptor is invalid, which could be guaranteed impossible by
    // the 'PosixQueue' class, but since the definition of "valid descriptor"
    // is not perfectly clear, this component accomodates the possibility of
    // those operations failing anyway.
{
    switch (errorNumber) {
      case EBADF:
        return OPERATION::e_BAD_DESCRIPTOR;
      default:
        LOG_UNEXPECTED(errorNumber);
        return OPERATION::e_UNKNOWN;
    }
}

Send::Result convertSendError(int errorNumber)
{
    switch (errorNumber) {
      case EAGAIN:
        return Send::e_FULL;                                          // RETURN
      case EBADF:
        return Send::e_WRONG_MODE;                                    // RETURN
      case EINTR:
        return Send::e_INTERRUPTED;                                   // RETURN
      case EINVAL:
        return Send::e_BAD_PRIORITY_OR_DEADLINE;                      // RETURN
      case EMSGSIZE:
        return Send::e_MESSAGE_TOO_LARGE;                             // RETURN
      case ETIMEDOUT:
        return Send::e_TIMED_OUT;                                     // RETURN
      default:
        LOG_UNEXPECTED(errorNumber);
        return Send::e_UNKNOWN;                                       // RETURN
    }
}

Receive::Result convertReceiveError(int errorNumber)
{
    switch (errorNumber) {
      case EAGAIN:
        return Receive::e_EMPTY;                                      // RETURN
      case EBADF:
        return Receive::e_WRONG_MODE;                                 // RETURN
      case EINTR:
        return Receive::e_INTERRUPTED;                                // RETURN
      case EINVAL:
        return Receive::e_BAD_DEADLINE;                               // RETURN
      case ETIMEDOUT:
        return Receive::e_TIMED_OUT;                                  // RETURN
      case EBADMSG:
        return Receive::e_CORRUPTED_MESSAGE;                          // RETURN
      case EMSGSIZE:
    // 'EMSGSIZE' is in the "unexpected" category since when 'PosixQueue'
    // receives a message, it already knows the maximum message size, so a
    // return code indicating that the specified buffer was too small is
    // unexpected.
      default:
        LOG_UNEXPECTED(errorNumber);
        return Receive::e_UNKNOWN;                                    // RETURN
    }
}

void randomQueueName(bsl::string *outputPtr)
{
    BSLS_ASSERT_SAFE(outputPtr);
    bsl::string& output = *outputPtr;

    output.assign(1, '/');
    output += bdlb::GuidUtil::guidToString(bdlb::GuidUtil::generate());

    // Shrink, if necessary, to fit minimum POSIX size spec.
    const bsl::string::size_type sizes[] = {
        output.size(), _POSIX_PATH_MAX - 1, _POSIX_NAME_MAX - 1};
    output.resize(*bsl::min_element(sizes, bdlb::ArrayUtil::end(sizes)));
}

void closeAndUnlinkTemporaryQueue(mqd_t queue, const bsl::string& name)
{
    BALL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

    int rc = mq_close(queue);
    if (rc == -1) {
        BALL_LOG_WARN << "Unable to close temporary queue. errno=" << errno
                      << BALL_LOG_END;
    }

    rc = mq_unlink(name.c_str());
    if (rc == -1) {
        BALL_LOG_WARN << "Unable to unlink temporary queue. errno=" << errno
                      << BALL_LOG_END;
    }
}

int temporaryQueue(const mq_attr *inputAttributes, mq_attr *outputAttributes)
    // Create and then destroy a message queue with a randomly generated name.
    // If the specified 'inputAttributes' is not zero, specify those attributes
    // when creating the queue. If the specified 'outputAttributes' is not
    // zero, query the created queue's attributes and store them in
    // 'outputAttributes' before destroying the queue. If the generated queue
    // name is not unique, retry an unspecified number of times with a diffent
    // randomly generated name. Return zero on success. If the maximum number
    // of retries is reached, return the nonzero 'errno'. If the creation of
    // the queue fails for some other reason, return the nonzero 'errno'. If
    // 'outputAttributes' is not zero and the querying of the queue's
    // attributes fails, return the nonzero 'errno'.
{
    BALL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

    bsl::string name;
    mqd_t       queue;
    const int   k_MAX_ATTEMPTS = 3;
    for (int attempt = 1; true; ++attempt) {
        randomQueueName(&name);
        // The choice of "write only" is arbitrary. What matters is "create
        // only."
        const int openFlags   = O_WRONLY | O_CREAT | O_EXCL;
        const int permissions = 0600;  // user read/write only
        queue = mq_open(name.c_str(), openFlags, permissions, inputAttributes);

        // error?
        if (queue == mqd_t(-1)) {
            switch (errno) {
              case EEXIST:
                // The one case we can handle. Maybe our 'name' was not unique,
                // so try again unless we've already tried too many times.
                if (attempt != k_MAX_ATTEMPTS) {
                    break;  // break from switch to re-enter loop
                }
              default: {
                BALL_LOG_TRACE
                    << "Unable to create a temporary queue with name=" << name
                    << " got errno=" << errno
                    << " which corresponds to the system error: "
                    << systemError(errno) << BALL_LOG_END;
                return errno;                                         // RETURN
              }
            }
        }
        else {
            // success
            break;  // break from loop so the success code below can execute
        }
    }

    // 'queue' now refers to an open message queue. Get its attributes, if
    // requested. In any case, unlink and close the queue.
    int rc = 0;

    if (outputAttributes) {
        rc = mq_getattr(queue, outputAttributes);
        if (rc == -1) {
            BALL_LOG_WARN
                << "Unable to get attributes of temporary queue. Using "
                   "fallback values. errno="
                << errno << BALL_LOG_END;
            BSLS_ASSERT_SAFE(errno);
            rc = errno;
        }
    }

    closeAndUnlinkTemporaryQueue(queue, name);

    return rc;
}

// pessimistically small fallback values
const long FALLBACK_MAX_MESSAGES     = 1;
const long FALLBACK_MAX_MESSAGE_SIZE = 1024;

mq_attr systemDefaults()
    // Return the attributes of a message queue created without specifying any
    // attributes.
{
    static const mq_attr *ptr = 0;
    BSLMT_ONCE_DO
    {
        BALL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

        static mq_attr fallbackAttributes;
        fallbackAttributes.mq_maxmsg      = FALLBACK_MAX_MESSAGES;
        fallbackAttributes.mq_msgsize     = FALLBACK_MAX_MESSAGE_SIZE;

        static mq_attr outputAttributes;

        const mq_attr *const inputAttributesPtr = 0;  // parameter not used
        if (temporaryQueue(inputAttributesPtr, &outputAttributes)) {
            // error
            ptr = &fallbackAttributes;
        }
        else {
            // success
            ptr = &outputAttributes;
        }

        BSLS_ASSERT_SAFE(ptr);
        BALL_LOG_DEBUG << "system default mq_attr calculated to be: mq_maxmsg="
                       << ptr->mq_maxmsg << " mq_msgsize=" << ptr->mq_msgsize
                       << BALL_LOG_END;
    }

    BSLS_ASSERT_SAFE(ptr);
    return *ptr;
}

long systemDefaultMaxMessages()
{
    return systemDefaults().mq_maxmsg;
}

long systemDefaultMaxMessageSize()
{
    return systemDefaults().mq_msgsize;
}

bool canCreateQueueWith(long maxMessages, long maxMessageSize)
    // Return whether a message queue can be created with the specified
    // 'maxMessages' as its 'mq_msgmax' attribute and with the specified
    // 'maxMessageSize' as its 'mq_msgsize' attribute.
{
    BALL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

    mq_attr inputAttributes;
    inputAttributes.mq_maxmsg  = maxMessages;
    inputAttributes.mq_msgsize = maxMessageSize;

    mq_attr *const outputAttributesPtr = 0;  // parameter not used
    const int      rc = temporaryQueue(&inputAttributes, outputAttributesPtr);

    return rc == 0;
}

bool canCreateQueueWithMaxMessageSize(long maxMessageSize)
    // Return whether a message queue can be created with the specified
    // 'maxMessageSize' as its 'mq_msgsize' attribute when its 'mq_msgmax'
    // attribute is defaulted.
{
    return canCreateQueueWith(systemDefaultMaxMessages(), maxMessageSize);
}

bool canCreateQueueWithMaxMessages(long maxMessages)
    // Return whether a message queue can be created with the specified
    // 'maxMessages' as its 'mq_msgmax' attribute when its 'mq_msgsize'
    // attribute is defaulted.
{
    return canCreateQueueWith(maxMessages, systemDefaultMaxMessageSize());
}

long systemMaxMaxMessages()
    // Return the largest value for a queue's 'mq_msgmax' attribute that this
    // system will allow when the queue's 'mq_msgsize' attribute is defaulted.
{
    static const long *ptr = 0;
    BSLMT_ONCE_DO
    {
        BALL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

        static const long value = ipcu::AlgoUtil::findMaxIf(
                   systemDefaultMaxMessages(), &canCreateQueueWithMaxMessages);
        ptr = &value;

        BALL_LOG_DEBUG
            << "system maximum value for mq_msgmax calcualated to be " << value
            << BALL_LOG_END;
    }

    BSLS_ASSERT_SAFE(ptr);
    return *ptr;
}

long systemMaxMaxMessageSize()
    // Return the largest value for a queue's 'mq_msgsize' attribute that this
    // system will allow when the queue's 'mq_msgmax' attribute is defaulted.
{
    static const long *ptr = 0;
    BSLMT_ONCE_DO
    {
        BALL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

        static const long value = ipcu::AlgoUtil::findMaxIf(
             systemDefaultMaxMessageSize(), &canCreateQueueWithMaxMessageSize);
        ptr = &value;

        BALL_LOG_DEBUG
            << "system maximum value for mq_msgsize calculated to be " << value
            << BALL_LOG_END;
    }

    BSLS_ASSERT_SAFE(ptr);
    return *ptr;
}

timespec toTimespec(const bsls::TimeInterval& interval)
{
    timespec result;
    result.tv_sec   = interval.seconds();
    result.tv_nsec  = interval.nanoseconds();

    return result;
}

}  // close unnamed namespace

                        // =============================
                        // class PosixQueue_NativeHandle
                        // =============================

class PosixQueue_NativeHandle {
  public:
    mqd_t d_descriptor;

    PosixQueue_NativeHandle()
    : d_descriptor()
    {
    }
};

                                // ----------------
                                // class PosixQueue
                                // ----------------

// CREATORS
PosixQueue::PosixQueue(bslma::Allocator *allocator)
: d_handle(new (*bslma::Default::allocator(allocator)) NativeHandle())
, d_name(allocator)
, d_openState(e_CLOSED)
, d_maxMessageSize(FALLBACK_MAX_MESSAGE_SIZE)
{
}

PosixQueue::~PosixQueue()
{
    close();

    BSLS_ASSERT_SAFE(d_handle);
    allocator()->deleteObject(d_handle);
}

// MANIPULATORS
Open::Result PosixQueue::open(const bslstl::StringRef& name,
                              OpenMode                 openMode,
                              CreateMode               createMode,
                              Attributes               attributes)
{
    // TODO This function is too long.

    // Determine how to open the queue and which permissions to use if the
    // queue is to be created.
    int openFlags = 0;
    if (openMode.d_mode.is<ReadOnly>()) {
        openFlags |= O_RDONLY;
    }
    else if (openMode.d_mode.is<WriteOnly>()) {
        openFlags |= O_WRONLY;
    }
    else {
        BSLS_ASSERT(openMode.d_mode.is<ReadWrite>());
        openFlags |= O_RDWR;
    }

    int permissions = 0;
    if (createMode.d_mode.is<OpenOnly>()) {
        // 'permissions' will be ignored.
    }
    else if (createMode.d_mode.is<CreateOnly>()) {
        openFlags |= O_CREAT | O_EXCL;
        permissions = createMode.d_mode.the<CreateOnly>().d_permissions;
    }
    else {
        BSLS_ASSERT(createMode.d_mode.is<OpenOrCreate>());
        openFlags |= O_CREAT;
        permissions = createMode.d_mode.the<CreateOnly>().d_permissions;
    }

    // Determine which attributes the queue will have if created.
    mq_attr        attrs;
    const mq_attr *attrsPtr = &attrs;

    // if both the "max messages" and "max message size" attributes are set to
    // 'Default', or if we won't be creating a queue, then we don't need
    // to calculate the 'mq_attr' fields.
    if ((attributes.d_maxMessages.is<Default>() &&
         attributes.d_maxMessageSize.is<Default>()) ||
        createMode.d_mode.is<OpenOnly>()) {
        attrsPtr = 0;
    }
    else {
        // set mq_maxmsg
        if (attributes.d_maxMessages.is<int>()) {
            attrs.mq_maxmsg = attributes.d_maxMessages.the<int>();
        }
        else if (attributes.d_maxMessages.is<Default>()) {
            attrs.mq_maxmsg = systemDefaultMaxMessages();
        }
        else {
            BSLS_ASSERT(attributes.d_maxMessages.is<Max>());
            attrs.mq_maxmsg = maxMaxMessages();
        }

        // set mq_msgsize
        if (attributes.d_maxMessageSize.is<int>()) {
            attrs.mq_msgsize = attributes.d_maxMessageSize.the<int>();
        }
        else if (attributes.d_maxMessageSize.is<Default>()) {
            attrs.mq_msgsize = systemDefaultMaxMessageSize();
        }
        else {
            BSLS_ASSERT(attributes.d_maxMessageSize.is<Max>());
            attrs.mq_msgsize = maxMaxMessageSize();
        }
    }

    // Open the message queue.
    const bsl::string nameString(name);
    const mqd_t       queue =
        mq_open(nameString.c_str(), openFlags, permissions, attrsPtr);

    // Error?
    if (queue == mqd_t(-1)) {
        switch (errno) {
          case EACCES:
            return Open::e_PERMISSION_DENIED;                         // RETURN
          case EEXIST:
            return Open::e_ALREADY_EXISTS;                            // RETURN
          case EINTR:
            return Open::e_INTERRUPTED;                               // RETURN
          case EINVAL:
            return Open::e_INVALID_PARAMETER;                         // RETURN
          case EMFILE:
            return Open::e_LIMIT_REACHED;                             // RETURN
          case ENAMETOOLONG:
            return Open::e_NAME_TOO_LONG;                             // RETURN
          case ENFILE:
            return Open::e_LIMIT_REACHED;                             // RETURN
          case ENOENT:
            return Open::e_DOES_NOT_EXIST;                            // RETURN
          case ENOSPC:
            return Open::e_NOT_ENOUGH_SPACE;                          // RETURN
          case ESPIPE:
            // seen on Solaris when a leading '/' isn't used.
            return Open::e_INVALID_PARAMETER;                         // RETURN
          default:
            LOG_UNEXPECTED(errno);
            return Open::e_UNKNOWN;                                   // RETURN
        }
    }

    // Success
    d_openState            = e_BLOCKING;
    d_name                 = nameString;
    d_handle->d_descriptor = queue;

    // Get the maximum message size set for the queue. It might be that we did
    // not create the queue, so we must query it here.
    BSLS_ASSERT_SAFE(d_handle);
    if (mq_getattr(d_handle->d_descriptor, &attrs) == -1) {
        // Error getting attributes of the queue just created.
        // 'd_maxMessageSize' will retain its default value, which is
        // technically larger than the minimum required maximum message size
        // guaranteed by POSIX, but really it's unlikely you'll find a system
        // with a max smaller than this library's chosen default.
    }
    else {
        d_maxMessageSize = attrs.mq_msgsize;
    }

    return Open::e_SUCCESS;
}

Close::Result PosixQueue::close()
{
    if (d_openState == e_CLOSED) {
        return Close::e_CLOSED;                                       // RETURN
    }

    BSLS_ASSERT_SAFE(d_handle);
    const mqd_t queue      = d_handle->d_descriptor;
    d_handle->d_descriptor = mqd_t();
    d_name.clear();

    if (mq_close(queue) == -1) {
        return convertBasicError<Close>(errno);                       // RETURN
    }

    d_openState = e_CLOSED;
    return Close::e_SUCCESS;
}

SetNonBlocking::Result PosixQueue::setNonBlocking(bool nonBlocking)
{
    if ((nonBlocking && d_openState == e_NONBLOCKING) ||
        (!nonBlocking && d_openState == e_BLOCKING)) {
        return SetNonBlocking::e_SUCCESS;                             // RETURN
    }
    if (d_openState == e_CLOSED) {
        return SetNonBlocking::e_CLOSED;                              // RETURN
    }

    mq_attr attributes;
    BSLS_ASSERT_SAFE(d_handle);
    if (mq_getattr(d_handle->d_descriptor, &attributes) == -1) {
        return convertBasicError<SetNonBlocking>(errno);              // RETURN
    }

    // Set the non-blocking bit to 'nonBlocking'.
    if (nonBlocking) {
        attributes.mq_flags |= O_NONBLOCK;
    }
    else {
        attributes.mq_flags &= ~O_NONBLOCK;
    }

    // Note that the third argument of 'mq_setattr' is a pointer to a 'mq_attr'
    // into which the old attributes can be written. We specify null since we
    // don't care.
    if (mq_setattr(d_handle->d_descriptor, &attributes, 0) == -1) {
        return convertBasicError<SetNonBlocking>(errno);              // RETURN
    }

    return SetNonBlocking::e_SUCCESS;
}

Receive::Result PosixQueue::receive(bsl::string *outputPtr, unsigned *priority)
{
    BSLS_ASSERT_OPT(outputPtr);
    BSLS_ASSERT_SAFE(d_handle);

    bsl::string& output = *outputPtr;

    output.resize(d_maxMessageSize);
    BSLS_ASSERT_OPT(!output.empty());

    const ssize_t rc = mq_receive(
                  d_handle->d_descriptor, &output[0], output.size(), priority);

    if (rc == -1) {
        return convertReceiveError(errno);                            // RETURN
    }

    // On success, the returned value is the size of the message received.
    // Resize 'output' so that there aren't any trailing null characters;
    const ssize_t messageSize = rc;
    BSLS_ASSERT_OPT(ssize_t(output.size()) >= messageSize);
    output.resize(messageSize);

    return Receive::e_SUCCESS;
}

Receive::Result PosixQueue::receive(bsl::string               *outputPtr,
                                    const bsls::TimeInterval&  deadline,
                                    unsigned                  *priority)
{
    // TODO This is a near copy/paste of the other 'receive'.
    BSLS_ASSERT_OPT(outputPtr);
    BSLS_ASSERT_SAFE(d_handle);

    bsl::string& output = *outputPtr;

    output.resize(d_maxMessageSize);
    BSLS_ASSERT_OPT(!output.empty());

    const timespec absoluteTime = toTimespec(deadline);
    const ssize_t  rc           = mq_timedreceive(d_handle->d_descriptor,
                                       &output[0],
                                       output.size(),
                                       priority,
                                       &absoluteTime);

    if (rc == -1) {
        return convertReceiveError(errno);                            // RETURN
    }

    // On success, the returned value is the size of the message received.
    // Resize 'output' so that there aren't any trailing null characters;
    const ssize_t messageSize = rc;
    BSLS_ASSERT_OPT(ssize_t(output.size()) >= messageSize);
    BSLS_ASSERT_SAFE(messageSize >= 0);
    output.resize(messageSize);

    return Receive::e_SUCCESS;
}

Send::Result PosixQueue::send(const bslstl::StringRef& payload,
                              unsigned                 priority)
{
    BSLS_ASSERT_SAFE(d_handle);

    if (mq_send(d_handle->d_descriptor,
                payload.data(),
                payload.length(),
                priority) == -1) {
        return convertSendError(errno);                               // RETURN
    }

    return Send::e_SUCCESS;
}

Send::Result PosixQueue::send(const bslstl::StringRef&  payload,
                              const bsls::TimeInterval& deadline,
                              unsigned                  priority)
{
    BSLS_ASSERT_SAFE(d_handle);

    const timespec absoluteTime = toTimespec(deadline);

    if (mq_timedsend(d_handle->d_descriptor,
                     payload.data(),
                     payload.length(),
                     priority,
                     &absoluteTime) == -1) {
        return convertSendError(errno);                               // RETURN
    }

    return Send::e_SUCCESS;
}

// ACCESSORS
const bsl::string& PosixQueue::name() const
{
    return d_name;
}

bool PosixQueue::isOpen() const
{
    return d_openState != e_CLOSED;
}

long PosixQueue::maxMessageSize() const
{
    return d_maxMessageSize;
}

long PosixQueue::numCurrentMessages() const
{
    mq_attr attrs;
    BSLS_ASSERT_SAFE(d_handle);
    if (mq_getattr(d_handle->d_descriptor, &attrs) == -1) {
        BALL_LOG_SET_CATEGORY(k_LOG_CATEGORY);
        BALL_LOG_WARN << "Unable to get queue attributes. Returning zero for "
                         "'numCurrentMessages()'."
                      << BALL_LOG_END;
        return 0;                                                     // RETURN
    }

    return attrs.mq_curmsgs;
}

bslma::Allocator *PosixQueue::allocator() const
{
    bslma::Allocator *const result = d_name.get_allocator().mechanism();
    BSLS_ASSERT_SAFE(result);

    return result;
}

// CLASS METHODS
Unlink::Result PosixQueue::unlink(const bsl::string& name)
{
    // success?
    if (mq_unlink(name.c_str()) != -1) {
        return Unlink::e_SUCCESS;
    }

    // error
    switch (errno) {
      case EACCES:
        return Unlink::e_PERMISSION_DENIED;                           // RETURN
      case EINTR:
        return Unlink::e_INTERRUPTED;                                 // RETURN
      case EINVAL:
        // Encountered on Linux when an empty 'name' was specified.
        return Unlink::e_INVALID_PARAMETER;                           // RETURN
      case ENOENT:
        return Unlink::e_DOES_NOT_EXIST;                              // RETURN
      case ENAMETOOLONG:
        return Unlink::e_NAME_TOO_LONG;                               // RETURN
      default:
        LOG_UNEXPECTED(errno);
        return Unlink::e_UNKNOWN;                                     // RETURN
    }
}

long PosixQueue::maxMaxMessages()
{
    return systemMaxMaxMessages();
}

long PosixQueue::maxMaxMessageSize()
{
    return systemMaxMaxMessageSize();
}

long PosixQueue::defaultMaxMessages()
{
    return systemDefaultMaxMessages();
}

long PosixQueue::defaultMaxMessageSize()
{
    return systemDefaultMaxMessageSize();
}

#undef LOG_UNEXPECTED

}  // close package namespace
}  // close enterprise namespace
