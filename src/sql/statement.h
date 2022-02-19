#ifndef __SQL_STATEMENT_H
#define __SQL_STATEMENT_H

#include <memory>
#include <string>
#include "resultset.h"

namespace raymond {
	class Connection;
	class Statement {
	friend class Connection;
	protected:
		Statement() {}
		Statement(const Statement& s) {}
		//Statement& operator=(const Statement& s) {return s;}
	public:
		typedef std::shared_ptr<Statement> ptr;

		virtual bool query(std::string sql) = 0;
		virtual bool real_query(const char* sql, size_t length) = 0;
		virtual ResultSet::ptr getResultSet() = 0;	
		virtual std::string getError() = 0;
		virtual int getErrno() = 0;
	};
}

#endif
