#include "odbcpp.hpp"

namespace odbcpp {

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
