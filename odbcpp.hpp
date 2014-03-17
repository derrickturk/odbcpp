#ifndef ODBCPP_HPP

#include <stdexcept>
#include <type_traits>
#include <string>

#include "windows.h"
#include "sql.h"
#include "sqlext.h"

namespace odbcpp {

namespace detail {

enum class handle_type : char {
    environment,
    connection,
    statement,
    descriptor
};

template<handle_type HType>
struct handle_traits;

template<>
struct handle_traits<handle_type::environment> {
    using native_type = SQLHENV;
    using context_type = void;
    static const SQLSMALLINT native_tag = SQL_HANDLE_ENV;
    static const char* const alloc_fail_msg;
};

template<>
struct handle_traits<handle_type::connection> {
    using native_type = SQLHDBC;
    using context_type = SQLHENV;
    static const SQLSMALLINT native_tag = SQL_HANDLE_DBC;
    static const char* const alloc_fail_msg;
};

template<>
struct handle_traits<handle_type::statement> {
    using native_type = SQLHSTMT;
    using context_type = SQLHDBC;
    static const SQLSMALLINT native_tag = SQL_HANDLE_STMT;
    static const char* const alloc_fail_msg;
};

template<>
struct handle_traits<handle_type::descriptor> {
    using native_type = SQLHDESC;
    using context_type = SQLHDBC;
    static const SQLSMALLINT native_tag = SQL_HANDLE_DESC;
    static const char* const alloc_fail_msg;
};

template<handle_type HType>
class handle {
    public:
        using native_handle = typename handle_traits<HType>::native_type;
        using context_type = typename handle_traits<HType>::context_type;

        template<class C=context_type,
            typename std::enable_if<
                std::is_same<C, void>::value,
            int>::type = 0>
        handle()
        {
            auto ret = SQLAllocHandle(handle_traits<HType>::native_tag,
                    SQL_NULL_HANDLE, &h_);
            if (!SQL_SUCCEEDED(ret))
                throw std::runtime_error(
                        handle_traits<HType>::alloc_fail_msg);
        }

        template<class C=context_type>
        handle(const typename std::enable_if<
                !std::is_same<C, void>::value,
                C>::type& context)
        {
            auto ret = SQLAllocHandle(handle_traits<HType>::native_tag,
                    context, &h_);
            if (!SQL_SUCCEEDED(ret))
                throw std::runtime_error(
                        handle_traits<HType>::alloc_fail_msg);
        }

        handle(const handle&) = delete;

        handle(handle&& other) noexcept
        {
            h_ = other.h_;
            other.h_ = SQL_NULL_HANDLE;
        }

        handle& operator=(const handle&) = delete;

        handle& operator=(handle&& other) noexcept
        {
            h_ = other.h_;
            other.h_ = SQL_NULL_HANDLE;

            return *this;
        }

        operator native_handle() noexcept
        {
            return h_;
        }

        ~handle() noexcept
        {
            if (h_ != SQL_NULL_HANDLE)
                SQLFreeHandle(handle_traits<HType>::native_tag, h_);
        }

    private:
        typename handle_traits<HType>::native_type h_;
};

}

using string = std::basic_string<SQLCHAR>;
inline string make_string(const char* str) noexcept;
inline string make_string(const std::string& str);

enum class data_type {
    character,
    wide_character,
    short_integer,
    unsigned_short_integer,
    integer,
    unsigned_integer,
    single_float,
    double_float,
    bit,
    byte,
    unsigned_byte,
    long_integer,
    unsigned_long_integer,
    binary,
    bookmark,
    var_bookmark,
    date,
    time,
    timestamp,
    numeric,
    guid,
    interval
};

class datum {
    public:
        datum(data_type type, void* data);

    private:
        data_type type_;
        union odbc_datum {
            SQLCHAR* character;
            SQLWCHAR* wide_character;
            SQLSMALLINT short_integer;
            SQLUSMALLINT unsigned_short_integer;
            SQLREAL single_float;
            SQLDOUBLE double_float;
            SQLCHAR bit;
            SQLSCHAR byte;
            SQLCHAR unsigned_byte;
            SQLBIGINT long_integer;
            SQLUBIGINT unsigned_long_integer;
            SQLCHAR* binary;
            BOOKMARK bookmark;
            SQLCHAR* var_bookmark;
            SQL_DATE_STRUCT date;
            SQL_TIME_STRUCT time;
            SQL_TIMESTAMP_STRUCT timestamp;
            SQL_NUMERIC_STRUCT numeric;
            SQLGUID guid;
            SQL_INTERVAL_STRUCT interval;
        } datum_;
};

class query;

class connection {
    public:
        connection()
            : conn_(shared_env_), connected_(false) {}

        connection(const string& conn_str)
            : connection() { connect(conn_str); }

        template<class StrType>
        connection(const StrType& conn_str)
            : connection(make_string(conn_str)) {}

        connection(const connection&) = delete;

        connection(connection&& other) noexcept = default;

        connection& operator=(const connection&) = delete;

        connection& operator=(connection&& other) noexcept = default;

        ~connection() noexcept { disconnect(); }

        bool connect(const string& conn_str, bool prompt=false);

        void disconnect() noexcept
        {
            if (connected_)
                SQLDisconnect(conn_);
        }

        explicit operator bool() const noexcept { return connected_; }

        query make_query();

    private:
        detail::handle<detail::handle_type::connection> conn_;

        bool connected_;

        static detail::handle<detail::handle_type::environment> shared_env_;

        static struct env_initializer {
            env_initializer();
        } env_init_;
};

class query {
    public:
        query(detail::handle<detail::handle_type::connection>& conn)
            : stmt_(conn), ready_(false) {}

        query(const query&) = delete;

        query(query&&) noexcept = default;

        query& operator=(const query&) = delete;

        query& operator=(query&&) noexcept = default;

        ~query() noexcept = default;

        void execute(const string& statement);

        template<class StrType>
        void execute(const StrType& statement)
        {
            execute(make_string(statement));
        }

    private:
        detail::handle<detail::handle_type::statement> stmt_;
        bool ready_;
};

inline query connection::make_query()
{
    return query(conn_);
}

inline connection::env_initializer::env_initializer()
{
    auto ret = SQLSetEnvAttr(shared_env_, SQL_ATTR_ODBC_VERSION,
            reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);

    if (!SQL_SUCCEEDED(ret))
        throw std::runtime_error("Failed to set environment attributes.");
}

inline string make_string(const char* str) noexcept
{
    return string(reinterpret_cast<const string::value_type*>(str));
}

inline string make_string(const std::string& str)
{
    return string(begin(str), end(str));
}

}

#define ODBCPP_HPP
#endif
