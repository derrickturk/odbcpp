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

    conn.connect(make_string("DRIVER=sql server;SERVER=VM-TXSQLDEV02\\BBE;Trusted Connection=Yes;Database=DSS_SFS;"));
    assert(conn);
}
