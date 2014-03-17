#ifndef ODBCPP_HPP

#include <stdexcept>
#include <type_traits>

#include "windows.h"
#include "sql.h"

namespace odbcpp {

namespace detail {

enum class handle_type : char {
    environment,
    connection,
    statement,
    descriptor
};

template<handle_type HType>
class handle;

}

class connection {
};

namespace detail {

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
                        "Failed to allocate environment handle.");
        }

        operator native_handle() noexcept
        {
            return h_;
        }

        ~handle() noexcept
        {
            SQLFreeHandle(handle_traits<HType>::native_tag, h_);
        }

    private:
        typename handle_traits<HType>::native_type h_;
};

}

}

#define ODBCPP_HPP
#endif
