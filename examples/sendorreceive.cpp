
#include <ipcmq_format.h>
#include <ipcmq_queuereceiver.h>
#include <ipcmq_queuesender.h>
#include <ipcmq_receiver.h>
#include <ipcmq_sender.h>

#include <bsl_iostream.h>
#include <bsl_string.h>

#include <bsls_assert.h>
#include <bsls_timeinterval.h>

using namespace BloombergLP;

int send(ipcmq::Sender *queue, const bslstl::StringRef& message)
{
    BSLS_ASSERT(queue);

    return queue->send(message);
}

int sendMain(int argc, char *argv[])
{
    BSLS_ASSERT(argc == 3);

    ipcmq::QueueSender queue(argv[1], ipcmq::Format::e_RAW);
    if (!queue) {
        // TODO describe
        // bsl::cerr << "Error: " << ipcmq::description(queue.openResult())
        //           << '\n';
        return 1;                                                     // RETURN
    }

    const int rc = send(&queue, argv[2]);
    if (rc) {
        // TODO (haven't upgraded the error reporting for Queue yet).
    }

    return rc;
}

int receive(ipcmq::Receiver *queuePtr, const bslstl::StringRef& flavor)
{
    BSLS_ASSERT(queuePtr);
    ipcmq::Receiver& queue = *queuePtr;

    bsl::string message;
    unsigned    priority;
    int         rc;

    if (flavor == "blocking") {
        rc = queue.receive(&message, &priority);
    }
    else if (flavor == "nonblocking") {
        rc = queue.tryReceive(&message, &priority);
    }
    else {
        BSLS_ASSERT(flavor == "timeout");
        rc = queue.receive(
                      &message, bsls::TimeInterval().addSeconds(3), &priority);
    }

    // success?
    if (rc == 0) {
        bsl::cout << "Received a message with priority " << priority << ": "
                  << message << '\n';
    }

    return rc;
}

int receiveMain(int argc, char *argv[])
{
    BSLS_ASSERT(argc == 3);
    ipcmq::QueueReceiver queue(argv[1], ipcmq::Format::e_RAW);
    if (!queue) {
        // TODO describe
        return 1;                                                     // RETURN
    }

    const int rc = receive(&queue, argv[2]);
    if (rc) {
        // TODO describe
    }

    return rc;
}

int main(int argc, char *argv[])
{
    BSLS_ASSERT(argc > 1);
    const bslstl::StringRef command = argv[1];

    if (command == "send") {
        return sendMain(argc - 1, argv + 1);                          // RETURN
    }

    BSLS_ASSERT(command == "receive");
    return receiveMain(argc - 1, argv + 1);
}
