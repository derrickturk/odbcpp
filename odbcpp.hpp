#ifndef ODBCPP_HPP

#include <stdexcept>
#include <type_traits>
#include <vector>
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
                std::is_void<C>::value,
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
                !std::is_void<C>::value,
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
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) tag,
#include "data_types.def"
#undef FOR_EACH_DATA_TYPE
};

namespace detail {

template<data_type Tag>
struct data_type_traits;

#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) \
template<> \
struct data_type_traits<data_type::tag> { \
    using odbc_type = type; \
    static const bool is_pointer = std::is_pointer<odbc_type>::value; \
    static const std::size_t size = sizeof(odbc_type); \
    static const SQLSMALLINT odbc_sql_tag = sql_tag; \
    static const SQLSMALLINT odbc_c_tag = c_tag; \
};

#include "data_types.def"

#undef FOR_EACH_DATA_TYPE

inline data_type type_from_odbc_sql_tag(SQLSMALLINT odbc_sql_tag)
{
    switch (odbc_sql_tag) {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) \
        case sql_tag : return data_type::tag;

#include "data_types.def"

#undef FOR_EACH_DATA_TYPE
    }

    throw std::invalid_argument("Bad ODBC SQL type tag!");
}

inline SQLSMALLINT odbc_c_tag_from_type(data_type type)
{
    switch (type) {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) \
        case data_type::tag : return c_tag;

#include "data_types.def"

#undef FOR_EACH_DATA_TYPE
    }

    throw std::invalid_argument("Bad type tag!");
}

inline SQLSMALLINT odbc_sql_tag_from_type(data_type type)
{
    switch (type) {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) \
        case data_type::tag : return sql_tag;

#include "data_types.def"

#undef FOR_EACH_DATA_TYPE
    }

    throw std::invalid_argument("Bad type tag!");
}


}

class datum {
    public:
        datum(data_type type, void* data);

        data_type type() const { return type_; }

        operator bool() const { return !null_; }

        template<data_type Tag>
        typename detail::data_type_traits<Tag>::odbc_type get() const
        {
            if (type_ != Tag)
                throw std::invalid_argument("Invalid type for access.");

            return get_impl<Tag>();
        }

    private:
        data_type type_;
        bool null_;
        union odbc_datum {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) type tag;
#include "data_types.def"
#undef FOR_EACH_DATA_TYPE
        } datum_;

        template<data_type Tag>
        typename detail::data_type_traits<Tag>::odbc_type
        get_impl() const noexcept;
};

#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) \
template<> \
inline type datum::get_impl<data_type::tag>() const noexcept \
{ \
    return datum_.tag; \
}

#include "data_types.def"

#undef FOR_EACH_DATA_TYPE

class query;

struct field {
    std::string name;
    data_type type;
    std::size_t column_size;
    std::size_t decimal_digits;
    bool nullable;
    bool name_truncated;
};

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
            : stmt_(conn), fields_(), ready_(false) {}

        query(const query&) = delete;

        query(query&&) = default;

        query& operator=(const query&) = delete;

        query& operator=(query&&) = default;

        ~query() noexcept = default;

        void execute(const string& statement);

        template<class StrType>
        void execute(const StrType& statement)
        {
            execute(make_string(statement));
        }

        const std::vector<field>& fields() const;

        datum get(std::size_t field) const;

    private:
        detail::handle<detail::handle_type::statement> stmt_;
        std::vector<field> fields_;
        bool ready_;

        void update_fields();
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

inline const std::vector<field>& query::fields() const
{
    if (!ready_)
        throw std::runtime_error("No executed statement!");

    return fields_;
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
