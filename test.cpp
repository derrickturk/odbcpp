#include "odbcpp.hpp"

#include "sql.h"
#include "sqlext.h"

#include <cassert>
#include <iostream>

int main()
{
    using namespace odbcpp;

    connection conn("DRIVER=sql server;SERVER=VM-TXSQLDEV02\\BBE;Trusted Connection=Yes;Database=DSS_SFS;");

    auto q = conn.make_query();
    q.execute("select * from dbo.snapshot_merge_WellMaster");

    auto fs = q.fields();
    for (const auto& f: fs) {
        std::cout << "Field: " << f.name << '\n'
            << "Type: " << type_name(f.type) << '\n';
    }
}
