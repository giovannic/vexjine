package vtf_tests.demo.servers;

import java.sql.SQLException;

import vtf_tests.demo.CacheImpl;
import vtf_tests.demo.SqlDriverInfo;
import vtf_tests.demo.loggers.Loggers;
import vtf_tests.serverClientTest.Request;

public class MySqlDBCachedAccelerated extends MySqlDBCached {

	public MySqlDBCachedAccelerated(CacheImpl cache, SqlDriverInfo sqlDriverInfo) {
		super(cache, sqlDriverInfo);
	}

	@virtualtime.Accelerate(speedup = 0.5)
	protected Object makeRequestToDBserver(Request request) {
		try {
			long startTime = Loggers.selectResponseTimeLogger.onEventStart();
			Integer returnValue = info.getAllRecordsWithLetter(request.getChar());
			Loggers.selectRequestStartTimestamp.onEventEnd(startTime);
			Loggers.selectResponseTimeLogger.onEventEnd(startTime);
			
			return returnValue;
		} catch (SQLException e) {
			System.out.println("SQLException on select " + e.getMessage());
			e.printStackTrace();
			return 0;
		}
	}

}