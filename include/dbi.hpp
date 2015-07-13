//
// ITF Database Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_DBI_H
#define ITF_DBI_H

#define MYSQLPP_MYSQL_HEADERS_BURIED

#include <mysql++/mysql++.h>

#include <string>
#include <map>
#include <vector>

#include "common.hpp"


class CDbi {
 public:
    CDbi();
    ~CDbi();
    // connect to mysql server
    bool Connect(const std::string& server, const std::string& user, const std::string& pass);
    // choose which database to use
    bool UseDB(const std::string& db_name);
    // Query() is used to SELECT the table contents
    std::vector<std::map<std::string, std::string> > Query(const std::string& sql);
    // RunSQL is used to run other sqls except for SELECT
    bool RunSQL(const std::string& sql);

 private:
    mysqlpp::Connection conn_;
};


#endif  // ITF_DBI_H
