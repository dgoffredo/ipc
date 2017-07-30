
#include <bdlf_bind.h>

#include <bsl_iostream.h>
#include <bsl_string.h>

#include <bslmt_threadutil.h>

#include <bsls_assert.h>
#include <bsls_timeinterval.h>

#include <ipcmq_posixqueue.h>

using namespace BloombergLP;

void readForever(ipcmq::PosixQueue *queuePtr)
{
    BSLS_ASSERT(queuePtr);
    ipcmq::PosixQueue& queue = *queuePtr;

    bsl::string message;
    for (;;) {
        using namespace ipcmq::PosixQueueTypes;
        const Receive::Result rc = queue.receive(&message);
        if (rc) {
            bsl::cerr << "Error on recieve: " << ipcmq::description(rc)
                      << '\n';
            break;
        }
    }
}

// See whether closing a queue while a thread is blocking on mq_receive is an
// error.
int main(int argc, char *argv[])
{
    BSLS_ASSERT(argc == 2);

    using namespace ipcmq::PosixQueueTypes;
    ipcmq::PosixQueue  queue;
    const Open::Result rc1 = queue.open(argv[1], ReadOnly(), OpenOrCreate());
    if (rc1) {
        bsl::cerr << "Error opening queue: " << ipcmq::description(rc1) << '\n';
        return 1;
    }

    bslmt::ThreadUtil::Handle thread;
    const int                 rc2 = bslmt::ThreadUtil::create(
                          &thread, bdlf::BindUtil::bind(&readForever, &queue));
    if (rc2) {
        bsl::cerr << "bslmt::ThreadUtil::create returned error code " << rc2
                  << '\n';
        return 2;
    }

    bslmt::ThreadUtil::sleep(bsls::TimeInterval().addSeconds(3));

    queue.close();
    bslmt::ThreadUtil::join(thread);

    bsl::cout << "Goodbye." << bsl::endl;
}
