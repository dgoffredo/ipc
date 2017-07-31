#include <ball_defaultobserver.h>
#include <ball_log.h>
#include <ball_loggermanager.h>
#include <ball_loggermanagerconfiguration.h>

#include <bdlt_currenttime.h>

#include <bsl_iostream.h>

#include <bsls_assert.h>

#include <ipcmq_posixqueue.h>

using namespace BloombergLP;

namespace {

ball_LoggerManagerConfiguration defaultLoggingConfiguration()
{
    ball_LoggerManagerConfiguration configuration;
    configuration.setDefaultThresholdLevelsIfValid(ball_Severity::BALL_DEBUG);
    return configuration;
}

class BallCrap {
    ball_DefaultObserver          d_observer;
    ball_LoggerManagerScopedGuard d_scopedGuard;

  public:
    BallCrap()
    : d_observer(bsl::cout)
    , d_scopedGuard(&d_observer, defaultLoggingConfiguration())
    {
    }
};

}  // close unnamed namespace

int main(int argc, char *argv[])
{
    BSLS_ASSERT(argc >= 2);

    BallCrap ball;

    ipcmq::PosixQueue queue;

    using namespace ipcmq::PosixQueueTypes;

    // TODO It would be nice to say 'Try(10000)', which would set it to
    //      'min(...max value..., 10000)'.
    Attributes attributes;
    attributes.d_maxMessages    = Max();
    attributes.d_maxMessageSize = Default();

    Open::Result rc1 =
        queue.open(argv[1], ReadWrite(), OpenOrCreate(), attributes);
    bsl::cout << "open() returned: " << ipcmq::description(rc1) << '\n';

    SetNonBlocking::Result rc2 = queue.setNonBlocking(true);
    bsl::cout << "setNonBlocking(true) returned: " << ipcmq::description(rc2)
              << '\n';

    SetNonBlocking::Result rc3 = queue.setNonBlocking(false);
    bsl::cout << "setNonBlocking(false) returned: " << ipcmq::description(rc3)
              << '\n';

    if (argc >= 3) {
        const bslstl::StringRef command = argv[2];
        if (command == "unlink") {
            BSLS_ASSERT(argc == 3);

            Unlink::Result rc4 = ipcmq::PosixQueue::unlink(queue.name());
            bsl::cout << "unlink(\"" << queue.name()
                      << "\") returned: " << ipcmq::description(rc4) << '\n';
        }
        else if (command == "receive" || command == "timedreceive") {
            BSLS_ASSERT(argc == 3);
            Receive::Result rc;
            bsl::string     message;
            if (command == "receive") {
                rc = queue.receive(&message);
            }
            else {
                BSLS_ASSERT(command == "timedreceive");
                rc = queue.receive(&message,
                                   bdlt::CurrentTime::now().addSeconds(5.0));
            }

            bsl::cout << "receive returned: " << ipcmq::description(rc) << '\n'
                      << "message: " << message << '\n';
        }
        else {
            BSLS_ASSERT(command == "send" || command == "timedsend");
            BSLS_ASSERT(argc == 4);
            const bslstl::StringRef message = argv[3];
            Send::Result            rc;
            if (command == "send") {
                rc = queue.send(message);
            }
            else {
                BSLS_ASSERT(command == "timedsend");
                rc = queue.send(message,
                                bdlt::CurrentTime::now().addSeconds(5.0));
            }

            bsl::cout << "send(\"" << message
                      << "\") returned: " << ipcmq::description(rc) << '\n';
        }
    }

    Close::Result rc5 = queue.close();
    bsl::cout << "close() returned: " << ipcmq::description(rc5) << '\n';
}
