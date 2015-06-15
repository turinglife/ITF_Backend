//
// Copyright 2015 CUHK
//

#include "dbi.hpp"

#include <string>

bool CDbi::Connect(const std::string& user, const std::string& pass, const std::string& url) {
    try {
        sql::Driver * driver = sql::mysql::get_driver_instance();
        /* Using the Driver to create a connection */
        con_.reset(driver->connect(url, user, pass));
    } catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }
    return true;
}

bool CDbi::UseDB(const std::string& database) {
    /* Creating a "simple" statement - "simple" = not a prepared statement */
    boost::scoped_ptr< sql::Statement > stmt(con_->createStatement());

    /* Create a test table demonstrating the use of sql::Statement.execute() */
    stmt->execute("USE " + database);
    return true;
}

