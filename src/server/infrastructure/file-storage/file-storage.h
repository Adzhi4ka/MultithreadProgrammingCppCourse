#pragma once

#include <filesystem>
#include <utility>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace infrastructure::db::file_storage {

    class FileStorage {

            static constexpr int kInvalidFd = -1;
            int m_fd{kInvalidFd};

        private:

            explicit FileStorage(int fd) noexcept : m_fd(fd) {}
    
        public:

            FileStorage() = delete;

           ~FileStorage() {
                ::close(m_fd);
            }

            FileStorage(FileStorage&& other) noexcept : m_fd(std::exchange(other.m_fd, kInvalidFd)) {}

            FileStorage& operator=(FileStorage&& other) noexcept {
                m_fd = std::exchange(other.m_fd, kInvalidFd);
                return *this;
            }

            static FileStorage openReadOnly(const std::filesystem::path& path) {
                return FileStorage(openImpl(path, O_RDONLY, 0));
            }

            static FileStorage openWriteOnly(const std::filesystem::path& path) {
                return FileStorage(openImpl(path, O_WRONLY, 0));
            }

            static FileStorage openReadWrite(const std::filesystem::path& path) {
                return FileStorage(openImpl(path, O_RDWR, 0));
            }

            static FileStorage createNew(const std::filesystem::path& path, mode_t mode = 0644) {
                return FileStorage(openImpl(path, O_CREAT | O_EXCL | O_WRONLY, mode));
            }

        private:

            static int openImpl(const std::filesystem::path& path, int flags, mode_t mode) {
                int fd = -1;

                do {
                    if (flags & O_CREAT) {
                        fd = ::open(path.c_str(), flags, mode);
                    } else {
                        fd = ::open(path.c_str(), flags);
                    }
                } while (fd == -1 && errno == EINTR);

                if (fd == -1) {
                    throw std::system_error(errno, std::generic_category(),
                        "open failed: " + path.string());
                }

                return fd;
            }

        public:

            auto size() const {
                struct stat st {};
                if (::fstat(m_fd, &st) == -1) {
                    throw std::system_error(errno, std::generic_category(), "fstat failed");
                }
                return st.st_size;
            }

            inline auto getFd() const noexcept {
                return m_fd;
            }

    };

}