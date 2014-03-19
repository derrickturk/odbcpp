#ifndef ODBCPP_STREAMS_HPP

#include <iostream>

#include "odbcpp.hpp"

namespace odbcpp {

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
            union {
                unsigned char* up;
                char* p;
            };

            case data_type::byte:
                return os << std::hex << d.get<data_type::byte>() << std::dec;

            case data_type::bit:
                return os << static_cast<bool>(d.get<data_type::bit>());

            case data_type::character:
                p = reinterpret_cast<char*>(d.get<data_type::character>());
            case data_type::varchar:
                p = reinterpret_cast<char*>(d.get<data_type::varchar>());
            case data_type::long_varchar:
                p = reinterpret_cast<char*>(d.get<data_type::long_varchar>());
                while (char c = *p++)
                    os << os.widen(c);
                return os;

            case data_type::binary:
                up = d.get<data_type::binary>();
            case data_type::varbinary:
                up = d.get<data_type::varbinary>();
            case data_type::long_varbinary:
                up = d.get<data_type::long_varbinary>();
                os << std::hex;
                for (std::size_t i = 0; i < d.length(); ++i)
                    os << p[i];
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
