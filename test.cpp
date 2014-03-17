#include "odbcpp.hpp"

#include "sql.h"
#include "sqlext.h"

#include <cassert>

int main()
{
    using namespace odbcpp;

    connection conn("test dsn");

    auto q = conn.make_query();
    q.execute("select * from test");
}
