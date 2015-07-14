//
// Copyright 2015 CUHK
//

#include "dbi.hpp"

#include <iostream>
#include <string>
#include <map>
#include <vector>

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

std::vector<std::map<std::string, std::string> > CDbi::Query(const std::string& sql) {
    mysqlpp::Query query = conn_.query(sql);
    mysqlpp::StoreQueryResult res = query.store();
    std::vector<std::map<std::string, std::string> > res_vec;

    for (size_t row = 0; row < res.num_rows(); ++row) {
        std::map<std::string, std::string> m_map;
        for (size_t i = 0; i < res.num_fields(); ++i) {
            m_map[res.field_name(i)] = std::string(res[row][res.field_name(i).c_str()]);
        }
        res_vec.push_back(m_map);
    }

    return res_vec;
}

bool CDbi::RunSQL(const std::string& sql) {
    mysqlpp::Query query = conn_.query(sql);
    return query.exec();
}
