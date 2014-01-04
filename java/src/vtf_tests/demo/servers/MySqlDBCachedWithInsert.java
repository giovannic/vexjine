package vtf_tests.demo.servers;

import java.sql.SQLException;

import vtf_tests.demo.CacheImpl;
import vtf_tests.demo.SqlDriverInfo;
import vtf_tests.demo.loggers.Loggers;
import vtf_tests.serverClientTest.Request;

public class MySqlDBCachedWithInsert extends ModelDbAllUpdateRealCache {

	protected SqlDriverInfo info;
	public MySqlDBCachedWithInsert(CacheImpl cache, SqlDriverInfo sqlDriverInfo) {
		super(cache);
		this.info = sqlDriverInfo;
	}
		
	protected int makeInsertingRequestToDBserver(Request request) {
		long startTime = Loggers.insertResponseTimeLogger.onEventStart();
		info.updateCustomerInfo(request.getChar());
		Loggers.insertRequestStartTimestamp.onEventEnd(startTime);
		Loggers.insertResponseTimeLogger.onEventEnd(startTime);
		
		return 0;
	}

	public void cleanup() {
		/*
		try {
			info.disconnect();
			
		} catch (SQLException e) {
			System.out.println("SQLException on disconnect " + e.getMessage());
			e.printStackTrace();
			
		}
		// this is a bit of a hack
		*/ 
		
	}
}
