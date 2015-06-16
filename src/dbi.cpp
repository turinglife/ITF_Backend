//
// Copyright 2015 CUHK
//

#include "dbi.hpp"

#include <string>

CDbi::CDbi() {
    // Create object without connecting to the database server. 
    // If true, exceptions are thrown on errors
    conn_ = mysqlpp::Connection(false);
}

CDbi::~CDbi() {
    conn_.disconnect();
}

bool CDbi::Connect(const std::string& server, const std::string& user, const std::string& pass) {
    if (conn_.connect(0, server.c_str(), user.c_str(), pass.c_str())) {
        return true;
    } else {
        std::cerr << "ERROR: " << conn_.error() << std::endl;
        return false;
    }
}

bool CDbi::UseDB(const std::string& db_name) {
    if (conn_.select_db(db_name)) {
        return true;
    } else {
        return false;
    }
}
