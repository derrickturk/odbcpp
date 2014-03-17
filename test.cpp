#include "odbcpp.hpp"

#include "sql.h"
#include "sqlext.h"

#include <cassert>

int main()
{
    using namespace odbcpp;

    connection conn("DRIVER=sql server;SERVER=VM-TXSQLDEV02\\BBE;Trusted Connection=Yes;Database=DSS_SFS;");

    auto q = conn.make_query();

}
