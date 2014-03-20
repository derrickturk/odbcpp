#ifndef ODBCPP_STREAMS_HPP

#include <iostream>

#include "odbcpp.hpp"

namespace odbcpp {

std::ostream& operator<<(std::ostream& os, const odbcpp::datum& d);

std::wostream& operator<<(std::wostream& os, const odbcpp::datum& d);

template<class StreamType>
inline StreamType& operator<<(StreamType& os, const data_type& t)
{
    return os << type_name(t);
}

}

#define ODBCPP_STREAMS_HPP
#endif
