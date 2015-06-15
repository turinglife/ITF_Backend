//
// ITF Database Class Interfaces, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_DBI_H
#define ITF_DBI_H

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/warning.h>
#include <cppconn/metadata.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/statement.h>
#include <mysql_driver.h>
#include <mysql_connection.h>

#include <string>

#include "common.hpp"


class CDbi {
 public:
    bool Connect(const std::string& user, const std::string& pass, const std::string& url);
    bool UseDB(const std::string& database);

 private:
    boost::scoped_ptr< sql::Connection > con_;
};


#endif  // ITF_DBI_H
