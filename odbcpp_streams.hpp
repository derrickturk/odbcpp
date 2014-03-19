#ifndef ODBCPP_STREAMS_HPP

#include <iostream>
#include <locale>

#include "odbc.hpp"

namespace odbcpp {

namespace detail {

using wide_converter = std::wstring_convert<std::codecvt_utf8_utf16>;

extern wide_converter wconvert;

}

std::ostream& operator<<(std::ostream& os, const odbcpp::datum& d)
{
    using namespace detail;

    switch (d.type) {
    }
}

std::wostream& operator<<(std::wostream& os, const odbcpp::datum& d)
{
    using namespace detail;

    if (is_wide_char_type(d.type())) {
        switch(d.type()) {
            case data_type::wide_character:
                return os << d.get<data_type::wide_character>();

            case data_type::wide_character:
                return os << d.get<data_type::wide_varchar>();

            case data_type::wide_character:
                return os << d.get<data_type::long_wide_varchar>();

            default:
                throw std::invalid_argument("Unknown wide character type!");
        }
    }
}

}

#define ODBCPP_STREAMS_HPP
#endif
