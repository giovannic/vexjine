package vtf_tests;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintStream;
import java.sql.*;
import java.util.Random;
import java.util.Vector;
//import com.mysql.jdbc.Driver;

import virtualtime.EventNotifier;
import virtualtime.EventNotifiervtonly;


//import org.apache.derby.impl.sql.GenericStatement;

public class IoTest implements Runnable {

	private static boolean usingVT = false;
	private static final int SELECTQUERIES = 12;
	private static final int QUERYTYPES = 15;

	private static String dbClassName;
	private static String CONNECTION; 

	public static boolean usingHeavyQueryWorkload = true;
	private static void selectMysqlJDBCDriver(String host) {

		dbClassName = "com.mysql.jdbc.Driver";
		CONNECTION = "jdbc:mysql://" + host + "/tpcwdb?user=nb605&password=admin&useReadAheadInput=false";
		//System.out.println("selecting mysql driver " + CONNECTION);
		usingHeavyQueryWorkload = true;
	}

	private static void selectDerbyJDBCDriver() {
		dbClassName = "org.apache.derby.jdbc.EmbeddedDriver";
		CONNECTION = "jdbc:derby:/homes/nb605/VTF/benchmarks/Derby/tpcwdb;";
		usingHeavyQueryWorkload = false;	// too slow 
	}

	private static void selectH2JDBCDriver() {
		dbClassName = "org.h2.Driver";
		CONNECTION = "jdbc:h2:/homes/nb605/VTF/benchmarks/H2/tpcwdb";
		usingHeavyQueryWorkload = true;
	}


	private static void selectJDBCDriver() {
		try {
			if (System.getProperty("driver").equals("derby")) {
				selectDerbyJDBCDriver();

			} else if (System.getProperty("driver").equals("h2")) {
				selectH2JDBCDriver();

			} else if (System.getProperty("driver").equals("remotemysql")) {
				try {
					String remoteHost = System.getProperty("remotehost");
					selectMysqlJDBCDriver(remoteHost + ".doc.ic.ac.uk:3306");
				} catch (Exception e) {
					selectMysqlJDBCDriver("porsche.doc.ic.ac.uk:3306");
				}


			} else {
				selectMysqlJDBCDriver("localhost");
			}
		} catch (Exception e) {
			selectMysqlJDBCDriver("localhost");
		}
	}

	private static boolean isProfilingRun() {
		try {
			if (System.getProperty("profiling").equals("true")) {
				return true;
			} 
		} catch (Exception e) {
		}
		return false;
	}

	private static int queries;
	private static boolean warmupTest;
	private static int[] globalQueriesPerType;
	private static int[] recordsPerQueryType;
	private static long totaldiff =0 ;
	private static double assignedRate = 1;	
	private static int querystandard;


	private static boolean profileQueries = false;

	private static Vector<Integer> queueSize;

	private void mainTest(Connection c, String s, int queryType) throws SQLException {
		PreparedStatement get_name = c.prepareStatement(s);
		if (queryType > SELECTQUERIES) {
			get_name.executeUpdate();
		} else {
			ResultSet rs = get_name.executeQuery();
			rs.close();
		}	
	}


	private static long[] diffs; 
	private static int[] samples; 
	private static Vector<Long> allSamples;
	
	public static double profileServer()  throws ClassNotFoundException,SQLException {
		Class.forName(dbClassName);
		diffs = new long[QUERYTYPES];
		samples = new int[QUERYTYPES];
		allSamples = new Vector<Long>();
		
		double previousResult = 0.0, lastResult = 0.0;
		int profilingQueries = 100; 
		while (previousResult * lastResult == 0 || covOf(previousResult, lastResult) > 1.0) {
			previousResult = lastResult;
			lastResult = profileServer(profilingQueries);
			profilingQueries += 100;
			System.out.println("temp: " + lastResult);
		}

		for (int i = 0; i<QUERYTYPES; i++) {
			if (samples[i] > 0) {
				System.out.println(i + " " + samples[i] + " " + (diffs[i]/samples[i])/1000000000.0);
			}
		}
		
		System.out.println("\n\n\nAll SAMPLES");
		for (Long l : allSamples) {
			System.out.println(l);
		}

		return (lastResult+previousResult)/2;

	}

	public static double profileServer(int profilingQueries) {
		long totalThreadTime = 0;


		try {
			// Now try to connect

			Connection c = java.sql.DriverManager.getConnection(CONNECTION);

			long start=0,diff=0;
			RandomQuery rq = new RandomQuery();
			Random r = new Random(1000);
			String s;
			for (int i = 0; i<profilingQueries; ++i) {

				start = System.nanoTime();

				int queryType = r.nextInt(QUERYTYPES);
				s = rq.getQueryString(queryType);
				PreparedStatement get_name = c.prepareStatement(s);
				if (queryType > SELECTQUERIES) {
					get_name.executeUpdate();
				} else {
					ResultSet rs = get_name.executeQuery();
					rs.close();
				}

				diff = System.nanoTime();		
				diff -= start;
				allSamples.add(diff);
				//System.out.println(queryType + " " + diff);
				diffs[queryType] += diff;
				++samples[queryType];
				totalThreadTime += diff;
			}
			c.close();	
		} catch (java.lang.Exception ex) {
			ex.printStackTrace();
		}


		return (double)totalThreadTime / (double)profilingQueries / 1000000000.0;
	}

	public static int globalQueueSize = 0;
	public void run(){
		Vector<Integer> threadLocalQueueSize = new Vector<Integer>();
		int currentQueueSize = 0;

		long[] queryDuration = new long[QUERYTYPES];
		int[] queryTimes = new int[QUERYTYPES];
		long totalThreadTime = 0;
		//System.out.println("queries " + queries + " assignedRate : " + assignedRate + " threads " + threads);
		try {
			// Now try to connect

			Connection c = java.sql.DriverManager.getConnection(CONNECTION);

			long start=0,diff=0;


			RandomQuery rq = new RandomQuery();
			Random r = new Random(1000);
			String s;

			//			Exp2 rtimer = new Exp2(0.0050);	// 200ms
			//Exp2 rtimer = new Exp2(0.0100);				// 100ms
			//			Exp2 rtimer = new Exp2(0.0200);	// 50ms
			//			Exp2 rtimer = new Exp2(0.0400); // 25ms
			//			Exp2 rtimer = new Exp2(0.0500);	// 20ms

			Exp2 rtimer = null;
			if (assignedRate > 0.0) {
				rtimer = new Exp2(1.0 / assignedRate); 
			}

			for (int i = 0; i<queries; ++i) {

				if (assignedRate > 0.0) {
					try {
						long w = (long)rtimer.next();
						while (w == 0) {
							w = (long)rtimer.next();
						}
						//			System.out.println(w);
						Thread.sleep(w);
					} catch (Exception ie) {


					}
				}

				++globalQueueSize;
				//				System.out.println(globalQueueSize + " " + System.currentTimeMillis());
				++currentQueueSize;
				threadLocalQueueSize.add(currentQueueSize);

				if (usingVT) {
					start = EventNotifier.getThreadVirtualTime();
				} else {
					start = System.nanoTime();
				}	


				int queryType = r.nextInt(QUERYTYPES);
				s = rq.getQueryString(queryType);
				mainTest(c, s, queryType);


				if (usingVT) {
					diff = EventNotifier.getThreadVirtualTime();
				} else {
					diff = System.nanoTime();
				}
				/*
int percentFinished = (int)(100*((double)i/(double)queries));
if ( percentFinished % 10 == 0 && Thread.currentThread().getName().equals("requestingThread1")) {
	System.out.println("\n" + diff + ": completed " + percentFinished + "% requests\n");
}
				 */

				diff -= start;

				--globalQueueSize;
				--currentQueueSize;
				if (profileQueries) {
					queryDuration[queryType] += diff;
					++queryTimes[queryType];
				}
				totalThreadTime += diff;

			}

			c.close();	
		} catch (java.lang.Exception ex) {
			ex.printStackTrace();
			//System.exit(0);

		}

		if (!warmupTest) {
			int itemsToAdd = threadLocalQueueSize.size();
			synchronized(IoTest.class) {
				totaldiff += totalThreadTime;	
				for (int i  =0 ; i<itemsToAdd; i++) {
					queueSize.add(threadLocalQueueSize.elementAt(i));
				}
			}
		}					

		if (profileQueries) {
			System.out.println("Query\tAverage Time");
			for (int i =0  ; i<QUERYTYPES; i++) {
				try {
					System.out.println(i + "\t" + (double)(queryDuration[i]/queryTimes[i])/1000000000.0);
				} catch (ArithmeticException e) {
					System.out.println(i + "\tNaN");
				}
			}
		}

	}

	private static void warmupTest() {
		try {
			warmupTest = true;
			Class.forName(dbClassName);
			Connection c = java.sql.DriverManager.getConnection(CONNECTION);
			RandomQuery rq = new RandomQuery();
			Random r = new Random();
			String s;
			PreparedStatement get_name;
			ResultSet rs; 

			int queryType = r.nextInt(QUERYTYPES);
			while (queryType == 11) {
				queryType = r.nextInt(QUERYTYPES);
			}
			s = rq.getQueryString(queryType);
			get_name = c.prepareStatement(s);

			if (queryType > SELECTQUERIES) {
				get_name.executeUpdate();
			} else {
				rs = get_name.executeQuery();
				// Results
				rs.close();

			}
			warmupTest = false;
		} catch (Exception ie) {
			ie.printStackTrace();
		}

	}

	// returns the average response time
	private static double mainDest(int threads) throws ClassNotFoundException,SQLException {
		//
		//		globalQueriesPerType = new int[QUERYTYPES];
		//		recordsPerQueryType = new int[QUERYTYPES];		
		//		for (int i = 0; i<QUERYTYPES; ++i) {
		//			globalQueriesPerType[i] = 0;
		//			recordsPerQueryType[i] = 0;
		//		}						
		//							 
		// Class.forName(xxx) loads the jdbc classes and creates a drivermanager class factory
		Class.forName(dbClassName);

		Thread[] threadsArray = new Thread[threads];
		for (int i =0 ; i<threads; i++) {
			threadsArray[i] = new Thread(new IoTest(), "requestingThread" + i);
		}

		for (int i =0 ; i<threads; i++) {
			threadsArray[i].start();
		}

		for (int i =0 ; i<threads; i++) {
			try {
				threadsArray[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

		double averageResponseTime = (double)totaldiff/(queries*threads*1000000000.0); 
		return averageResponseTime;

	}


	private static int recursiveCall(int i) {
		if (i > 0) {
			double temp  = 0;
			for (int k =0 ; k<340; k++) {
				temp += k;
			}

			return 1+recursiveCall(i-1);
		} else {
			return 0;
		}

	}

	private static double covOf(double v1, double v2) {
		double avg = (v1+v2)/2.0;
		double cov = 100 * Math.sqrt( Math.pow(v1-avg,2) + Math.pow(v2-avg,2)) / avg;
		//		System.out.println("\t\tCov = " + cov);
		return cov;
	}

	public static void main(String[] args) throws ClassNotFoundException,SQLException 
	{

		selectJDBCDriver();

		if (isProfilingRun()) {
			System.out.println("Profile server. Avg query time: " + profileServer());
		} else {
			//GenericStatement.totalWaitingTime = 0;
			//GenericStatement.totalWaitingTimeSamples = 0;
			double previousResult = 0.0, lastResult = 0.0;
//			while (previousResult * lastResult == 0 || covOf(previousResult, lastResult) > 10.0) {
//				previousResult = lastResult;
			for (int i = 0 ; i<10; i++) {
				previousResult = lastResult;
				lastResult = mainx(args);
				System.out.println("temp : " + lastResult);
			}
//			}
			System.out.println((lastResult+previousResult)/2);
			// + " " + GenericStatement.totalWaitingTime + " " + GenericStatement.totalWaitingTimeSamples + " " + (GenericStatement.totalWaitingTime/GenericStatement.totalWaitingTimeSamples) );
		}		
	}

	public static double mainx(String[] args) throws ClassNotFoundException,SQLException
	{
		//		EventNotifier.disableProfilingMatrix();
		queries = 1;
		//		assignedRate = 0.0;
		querystandard = -1;
		globalQueriesPerType = new int[QUERYTYPES];
		recordsPerQueryType = new int[QUERYTYPES];
		warmupTest();

		queueSize = new Vector<Integer>();
		for (int i = 0; i<QUERYTYPES; ++i) {
			globalQueriesPerType[i] = 0;
			recordsPerQueryType[i] = 0;
		}						

		int threads = 1;
		if (args.length >= 2) {
			queries = Integer.parseInt(args[0]);
			threads = Integer.parseInt(args[1]);


			if (args.length >= 3) {
				//querystandard = Integer.parseInt(args[2]);
				assignedRate = Double.parseDouble(args[2]);
			}
		}


		if (args.length > 3) {
			usingVT = true;
		}

		queueSize.clear();
		totaldiff =0 ;

		long start = System.nanoTime();	
		double averageResponseTime = mainDest(threads);
		long diff = System.nanoTime();	

		diff -= start;

		int ssum =0 ;
		for (int i =0 ; i<queueSize.size(); i++) {
			ssum += queueSize.elementAt(i);

		}
		double avgqueuesize = (double)ssum / (double)queueSize.size();

		//	double throughput = (1000000000.0*(double)threads*queries)/(double)diff;
		//		System.out.println(averageResponseTime + " " + (diff)/1000000000.0 + " " + avgqueuesize);// + "," + diff + "," +throughput);


		//		System.out.println(averageResponseTime);// + " " + (diff)/1000000000.0);


		//		if(!usingVT) {
		//			System.out.println((double)diff/1000000000.0 + "\n");
		//		}
		int sum = 0; 
		for (int i = 0; i<QUERYTYPES-2; ++i) {
			sum += recordsPerQueryType[i];
		}

		return averageResponseTime;
		//		System.out.println("\tRecords transferred: \t" + sum + "\tUpdates:\t" + (globalQueriesPerType[QUERYTYPES-2]+globalQueriesPerType[QUERYTYPES-1]));

	}
} 


class RandomQuery {
	private String getRandomLetters(int length) {
		Random r = new Random();
		String s = "abegilnorstu";
		int pos = r.nextInt(s.length());
		return s.substring(pos, pos+1);
	}
	private int getRandomNumber(int max) {
		Random r = new Random();
		return r.nextInt(max);
	}
	private float getRandomFloat() {
		Random r = new Random();
		return r.nextFloat();
	}

	private static int MAX_CUSTOMERS = 28800;
	private static int MAX_ITEMS = 1000;

	public String getQueryString(int selection) {
		if (IoTest.usingHeavyQueryWorkload) {
			switch (selection) {
			case 1: return "SELECT c_fname,c_lname FROM customer WHERE c_id = " + getRandomNumber(MAX_CUSTOMERS);
			case 2: return "SELECT * FROM item, author WHERE item.i_a_id = author.a_id AND item.i_subject LIKE '%"+ getRandomLetters(1) +"%' ORDER BY item.i_title ";
			case 3: return "SELECT * FROM item,author WHERE item.i_a_id = author.a_id AND i_id = " + getRandomNumber(MAX_ITEMS);
			case 4: return "SELECT * FROM item, author WHERE item.i_a_id = author.a_id AND item.i_subject LIKE '%"+ getRandomLetters(1) +"%' ORDER BY item.i_title ";
			case 5: return "SELECT * FROM item, author WHERE item.i_a_id = author.a_id AND item.i_title LIKE '%"+getRandomLetters(1)+"%' ORDER BY item.i_title ";
			case 6: return "SELECT * FROM author, item WHERE author.a_lname LIKE '%"+getRandomLetters(1)+"%' AND item.i_a_id = author.a_id ORDER BY item.i_title "; 
			case 7: return "SELECT i_id, i_title, a_fname, a_lname FROM item, author WHERE item.i_a_id = author.a_id AND item.i_subject LIKE '%"+getRandomLetters(1)+"%' ORDER BY item.i_pub_date DESC,item.i_title "; 
			case 8: return "SELECT i_id, i_title, a_fname, a_lname FROM item, author, order_line WHERE item.i_id = order_line.ol_i_id AND item.i_a_id = author.a_id AND order_line.ol_o_id > (SELECT MAX(o_id)-3333 FROM orders) AND item.i_subject LIKE '%"+getRandomLetters(1) +"%' GROUP BY i_id, i_title, a_fname, a_lname ORDER BY SUM(ol_qty) DESC ";
			case 9: return "SELECT J.i_id,J.i_thumbnail from item I, item J where (I.i_related1 = J.i_id or I.i_related2 = J.i_id or I.i_related3 = J.i_id or I.i_related4 = J.i_id or I.i_related5 = J.i_id) and I.i_id = " + getRandomNumber(MAX_ITEMS);
			case 10: return "SELECT c_passwd FROM customer WHERE c_id = " + getRandomNumber(MAX_CUSTOMERS);
			case 11: return "SELECT ol_i_id FROM orders, order_line WHERE orders.o_id = order_line.ol_o_id AND NOT (order_line.ol_i_id = "+ getRandomNumber(MAX_ITEMS) +") AND orders.o_c_id IN (SELECT o_c_id FROM orders, order_line WHERE orders.o_id = order_line.ol_o_id AND orders.o_id > (SELECT MAX(o_id)-10000 FROM orders) AND order_line.ol_i_id = " + getRandomNumber(MAX_ITEMS) +") GROUP BY ol_i_id ORDER BY SUM(ol_qty) DESC "; 
			case 12: return "SELECT c_uname FROM customer WHERE c_id = " + getRandomNumber(MAX_CUSTOMERS);
			case 13: return "UPDATE item SET i_related1 = "+ getRandomNumber(MAX_ITEMS) +", i_related2 = "+ getRandomNumber(MAX_ITEMS) +", i_related3 = "+ getRandomNumber(MAX_ITEMS) +", i_related4 = "+ getRandomNumber(MAX_ITEMS) +", i_related5 = "+ getRandomNumber(MAX_ITEMS) +" WHERE i_id = "+ getRandomNumber(MAX_ITEMS);
			case 14: return "UPDATE item SET i_cost = "+getRandomFloat()+", i_image = 'img1/image_1.gif', i_thumbnail = 'img4/thumb_4.gif', i_pub_date = CURRENT_DATE WHERE i_id = " + getRandomNumber(MAX_ITEMS);
			default: return "SELECT i_related1 FROM item where i_id = " + getRandomNumber(MAX_ITEMS);
			}

		} else {
			switch (selection) {
			case 1: 
			case 4:
				return "SELECT c_fname,c_lname FROM customer WHERE c_id = " + getRandomNumber(MAX_CUSTOMERS);
			case 2: 
			case 5:

			case 3: 
			case 6:		
				return "SELECT * FROM item,author WHERE item.i_a_id = author.a_id AND i_id = " + getRandomNumber(MAX_ITEMS);
			case 7: return "SELECT i_id, i_title, a_fname, a_lname FROM item, author WHERE item.i_a_id = author.a_id AND item.i_subject LIKE '%"+getRandomLetters(1)+"%' ORDER BY item.i_pub_date DESC,item.i_title "; 
			case 8: return "SELECT i_id, i_title, a_fname, a_lname FROM item, author, order_line WHERE item.i_id = order_line.ol_i_id AND item.i_a_id = author.a_id AND order_line.ol_o_id > (SELECT MAX(o_id)-3333 FROM orders) AND item.i_subject LIKE '%"+getRandomLetters(1) +"%' GROUP BY i_id, i_title, a_fname, a_lname ORDER BY SUM(ol_qty) DESC ";
			case 9: return "SELECT J.i_id,J.i_thumbnail from item I, item J where (I.i_related1 = J.i_id or I.i_related2 = J.i_id or I.i_related3 = J.i_id or I.i_related4 = J.i_id or I.i_related5 = J.i_id) and I.i_id = " + getRandomNumber(MAX_ITEMS);
			case 10: return "SELECT c_passwd FROM customer WHERE c_id = " + getRandomNumber(MAX_CUSTOMERS);
			case 11: return "SELECT * FROM author, item WHERE author.a_lname LIKE '%"+getRandomLetters(1)+"%' AND item.i_a_id = author.a_id ORDER BY item.i_title "; 
			case 12: return "SELECT c_uname FROM customer WHERE c_id = " + getRandomNumber(MAX_CUSTOMERS);
			case 13: return "UPDATE item SET i_related1 = "+ getRandomNumber(MAX_ITEMS) +", i_related2 = "+ getRandomNumber(MAX_ITEMS) +", i_related3 = "+ getRandomNumber(MAX_ITEMS) +", i_related4 = "+ getRandomNumber(MAX_ITEMS) +", i_related5 = "+ getRandomNumber(MAX_ITEMS) +" WHERE i_id = "+ getRandomNumber(MAX_ITEMS);
			case 14: return "UPDATE item SET i_cost = "+getRandomFloat()+", i_image = 'img1/image_1.gif', i_thumbnail = 'img4/thumb_4.gif', i_pub_date = CURRENT_DATE WHERE i_id = " + getRandomNumber(MAX_ITEMS);
			default: return "SELECT i_related1 FROM item where i_id = " + getRandomNumber(MAX_ITEMS);
			}
		}

	}
}


/**
 * Exponential distribution class.
 * Copied from the Simulation and Modelling course toolkit.
 */
class Exp2  {
	private double rate ;

	public Exp2( double r ) {
		rate = r ;
	}

	public double next() {
		return -Math.log( Math.random() ) / rate ;
	}

	public static double exp( double lam ) {
		return -Math.log( Math.random() ) / lam ;
	}
}



