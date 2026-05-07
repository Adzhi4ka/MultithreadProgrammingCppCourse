#include <sodium.h>

#include <stdexcept>
#include <string>
#include <string_view>

namespace infrastructure::security {

inline void initialize() {
    if (sodium_init() < 0) {
        throw std::runtime_error("sodium_init failed");
    }
}

inline std::string hashPassword(std::string_view rawPassword) {
    std::string hashedPassword(crypto_pwhash_STRBYTES, '\0');

    if (crypto_pwhash_str(hashedPassword.data(), rawPassword.data(), rawPassword.size(), crypto_pwhash_OPSLIMIT_MIN,
                          crypto_pwhash_MEMLIMIT_MIN) != 0) {

        throw std::runtime_error("crypto_pwhash_str failed");
    }

    return hashedPassword;
}

inline bool verifyPassword(std::string_view rawPassword, std::string_view storedHash) noexcept {
    return crypto_pwhash_str_verify(storedHash.data(), rawPassword.data(), rawPassword.size()) == 0;
}

}  // namespace infrastructure::security