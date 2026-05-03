#pragma once

#include <array>
#include <cstdint>
#include <string>

#include <sys/types.h>

namespace infrastructure::file_storage {

    class FileStorage {

            static constexpr int kInvalidFd = -1;
            int m_fd{kInvalidFd};

        public:

            static std::array<char, 124> m_pathBuf;
            static std::size_t m_prefixLen;

        private:

            explicit FileStorage(int fd) noexcept;
    
        public:

            FileStorage() = delete;

           ~FileStorage();

            FileStorage(const FileStorage& other) = delete;
            FileStorage& operator=(const FileStorage& other) = delete;

            FileStorage(FileStorage&& other) noexcept;
            FileStorage& operator=(FileStorage&& other) noexcept;

            static FileStorage openReadOnly(uint64_t path);

            static FileStorage openWriteOnly(uint64_t path);

            static FileStorage openReadWrite(uint64_t path);

            static FileStorage createNew(uint64_t path, mode_t mode = 0644);

            static void remove(uint64_t path);

            static std::string makePath(uint64_t path);

        private:

            static int openImpl(std::array<char, 124> path, int flags, mode_t mode);

            static inline auto fillAsciiHex(uint64_t value) {
                static constexpr char hex[] = "0123456789abcdef";

                auto pathBuf = m_pathBuf;

                for (int i = 15; i >= 0; --i) {
                    pathBuf[m_prefixLen + i] = hex[value & 0xF];
                    value >>= 4;
                }
                pathBuf[m_prefixLen + 16] = '\0';

                return pathBuf;                
            }

        public:

            off_t size() const;

            inline auto getFd() const noexcept {
                return m_fd;
            }

    };

}