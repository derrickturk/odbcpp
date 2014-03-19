#ifndef ODBCPP_STREAMS_HPP

#include <iostream>
#include <locale>

#include "odbcpp.hpp"

namespace odbcpp {

namespace detail {

using wide_converter = std::wstring_convert<std::codecvt_utf8_utf16>;

extern wide_converter wconvert;

}

std::ostream& operator<<(std::ostream& os, const odbcpp::datum& d)
{
    using namespace odbcpp::detail;

    switch (d.type()) {
        default: throw std::runtime_error("Warnings.");
    }

    return os;
}

std::wostream& operator<<(std::wostream& os, const odbcpp::datum& d)
{
    using namespace odbcpp::detail;

    if (is_wide_char_type(d.type())) {
        switch (d.type()) {
            case data_type::wide_character:
                return os << d.get<data_type::wide_character>();

            case data_type::wide_varchar:
                return os << d.get<data_type::wide_varchar>();

            case data_type::long_wide_varchar:
                return os << d.get<data_type::long_wide_varchar>();

            default:
                throw std::invalid_argument("Unknown wide character type!");
        }
    }

    if (is_narrow_char_type(d.type())) {
        switch (d.type()) {
            unsigned char* p;

            case data_type::character:
                return os << wconvert.to_bytes(d.get<data_type::character>());

            case data_type::varchar:
                return os << wconvert.to_bytes(d.get<data_type::character>());

            case data_type::long_varchar:
                return os << wconvert.to_bytes(d.get<data_type::character>());

            case data_type::byte:
                return os << std::hex << d.get<data_type::byte>() << std::dec;

            case data_type::bit:
                return os << static_cast<bool>(d.get<data_type::bit>());

            case data_type::binary:
                p = d.get<data_type::binary>();
            case data_type::varbinary:
                p = d.get<data_type::varbinary>();
            case data_type::long_varbinary:
                p = d.get<data_type::long_varbinary>();
                os << std::hex;
                while (auto c = static_cast<unsigned char>(*p++) != 0)
                    os << c;
                os << std::dec;
                return os;

            default:
                throw std::invalid_argument("Unknown character type!");
        }
    }

    return os;
}

}

#define ODBCPP_STREAMS_HPP
#endif
