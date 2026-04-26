#pragma once

#include <boost/json.hpp>

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace presentation::http {

    namespace json = boost::json;

    inline std::optional<json::object> parseJsonObject(std::string_view body) {
        boost::system::error_code ec;
        json::value value = json::parse(body, ec);

        if (ec || !value.is_object()) {
            return std::nullopt;
        }

        return value.as_object();
    }

    inline std::optional<std::string> getStringField(const json::object& object,
                                                     std::string_view fieldName) {
        const auto it = object.find(fieldName);
        if (it == object.end() || !it->value().is_string()) {
            return std::nullopt;
        }

        return std::string{it->value().as_string().begin(), it->value().as_string().end()};
    }

    inline std::optional<int64_t> getInt64Field(const json::object& object,
                                                std::string_view fieldName) {
        const auto it = object.find(fieldName);
        if (it == object.end()) {
            return std::nullopt;
        }

        if (it->value().is_int64()) {
            return it->value().as_int64();
        }

        if (it->value().is_uint64()) {

            return it->value().as_uint64();
        }

        return std::nullopt;
    }

    inline std::optional<std::string_view> getQueryParam(std::string_view target,
                                                         std::string_view key) {
        const auto queryPos = target.find('?');
        if (queryPos == std::string_view::npos || queryPos + 1 >= target.size()) {
            return std::nullopt;
        }

        std::string_view query = target.substr(queryPos + 1);

        while (!query.empty()) {
            const auto ampPos = query.find('&');
            const auto pair = query.substr(0, ampPos);

            const auto eqPos = pair.find('=');
            const auto currentKey = pair.substr(0, eqPos);

            if (currentKey == key) {
                if (eqPos == std::string_view::npos) {
                    return std::string_view{};
                }

                return pair.substr(eqPos + 1);
            }

            if (ampPos == std::string_view::npos) {
                break;
            }

            query.remove_prefix(ampPos + 1);
        }

        return std::nullopt;
    }

    inline std::optional<int64_t> parseInt64(std::string_view value) {
        int64_t parsed;

        const auto [ptr, ec] = std::from_chars(value.data(), value.end(), parsed);
        if (ec != std::errc{} || ptr != value.end()) {
            return std::nullopt;
        }

        return parsed;
    }

    inline std::string serializeJson(const json::value& value) {
        return json::serialize(value);
    }

}