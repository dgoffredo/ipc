ipcmq
=====

Inter-process Communication via POSIX Message Queues

Components
----------

#### ipcmq\_sender
Provides the `ipcmq::Sender` protocol for sending messages to a queue.

#### ipcmq\_receiver
Provides the `ipcmq::Receiver` protocol for receiving messages from a queue.

#### ipcmq\_posixqueue
Provides `ipcmq::PosixQueue`, a thin wrapper around POSIX's `mq_*` functions
for creating, destroying, opening, closing, sending to, and receiving from
message queues. This component is meant to be used in the implementation of
higher-level components in this package, but might be generally useful. Note
that `mq_notify` is not supported.

#### ipcmq\_queuesender
Provides `ipcmq::QueueSender`, an implementation of the `ipcmq::Sender`
protocol using an `ipcmq::PosixQueue` opened in write mode.

#### ipcmq\_queuereceiver
Provides `ipcmq::QueueReceiver`, an implementation of the `ipcmq::Receiver`
protocol using an `ipcmq::PosixQueue` opened in read mode.

#### ipcmq\_queue
Provides `ipcmq::Queue`, an implementation of the `ipcmq::Sender` and
`ipcmq::Receiver` protocols using an `ipc::PosixQueue` opened in read/write
mode.

#### ipcmq\_consumer
Provides `ipcmq::Consumer`, a class that manages a dedicated thread that
receives from a message queue using an `ipc::QueueReceiver` instance and
invokes a specified callback for each message received.

#### ipcmq\_format
Provides `ipcmq::Format`, a `struct` acting as a namespace for an enumeration
of message formats supported by this package.

#### ipcmq\_formatutil
Provides `ipcmq::FormatUtil`, a `struct` acting as a namespace for functions
that encode and decode messages in the formats supported by this package.

Message Format
--------------

POSIX message queues contain messages that are just sequences of bytes, as with
any other Unix-flavored IPC. However, since POSIX message queue messages have
an implementation-defined maximum size that is often no larger than a memory
page, it is convenient to have a message format that contains message data if
it can fit, but otherwise refers to some message-external location where the
full, large, payload can be found. Thus, `ipcmq` defines two message formats:

### `ipcmq::Format::e_RAW`

The raw message format is just bytes in the message. If the message is too
large, then sending will fail.

### `ipcmq::Format::e_EXTENDED`

The extended message format ends each message with a discriminator byte that
is: 

- zero if the message payload can fit inside of the message, in which case the
  preceding bytes of the message are the payload
- one if the message payload is too large to fit inside of the message, in
  which case the preceding bytes of of the message are the full path to a
  temporary file that contains the message payload
- some other value for future use. As of this writing, receivers will log a
  diagnostic and ignore such messages when they are encountered.

The receiver of an extended message with an external payload is responsible for
deleting the temporary file.

Note that since POSIX message queues can be shared between a program using
`ipcmq` and another program that is not, using the extended message format
is to assume that all senders and receivers for the message queue are either
using `ipcmq` with the extended format, or using some other code that
implements the same message format.

Example Usage
-------------

Suppose an application occasionally sends a large message to a queue
representing the work queue of some local service. The sending application
sends infrequently enough that is makes sense to open and close the queue each
time, and is willing to wait (block) during the send if the queue is full. In
this case, a one-off `ipcmq::QueueSender` is the best fit:
```C++
int writeHome(const bsl::string& letter)
{
    ipcmq::QueueSender queue("/home", ipcmq::Format::e_EXTENDED);
    if (!queue) {
        bsl::cerr << "Error: unable to open home queue: "
                  << queue.description(queue.openResult()) << '\n';
        return 1;
    }

    const int rc = queue.send(letter);
    if (rc) {
        bsl::cerr << "Error: unable to send a letter home: "
                  << queue.description(rc) << '\n';
        return 2;
    }

    return 0;
}
```
Or, without reporting errors:
```C++
ipcmq::QueueSender queue("/home", ipcmq::Format::e_EXTENDED);
queue.send(letter);
```
Suppose an application wants to drain a specified queue, printing each payload
to `stdout` followed by a newline. In this case, an `icpmq::QueueReceiver` in
a loop is the best fit:
```C++
int main(int argc, char *argv[])
{
   if (argc != 2) {
       usage();
       return 1;
   }

   const char *const name = argv[1];
   ipcmq::QueueReceiver queue(name, ipcmq::Format::e_EXTENDED);
   if (!queue) {
       bsl::cerr << "Unable to open queue " << name << ": "
                 << queue.description(queue.openResult()) << '\n';
       return 2;
   }

   bsl::string message;
   for (;;) {
       const int rc = queue.receive(&message);
       if (rc) {
           bsl::cerr << "Unable to receive message: "
                     << queue.description(rc) << '\n';
           return 3;
       }

       bsl::cout << message << '\n';
   }
}
```
Suppose an application want to drain a specified set of queues, printing each
payload to `stdout` followed by a newline. In this case, one `ipcmq::Consumer`
per queue is the best fit:
```C++
void handleMessage(bslmt::Mutex       *coutMutex,
                   const bsl::string&  message)
{
    bdlmt::LockGuard<bslmt::Mutex> lock(coutMutex);

    bsl::cout << message << '\n';
}

int main(int argc, char *argv[])
{
    using namespace bdlf::PlaceHolders;
    bslmt::Mutex coutMutex;

    bsl::cerr << "Enter any text to exit.\n";

    // Create an 'ipcmq::Consumer' for each queue specified on the command
    // line. Each will receive from one of the queues and print its messages to
    // 'bsl::cout' until destroyed.
    bsl::vector<bsl::shared_ptr<ipcmq::Consumer> > consumers;
    for (const char *const *arg = argv + 1; *arg; ++arg) {
        consumers.push_back(bsl::make_shared<ipcmq::Consumer>(
            *arg,
            ipcmq::Format::e_EXTENDED,
            bdlf::BindUtil::bind(&handleMessage, &coutMutex, _1)));
    }

    bsl::cin.get();  // block for user input
}
```
Finally, suppose an application wishes to reenact a classic scene from a
masterpiece of film:
```C++
bool satisfied(const bsl::string&)
{
    return false;
}

void jesse(ipcmq::Receiver *from, ipcmq::Sender *to)
{
    bsl::string response;
    do {
        to->send("Dude, what does mine say?");
        from->receive(&response);
    } while (!satisfied(response));
}

void chester(ipcmq::Receiver *from, ipcmq::Sender *to)
{
    bsl::string response;
    do {
        to->send("Sweet! What does mine say?");
        from->receive(&response);
    } while (!satisfied(response));
}

int main()
{
    ipcmq::Queue jesseQueue("/jesse", ipcmq::Format::e_RAW);
    ipcmq::Queue chesterQueue("/chester", ipcmq::Format::e_RAW);

    typedef bslmt::ThreadUtil Thread;
    typedef bdlf::BindUtil    Bind;

    Thread::Handle jesseThread, chesterThread;

    Thread::create(&jesseThread,
                   Bind::bind(&jesse, &chesterQueue, &jesseQueue));

    Thread::create(&chesterThread,
                   Bind::bind(&chester, &jesseQueue, &chesterQueue));

    Thread::join(jesseThread);
    Thread::join(chesterThread);
}
```
