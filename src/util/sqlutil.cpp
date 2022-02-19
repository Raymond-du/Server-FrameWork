#include "sqlutil.h"

namespace raymond {

	Connection* getConnection() {
		static MysqlConnection g_connection;
		if (!g_connection.isConnected()) {
			if (false == g_connection.connect(sqlHost->getValue(),
													sqlUser->getValue(),
													sqlPasswd->getValue(),
													sqlDatabase->getValue(),
													sqlPort->getValue())) {
				return nullptr;
			}
		}
		return &g_connection;
	}
}
