#include "odbcpp.hpp"

int main()
{
    using namespace odbcpp;

    detail::handle<detail::handle_type::environment> henv;
    detail::handle<detail::handle_type::connection> conn(henv);
}
