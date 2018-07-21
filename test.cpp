#include "odbcpp.hpp"
#include "odbcpp_streams.hpp"

#include "sql.h"
#include "sqlext.h"

#include <cassert>
#include <iostream>

int main(int argc, char *argv[])
{
    using namespace odbcpp;

    if (argc != 2) {
        std::cerr << "Usage: " << (argc ? argv[0] : "odbcpp")
            << " connection-string\n";
        return 0;
    }

    char line[1024];
    connection conn(argv[1]);

    while (std::cin.getline(line, 1024)) {
        auto q = conn.make_query();
        q.execute(line);

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
    };
}
