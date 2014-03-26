#include "odbcpp_streams.hpp"

#include <iomanip>

#define DIRECT_OUTPUT_CASE(type, datum, str) \
    case data_type::type: \
        return str << datum.get<data_type::type>();

namespace odbcpp {

std::ostream& operator<<(std::ostream& os, const odbcpp::datum& d)
{
    using namespace odbcpp::detail;

    if (!d)
        return os << "<NULL>";

    if (is_narrow_char_type(d.type())) {
        switch (d.type()) {
            union {
                unsigned char* up;
                char* p;
            };
            char fill;

            case data_type::byte:
                fill = os.fill('0');
                os << std::hex << std::setw(2)
                    << static_cast<unsigned>(d.get<data_type::byte>())
                    << std::dec;
                os.fill(fill);
                return os;

            case data_type::bit:
                return os << static_cast<bool>(d.get<data_type::bit>());

            case data_type::character:
                p = reinterpret_cast<char*>(d.get<data_type::character>());
                return os << p;

            case data_type::varchar:
                p = reinterpret_cast<char*>(d.get<data_type::varchar>());
                return os << p;

            case data_type::long_varchar:
                p = reinterpret_cast<char*>(d.get<data_type::long_varchar>());
                return os << p;

            case data_type::binary:
                up = d.get<data_type::binary>();
                goto walk_binary;

            case data_type::varbinary:
                up = d.get<data_type::varbinary>();
                goto walk_binary;

            case data_type::long_varbinary:
                up = d.get<data_type::long_varbinary>();
walk_binary:
                fill = os.fill('0');
                os << std::hex;
                for (std::size_t i = 0; i < d.length(); ++i)
                    os << (i ? " " : "") << std::setw(2)
                        << static_cast<unsigned>(up[i]);
                os << std::dec;
                os.fill(fill);
                return os;

            default:
                throw std::invalid_argument("Unknown character type!");
        }
    }

    if (is_wide_char_type(d.type())) {
        switch (d.type()) {
            wchar_t* p;

            case data_type::wide_character:
                p = d.get<data_type::wide_character>();
                goto walk_wide;

            case data_type::wide_varchar:
                p = d.get<data_type::wide_varchar>();
                goto walk_wide;

            case data_type::long_wide_varchar:
                p = d.get<data_type::long_wide_varchar>();
walk_wide:
                while (wchar_t c = *p++)
                    os << os.narrow(c, '?');
                return os;

            default:
                throw std::invalid_argument("Unknown wide character type!");
        }
    }

    if (is_scalar_type(d.type())) {
        switch (d.type()) {
            DIRECT_OUTPUT_CASE(short_integer, d, os)
            DIRECT_OUTPUT_CASE(integer, d, os)
            DIRECT_OUTPUT_CASE(long_integer, d, os)
            DIRECT_OUTPUT_CASE(single_float, d, os)
            DIRECT_OUTPUT_CASE(double_float, d, os)
            DIRECT_OUTPUT_CASE(default_float, d, os)

            default:
                return os << "<" << type_name(d.type()) << ">";
        }
    }

    if (is_struct_type(d.type())) {
        switch (d.type()) {
            case data_type::date:
                return os << "< date: " << d.get<data_type::date>().year << "-"
                    << d.get<data_type::date>().month << "-"
                    << d.get<data_type::date>().day << " >";

            case data_type::time:
                return os << "< time: " << d.get<data_type::time>().hour << ":"
                    << d.get<data_type::time>().minute << ":"
                    << d.get<data_type::time>().second << " >";

            case data_type::timestamp:
                return os << "< timestamp: "
                    << d.get<data_type::timestamp>().year << "-"
                    << d.get<data_type::timestamp>().month << "-"
                    << d.get<data_type::timestamp>().day << " "
                    << d.get<data_type::timestamp>().hour << ":"
                    << d.get<data_type::timestamp>().minute << ":"
                    << d.get<data_type::timestamp>().second << "+"
                    << d.get<data_type::timestamp>().fraction << "e-9 >";

            default:
                return os << "<" << type_name(d.type()) << ">";
        }
    }

    if (is_pointer_type(d.type()))
        throw std::runtime_error("Unknown pointer type!");

    throw std::runtime_error("Unknown type!");
}

std::wostream& operator<<(std::wostream& os, const odbcpp::datum& d)
{
    using namespace odbcpp::detail;

    if (!d)
        return os << "<NULL>";

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
            char fill;

            case data_type::byte:
                fill = os.fill('0');
                os << std::hex << std::setw(2)
                    << static_cast<unsigned>(d.get<data_type::byte>())
                    << std::dec;
                os.fill(fill);
                return os;

            case data_type::bit:
                return os << static_cast<bool>(d.get<data_type::bit>());

            case data_type::character:
                p = reinterpret_cast<char*>(d.get<data_type::character>());
                goto walk_narrow;

            case data_type::varchar:
                p = reinterpret_cast<char*>(d.get<data_type::varchar>());
                goto walk_narrow;

            case data_type::long_varchar:
                p = reinterpret_cast<char*>(d.get<data_type::long_varchar>());
walk_narrow:
                while (char c = *p++)
                    os << os.widen(c);
                return os;

            case data_type::binary:
                up = d.get<data_type::binary>();
                goto walk_binary;

            case data_type::varbinary:
                up = d.get<data_type::varbinary>();
                goto walk_binary;

            case data_type::long_varbinary:
                up = d.get<data_type::long_varbinary>();
walk_binary:
                fill = os.fill('0');
                os << std::hex;
                for (std::size_t i = 0; i < d.length(); ++i)
                    os << (i ? " " : "") << std::setw(2)
                        << static_cast<unsigned>(up[i]);
                os << std::dec;
                os.fill(fill);
                return os;

            default:
                throw std::invalid_argument("Unknown character type!");
        }
    }

    if (is_scalar_type(d.type())) {
        switch (d.type()) {
            DIRECT_OUTPUT_CASE(short_integer, d, os)
            DIRECT_OUTPUT_CASE(integer, d, os)
            DIRECT_OUTPUT_CASE(long_integer, d, os)
            DIRECT_OUTPUT_CASE(single_float, d, os)
            DIRECT_OUTPUT_CASE(double_float, d, os)
            DIRECT_OUTPUT_CASE(default_float, d, os)

            default:
                return os << "<" << type_name(d.type()) << ">";
        }
    }

    if (is_struct_type(d.type())) {
        switch (d.type()) {
            case data_type::date:
                return os << "< date: " << d.get<data_type::date>().year << "-"
                    << d.get<data_type::date>().month << "-"
                    << d.get<data_type::date>().day << " >";

            case data_type::time:
                return os << "< time: " << d.get<data_type::time>().hour << ":"
                    << d.get<data_type::time>().minute << ":"
                    << d.get<data_type::time>().second << " >";

            case data_type::timestamp:
                return os << "< timestamp: "
                    << d.get<data_type::timestamp>().year << "-"
                    << d.get<data_type::timestamp>().month << "-"
                    << d.get<data_type::timestamp>().day << " "
                    << d.get<data_type::timestamp>().hour << ":"
                    << d.get<data_type::timestamp>().minute << ":"
                    << d.get<data_type::timestamp>().second << "+"
                    << d.get<data_type::timestamp>().fraction << "e-9 >";

            default:
                return os << "<" << type_name(d.type()) << ">";
        }
    }

    if (is_pointer_type(d.type()))
        throw std::runtime_error("Unknown pointer type!");

    throw std::runtime_error("Unknown type!");
}

}
