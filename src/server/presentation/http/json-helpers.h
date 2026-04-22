#pragma once

#include <boost/json.hpp>

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

    inline std::string serializeJson(const json::value& value) {
        return json::serialize(value);
    }

}