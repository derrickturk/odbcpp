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

    for (std::size_t i = 0; i < q.fields().size(); ++i) {
        std::cout << "Field " << i << ": " << q.fields()[i].name << '\n';
        std::cout << "Type: " << type_name(q.fields()[i].type) << '\n';

        auto&& d = q.get(i);

        std::cout << "Got datum { type: " << type_name(d.type()) << ","
            << (d ? " not " : " ") << "null }\n";
    }
}
