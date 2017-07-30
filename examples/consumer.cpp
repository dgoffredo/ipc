
#include <ipcmq_consumer.h>
#include <ipcmq_format.h>

#include <bsl_iostream.h>

void handleMessage(bsl::string *message, unsigned priority)
{
    bsl::cout << "Received a priority " << priority << " message:\n"
              << *message << '\n';
}

int main()
{
    using namespace BloombergLP;
    ipcmq::Consumer consumer(
                            "/foo", ipcmq::Format::e_EXTENDED, &handleMessage);

    bsl::cin.get();
}
