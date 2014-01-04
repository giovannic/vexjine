package vtf_tests.demo;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Random;

import vtf_tests.demo.loggers.DurationLogger;
import vtf_tests.demo.loggers.Loggers;

public class SqlDriverInfo {

	private String dbClassName;
	private String connectionString;
	private boolean keepSessionOpen;
	private Connection connection;
	protected Random random;
	
	public SqlDriverInfo(String dbClassName, String connectionString, boolean keepSessionOpen) {
		this.dbClassName = dbClassName;
		this.connectionString = connectionString;
		this.keepSessionOpen = keepSessionOpen;
		
		try {
			Class.forName(dbClassName);
			connection = java.sql.DriverManager.getConnection(connectionString);
		} catch (ClassNotFoundException e) {
			connection = null;
			e.printStackTrace();
		} catch (SQLException e) {
			connection = null;
			e.printStackTrace();
		}
		random = new Random();
	}

	
	
//	public Integer getMySqlServerResponse(char c) throws SQLException  {
//		PreparedStatement getCount = cachedConnectionToMySqlServer.
//			prepareStatement(getQueryStringOfAllRecordsWithLetter(c));
//		ResultSet countResult = getCount.executeQuery();
//		countResult.next();
//		Integer count = countResult.getInt(1);
//		countResult.close();
//		return count;
//	}
	
	
	
	
	public Integer getAllRecordsWithLetter(char char1) throws SQLException  {
//long q1 = System.nanoTime();

		Connection c = null;
		if (!keepSessionOpen) {
			c = java.sql.DriverManager.getConnection(connectionString);
		} else {
			c = connection;
		}

		PreparedStatement get_name = c.prepareStatement(getQueryStringOfAllRecordsWithLetter(char1));


		ResultSet rs = get_name.executeQuery();

		rs.next();
		
		Integer count = rs.getInt(1);
		rs.close();
		
		if (!keepSessionOpen) {
			System.out.println("closing connection");	
			c.close();
		}
//System.out.println((System.nanoTime() - q1)/1e9);
		return count;
	}

	
	public void updateCustomerInfo(char char1)  {
		try {
			Connection c = null;
			if (!keepSessionOpen) {
				c = java.sql.DriverManager.getConnection(connectionString);
			} else {
				c = connection;
			}

			if (c.isClosed()) {
				return;
			}
		

			PreparedStatement get_name = c.prepareStatement(getOrderInsertingQueryStringWithLetter(char1));
			get_name.executeUpdate();
		
			if (!keepSessionOpen) {
				System.out.println("closing connection");	
				c.close();
			}
		} catch (Exception e) {	// hack allowing 
//			System.out.println("Connection closed by main server");
		}

	}
	
	public void disconnect() throws SQLException {
		if (keepSessionOpen) {	
			connection.close();
		}
	}
	
	private String getQueryStringOfAllRecordsWithLetter(char char1) {
		// approx 7ms
//		return "SELECT COUNT(i_id), i_id, i_title, a_fname, a_lname FROM item, author WHERE item.i_a_id = author.a_id AND item.i_subject LIKE '%"+char1+"%' ORDER BY item.i_pub_date DESC,item.i_title";
		// appox 11sec
//		return "SELECT COUNT(o_id), c_uname, c_fname, c_lname FROM orders, customer WHERE orders.o_c_id = customer.c_id AND (c_uname LIKE '%"+char1+"%' OR c_fname LIKE '%"+char1+"%' OR c_lname LIKE '%"+char1+"%') ORDER BY c_lname ASC, c_fname ASC";
		
		return "SELECT COUNT(o_id) FROM orders, customer WHERE customer.c_id<1900 AND orders.o_c_id = customer.c_id AND c_lname LIKE '%"+char1+"%'";
	}	

	
	private String getOrderInsertingQueryStringWithLetter(char char1) {
		return "UPDATE customer SET c_lname = '" + generateRandomLastName(char1) + "' WHERE c_id<1900 ORDER BY RAND() LIMIT 1";
	}
		
	private String generateRandomLastName(char char1) {
		Random r = new Random();
		StringBuilder str = new StringBuilder(char1);
		for (int i = 0; i<10; i++) {
			int c = 65 +random.nextInt(26);
			if (random.nextBoolean()) {
				c += 32;
			}
			str.append((char)c);
		}
		return str.toString();
	}

	public static void main(String[] args) {
		SqlDriverInfo driver = null;
		if (args[1].equals("local")) {
			driver = new SqlDriverInfo("com.mysql.jdbc.Driver", "jdbc:mysql://localhost:3306/tpcwdb?user=nb605&password=admin&useReadAheadInput=false", true);
		} else {
			driver = new SqlDriverInfo("com.mysql.jdbc.Driver", "jdbc:mysql://irmabunt.doc.ic.ac.uk:3306/tpcwdb?user=nb605&password=admin&useReadAheadInput=false", true);
		}
		int iterations = Integer.parseInt(args[0]);

		DemoConfiguration.logOutputPattern = new String("0.0000");
		Loggers.selectResponseTimeLogger = new DurationLogger("SELECT duration");
		Loggers.insertResponseTimeLogger = new DurationLogger("INSERT duration");
		try {
			long start = System.currentTimeMillis();
			for (int i = 0; i<iterations; i++) {
				char c = (char)(58 + i % 26);
				long startTime = Loggers.selectResponseTimeLogger.onEventStart();
				driver.getAllRecordsWithLetter(c);
				Loggers.selectResponseTimeLogger.onEventEnd(startTime);
			}
			long end = System.currentTimeMillis();
			System.out.println("Mean SELECT time: " + (end-start)/(double)iterations + " ms");
			Loggers.selectResponseTimeLogger.print(System.out);	
			
			start = System.currentTimeMillis();
			for (int i = 0; i<iterations; i++) {
				char c = (char)(58 + i % 26);
				long startTime = Loggers.insertResponseTimeLogger.onEventStart();
				driver.updateCustomerInfo(c);
				Loggers.insertResponseTimeLogger.onEventEnd(startTime);
			}
			end = System.currentTimeMillis();
			System.out.println("Mean UPDATE time: " + (end-start)/(double)iterations + " ms");
			Loggers.insertResponseTimeLogger.print(System.out);
			
		} catch (Exception e) {
			e.printStackTrace();
		}
		
	}
}
