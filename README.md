ipc
===

![inter-process communication](ipc.jpg)

POSIX Inter-process Communication in C++

Why
---

I wanted to write a portable C++03 compatible wrapper around [POSIX's message
queues][posix mq]. [Boost.Interprocess][boost.interprocess]'s
[MessageQueue][boost.interprocess.messagequeue] class notably does not use
POSIX's message queue functions.

What
----

`ipc` is a BDE-style package group of POSIX-portable inter-process
communication mechanisms. It's possible that it will only implement message
queues. Note that, unlike [Boost.Interprocess][boost.interprocess], `ipc` does
not attempt to be a maximally portable general purpose library in the sense
that it has separate implementations for different platforms (e.g. Windows).
Instead, it is a wrapper around the lowest common denominator of facilities
specified in POSIX.

Build
-----

`ipc` is built using a BDE-style-specific variant of the `waf` build tool,
because that's what `BDE` uses. For more information, see [how to build a
repository with BDE's waf][bde-build].

More
----

Since `ipc` uses BDE extensively, all of its components are placed in the same
"enterprise namespace" as is BDE, `namespace BloombergLP`.

For questions, comments, or concerns, open up a github "issue" or reach out to
[me][me]. To contribute, see [CONTRIBUTING.md](CONTRIBUTING.md).

[posix mq]: http://pubs.opengroup.org/onlinepubs/009695399/basedefs/mqueue.h.html
[boost.interprocess]: http://www.boost.org/doc/libs/1_63_0/doc/html/interprocess.html
[boost.interprocess.messagequeue]: https://github.com/boostorg/interprocess/blob/develop/include/boost/interprocess/ipc/message_queue.hpp
[bde-build]: http://bloomberg.github.io/bde-tools/waf.html#building-multiple-repos-using-workspaces
[me]: mailto:dmgoffredo@gmail.com
