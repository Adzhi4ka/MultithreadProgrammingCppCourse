#include "file-storage.h"

#include <system_error>
#include <unistd.h>
#include <utility>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace infrastructure::file_storage {

        FileStorage::FileStorage(int fd) noexcept : m_fd(fd) {}
    
        FileStorage::~FileStorage() {
            if (m_fd != kInvalidFd) {
                ::close(m_fd);
            }
        }

        FileStorage::FileStorage(FileStorage&& other) noexcept : m_fd(std::exchange(other.m_fd, kInvalidFd)) {}

        FileStorage& FileStorage::operator=(FileStorage&& other) noexcept {
            if (this != &other) {
                if (m_fd != kInvalidFd) {
                    ::close(m_fd);
                }
                m_fd = std::exchange(other.m_fd, kInvalidFd);
            }

            return *this;
        }

        FileStorage FileStorage::openReadOnly(uint64_t path) {
            return FileStorage(openImpl(fillAsciiHex(path), O_RDONLY, 0));
        }

        FileStorage FileStorage::openWriteOnly(uint64_t path) {
            return FileStorage(openImpl(fillAsciiHex(path), O_WRONLY, 0));
        }

        FileStorage FileStorage::openReadWrite(uint64_t path) {
            return FileStorage(openImpl(fillAsciiHex(path), O_RDWR, 0));
        }

        FileStorage FileStorage::createNew(uint64_t path, mode_t mode) {
            return FileStorage(openImpl(fillAsciiHex(path), O_CREAT | O_EXCL | O_RDWR, mode));
        }

        int FileStorage::openImpl(std::array<char, 124> path, int flags, mode_t mode) {
            int fd = -1;

            do {
                if (flags & O_CREAT) {
                    fd = ::open(path.data(), flags, mode);
                } else {
                    fd = ::open(path.data(), flags);
                }
            } while (fd == -1 && errno == EINTR);

            if (fd == -1) {
                throw std::system_error(errno, std::generic_category(), path.data());
            }

            return fd;
        }

        void FileStorage::remove(uint64_t path) {
            auto pathBuf = fillAsciiHex(path);

            if (::unlink(pathBuf.data()) == -1) {
                throw std::system_error(errno, std::generic_category(), pathBuf.data());
            }
        }

        std::string FileStorage::makePath(uint64_t path) {
            auto pathBuf = fillAsciiHex(path);
            return std::string(pathBuf.data());
        }

        off_t FileStorage::size() const {
            struct stat st {};
            if (::fstat(m_fd, &st) == -1) {
                throw std::system_error(errno, std::generic_category(), "fstat failed");
            }
            return st.st_size;
        }

}