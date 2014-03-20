#include "odbcpp.hpp"
#include "odbcpp_streams.hpp"

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

    if (q) {
        for (unsigned i = 0; i < q.fields().size(); ++i)
            std::cout << (i ? "\t" : "") << q.fields()[i].name << " :: "
                << q.fields()[i].type;
        std::cout << '\n';
    }

    while (q) {
        for (unsigned i = 0; i < q.fields().size(); ++i)
            std::cout << (i ? "\t" : "") << *(q.get(i));
        std::cout << '\n';
        q.advance();
    }
}
