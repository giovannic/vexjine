package vtf_tests.demo.servers;

import java.sql.SQLException;

import vtf_tests.demo.CacheImpl;
import vtf_tests.demo.SqlDriverInfo;
import vtf_tests.demo.loggers.Loggers;
import vtf_tests.serverClientTest.Request;


public class MySqlDBCached extends ModelDbSelectRealCache {

	
	public MySqlDBCached(CacheImpl cache, SqlDriverInfo sqlDriverInfo) {
		super(cache);
		this.info = sqlDriverInfo;
	}
	
	protected SqlDriverInfo info;
	protected Object makeRequestToDBserver(Request request) {
		try {
//System.out.println("select: " + System.currentTimeMillis());
			Integer returnValue = info.getAllRecordsWithLetter(request.getChar());
			return returnValue;
		} catch (SQLException e) {
			System.out.println("SQLException on select " + e.getMessage());
			e.printStackTrace();
			return 0;
		}
	}

	public void cleanup() {
		try {
			info.disconnect();
			
		} catch (SQLException e) {
			System.out.println("SQLException on disconnect " + e.getMessage());
			e.printStackTrace();
			
		}
	}
}
