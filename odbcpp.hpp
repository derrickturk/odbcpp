#ifndef ODBCPP_HPP

#include <stdexcept>
#include <type_traits>
#include <vector>
#include <memory>
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
                        std::string(handle_traits<HType>::alloc_fail_msg)
                        + " : " + error_message());
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
                        std::string(handle_traits<HType>::alloc_fail_msg)
                        + " : " + error_message());
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

        std::string error_message() noexcept;

    private:
        typename handle_traits<HType>::native_type h_;
};

}

using string = std::basic_string<SQLCHAR>;
inline string make_string(const char* str) noexcept;
inline string make_string(const std::string& str);

enum class data_type {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) tag,
#include "nonpointer_types.def"
#include "pointer_types.def"
#undef FOR_EACH_DATA_TYPE
};

namespace detail {

// redundant, but GCC freaks out
constexpr const char* const type_names[] = {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) #tag,
#include "nonpointer_types.def"
#include "pointer_types.def"
#undef FOR_EACH_DATA_TYPE
};

template<data_type Tag>
struct data_type_traits;

#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) \
template<> \
struct data_type_traits<data_type::tag> { \
    using odbc_type = type; \
    static const bool is_pointer = std::is_pointer<odbc_type>::value; \
    static const bool is_scalar = std::is_scalar<odbc_type>::value; \
    static const bool is_wide_char = std::is_same< \
        std::remove_pointer<odbc_type>::type, \
        wchar_t>::value; \
    static const std::size_t size = sizeof(odbc_type); \
    static const SQLSMALLINT odbc_sql_tag = sql_tag; \
    static const SQLSMALLINT odbc_c_tag = c_tag; \
};

#include "nonpointer_types.def"
#include "pointer_types.def"

#undef FOR_EACH_DATA_TYPE

inline data_type type_from_odbc_sql_tag(SQLSMALLINT odbc_sql_tag)
{
    switch (odbc_sql_tag) {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) \
        case sql_tag : return data_type::tag;

#include "nonpointer_types.def"
#include "pointer_types.def"

#undef FOR_EACH_DATA_TYPE
    }

    throw std::invalid_argument("Bad ODBC SQL type tag!");
}

inline SQLSMALLINT odbc_c_tag_from_type(data_type type)
{
    switch (type) {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) \
        case data_type::tag : return c_tag;

#include "nonpointer_types.def"
#include "pointer_types.def"

#undef FOR_EACH_DATA_TYPE
    }

    throw std::invalid_argument("Bad type tag!");
}

inline SQLSMALLINT odbc_sql_tag_from_type(data_type type)
{
    switch (type) {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) \
        case data_type::tag : return sql_tag;

#include "nonpointer_types.def"
#include "pointer_types.def"

#undef FOR_EACH_DATA_TYPE
    }

    throw std::invalid_argument("Bad type tag!");
}

inline bool is_pointer_type(data_type type)
{
    switch (type) {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_type) \
        case data_type::tag : \
            return data_type_traits<data_type::tag>::is_pointer;

#include "nonpointer_types.def"
#include "pointer_types.def"

#undef FOR_EACH_DATA_TYPE
    }

    throw std::invalid_argument("Bad type tag!");
}

inline bool is_scalar_type(data_type type)
{
    switch (type) {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_type) \
        case data_type::tag : \
            return data_type_traits<data_type::tag>::is_scalar;

#include "nonpointer_types.def"
#include "pointer_types.def"

#undef FOR_EACH_DATA_TYPE
    }

    throw std::invalid_argument("Bad type tag!");
}

inline bool is_wide_char_type(data_type type)
{
    switch (type) {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_type) \
        case data_type::tag : \
            return data_type_traits<data_type::tag>::is_wide_char;

#include "nonpointer_types.def"
#include "pointer_types.def"

#undef FOR_EACH_DATA_TYPE
    }

    throw std::invalid_argument("Bad type tag!");
}

inline std::size_t element_size(data_type type)
{
    switch (type) {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_type) \
        case data_type::tag : return sizeof(type);

#include "nonpointer_types.def"
#include "pointer_types.def"

#undef FOR_EACH_DATA_TYPE
    }

    throw std::invalid_argument("Bad type tag!");
}


}

constexpr const char* type_name(data_type type) noexcept
{
    return detail::type_names[static_cast<int>(type)];
}

class datum {
    public:
        datum(datum&&) = default;

        datum& operator=(datum&&) = default;

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
        datum(data_type type)
            : type_(type), null_(false), ptr_(nullptr), datum_() {}

        data_type type_;
        bool null_;
        std::unique_ptr<unsigned char[]> ptr_;
        union odbc_datum {
#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) type tag;
#include "nonpointer_types.def"
#include "pointer_types.def"
#undef FOR_EACH_DATA_TYPE
        } datum_;

        template<data_type Tag>
        typename detail::data_type_traits<Tag>::odbc_type
        get_impl() const noexcept;

    friend class query;
};

#define FOR_EACH_DATA_TYPE(tag, type, c_tag, sql_tag) \
template<> \
inline type datum::get_impl<data_type::tag>() const noexcept \
{ \
    return datum_.tag; \
}

#include "nonpointer_types.def"
#include "pointer_types.def"

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

        connection(connection&&) noexcept = default;

        connection& operator=(const connection&) = delete;

        connection& operator=(connection&&) noexcept = default;

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
            : stmt_(conn), fields_(), ready_(false), empty_(false) {}

        query(const query&) = delete;

        query(query&&) = default;

        query& operator=(const query&) = delete;

        query& operator=(query&&) = default;

        ~query() noexcept = default;

        explicit operator bool() const { return ready_ && !empty_; }

        void execute(const string& statement);

        void advance();

        template<class StrType>
        void execute(const StrType& statement)
        {
            execute(make_string(statement));
        }

        const std::vector<field>& fields() const;

        datum get(std::size_t field);

    private:
        detail::handle<detail::handle_type::statement> stmt_;
        std::vector<field> fields_;
        bool ready_;
        bool empty_;

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
        throw std::runtime_error(
                std::string("Failed to set environment attributes.")
                + " : " + shared_env_.error_message());
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

namespace detail {

template<handle_type HType>
std::string handle<HType>::error_message() noexcept
{
    static const std::size_t msg_max_len = 256;

    char msg[msg_max_len];
    char code[6];

    std::string err_msg;

    SQLSMALLINT rec_no = 1;
    while (true) {
        SQLINTEGER native_error;
        SQLSMALLINT ret_len;
        auto ret = SQLGetDiagRec(handle_traits<HType>::native_tag, h_, rec_no,
                reinterpret_cast<SQLCHAR*>(&code[0]), &native_error,
                reinterpret_cast<SQLCHAR*>(&msg[0]), msg_max_len, &ret_len);

        if (!SQL_SUCCEEDED(ret))
            return err_msg;

        if (rec_no != 1)
            err_msg += " | ";

        err_msg += code;
        err_msg += ": ";
        err_msg += msg;
        /* g++ broken! no to_string wtf
        err_msg += " (native code: ";
        err_msg += std::to_string(native_error);
        err_msg += ")";
        */

        ++rec_no;
    }
}

}

}

#define ODBCPP_HPP
#endif
