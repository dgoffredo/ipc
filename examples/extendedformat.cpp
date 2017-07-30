
#include <ipcmq_format.h>
#include <ipcmq_queuereceiver.h>
#include <ipcmq_queuesender.h>

#include <bdlf_bind.h>

#include <bsl_fstream.h>
#include <bsl_iostream.h>
#include <bsl_sstream.h>
#include <bsl_string.h>

#include <bslmt_threadutil.h>

#include <bsls_assert.h>

using namespace BloombergLP;

void send(char *argv[])
{
    ipcmq::QueueSender sender("/foo", ipcmq::Format::e_EXTENDED);

    for (const char *const *arg = argv + 1; *arg; ++arg) {
        bsl::ifstream      file(*arg);
        bsl::ostringstream oss;
        oss << file.rdbuf();
        const int rc = sender.send(oss.str());
        if (rc) {
            bsl::cerr << "Error: " << sender.description(rc) << '\n';
        }
        else {
            bsl::cout << "Sent a message.\n";
        }
    }
}

void receive()
{
    ipcmq::QueueReceiver receiver("/foo", ipcmq::Format::e_EXTENDED);
    for (bsl::string message; 0 == receiver.receive(&message);
         bsl::cout << message << '\n')
        ;
}

int main(int argc, char *argv[])
{
    BSLS_ASSERT(argc > 1);

    typedef bslmt::ThreadUtil Thread;
    typedef bdlf::BindUtil    Bind;

    Thread::Handle senderThread;
    // Thread::Handle receiverThread;
    Thread::create(&senderThread, Bind::bind(&send, argv));
    // Thread::create(&receiverThread, &receive);

    Thread::join(senderThread);
    // Thread::join(receiverThread);
}
