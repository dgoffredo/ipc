#ifndef INCLUDED_IPCMQ_POSIXQUEUE
#define INCLUDED_IPCMQ_POSIXQUEUE

#include <ipcmq_posixqueueerrors.h>

#include <bdeut_variant.h>

#include <bsl_string.h>

namespace BloombergLP {
namespace bslma { class Allocator; }
namespace bsls { class TimeInterval; }
namespace ipcmq {

                        // =========================
                        // namespace PosixQueueTypes
                        // =========================

// This namespace contains types used in the interface of 'PosixQueue'. It
// contains return code enumerations and configuration types specified as
// arguments to 'PosixQueue::open'.
namespace PosixQueueTypes {

struct ReadOnly {
};

struct WriteOnly {
};

struct ReadWrite {
};

struct OpenMode {
    bdeut_Variant3<ReadOnly, WriteOnly, ReadWrite> d_mode;

    template <typename MODE>
    OpenMode(MODE mode)
    : d_mode(mode)
    {
    }

  private:
    OpenMode();  // = delete
};

struct Max {
};

struct Default {
};

struct Attributes {
    bdeut_Variant3<int, Max, Default> d_maxMessages;
    bdeut_Variant3<int, Max, Default> d_maxMessageSize;

    Attributes()
    : d_maxMessages(Default())
    , d_maxMessageSize(Default())
    {
    }
};

struct OpenOnly {
};

struct CreateOnly {
    static const int s_userReadWrite = 0600;
    int              d_permissions;

    explicit CreateOnly(int permissions = s_userReadWrite)
    : d_permissions(permissions)
    {
    }
};

struct OpenOrCreate {
    static const int d_userReadWrite = 0600;
    int              d_permissions;

    explicit OpenOrCreate(int permissions = d_userReadWrite)
    : d_permissions(permissions)
    {
    }
};

struct CreateMode {
    bdeut_Variant3<OpenOnly, CreateOnly, OpenOrCreate> d_mode;

    template <typename MODE>
    CreateMode(MODE mode)
    : d_mode(mode)
    {
    }

  private:
    CreateMode();  // = delete
};

}  // close component types namespace

class PosixQueue_NativeHandle;  // component-private opaque handle type

                                // ================
                                // class PosixQueue
                                // ================

class PosixQueue {
    // This class represents a POSIX message queue.

  public:
#define ALIAS(TYPENAME) typedef PosixQueueTypes::TYPENAME TYPENAME
    // PUBLIC TYPES
    typedef PosixQueue_NativeHandle NativeHandle;
    ALIAS(Attributes);
    ALIAS(Close);
    ALIAS(CreateMode);
    ALIAS(Default);
    ALIAS(Max);
    ALIAS(Open);
    ALIAS(OpenMode);
    ALIAS(Receive);
    ALIAS(Send);
    ALIAS(SetNonBlocking);
    ALIAS(Unlink);
#undef ALIAS

  private:
    // PRIVATE TYPES
    enum OpenState { e_BLOCKING, e_NONBLOCKING, e_CLOSED };

    // DATA
    PosixQueue_NativeHandle *d_handle;
    bsl::string              d_name;
    OpenState                d_openState;
    long                     d_maxMessageSize;

    PosixQueue(const PosixQueue& other);              // = delete
    PosixQueue& operator =(const PosixQueue& other);  // = delete

  public:
    // CREATORS
    explicit PosixQueue(bslma::Allocator *allocator = 0);
    ~PosixQueue();

    // MANIPULATORS
    Open::Result open(const bslstl::StringRef& name,
                      OpenMode                 openMode,
                      CreateMode               createMode,
                      Attributes               attributes = Attributes());
        // Open the message queue having the specified 'name' for reading or
        // writing or both depending on the specified 'openMode', and possibly
        // creating the queue or not or requiring creation based on the
        // specified 'createMode'. If the queue does not exist and creating is
        // permitted, create the queue having the optionally specified
        // 'attributes'. If the queue already exists, then 'attributes' may be
        // used to restrict the properties of the queue as seen through this
        // object. Return zero on success or another 'Open::Result' value
        // otherwise. Note that specifying 'Max()' for any of the fields in
        // 'attributes' might exhaust system resources.

    Close::Result close();
        // Close the message queue. Note that this function does not unlink the
        // queue.

    Send::Result send(const bslstl::StringRef& payload, unsigned priority = 0);
    Send::Result send(const bslstl::StringRef&  payload,
                      const bsls::TimeInterval& deadline,
                      unsigned                  priority = 0);
        // Enqueue a message having the specified 'payload' and the optionally
        // specified 'priority', where higher values of 'priority' are received
        // before lower ones. Optionally specify a 'deadline', which is an
        // absolute offset from the epoch, after which this function will
        // return 'Send::e_TIMED_OUT' if the queue is still full. Return zero
        // on success or another 'Send::Result' value if an error occurs.

    Receive::Result receive(bsl::string *output, unsigned *priority = 0);
    Receive::Result receive(bsl::string               *output,
                            const bsls::TimeInterval&  deadline,
                            unsigned                  *priority = 0);
        // Assign through the specified 'output' the next available message. If
        // the optionally specified 'priority' is not zero, write the priority
        // of the received message through it. Optionally specify a 'deadline',
        // which is an absolute offset from the epoch, after which this
        // function will return 'Receive::e_TIMED_OUT' if the queue is still
        // empty. Return zero on success or another 'Receive::Result' value if
        // an error occurs.

    SetNonBlocking::Result setNonBlocking(bool nonBlocking);
        // Set whether 'send' and 'receive' return immediately. On success, if
        // the specified 'nonBlocking' is 'true', then calls to 'send' and
        // 'receive' will return immediately, and if the specified
        // 'nonBlocking' is 'false', then calls to 'send' and 'receive' might
        // block if the queue is full or empty, respectively.

    // ACCESSORS
    const bsl::string& name() const;
        // Return the name of the currently opened queue, or an empty string if
        // this queue is not currently open.

    bool isOpen() const;
        // Return whether this object currently represents an open message
        // queue.

    long maxMessageSize() const;
        // Return the current maximum allowed message size for this queue.

    long numCurrentMessages() const;
        // Return the number of messages currently enqueued in this queue.
        // Return zero of this queue is not open or if an error occurs.

    bslma::Allocator *allocator() const;
        // Return the allocator that supplies memory for this object.

    // CLASS METHODS
    static Unlink::Result unlink(const bsl::string& name);
        // Mark for deletion the message queue with the specified 'name'. If
        // successful, the system will delete the queue once all currently open
        // handles to it are closed.

    static long maxMaxMessages();
        // Return the maximum number of messages that the system will allow to
        // be specified when opening a message queue, assuming that the maximum
        // message size is defaulted. Note that this value is calculated once
        // at runtime and then cached.

    static long maxMaxMessageSize();
        // Return the maximum message size that the system will allow to be
        // specified when opening a message queue, assuming that the maximum
        // number of messages is defaulted. Note that this value is calculated
        // once at runtime and then cached.

    static long defaultMaxMessages();
        // Return the maximum number of messages that a default-created queue
        // can hold before blocking senders. Note that this value is calculated
        // once at runtime and then cached.

    static long defaultMaxMessageSize();
        // Return the maximum message size for a default-created queue. Note
        // that this value is calculated once at runtime and then cached.
};

}  // close package namespace
}  // close enterprise namespace

#endif
