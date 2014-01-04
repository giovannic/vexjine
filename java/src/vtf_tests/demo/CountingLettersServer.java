package vtf_tests.demo;

import java.sql.SQLException;

import vtf_tests.demo.loggers.Loggers;
import vtf_tests.serverClientTest.Request;

public class CountingLettersServer extends CountingLettersCacheImplServer {
	protected SqlDriverInfo info;

	public CountingLettersServer(int requests, ArrayCache cache, SqlDriverInfo sqlDriverInfo) {
		super(requests, cache);
		this.info = sqlDriverInfo;
	}

	
	protected Object makeRequestToDBserver(Request request) {
		try {
			Integer returnValue = info.getAllRecordsWithLetter(request.getChar());			
			return returnValue;
/*
		} catch (ClassNotFoundException e) {
			System.out.println("ClassNotFoundException " + e.getMessage());
			e.printStackTrace();
			return 0;
*/
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
