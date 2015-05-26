/**
 * @file    SQLiteCpp.h
 * @ingroup SQLiteCpp
 * @brief   SQLiteC++ is a smart and simple C++ SQLite3 wrapper. This file is only "easy include" for other files.
 *
 * Include this main header file in your project to gain access to all functionality provided by the wrapper.
 *
 * Copyright (c) 2012-2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
/**
 * @defgroup SQLiteCpp SQLiteC++
 * @brief    SQLiteC++ is a smart and simple C++ SQLite3 wrapper. This file is only "easy include" for other files.
 */
#pragma once


// Include useful headers of SQLiteC++
#include <cassert>
#include <stdexcept>
#include <string>
#include <sqlite3.h>


/**
 * SQLITECPP_ASSERT SQLITECPP_ASSERT() is used in destructors, where exceptions shall not be thrown
 *
 * Define SQLITECPP_ENABLE_ASSERT_HANDLER at the project level
 * and define a SQLite::assertion_failed() assertion handler
 * to tell SQLiteC++ to use it instead of assert() when an assertion fail.
*/
#ifdef SQLITECPP_ENABLE_ASSERT_HANDLER

// if an assert handler is provided by user code, use it instead of assert()
namespace SQLite
{
    // declaration of the assert handler to define in user code
    void assertion_failed(const char* apFile, const long apLine, const char* apFunc,
                          const char* apExpr, const char* apMsg);

#ifdef _MSC_VER
    #define __func__ __FUNCTION__
#endif
// call the assert handler provided by user code
#define SQLITECPP_ASSERT(expression, message) \
    if (!(expression))  SQLite::assertion_failed(__FILE__, __LINE__, __func__, #expression, message)
} // namespace SQLite

#else

// if no assert handler provided by user code, use standard assert()
// (note: in debug mode, assert() does nothing)
#define SQLITECPP_ASSERT(expression, message)   assert(expression && message)

#endif

// Assertion.h ----------------------------------------------




namespace SQLite
{


/**
 * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
 */
class Exception : public std::runtime_error
{
public:
    /**
     * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
     *
     * @param[in] aErrorMessage The string message describing the SQLite error
     */
    explicit Exception(const std::string& aErrorMessage) :
        std::runtime_error(aErrorMessage)
    {
    }
};


}  // namespace SQLite


/// Compatibility with non-clang compilers.
#ifndef __has_feature
    #define __has_feature(x) 0
#endif

// Detect whether the compiler supports C++11 noexcept exception specifications.
#if (defined(__GNUC__) && (__GNUC__ >= 4 && __GNUC_MINOR__ >= 7 ) && defined(__GXX_EXPERIMENTAL_CXX0X__))
// GCC 4.7 and following have noexcept
#elif defined(__clang__) && __has_feature(cxx_noexcept)
// Clang 3.0 and above have noexcept
#else
    #define noexcept    throw()
#endif

// Exception.h -------------------------------------------------------




namespace SQLite
{


// Forward declaration
class Database;
class Column;

/**
 * @brief RAII encapsulation of a prepared SQLite Statement.
 *
 * A Statement is a compiled SQL query ready to be executed step by step
 * to provide results one row at a time.
 *
 * Resource Acquisition Is Initialization (RAII) means that the Statement
 * is compiled in the constructor and finalized in the destructor, so that there is
 * no need to worry about memory management or the validity of the underlying SQLite Statement.
 *
 * Thread-safety: a Statement object shall not be shared by multiple threads, because :
 * 1) in the SQLite "Thread Safe" mode, "SQLite can be safely used by multiple threads
 *    provided that no single database connection is used simultaneously in two or more threads."
 * 2) the SQLite "Serialized" mode is not supported by SQLiteC++,
 *    because of the way it shares the underling SQLite precompiled statement
 *    in a custom shared pointer (See the inner class "Statement::Ptr").
 */
class Statement
{
public:
    class Ptr;

    /**
     * @brief Compile and register the SQL query for the provided SQLite Database Connection
     *
     * @param[in] aDatabase the SQLite Database Connection
     * @param[in] apQuery   an UTF-8 encoded query string
     *
     * Exception is thrown in case of error, then the Statement object is NOT constructed.
     */
    Statement(Database& aDatabase, const char* apQuery);

    /**
     * @brief Compile and register the SQL query for the provided SQLite Database Connection
     *
     * @param[in] aDatabase the SQLite Database Connection
     * @param[in] aQuery    an UTF-8 encoded query string
     *
     * Exception is thrown in case of error, then the Statement object is NOT constructed.
     */
    Statement(Database& aDatabase, const std::string& aQuery);

    /**
     * @brief Finalize and unregister the SQL query from the SQLite Database Connection.
     */
    virtual ~Statement() noexcept; // nothrow

    /**
     * @brief Reset the statement to make it ready for a new execution.
     */
    void reset();

    /**
     * @brief Clears away all the bindings of a prepared statement.
     */
    void clearBindings(void); // throw(SQLite::Exception)

    ////////////////////////////////////////////////////////////////////////////
    // Bind a value to a parameter of the SQL statement,
    // in the form "?" (unnamed), "?NNN", ":VVV", "@VVV" or "$VVV".
    //
    // Can use the parameter index, starting from "1", to the higher NNN value,
    // or the complete parameter name "?NNN", ":VVV", "@VVV" or "$VVV"
    // (prefixed with the corresponding sign "?", ":", "@" or "$")
    //
    // Note that for text and blob values, the SQLITE_TRANSIENT flag is used,
    // which tell the sqlite library to make its own copy of the data before the bind() call returns.
    // This choice is done to prevent any common misuses, like passing a pointer to a
    // dynamic allocated and temporary variable (a std::string for instance).
    // This is under-optimized for static data (a static text define in code)
    // as well as for dynamic allocated buffer which could be transfer to sqlite
    // instead of being copied.

    /**
     * @brief Bind an int value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    void bind(const int aIndex, const int&           aValue);
    /**
     * @brief Bind a 64bits int value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    void bind(const int aIndex, const sqlite3_int64& aValue);
    /**
     * @brief Bind a double (64bits float) value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    void bind(const int aIndex, const double&        aValue);
    /**
     * @brief Bind a string value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     *
     * @note This uses the SQLITE_TRANSIENT flag, making a copy of the data, for SQLite internal use
     */
    void bind(const int aIndex, const std::string&   aValue);
    /**
     * @brief Bind a text value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     *
     * @note This uses the SQLITE_TRANSIENT flag, making a copy of the data, for SQLite internal use
     */
    void bind(const int aIndex, const char*          apValue);
    /**
     * @brief Bind a binary blob value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     *
     * @note This uses the SQLITE_TRANSIENT flag, making a copy of the data, for SQLite internal use
     */
    void bind(const int aIndex, const void*          apValue, const int aSize);
    /**
     * @brief Bind a NULL value to a parameter "?", "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    void bind(const int aIndex);

    /**
     * @brief Bind an int value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    void bind(const char* apName, const int&            aValue);
    /**
     * @brief Bind a 64bits int value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    void bind(const char* apName, const sqlite3_int64&  aValue);
    /**
     * @brief Bind a double (64bits float) value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    void bind(const char* apName, const double&         aValue);
    /**
     * @brief Bind a string value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     *
     * @note This uses the SQLITE_TRANSIENT flag, making a copy of the data, for SQLite internal use
     */
    void bind(const char* apName, const std::string&    aValue);
    /**
     * @brief Bind a text value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     *
     * @note This uses the SQLITE_TRANSIENT flag, making a copy of the data, for SQLite internal use
     */
    void bind(const char* apName, const char*           apValue);
    /**
     * @brief Bind a binary blob value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     *
     * @note This uses the SQLITE_TRANSIENT flag, making a copy of the data, for SQLite internal use
     */
    void bind(const char* apName, const void*           apValue, const int aSize);
    /**
     * @brief Bind a NULL value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    void bind(const char* apName); // bind NULL value


    /**
     * @brief Bind an int value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    inline void bind(const std::string& aName, const int&            aValue)
    {
        bind(aName.c_str(), aValue);
    }
    /**
     * @brief Bind a 64bits int value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    inline void bind(const std::string& aName, const sqlite3_int64&  aValue)
    {
        bind(aName.c_str(), aValue);
    }
    /**
     * @brief Bind a double (64bits float) value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    inline void bind(const std::string& aName, const double&         aValue)
    {
        bind(aName.c_str(), aValue);
    }
    /**
     * @brief Bind a string value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     *
     * @note This uses the SQLITE_TRANSIENT flag, making a copy of the data, for SQLite internal use
     */
    inline void bind(const std::string& aName, const std::string&    aValue)
    {
        bind(aName.c_str(), aValue);
    }
    /**
     * @brief Bind a text value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     *
     * @note This uses the SQLITE_TRANSIENT flag, making a copy of the data, for SQLite internal use
     */
    inline void bind(const std::string& aName, const char*           apValue)
    {
        bind(aName.c_str(), apValue);
    }
    /**
     * @brief Bind a binary blob value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     *
     * @note This uses the SQLITE_TRANSIENT flag, making a copy of the data, for SQLite internal use
     */
    inline void bind(const std::string& aName, const void*           apValue, const int aSize)
    {
        bind(aName.c_str(), apValue, aSize);
    }
    /**
     * @brief Bind a NULL value to a named parameter "?NNN", ":VVV", "@VVV" or "$VVV" in the SQL prepared statement (aIndex >= 1)
     */
    inline void bind(const std::string& aName) // bind NULL value
    {
        bind(aName.c_str());
    }


    ////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Execute a step of the prepared query to fetch one row of results.
     *
     *  While true is returned, a row of results is available, and can be accessed
     * thru the getColumn() method
     *
     * @see exec() execute a one-step prepared statement with no expected result
     * @see Database::exec() is a shortcut to execute one or multiple statements without results
     *
     * @return - true  (SQLITE_ROW)  if there is another row ready : you can call getColumn(N) to get it
     *                               then you have to call executeStep() again to fetch more rows until the query is finished
     *         - false (SQLITE_DONE) if the query has finished executing : there is no (more) row of result
     *                               (case of a query with no result, or after N rows fetched successfully)
     *
     * @throw SQLite::Exception in case of error
     */
    bool executeStep();

    /**
     * @brief Execute a one-step query with no expected result.
     *
     *  This method is useful for any kind of statements other than the Data Query Language (DQL) "SELECT" :
     *  - Data Definition Language (DDL) statements "CREATE", "ALTER" and "DROP"
     *  - Data Manipulation Language (DML) statements "INSERT", "UPDATE" and "DELETE"
     *  - Data Control Language (DCL) statements "GRANT", "REVOKE", "COMMIT" and "ROLLBACK"
     *
     * It is similar to Database::exec(), but using a precompiled statement, it adds :
     * - the ability to bind() arguments to it (best way to insert data),
     * - reusing it allows for better performances (efficient for multiple insertion).
     *
     * @see executeStep() execute a step of the prepared query to fetch one row of results
     * @see Database::exec() is a shortcut to execute one or multiple statements without results
     *
     * @return number of row modified by this SQL statement (INSERT, UPDATE or DELETE)
     *
     * @throw SQLite::Exception in case of error, or if row of results are returned !
     */
    int exec();

    ////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Return a copy of the column data specified by its index
     *
     *  Can be used to access the data of the current row of result when applicable,
     * while the executeStep() method returns true.
     *
     *  Throw an exception if there is no row to return a Column from :
     * - before any executeStep() call
     * - after the last executeStep() returned false
     * - after a reset() call
     *
     *  Throw an exception if the specified index is out of the [0, getColumnCount()) range.
     *
     * @param[in] aIndex    Index of the column, starting at 0
     *
     * @note    This method is no more const, starting in v0.5,
     *          which reflects the fact that the returned Column object will
     *          share the ownership of the underlying sqlite3_stmt.
     *
     * @warning The resulting Column object must not be memorized "as-is".
     *          Is is only a wrapper around the current result row, so it is only valid
     *          while the row from the Statement remains valid, that is only until next executeStep() call.
     *          Thus, you should instead extract immediately its data (getInt(), getText()...)
     *          and use or copy this data for any later usage.
     */
    Column  getColumn(const int aIndex);

    /**
     * @brief Test if the column value is NULL
     *
     * @param[in] aIndex    Index of the column, starting at 0
     *
     * @return true if the column value is NULL
     */
    bool    isColumnNull(const int aIndex) const;

    ////////////////////////////////////////////////////////////////////////////

    /// @brief Return the UTF-8 SQL Query.
    inline const std::string& getQuery() const
    {
        return mQuery;
    }
    /// @brief Return the number of columns in the result set returned by the prepared statement
    inline int getColumnCount() const
    {
        return mColumnCount;
    }
    /// @brief true when a row has been fetched with executeStep()
    inline bool isOk() const
    {
        return mbOk;
    }
    /// @brief true when the last executeStep() had no more row to fetch
    inline bool isDone() const
    {
        return mbDone;
    }
    /// @brief Return the numeric result code for the most recent failed API call (if any).
    inline int getErrorCode() const noexcept // nothrow
    {
        return sqlite3_errcode(mStmtPtr);
    }
    /// @brief Return the extended numeric result code for the most recent failed API call (if any).
    inline int getExtendedErrorCode() const noexcept // nothrow
    {
        return sqlite3_extended_errcode(mStmtPtr);
    }
    /// Return UTF-8 encoded English language explanation of the most recent failed API call (if any).
    inline const char* errmsg() const noexcept // nothrow
    {
        return sqlite3_errmsg(mStmtPtr);
    }

public:
    /**
     * @brief Shared pointer to the sqlite3_stmt SQLite Statement Object.
     *
     * Manage the finalization of the sqlite3_stmt with a reference counter.
     *
     * This is a internal class, not part of the API (hence full documentation is in the cpp).
     */
    class Ptr
    {
    public:
        // Prepare the statement and initialize its reference counter
        Ptr(sqlite3* apSQLite, std::string& aQuery);
        // Copy constructor increments the ref counter
        Ptr(const Ptr& aPtr);
        // Decrement the ref counter and finalize the sqlite3_stmt when it reaches 0
        ~Ptr() noexcept; // nothrow (no virtual destructor needed here)

        /// @brief Inline cast operator returning the pointer to SQLite Database Connection Handle
        inline operator sqlite3*() const
        {
            return mpSQLite;
        }

        /// @brief Inline cast operator returning the pointer to SQLite Statement Object
        inline operator sqlite3_stmt*() const
        {
            return mpStmt;
        }

    private:
        /// @{ Unused/forbidden copy operator
        Ptr& operator=(const Ptr& aPtr);
        /// @}

    private:
        sqlite3*        mpSQLite;    //!< Pointer to SQLite Database Connection Handle
        sqlite3_stmt*   mpStmt;      //!< Pointer to SQLite Statement Object
        unsigned int*   mpRefCount;  //!< Pointer to the heap allocated reference counter of the sqlite3_stmt
                                     //!< (to share it with Column objects)
    };

private:
    /// @{ Statement must be non-copyable
    Statement(const Statement&);
    Statement& operator=(const Statement&);
    /// @}

    /**
     * @brief Check if a return code equals SQLITE_OK, else throw a SQLite::Exception with the SQLite error message
     *
     * @param[in] SQLite return code to test against the SQLITE_OK expected value
     */
    void check(const int aRet);

private:
    std::string     mQuery;         //!< UTF-8 SQL Query
    Ptr             mStmtPtr;       //!< Shared Pointer to the prepared SQLite Statement Object
    int             mColumnCount;   //!< Number of columns in the result of the prepared statement
    bool            mbOk;           //!< true when a row has been fetched with executeStep()
    bool            mbDone;         //!< true when the last executeStep() had no more row to fetch
};


}  // namespace SQLite

// Statement.h ---------------------------------------------------------------


namespace SQLite
{


/**
 * @brief Encapsulation of a Column in a row of the result pointed by the prepared Statement.
 *
 *  A Column is a particular field of SQLite data in the current row of result
 * of the Statement : it points to a single cell.
 *
 * Its value can be expressed as a text, and, when applicable, as a numeric
 * (integer or floating point) or a binary blob.
 *
 * Thread-safety: a Column object shall not be shared by multiple threads, because :
 * 1) in the SQLite "Thread Safe" mode, "SQLite can be safely used by multiple threads
 *    provided that no single database connection is used simultaneously in two or more threads."
 * 2) the SQLite "Serialized" mode is not supported by SQLiteC++,
 *    because of the way it shares the underling SQLite precompiled statement
 *    in a custom shared pointer (See the inner class "Statement::Ptr").
 */
class Column
{
public:
    // Make clang happy by explicitly implementing the copy-constructor:
    Column(const Column & aOther) :
        mStmtPtr(aOther.mStmtPtr),
        mIndex(aOther.mIndex)
    {
    }

    /**
     * @brief Encapsulation of a Column in a Row of the result.
     *
     * @param[in] aStmtPtr  Shared pointer to the prepared SQLite Statement Object.
     * @param[in] aIndex    Index of the column in the row of result
     */
    Column(Statement::Ptr& aStmtPtr, int aIndex)    noexcept; // nothrow
    /// @brief Simple destructor
    virtual ~Column()                               noexcept; // nothrow

    // default copy constructor and assignment operator are perfectly suited :
    // they copy the Statement::Ptr which in turn increments the reference counter.

    /**
     * @brief Return a pointer to the named assigned to a result column (potentially aliased)
     */
    const char*     getName() const noexcept; // nothrow

    #ifdef SQLITE_ENABLE_COLUMN_METADATA
    /**
     * @brief Return a pointer to the table column name that is the origin of this result column
     * 
     *  Require definition of the SQLITE_ENABLE_COLUMN_METADATA preprocessor macro :
     * - when building the SQLite library itself (which is the case for the Debian libsqlite3 binary for instance),
     * - and also when compiling this wrapper.
     */
    const char*     getOriginName() const noexcept; // nothrow
#endif

    /// @brief Return the integer value of the column.
    int             getInt() const noexcept; // nothrow
    /// @brief Return the 64bits integer value of the column.
    sqlite3_int64   getInt64() const noexcept; // nothrow
    /// @brief Return the double (64bits float) value of the column.
    double          getDouble() const noexcept; // nothrow
    /**
     * @brief Return a pointer to the text value (NULL terminated string) of the column.
     *
     * @warning The value pointed at is only valid while the statement is valid (ie. not finalized),
     *          thus you must copy it before using it beyond its scope (to a std::string for instance).
     */
    const char*     getText(const char* apDefaultValue = "") const noexcept; // nothrow
    /**
     * @brief Return a pointer to the binary blob value of the column.
     *
     * @warning The value pointed at is only valid while the statement is valid (ie. not finalized),
     *          thus you must copy it before using it beyond its scope (to a std::string for instance).
     */
    const void*     getBlob() const noexcept; // nothrow

    /**
     * @brief Return the type of the value of the column
     *
     * Return either SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT, SQLITE_BLOB, or SQLITE_NULL.
     *
     * @warning After a type conversion (by a call to a getXxx on a Column of a Yyy type),
     *          the value returned by sqlite3_column_type() is undefined.
     */
    int getType() const noexcept; // nothrow

    /// @brief Test if the column is an integer type value (meaningful only before any conversion)
    inline bool isInteger() const noexcept // nothrow
    {
        return (SQLITE_INTEGER == getType());
    }
    /// @brief Test if the column is a floating point type value (meaningful only before any conversion)
    inline bool isFloat() const noexcept // nothrow
    {
        return (SQLITE_FLOAT == getType());
    }
    /// @brief Test if the column is a text type value (meaningful only before any conversion)
    inline bool isText() const noexcept // nothrow
    {
        return (SQLITE_TEXT == getType());
    }
    /// @brief Test if the column is a binary blob type value (meaningful only before any conversion)
    inline bool isBlob() const noexcept // nothrow
    {
        return (SQLITE_BLOB == getType());
    }
    /// @brief Test if the column is NULL (meaningful only before any conversion)
    inline bool isNull() const noexcept // nothrow
    {
        return (SQLITE_NULL == getType());
    }

    /**
     * @brief Return the number of bytes used by the text (or blob) value of the column
     *
     * Return either :
     * - size in bytes (not in characters) of the string returned by getText() without the '\0' terminator
     * - size in bytes of the string representation of the numerical value (integer or double)
     * - size in bytes of the binary blob returned by getBlob()
     * - 0 for a NULL value
     */
    int getBytes() const noexcept;

    /// @brief Alias returning the number of bytes used by the text (or blob) value of the column
    inline int size() const noexcept
    {
        return getBytes ();
    }

    /// @brief Inline cast operator to int
    inline operator int() const
    {
        return getInt();
    }
    /// @brief Inline cast operator to 64bits integer
    inline operator sqlite3_int64() const
    {
        return getInt64();
    }
    /// @brief Inline cast operator to double
    inline operator double() const
    {
        return getDouble();
    }
    /**
     * @brief Inline cast operator to char*
     *
     * @see getText 
     */
    inline operator const char*() const
    {
        return getText();
    }
    /**
     * @brief Inline cast operator to void*
     *
     * @see getBlob
     */
    inline operator const void*() const
    {
        return getBlob();
    }
#ifdef __GNUC__
    // NOTE : the following is required by GCC to cast a Column result in a std::string
    // (error: conversion from ‘SQLite::Column’ to non-scalar type ‘std::string {aka std::basic_string<char>}’)
    // but is not working under Microsoft Visual Studio 2010 and 2012
    // (error C2440: 'initializing' : cannot convert from 'SQLite::Column' to 'std::basic_string<_Elem,_Traits,_Ax>'
    //  [...] constructor overload resolution was ambiguous)
    /// Inline cast operator to std::string
    inline operator const std::string() const
    {
        return getText();
    }
#endif

    /// @brief Return UTF-8 encoded English language explanation of the most recent error.
    inline const char* errmsg() const
    {
        return sqlite3_errmsg(mStmtPtr);
    }
private:
    Statement::Ptr  mStmtPtr;   //!< Shared Pointer to the prepared SQLite Statement Object
    int             mIndex;     //!< Index of the column in the row of result
};

/**
 * @brief Standard std::ostream text inserter
 *
 * Insert the text value of the Column object, using getText(), into the provided stream.
 *
 * @param[in] aStream   Stream to use
 * @param[in] aColumn   Column object to insert into the provided stream
 *
 * @return  Reference to the stream used
 */
std::ostream& operator<<(std::ostream& aStream, const Column& aColumn);


}  // namespace SQLite

// Column.h --------------------------------------------------------

namespace SQLite
{


/**
 * @brief RAII management of a SQLite Database Connection.
 *
 * A Database object manage a list of all SQLite Statements associated with the
 * underlying SQLite 3 database connection.
 *
 * Resource Acquisition Is Initialization (RAII) means that the Database Connection
 * is opened in the constructor and closed in the destructor, so that there is
 * no need to worry about memory management or the validity of the underlying SQLite Connection.
 *
 * Thread-safety: a Database object shall not be shared by multiple threads, because :
 * 1) in the SQLite "Thread Safe" mode, "SQLite can be safely used by multiple threads
 *    provided that no single database connection is used simultaneously in two or more threads."
 * 2) the SQLite "Serialized" mode is not supported by SQLiteC++,
 *    because of the way it shares the underling SQLite precompiled statement
 *    in a custom shared pointer (See the inner class "Statement::Ptr").
 */
class Database
{
    friend class Statement; // Give Statement constructor access to the mpSQLite Connection Handle

public:
    /**
     * @brief Open the provided database UTF-8 filename.
     *
     * Uses sqlite3_open_v2() with readonly default flag, which is the opposite behavior
     * of the old sqlite3_open() function (READWRITE+CREATE).
     * This makes sense if you want to use it on a readonly filesystem
     * or to prevent creation of a void file when a required file is missing.
     *
     * Exception is thrown in case of error, then the Database object is NOT constructed.
     *
     * @param[in] apFilename    UTF-8 path/uri to the database file ("filename" sqlite3 parameter)
     * @param[in] aFlags        SQLITE_OPEN_READONLY/SQLITE_OPEN_READWRITE/SQLITE_OPEN_CREATE...
     * @param[in] apVfs         UTF-8 name of custom VFS to use, or nullptr for sqlite3 default
     *
     * @throw SQLite::Exception in case of error
     */
    Database(const char* apFilename, const int aFlags = SQLITE_OPEN_READONLY, const char * apVfs = NULL);

    /**
     * @brief Open the provided database UTF-8 filename.
     *
     * Uses sqlite3_open_v2() with readonly default flag, which is the opposite behavior
     * of the old sqlite3_open() function (READWRITE+CREATE).
     * This makes sense if you want to use it on a readonly filesystem
     * or to prevent creation of a void file when a required file is missing.
     *
     * Exception is thrown in case of error, then the Database object is NOT constructed.
     *
     * @param[in] aFilename     UTF-8 path/uri to the database file ("filename" sqlite3 parameter)
     * @param[in] aFlags        SQLITE_OPEN_READONLY/SQLITE_OPEN_READWRITE/SQLITE_OPEN_CREATE...
     * @param[in] aVfs          UTF-8 name of custom VFS to use, or empty string for sqlite3 default
     *
     * @throw SQLite::Exception in case of error
     */
    Database(const std::string& aFilename, const int aFlags = SQLITE_OPEN_READONLY, const std::string& aVfs = "");

    /**
     * @brief Close the SQLite database connection.
     *
     * All SQLite statements must have been finalized before,
     * so all Statement objects must have been unregistered.
     *
     * @warning assert in case of error
     */
    virtual ~Database() noexcept; // nothrow

    /**
     * @brief Shortcut to execute one or multiple statements without results.
     *
     *  This is useful for any kind of statements other than the Data Query Language (DQL) "SELECT" :
     *  - Data Manipulation Language (DML) statements "INSERT", "UPDATE" and "DELETE"
     *  - Data Definition Language (DDL) statements "CREATE", "ALTER" and "DROP"
     *  - Data Control Language (DCL) statements "GRANT", "REVOKE", "COMMIT" and "ROLLBACK"
     *
     * @see Statement::exec() to handle precompiled statements (for better performances) without results
     * @see Statement::executeStep() to handle "SELECT" queries with results
     *
     * @param[in] apQueries  one or multiple UTF-8 encoded, semicolon-separate SQL statements
     *
     * @return number of rows modified by the *last* INSERT, UPDATE or DELETE statement (beware of multiple statements)
     * @warning undefined for CREATE or DROP table: returns the value of a previous INSERT, UPDATE or DELETE statement.
     *
     * @throw SQLite::Exception in case of error
     */
    int exec(const char* apQueries);

    /**
     * @brief Shortcut to execute one or multiple statements without results.
     *
     *  This is useful for any kind of statements other than the Data Query Language (DQL) "SELECT" :
     *  - Data Manipulation Language (DML) statements "INSERT", "UPDATE" and "DELETE"
     *  - Data Definition Language (DDL) statements "CREATE", "ALTER" and "DROP"
     *  - Data Control Language (DCL) statements "GRANT", "REVOKE", "COMMIT" and "ROLLBACK"
     *
     * @see Statement::exec() to handle precompiled statements (for better performances) without results
     * @see Statement::executeStep() to handle "SELECT" queries with results
     *
     * @param[in] aQueries  one or multiple UTF-8 encoded, semicolon-separate SQL statements
     *
     * @return number of rows modified by the *last* INSERT, UPDATE or DELETE statement (beware of multiple statements)
     * @warning undefined for CREATE or DROP table: returns the value of a previous INSERT, UPDATE or DELETE statement.
     *
     * @throw SQLite::Exception in case of error
     */
    inline int exec(const std::string& aQueries)
    {
        return exec(aQueries.c_str());
    }

    /**
     * @brief Shortcut to execute a one step query and fetch the first column of the result.
     *
     *  This is a shortcut to execute a simple statement with a single result.
     * This should be used only for non reusable queries (else you should use a Statement with bind()).
     * This should be used only for queries with expected results (else an exception is fired).
     *
     * @warning WARNING: Be very careful with this dangerous method: you have to
     *          make a COPY OF THE result, else it will be destroy before the next line
     *          (when the underlying temporary Statement and Column objects are destroyed)
     *
     * @see also Statement class for handling queries with multiple results
     *
     * @param[in] apQuery  an UTF-8 encoded SQL query
     *
     * @return a temporary Column object with the first column of result.
     *
     * @throw SQLite::Exception in case of error
     */
    Column execAndGet(const char* apQuery);

    /**
     * @brief Shortcut to execute a one step query and fetch the first column of the result.
     *
     *  This is a shortcut to execute a simple statement with a single result.
     * This should be used only for non reusable queries (else you should use a Statement with bind()).
     * This should be used only for queries with expected results (else an exception is fired).
     *
     * @warning WARNING: Be very careful with this dangerous method: you have to
     *          make a COPY OF THE result, else it will be destroy before the next line
     *          (when the underlying temporary Statement and Column objects are destroyed)
     *
     * @see also Statement class for handling queries with multiple results
     *
     * @param[in] aQuery  an UTF-8 encoded SQL query
     *
     * @return a temporary Column object with the first column of result.
     *
     * @throw SQLite::Exception in case of error
     */
    inline Column execAndGet(const std::string& aQuery)
    {
        return execAndGet(aQuery.c_str());
    }

    /**
     * @brief Shortcut to test if a table exists.
     *
     *  Table names are case sensitive.
     *
     * @param[in] apTableName an UTF-8 encoded case sensitive Table name
     *
     * @return true if the table exists.
     *
     * @throw SQLite::Exception in case of error
     */
    bool tableExists(const char* apTableName);

    /**
     * @brief Shortcut to test if a table exists.
     *
     *  Table names are case sensitive.
     *
     * @param[in] aTableName an UTF-8 encoded case sensitive Table name
     *
     * @return true if the table exists.
     *
     * @throw SQLite::Exception in case of error
     */
    inline bool tableExists(const std::string& aTableName)
    {
        return tableExists(aTableName.c_str());
    }

    /**
     * @brief Set a busy handler that sleeps for a specified amount of time when a table is locked.
     *
     * @param[in] aTimeoutMs    Amount of milliseconds to wait before returning SQLITE_BUSY
     */
    inline int setBusyTimeout(int aTimeoutMs) noexcept // nothrow
    {
        return sqlite3_busy_timeout(mpSQLite, aTimeoutMs);
    }

    /**
     * @brief Get the rowid of the most recent successful INSERT into the database from the current connection.
     *
     *  Each entry in an SQLite table always has a unique 64-bit signed integer key called the rowid.
     * If the table has a column of type INTEGER PRIMARY KEY, then it is an alias for the rowid.
     *
     * @return Rowid of the most recent successful INSERT into the database, or 0 if there was none.
     */
    inline sqlite3_int64 getLastInsertRowid() const noexcept // nothrow
    {
        return sqlite3_last_insert_rowid(mpSQLite);
    }

    /**
     * @brief Get total number of rows modified by all INSERT, UPDATE or DELETE statement since connection.
     *
     * @return Total number of rows modified since connection to the database. DROP tables does not count.
     */
    inline int getTotalChanges() const noexcept // nothrow
    {
        return sqlite3_total_changes(mpSQLite);
    }

    /// @brief Return the filename used to open the database.
    inline const std::string& getFilename() const noexcept // nothrow
    {
        return mFilename;
    }

    /// @brief Return the numeric result code for the most recent failed API call (if any).
    inline int getErrorCode() const noexcept // nothrow
    {
        return sqlite3_errcode(mpSQLite);
    }

    /// @brief Return the extended numeric result code for the most recent failed API call (if any).
    inline int getExtendedErrorCode() const noexcept // nothrow
    {
        return sqlite3_extended_errcode(mpSQLite);
    }

    /// Return UTF-8 encoded English language explanation of the most recent failed API call (if any).
    inline const char* errmsg() const noexcept // nothrow
    {
        return sqlite3_errmsg(mpSQLite);
    }

    /**
     * @brief Create or redefine a SQL function or aggregate in the sqlite database. 
     *
     *  This is the equivalent of the sqlite3_create_function_v2 command.
     * @see http://www.sqlite.org/c3ref/create_function.html
     *
     * @note UTF-8 text encoding assumed.
     *
     * @param[in] apFuncName    Name of the SQL function to be created or redefined
     * @param[in] aNbArg        Number of arguments in the function
     * @param[in] abDeterministic Optimize for deterministic functions (most are). A random number generator is not.
     * @param[in] apApp         Arbitrary pointer of user data, accessible with sqlite3_user_data().
     * @param[in] apFunc        Pointer to a C-function to implement a scalar SQL function (apStep & apFinal NULL)
     * @param[in] apStep        Pointer to a C-function to implement an aggregate SQL function (apFunc NULL)
     * @param[in] apFinal       Pointer to a C-function to implement an aggregate SQL function (apFunc NULL)
     * @param[in] apDestroy     If not NULL, then it is the destructor for the application data pointer.
     *
     * @throw SQLite::Exception in case of error
     */
    void createFunction(const char* apFuncName,
                        int         aNbArg,
                        bool        abDeterministic,
                        void*       apApp,
                        void      (*apFunc)(sqlite3_context *, int, sqlite3_value **),
                        void      (*apStep)(sqlite3_context *, int, sqlite3_value **),
                        void      (*apFinal)(sqlite3_context *),  // NOLINT(readability/casting)
                        void      (*apDestroy)(void *));

    /**
     * @brief Create or redefine a SQL function or aggregate in the sqlite database. 
     *
     *  This is the equivalent of the sqlite3_create_function_v2 command.
     * @see http://www.sqlite.org/c3ref/create_function.html
     *
     * @note UTF-8 text encoding assumed.
     *
     * @param[in] aFuncName     Name of the SQL function to be created or redefined
     * @param[in] aNbArg        Number of arguments in the function
     * @param[in] abDeterministic Optimize for deterministic functions (most are). A random number generator is not.
     * @param[in] apApp         Arbitrary pointer of user data, accessible with sqlite3_user_data().
     * @param[in] apFunc        Pointer to a C-function to implement a scalar SQL function (apStep & apFinal NULL)
     * @param[in] apStep        Pointer to a C-function to implement an aggregate SQL function (apFunc NULL)
     * @param[in] apFinal       Pointer to a C-function to implement an aggregate SQL function (apFunc NULL)
     * @param[in] apDestroy     If not NULL, then it is the destructor for the application data pointer.
     *
     * @throw SQLite::Exception in case of error
     */
    inline void createFunction(const std::string&   aFuncName,
                               int                  aNbArg,
                               bool                 abDeterministic,
                               void*                apApp,
                               void               (*apFunc)(sqlite3_context *, int, sqlite3_value **),
                               void               (*apStep)(sqlite3_context *, int, sqlite3_value **),
                               void               (*apFinal)(sqlite3_context *), // NOLINT(readability/casting)
                               void               (*apDestroy)(void *))
    {
        return createFunction(aFuncName.c_str(), aNbArg, abDeterministic,
                              apApp, apFunc, apStep, apFinal, apDestroy);
    }

private:
    /// @{ Database must be non-copyable
    Database(const Database&);
    Database& operator=(const Database&);
    /// @}

    /**
     * @brief Check if aRet equal SQLITE_OK, else throw a SQLite::Exception with the SQLite error message
     */
    void check(const int aRet) const;

private:
    sqlite3*    mpSQLite;   //!< Pointer to SQLite Database Connection Handle
    std::string mFilename;  //!< UTF-8 filename used to open the database
};


}  // namespace SQLite

// Datebase.h ----------------------------------------------------

namespace SQLite
{


// Forward declaration
class Database;

/**
 * @brief RAII encapsulation of a SQLite Transaction.
 *
 * A Transaction is a way to group multiple SQL statements into an atomic secured operation;
 * either it succeeds, with all the changes committed to the database file,
 * or if it fails, all the changes are rolled back to the initial state.
 *
 * Resource Acquisition Is Initialization (RAII) means that the Transaction
 * begins in the constructor and is rollbacked in the destructor, so that there is
 * no need to worry about memory management or the validity of the underlying SQLite Connection.
 *
 * This method also offers big performances improvements compared to individually executed statements.
 *
 * Thread-safety: a Transaction object shall not be shared by multiple threads, because :
 * 1) in the SQLite "Thread Safe" mode, "SQLite can be safely used by multiple threads
 *    provided that no single database connection is used simultaneously in two or more threads."
 * 2) the SQLite "Serialized" mode is not supported by SQLiteC++,
 *    because of the way it shares the underling SQLite precompiled statement
 *    in a custom shared pointer (See the inner class "Statement::Ptr").
 */
class Transaction
{
public:
    /**
     * @brief Begins the SQLite transaction
     *
     * @param[in] aDatabase the SQLite Database Connection
     *
     * Exception is thrown in case of error, then the Transaction is NOT initiated.
     */
    explicit Transaction(Database& aDatabase);

    /**
     * @brief Safely rollback the transaction if it has not been committed.
     */
    virtual ~Transaction() noexcept; // nothrow

    /**
     * @brief Commit the transaction.
     */
    void commit();

private:
    // Transaction must be non-copyable
    Transaction(const Transaction&);
    Transaction& operator=(const Transaction&);
    /// @}

private:
    Database&   mDatabase;  //!< Reference to the SQLite Database Connection
    bool        mbCommited; //!< True when commit has been called
};


}  // namespace SQLite


/**
 * @brief Version numbers for SQLiteC++ are provided in the same way as sqlite3.h
 *
 * The [SQLITECPP_VERSION] C preprocessor macro in the SQLiteC++.h header
 * evaluates to a string literal that is the SQLite version in the
 * format "X.Y.Z" where X is the major version number
 * and Y is the minor version number and Z is the release number.
 *
 * The [SQLITECPP_VERSION_NUMBER] C preprocessor macro resolves to an integer
 * with the value (X*1000000 + Y*1000 + Z) where X, Y, and Z are the same
 * numbers used in [SQLITECPP_VERSION].
 */
#define SQLITECPP_VERSION           "0.9.9"
#define SQLITECPP_VERSION_NUMBER    0009009
