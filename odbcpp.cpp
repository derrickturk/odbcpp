#include "odbcpp.hpp"

#include <utility>
#include <cassert>

namespace odbcpp {

detail::handle<detail::handle_type::environment> connection::shared_env_ {};

connection::env_initializer connection::env_init_ {};

void query::execute(const string& statement)
{
    ready_ = false;

    auto ret = SQLExecDirect(stmt_,
            const_cast<string::value_type*>(statement.c_str()), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
        throw std::runtime_error("Statement execution failed!");

    update_fields();

    ready_ = true;
}

void query::update_fields()
{
    static const std::size_t max_len = 256;

    assert(ready_);

    SQLSMALLINT n_fields;
    auto ret = SQLNumResultCols(stmt_, &n_fields);
    if (!SQL_SUCCEEDED(ret))
        throw std::runtime_error("Unable to get field count!");

    std::vector<field> new_fields;
    new_fields.reserve(n_fields);

    SQLCHAR name_buf[max_len];
    SQLSMALLINT name_len, odbc_type, decimal_digits, nullable;
    SQLULEN col_size;
    for (std::size_t i = 1; i <= static_cast<SQLUSMALLINT>(n_fields); ++i) {
        auto ret = SQLDescribeCol(stmt_, i, name_buf, max_len,
                &name_len, &odbc_type, &col_size, &decimal_digits, &nullable);
        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error("Unable to get field metadata!");

        new_fields.push_back({
                std::string(reinterpret_cast<char*>(&name_buf[0])),
                detail::type_from_odbc_tag(odbc_type),
                col_size,
                static_cast<std::size_t>(decimal_digits),
                nullable != SQL_NO_NULLS,
                static_cast<std::size_t>(name_len) < max_len - 1
        });
    }

    fields_ = std::move(new_fields);
}

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
