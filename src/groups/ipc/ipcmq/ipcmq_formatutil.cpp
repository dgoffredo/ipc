
#include <ipcmq_formatutil.h>
#include <ipcmq_posixqueueerrors.h>

#include <bael_log.h>

#include <bdeu_arrayutil.h>

#include <bdlma_localsequentialallocator.h>

#include <bdls_filesystemutil.h>

#include <bsl_algorithm.h>
#include <bsl_cstdio.h>
#include <bsl_cstdlib.h>
#include <bsl_cstring.h>
#include <bsl_iomanip.h>

#include <bsls_assert.h>

#include <fcntl.h>     // open and related constants
#include <sys/stat.h>  // open and related constants
#include <unistd.h>    // close, write

namespace BloombergLP {
namespace ipcmq {
namespace {

const char k_LOG_CATEGORY[] = "IPCMQ.FORMATUTIL";

enum { e_ENCODER_ERROR, e_DECODER_ERROR };

const char *const k_ERROR_DESCRIPTIONS[] = {
    // e_ENCODER_ERROR
    "An error occurred while encoding the message.",
    // e_DECODER_ERROR
    "An error occurred while decoding the message."};

const char *errorOverflow(int errorCode)
{
    BSLS_ASSERT(errorCode >= 0);
    BSLS_ASSERT(errorCode < int(bdeu_ArrayUtil::size(k_ERROR_DESCRIPTIONS)));

    return k_ERROR_DESCRIPTIONS[errorCode];
}

const char k_EXTENDED_IN_PLACE      = 0;
const char k_EXTENDED_EXTERNAL_FILE = 1;

class EnvHasValue {
    // This class is a unary function-like object closed over an output string.
    // When invoked with the name of an environment variable, this class
    // returns whether a value is defined for that environment variable. If it
    // is, then its value is written to the bound string.

    bsl::string *d_output_p;

  public:
    explicit EnvHasValue(bsl::string *output)
    : d_output_p(output)
    {
        BSLS_ASSERT(output);
    }

    bool operator()(const char *variableName) const
    {
        const char *const value = bsl::getenv(variableName);
        if (value) {
            *d_output_p = value;
        }

        return value;
    }
};

int tempDirectoryPath(bsl::string *output)
    // Assign through the specified 'output' the path to the system temporary
    // directory. Return zero on success or a nonzero value otherwise. Note
    // that the value written to 'output' might or might not end with a
    // forward slash.
{
    BAEL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

    // This function does the same thing as POSIX implementations of
    // 'std::filesystem::temp_directory_path'.
    BSLS_ASSERT(output);

    const char *const variables[] = {"TMPDIR", "TMP", "TEMP", "TEMPDIR"};
    const char *const *const end  = bdeu_ArrayUtil::end(variables);

    bsl::string              value;
    const char *const *const variable =
        bsl::find_if(variables, end, EnvHasValue(&value));

    if (variable == end) {
        value = "/tmp";
    }

    const bool followLinks = true;
    if (!bdls::FilesystemUtil::isDirectory(value, followLinks)) {
        BAEL_LOG_WARN << "The path \"" << value << "\"";
        if (variable != end) {
            BAEL_STREAM
                << ", which is the value of the environment variable \""
                << *variable << "\",";
        }
        BAEL_STREAM << " is not a directory." << BAEL_LOG_END;
        return 1;                                                     // RETURN
    }

    *output = value;
    return 0;
}

int openTempFile(bsl::string *name)
    // Return a file descriptor to a temporary file that the current user can
    // read and write, and others can only read, and assign its full path
    // through the specified 'name'. Return '-1' if an error occurs. Note that
    // 'name' might still be modified even if this function fails.
{
    BAEL_LOG_SET_CATEGORY(k_LOG_CATEGORY);
    BSLS_ASSERT(name);

    if (tempDirectoryPath(name)) {
        return -1;                                                    // RETURN
    }

    *name += "/mq-message-";
    const bsl::string& prefix       = *name;
    const int          MAX_ATTEMPTS = 3;
    for (int attempt = 1; attempt <= MAX_ATTEMPTS; ++attempt) {
        // Note that 'name' and 'prefix' alias each other. I checked that this
        // is okay.
        bdls::FilesystemUtil::makeUnsafeTemporaryFilename(name, prefix);

        const int permissions =
            0644;  // user can read/write, everyone else can read.
        const int openMode = O_WRONLY | O_CREAT | O_EXCL;
        const int fd       = open(name->c_str(), openMode, permissions);
        if (fd != -1) {
            // success
            return fd;                                                // RETURN
        }

        BAEL_LOG_WARN << "Unable to create temporary file at \"" << *name
                      << "\". Attempt " << attempt << '/' << MAX_ATTEMPTS
                      << " errno: " << bsl::strerror(errno) << BAEL_LOG_END;
    }

    // ran out of retries
    return -1;
}

int writeToTempFile(const bslstl::StringRef& data, bsl::string *path)
    // Write the specified 'data' to a temporary file. Assign through the
    // specified 'path' the full path to the temporary file. Return zero on
    // success or a nonzero value otherwise. Note that the temporary file will
    // not be open by this process when this function returns. Also note that
    // 'path' might be modified even if this function fails.
{
    BAEL_LOG_SET_CATEGORY(k_LOG_CATEGORY);
    BSLS_ASSERT(path);

    const int fd = openTempFile(path);
    if (fd == -1) {
        return fd;                                                    // RETURN
    }

    struct Closer {
        int d_fd;
        Closer(int fd)
        : d_fd(fd)
        {
        }
        ~Closer() { close(d_fd); }
    } const closer(fd);

    const ssize_t writtenSize = write(fd, data.data(), data.length());
    if (writtenSize == -1) {
        BAEL_LOG_ERROR << "Unable to write to temporary file: "
                       << bsl::strerror(errno) << BAEL_LOG_END;
        return errno;                                                 // RETURN
    }

    if (writtenSize != ssize_t(data.length())) {
        BAEL_LOG_ERROR << "Tried to write " << data.length()
                       << " bytes to the temporary file \"" << *path
                       << "\" but only " << writtenSize << " were written."
                       << BAEL_LOG_END;
        return -1;                                                    // RETURN
    }

    return 0;
}

int readAndRemoveFile(bsl::string *bufferPtr)
    // Read into the specified 'bufferPtr' the contents of the file whose full
    // path is the current value of 'bufferPtr'. Return zero on success or a
    // nonzero value otherwise. If this function fails, 'bufferPtr' might still
    // have been modified.
{
    BAEL_LOG_SET_CATEGORY(k_LOG_CATEGORY);
    BSLS_ASSERT(bufferPtr);

    bsl::string&  buffer = *bufferPtr;
    const char   *path   = buffer.data();

    bsl::FILE *const message = bsl::fopen(path, "r");
    if (!message) {
        BAEL_LOG_ERROR << "Unable to open the file \"" << path
                       << "\" for reading: " << bsl::strerror(errno)
                       << BAEL_LOG_END;
        return 1;                                                     // RETURN
    }

    struct Closer {
        bsl::FILE *const  d_file;
        const char       *d_path;
        Closer(bsl::FILE *file, const char *path)
        : d_file(file)
        , d_path(path)
        {
        }
        ~Closer()
        {
            BAEL_LOG_SET_CATEGORY(k_LOG_CATEGORY);

            if (bsl::fclose(d_file)) {
                BAEL_LOG_WARN << "Unable to close file \"" << d_path << '\"'
                              << BAEL_LOG_END;
            }

            if (bsl::remove(d_path)) {
                BAEL_LOG_WARN << "Unable to remove file \"" << d_path << '\"'
                              << BAEL_LOG_END;
            }
        }

    } closer(message, path);

    const bdls::FilesystemUtil::Offset sizeSigned =
        bdls::FilesystemUtil::getFileSize(path);
    if (sizeSigned < 0) {
        BAEL_LOG_ERROR << "Unable to determine the size the the file \""
                       << path << '\"' << BAEL_LOG_END;
        return 2;                                                     // RETURN
    }
    else if (sizeSigned == 0) {
        // success since there's nothing to read
        return 0;                                                     // RETURN
    }

    const bsl::size_t size = sizeSigned;
    const bsl::size_t room = size + 1;  // allocate an extra char, so we can
                                        // detect if the file grew between
                                        // determining its size and reading it.

    buffer.insert(bsl::size_t(0), room, char(0));  // Make room before the path

    path = buffer.data() + room;  // 'path' shifted right with the insert.
    closer.d_path = path;

    const bsl::size_t countRead = bsl::fread(&buffer[0], 1, room, message);
    if (countRead < size) {
        BAEL_LOG_ERROR << "Unable to read entire contents of \"" << path
                       << "\". Expected " << size << " bytes but got only "
                       << countRead << BAEL_LOG_END;
        return 3;                                                     // RETURN
    }
    else if (countRead > size) {
        BSLS_ASSERT(countRead == room);

        BAEL_LOG_ERROR << "Read more bytes from \"" << path
                       << "\" than expected. Expected " << size << " but read "
                       << countRead << ". Maybe the file was modified."
                       << BAEL_LOG_END;
        return 4;                                                     // RETURN
    }

    BSLS_ASSERT(countRead == size);   // because math.
    BSLS_ASSERT(bsl::feof(message));  // because we tried to read past the end.
                                      // 'feof' will not perform any I/O, so if
                                      // the file changed since our read, it
                                      // won't be reflected here.
    // Get rid of trailing byte and path.
    buffer.resize(size);

    return 0;                                                         // RETURN
}

}  // close unnamed namespace

FormatUtil::Encoder FormatUtil::encoder(Format format)
{
    switch (format.value) {
      case Format::e_RAW:
        return &FormatUtil::encodeRaw;                                // RETURN
      default:
        BSLS_ASSERT(format == Format::e_EXTENDED);
        return &FormatUtil::encodeExtended;                           // RETURN
    }
}

FormatUtil::Decoder FormatUtil::decoder(Format format)
{
    switch (format.value) {
      case Format::e_RAW:
        return &FormatUtil::decodeRaw;                                // RETURN
      default:
        BSLS_ASSERT(format == Format::e_EXTENDED);
        return &FormatUtil::decodeExtended;                           // RETURN
    }
}

int FormatUtil::encodeRaw(long, bslstl::StringRef *, bsl::string *)
{
    return 0;
}

int FormatUtil::decodeRaw(bsl::string *)
{
    return 0;
}

int FormatUtil::encodeExtended(long               maxMessageSize,
                               bslstl::StringRef *originalAndOutput,
                               bsl::string       *messageBuffer)
{
    BSLS_ASSERT(originalAndOutput);
    BSLS_ASSERT(messageBuffer);

    bslstl::StringRef& message = *originalAndOutput;
    bsl::string&       buffer  = *messageBuffer;

    // If the message is not longer than 'maxMessageSize', then just write it
    // to the 'buffer' along with the trailing "in place" byte.
    if (long(message.length()) <= maxMessageSize) {
        // If 'message' aliases 'buffer' completely, then we don't have to
        // copy. Otherwise we do.
        if (message.data() != buffer.data() ||
            message.length() != buffer.size()) {
            buffer.assign(message);
        }

        buffer += k_EXTENDED_IN_PLACE;
        message = buffer;
        return 0;                                                     // RETURN
    }

    // The message is too large. Write it to a temporary file instead, and let
    // the value enqueued be the path to the file along with the trailing
    // "external file" byte.

    // 265 is chosen somewhat arbitrarily as "big enough for a temp path."
    bdlma::LocalSequentialAllocator<256> pathAllocator(
                                           buffer.get_allocator().mechanism());

    bsl::string path(&pathAllocator);
    if (writeToTempFile(message, &path)) {
        return makeError(e_ENCODER_ERROR);                            // RETURN
    }

    buffer = path;
    buffer += k_EXTENDED_EXTERNAL_FILE;
    message = buffer;
    return 0;
}

int FormatUtil::decodeExtended(bsl::string *originalAndOutput)
{
    BAEL_LOG_SET_CATEGORY(k_LOG_CATEGORY);
    BSLS_ASSERT(originalAndOutput);

    bsl::string& message = *originalAndOutput;
    if (message.empty()) {
        BAEL_LOG_ERROR << "The extended codec cannot decode an empty message."
                       << BAEL_LOG_END;
        return makeError(e_DECODER_ERROR);                            // RETURN
    }

    const char lastByte = message.back();
    if (lastByte == k_EXTENDED_IN_PLACE) {
        // success. Get rid of the trailing "indicator" byte.
        message.resize(message.size() - 1);
        return 0;                                                     // RETURN
    }
    else if (lastByte == k_EXTENDED_EXTERNAL_FILE) {
        // Interpret the message as a file path and use the file's contents.
        // First get rid of the tailing "indicator" byte.
        message.resize(message.size() - 1);
        if (readAndRemoveFile(&message)) {
            return makeError(e_DECODER_ERROR);                        // RETURN
        }

        // success
        return 0;                                                     // RETURN
    }

    BAEL_LOG_ERROR
        << "The final byte of message is 0x" << bsl::hex << int(lastByte)
        << ", which is not one of the accepted values for the extended codec."
        << BAEL_LOG_END;
    return makeError(e_DECODER_ERROR);
}

const char *FormatUtil::description(int errorCode)
{
    return ipcmq::description(errorCode, &errorOverflow);
}

}  // close package namespace
}  // close enterprise namespace
