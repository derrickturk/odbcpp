#include "odbcpp.hpp"

#include "sql.h"
#include "sqlext.h"

#include <cassert>

int main()
{
    using namespace odbcpp;

    connection conn;
    assert(!conn);

    assert(!conn.connect(make_string("bad string rising")));
}
