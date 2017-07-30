
#include <ipcmq_algoutil.h>

#include <bsl_iostream.h>
#include <bsl_limits.h>

using namespace BloombergLP;

bool notTooBig(int x)
{
    return x < 100000;
}

int main()
{
    for (int start = 0; start != 100000; ++start) {
        const int answer = ipcmq::AlgoUtil::findMaxIf(start, &notTooBig);
        bsl::cout << "starting at " << start << " gives the value " << answer
                  << '\n';
    }
}
