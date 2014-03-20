#ifndef ODBCPP_STREAMS_HPP

#include <iostream>
#include <string>
#include <sstream>

#include "odbcpp.hpp"

namespace odbcpp {

std::ostream& operator<<(std::ostream& os, const datum& d);

std::wostream& operator<<(std::wostream& os, const datum& d);

template<class StreamType>
inline StreamType& operator<<(StreamType& os, const data_type& t)
{
    return os << type_name(t);
}

inline std::string to_string(const datum& d)
{
    std::stringstream s;
    s << d;
    return s.str();
}

inline std::wstring to_wstring(const datum& d)
{
    std::wstringstream s;
    s << d;
    return s.str();
}

}

#define ODBCPP_STREAMS_HPP
#endif
