#include <iostream>
#include "sqlite3.h"
#include <string.h>
#include <exception>
using std::exception;
#include <string>
using std::string;
//#include
class ISQLRealisation
{
public:
    virtual void Connect(const char *dbName) = 0;
    virtual void CloseConnection() = 0;
    virtual void ExecuteQuery(const char *sqlQuery) = 0;
};

class Exception : public exception
{
protected:
    char message[1024];
public:
    Exception() {}
    Exception(const char *message, const char* file, const char* func, int line);
    ~Exception() throw() {}
    const char* what() const throw();
};

// Exception Realisation
Exception::Exception(const char *message, const char* file, const char* func, int line)
{
    sprintf(this->message, "In file %s func %s line %d message %s", file, func, line, message);
}

const char* Exception::what() const throw()
{
    return message;
}
#define ThrowException(msg) throw Exception(msg, __FILE__, __FUNCTION__, __LINE__);

struct ISQLTableColumn
{
    virtual string ToString() = 0;
};

struct SQLTableColumn: public ISQLTableColumn
{
    string _columnName;
    string _columnType;
    string _columnFlags;

    SQLTableColumn(): _columnName(""), _columnType(""), _columnFlags("") {}
    SQLTableColumn(const string &columnName, const string &columnType, const string &columnFlags="")
    {
        _columnName = columnName;
        _columnType = columnType;
        _columnFlags = columnFlags;
    }

    virtual string ToString() {};
};


struct SQLiteTableColumn: public SQLTableColumn
{
//    SQLiteTableColumn() {}
    string ToString() const
    {
        string result = _columnName;
        result += ' ';
        result += _columnType;
        result += ' ';
        result += _columnFlags;
        return result;
    }
    ~SQLiteTableColumn(){}
};

class SQLiteException: protected Exception
{
    //char message[1024];
public:
    SQLiteException()
    {

    }

    SQLiteException(const char *msg, const char *sqlmsg)
    {
        sprintf(message, "%s\n%s", msg, sqlmsg);
    }

    const char *what() const throw()
    {
        return message;
    }
};

#define ThrowSQLiteException(msg, sqlmsg) \
{\
    CloseConnection();\
    throw SQLiteException(msg, sqlmsg); \
}

class SQLiteRealisation: public ISQLRealisation
{
public:
    SQLiteRealisation(): _dataBase(NULL) {}
    void Connect(const char *dbName)
    {
        if( sqlite3_open(dbName, &_dataBase) )
            ThrowSQLiteException("Error connectiong to database", sqlite3_errmsg(_dataBase));
    }

    void CloseConnection()
    {
        sqlite3_close(_dataBase);
    }


    void ExecuteQuery(const char *query)
    {
        char *err;
        if (sqlite3_exec(_dataBase, query, 0, 0, &err))
        {
            char message[1024];
            strcpy(message, err);
            sqlite3_free(err);
            ThrowSQLiteException("Error execute query", message);
        }
    }

private:
    sqlite3 *_dataBase;
};


class SQLManager
{
    ISQLRealisation *realisation;
    string _dbName;
public:
    SQLManager()
    {
        realisation = new SQLiteRealisation();
        if (!realisation)
            ThrowException("Cant alloc sqlrealisation");
    }

    void ExcecuteQuery(const char *query)
    {
        realisation->ExecuteQuery(query);
    }

    void ExecuteQuery(const string &query)
    {
        realisation->ExecuteQuery(query.c_str());
    }

    void Connect(const char *dbName)
    {
        realisation->Connect(dbName);
        _dbName = dbName;
    }

    void CreateTable(const char *tableName, ISQLTableColumn *column1, ...)
    {
        va_list list;
        va_start(list, column1);
        string queryString = "CREATE TABLE ";
        queryString += _dbName + '.' + string(tableName);
        queryString += '(';
        for (ISQLTableColumn *col = column1; col != NULL; col = va_arg(list, ISQLTableColumn*))
        {
            queryString += ((SQLiteTableColumn*)(col))->ToString();
            queryString += ',';
        }
        queryString[queryString.length() - 1] = '\0';
        queryString += ')';
        va_end ( list );
        std::cout<<queryString<<std::endl;// Cleans up the list
        realisation->ExecuteQuery(queryString.c_str());
    }
};


int main()
{
    try
    {

        SQLiteRealisation realisation;
        SQLManager *manager = new SQLManager();
        manager->Connect("newDatabase");
        ///SQ
        SQLTableColumn column1, column2, column3;
        column1._columnType = "INT";
        column1._columnName = "ID";
        column1._columnFlags = "PRIMARY KEY NOT NULL";

        column2._columnType = "CHAR";
        column2._columnName = "Name";

        column3._columnType = "INT";
        column3._columnName = "Count";

        manager->CreateTable("NewTable", &column1, &column2, &column3);
        std::cout << "Hello, World!" << std::endl;
        return 0;
    }
    catch(SQLiteException &e)
    {
        std::cout<<e.what()<<std::endl;
    }
}