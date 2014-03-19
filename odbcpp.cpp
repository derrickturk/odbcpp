#include "odbcpp.hpp"

#include <utility>
#include <cassert>
#include <algorithm>

namespace odbcpp {

detail::handle<detail::handle_type::environment> connection::shared_env_ {};

connection::env_initializer connection::env_init_ {};

void query::execute(const string& statement)
{
    ready_ = false;

    auto ret = SQLExecDirect(stmt_,
            const_cast<string::value_type*>(statement.c_str()), SQL_NTS);

    if (!SQL_SUCCEEDED(ret))
        throw std::runtime_error(
                std::string("Statement execution failed!")
                + " : " + stmt_.error_message());

    update_fields();

    ret = SQLFetch(stmt_);
    if (ret == SQL_NO_DATA)
        empty_ = true;
    else if (!SQL_SUCCEEDED(ret))
        throw std::runtime_error(
                std::string("Failed to retrieve first row!")
                + " : " + stmt_.error_message());

    ready_ = true;
}

void query::advance()
{
    if (!ready_)
        throw std::runtime_error("No executed statement!");

    auto ret = SQLFetch(stmt_);
    if (ret == SQL_NO_DATA)
        empty_ = true;
    else if (!SQL_SUCCEEDED(ret))
        throw std::runtime_error(
                std::string("Failed to retrieve next row!")
                + " : " + stmt_.error_message());
}

void query::update_fields()
{
    static const std::size_t max_len = 256;

    SQLSMALLINT n_fields;
    auto ret = SQLNumResultCols(stmt_, &n_fields);
    if (!SQL_SUCCEEDED(ret))
        throw std::runtime_error(
                std::string("Unable to get field count!")
                + " : " + stmt_.error_message());

    std::vector<field> new_fields;
    new_fields.reserve(n_fields);

    SQLCHAR name_buf[max_len];
    SQLSMALLINT name_len, odbc_type, decimal_digits, nullable;
    SQLULEN col_size;
    for (std::size_t i = 1; i <= static_cast<SQLUSMALLINT>(n_fields); ++i) {
        auto ret = SQLDescribeCol(stmt_, i, name_buf, max_len,
                &name_len, &odbc_type, &col_size, &decimal_digits, &nullable);
        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(
                    std::string("Unable to get field metadata!")
                    + " : " + stmt_.error_message());

        new_fields.push_back({
                std::string(reinterpret_cast<char*>(&name_buf[0])),
                detail::type_from_odbc_sql_tag(odbc_type),
                col_size,
                static_cast<std::size_t>(decimal_digits),
                nullable != SQL_NO_NULLS,
                static_cast<std::size_t>(name_len) > max_len - 1
        });
    }

    fields_ = std::move(new_fields);
}

datum query::get(std::size_t field)
{
    static const std::size_t buf_chunk = 256;

    if (!ready_)
        throw std::runtime_error("No executed statement!");

    if (empty_)
        throw std::runtime_error("No data returned!");

    datum result(fields_[field].type);

    SQLLEN result_length;
    if (!detail::is_pointer_type(result.type_)) {
        auto ret = SQLGetData(stmt_, field + 1, // odbc uses 1-based indexing for columns
                detail::odbc_c_tag_from_type(result.type_),
                &result.datum_, sizeof(result.datum_), &result_length);
        if (!SQL_SUCCEEDED(ret))
            throw std::runtime_error(
                    std::string("Unable to retrieve data!")
                    + " : " + stmt_.error_message());

        if (result_length == SQL_NULL_DATA) {
            result.null_ = true;
        }

        return result;
    } else {
        std::size_t next_alloc = buf_chunk, alloc_total = 0;
        do {
            std::unique_ptr<unsigned char[]> buf(
                    new unsigned char[alloc_total + next_alloc]);

            if (result.ptr_)
                std::copy(result.ptr_.get(), result.ptr_.get() + alloc_total,
                        buf.get());

            result.ptr_ = std::move(buf);

            // you get back a null terminator for each chunk---
            unsigned char* this_request_ptr = result.ptr_.get()
                + (alloc_total ? alloc_total - 1 : 0);
            SQLLEN this_request_len = next_alloc + (alloc_total ? 1 : 0);
            alloc_total += next_alloc;
            // ---which is horrible and stupid
            auto ret = SQLGetData(stmt_, field + 1,
                    detail::odbc_c_tag_from_type(result.type_),
                    static_cast<void*>(this_request_ptr),
                    this_request_len,
                    &result_length);
            if (!SQL_SUCCEEDED(ret))
                throw std::runtime_error(
                        std::string("Unable to retrieve data!")
                        + " : " + stmt_.error_message());

            if (result_length == SQL_NULL_DATA) {
                result.null_ = true;
                return result;
            }

            if (result_length == SQL_NO_TOTAL) {
                next_alloc = buf_chunk;
            } else if (result_length > this_request_len) {
                next_alloc = result_length - (this_request_len) + 1;
            } else {
                switch (result.type_) {
#define FOR_EACH_DATA_TYPE(tag, _type, c_tag, sql_tag) \
                    case data_type::tag : \
                        result.datum_.tag = \
                            reinterpret_cast<_type>(result.ptr_.get()); \
                        result.len_ = \
                            (result_length \
                             + this_request_ptr - result.ptr_.get()) \
                            / sizeof(std::remove_pointer<_type>::type); \
                        break;
#include "pointer_types.def"

#undef FOR_EACH_DATA_TYPE

                    default: throw std::runtime_error("Invalid data type!");
                }

                return result;
            }
        } while (true);
    }

    return result;
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
