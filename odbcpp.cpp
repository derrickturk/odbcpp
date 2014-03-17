#include "odbcpp.hpp"

namespace odbcpp {

detail::handle<detail::handle_type::environment> connection::shared_env_ {};

connection::env_initializer connection::env_init_ {};

bool connection::connect(const string& conn_str, bool prompt)
{
    if (connected_)
        throw std::runtime_error("Attempt to connect when already connected!");

    auto ret = SQLDriverConnect(conn_, nullptr,
            const_cast<string::value_type*>(conn_str.c_str()), SQL_NTS,
            nullptr, 0, nullptr,
            prompt ? SQL_DRIVER_PROMPT : SQL_DRIVER_NOPROMPT);

    return (connected_ = SQL_SUCCEEDED(ret));
}

namespace detail {

const char* const handle_traits<handle_type::environment>::alloc_fail_msg =
        "Failed to allocate environment handle.";

const char* const handle_traits<handle_type::connection>::alloc_fail_msg =
        "Failed to allocate connection handle.";

const char* const handle_traits<handle_type::statement>::alloc_fail_msg =
        "Failed to allocate statement handle.";

const char* const handle_traits<handle_type::descriptor>::alloc_fail_msg =
        "Failed to allocate descriptor handle.";

}

}

#include "sql.h"
