#pragma once

#include <bit>
#include <shared_mutex>
#include <sodium.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace infrastructure::security {

    struct Token {
        static constexpr std::size_t kSize = 16;

        std::array<std::byte, kSize> bytes{};

        friend bool operator==(const Token&, const Token&) noexcept = default;
    };

    struct TokenHash {
        std::size_t operator()(const Token& token) const noexcept {
            static_assert(Token::kSize == 16);

            const auto h1 = std::bit_cast<uint64_t>(token.bytes.data());
            const auto h2 = std::bit_cast<uint64_t>(token.bytes.data() + 8);

            return h1 ^ (h2 + 0x9e3779b97f4a7c15ull + (h1 << 6) + (h1 >> 2));
        }
    };

    class AuthTokenStore {

            std::unordered_map<Token, int64_t, TokenHash> m_tokenToUserId;
            mutable std::shared_mutex m_mutex;

            static uint8_t decodeHexChar(char ch) noexcept {
                if (ch >= '0' && ch <= '9') {
                    return ch - '0';
                }
                if (ch >= 'a' && ch <= 'f') {
                    return 10 + (ch - 'a');
                }
                if (ch >= 'A' && ch <= 'F') {
                    return 10 + (ch - 'A');
                }

                return -1;
            }

            static std::optional<Token> parseHex(std::string_view hex) noexcept {
                if (hex.size() != Token::kSize * 2) {
                    return std::nullopt;
                }

                Token token{};

                for (std::size_t i = 0; i < Token::kSize; ++i) {
                    const auto hi = decodeHexChar(hex[2 * i]);
                    const auto lo = decodeHexChar(hex[2 * i + 1]);

                    if ((hi == uint8_t(-1)) || (lo == uint8_t(-1))) {
                        return std::nullopt;
                    }

                    token.bytes[i] = std::byte((hi << 4) | lo);
                }

                return token;
            }

            static std::string toHex(const Token& token) {
                static constexpr char kHex[] = "0123456789abcdef";

                std::string result;
                result.resize(Token::kSize * 2);

                for (std::size_t i = 0; i < Token::kSize; ++i) {
                    const auto value = (uint8_t)token.bytes[i];
                    result[2 * i]     = kHex[(value >> 4) & 0x0F];
                    result[2 * i + 1] = kHex[value & 0x0F];
                }

                return result;
            }

        public:

            std::string issueToken(int64_t userId) {
                Token token{};
                randombytes_buf(token.bytes.data(), token.bytes.size());

                {
                    std::lock_guard lock(m_mutex);
                    m_tokenToUserId.insert_or_assign(token, userId);
                }

                return toHex(token);
            }

            std::optional<int64_t> resolveUserId(std::string_view tokenHex) const {
                const auto token = parseHex(tokenHex);
                if (!token) {
                    return std::nullopt;
                }

                std::shared_lock sharedLock(m_mutex);

                const auto it = m_tokenToUserId.find(*token);
                if (it == m_tokenToUserId.end()) {
                    return std::nullopt;
                }

                return it->second;
            }

            void revoke(std::string_view tokenHex) {
                const auto token = parseHex(tokenHex);
                if (!token) {
                    return;
                }

                std::lock_guard lock(m_mutex);
                m_tokenToUserId.erase(*token);
            }

    };

}