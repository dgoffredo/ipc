#ifndef INCLUDED_IPCMQ_FORMATUTIL
#define INCLUDED_IPCMQ_FORMATUTIL

#include <ipcmq_format.h>

#include <bsl_string.h>

namespace BloombergLP {
namespace ipcmq {

struct FormatUtil {
    // This class is a namespace for a set of functions and types used to
    // encode and decode message queue messages according to various supported
    // protocols (formats).

    // TYPES
    typedef int (*Encoder)(long               maxMessageSize,
                           bslstl::StringRef *originalAndOutput,
                           bsl::string       *messageBuffer);

    typedef int (*Decoder)(bsl::string *originalAndOutput);

    // CLASS METHODS
    static Encoder encoder(Format format);

    static Decoder decoder(Format format);

    static int encodeRaw(long               maxMessageSize,
                         bslstl::StringRef *originalAndOutput,
                         bsl::string       *messageBuffer);
        // Do nothing. Return zero, which indicates success.

    static int decodeRaw(bsl::string *originalAndOutput);
        // Do nothing. Return zero, which indicates success.

    static int encodeExtended(long               maxMessageSize,
                              bslstl::StringRef *originalAndOutput,
                              bsl::string       *messageBuffer);
        // If the specified 'originalAndOutput' is small enough to fit in place
        // within a message, then copy 'originalAndOutput' into the specified
        // 'messageBuffer', append to 'messageBuffer' a byte indicating that
        // the message is in place and modify 'originalAndOutput' to refer to
        // 'messageBuffer'. If 'originalAndOutput' cannot fit in place within a
        // message, then create a temporary file, write 'originalAndOutput' to
        // it, copy the full path to the file into 'messageBuffer', and modify
        // 'originalAndOutput' to refer to 'messageBuffer'. Return zero on
        // success or a nonzero value otherwise.

    static int decodeExtended(bsl::string *originalAndOutput);
        // If the last byte of the specified 'originaAndOutput' indicates that
        // the message is in place, shrink 'originalAndOutput' by one byte byte
        // and return zero, which indicates success. If the last byte of
        // 'originalAndOutput' indicates that the message is a path to a file,
        // open the file whose full path is 'originalAndOutput' (except for the
        // last byte), assign the file's contents to the specified
        // 'messageBuffer', delete the file, and then modify
        // 'originalAndOutput' to refer to 'messageBuffer'.  Return zero on
        // success or a nonzero value otherwise. It is not considered an error
        // if deleting the file fails. If the last byte of 'originalAndOutput'
        // indicates neither of the above, return a nonzero value, which
        // indicates failure.

    static const char *description(int errorCode);
        // Return a description of the specified 'errorCode'. The behavior is
        // undefined unless 'errorCode' has the same value as the result of
        // some previous call to an encoder or decoder.
};

}  // close package namespace
}  // close enterprise namespace

#endif
