package vtf_profiler;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintStream;
import java.sql.*;
import java.util.Random;
//import com.mysql.jdbc.Driver;

import virtualtime.EventNotifier;
import virtualtime.EventNotifiervtonly;

public class IoProfiler implements Runnable {

	private static boolean usingVT = false;
	private static final int QUERYTYPES = 15;
	private static final String dbClassName = "com.mysql.jdbc.Driver";
	private static final String CONNECTION = "jdbc:mysql://localhost/tpcwdb?user=nb605&password=admin";
	private static int queries;
	private static boolean warmupTest;
	private static int[] globalQueriesPerType;
	private static int[] recordsPerQueryType;
	private static long totaldiff =0 ;
	
	private static double assignedRate = 0.0;
	private static long diff=0 ;
	private static long start = 0;

	private static int querystandard;
	
	private static void mainTest(Connection c, int queries) throws SQLException {
		
		RandomQuery rq = new RandomQuery();
		Random r = new Random();
		String s;
		PreparedStatement get_name;
		ResultSet rs; 
//		Exp2 rtimer = new Exp2(0.0025);	// 400ms
//		Exp2 rtimer = new Exp2(0.0100);	// 100ms
//		Exp2 rtimer = new Exp2(0.0200);	// 50ms
//		Exp2 rtimer = new Exp2(0.0400); // 25ms
//		Exp2 rtimer = new Exp2(0.0500);	// 20ms
			
		Exp2 rtimer;
		rtimer = new Exp2(1.0 / assignedRate); 

		double temp =0.0;
		for (int i = 0; i<queries; ++i) {
					
//			for(int j =0;j<10000000; j++) {
//				temp +=j;
//			}
//			for (int queryTypes = 0; queryTypes < 15; ++queryTypes) {
				int queryType = 11; // querystandard; 
				if (assignedRate > 0.0) {
					//System.out.println("to rate einai " + (1.0/assignedRate));
					try {
						long w = (long)rtimer.next();
						while (w == 0) {
							w = (long)rtimer.next();
						}
						//			System.out.println(w);
						Thread.sleep(w);
					} catch (InterruptedException ie) {


					}
				}
				//			times[i] = System.nanoTime();
				//			int queryType = 5;//r.nextInt(QUERYTYPES);
//				int queryType = r.nextInt(QUERYTYPES);

//				long streal, endreal, stvt, endvt;
//				
//				streal = EventNotifier.getPapiRealTime();
//				stvt = EventNotifier.getThreadVirtualTime();
				
				//			qtypes[i] = queryType; 
				//			++queriesPerType[queryType];
				s = rq.getQueryString(queryType);
				//			System.out.println(queryType);
				get_name = c.prepareStatement(s);

//				long sss = System.nanoTime();

				if (queryType > 12) {
					//int t = 
					get_name.executeUpdate();
					//				System.out.println(queryType + " " + t);
				} else {
					rs = get_name.executeQuery();
					//				int count = 0;
					// Results
					//				while (rs.next()) {						++count;					}
					rs.close();
					//				recordsPerQuerytype[queryType] += count;
				}
//				long eee = System.nanoTime();
//				System.out.println(Thread.currentThread().getId() + ": time of query " + queryType + " " + (eee-sss));
//				System.out.println(eee-sss);
//				endreal = EventNotifier.getPapiRealTime();
//				endvt = EventNotifier.getThreadVirtualTime();
//				System.out.println(queryType + " " + (endreal-streal) + " " + (endvt-stvt));
//			}
			
			
		}
		
	}
	
	public void run(){
//		System.out.println(Thread.currentThread().getName() + " started");
		 //EventNotifiervtonly.ThreadVirtualTime();//System.nanoTime();
		//System.out.println(diff);		
//		System.out.println("Total time for " + (queries * threads) + " random queries: " + 
//		System.out.println((double)diff/1000000000.0 + "\n");

//		int[] queriesPerType = new int[QUERYTYPES];
//		int[] recordsPerQuerytype = new int[QUERYTYPES];
//		for (int i = 0; i<QUERYTYPES; ++i) {
//			queriesPerType[i] = 0;
//			recordsPerQuerytype[i] = 0;
//		}		
			
		try {
			// Now try to connect
			Connection c = java.sql.DriverManager.getConnection(CONNECTION);
			int queryType = 11;
			RandomQuery rq = new RandomQuery();
			String s = rq.getQueryString(queryType);
			//			System.out.println(queryType);
			PreparedStatement get_name = c.prepareStatement(s);
			get_name.executeQuery();
			c.close();	
		} catch (java.lang.Exception ex) {
			ex.printStackTrace();
		}
		
//		for (int i = 0; i<queries; ++i) {
//			System.out.println(qtypes[i] +"," + times[i]);
//		}
//
//		synchronized(IoProfiler.class) {
//			for (int i = 0; i<QUERYTYPES; ++i) {
//				globalQueriesPerType[i] += queriesPerType[i];
//				recordsPerQueryType[i]  += recordsPerQuerytype[i];
//			}				
//		}
//		System.out.println(Thread.currentThread().getName() + " finished");
		
	}
	
	private static void warmupTest() {
		
		try {
			// Now try to connect
			Connection c = java.sql.DriverManager.getConnection(CONNECTION);
			
			mainTest(c, queries);
//			EventNotifier.disableProfilingMatrix();
			/*
			RandomQuery rq = new RandomQuery();
			Random r = new Random();
			String s;
			PreparedStatement get_name;
			ResultSet rs; 
			

			for (int i = 0; i<queries; ++i) {

//				times[i] = System.nanoTime();
				int queryType = 5;//r.nextInt(QUERYTYPES);
//				qtypes[i] = queryType; 
//				++queriesPerType[queryType];
				s = rq.getQueryString(queryType);
//				System.out.println(queryType);
				get_name = c.prepareStatement(s);
				if (queryType > 12) {
					int t = get_name.executeUpdate();
//					System.out.println(queryType + " " + t);
				} else {
					rs = get_name.executeQuery();
//					int count = 0;
					// Results
//					while (rs.next()) {						++count;					}
					rs.close();
//					recordsPerQuerytype[queryType] += count;
				}
		
				get_name.close();
//				times[i] = System.nanoTime() - times[i];
			
			}
			*/
			
			if (usingVT) {
				diff = EventNotifier.getThreadVirtualTime();
			} else {
				diff = System.nanoTime();
			}		
			diff -= start;
			if (!warmupTest) {
				synchronized(IoProfiler.class) {
					totaldiff += diff;
				}
			}
			c.close();	
		} catch (java.lang.Exception ex) {
			ex.printStackTrace();
			
		}
		
		
//		try {
//			
//			warmupTest = true;
//			Class.forName(dbClassName);
//			
//			Thread thread = new Thread(new IoProfiler(), "setupThread");
//			thread.start();
//		
//			thread.join();	
//			warmupTest = false;
//		} catch (Exception ie) {
//			ie.printStackTrace();
//		}
		//System.out.println("--------");
			
	}
	
	private static void mainDest(int threads) throws ClassNotFoundException,SQLException {
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
			threadsArray[i] = new Thread(new IoProfiler(), "requestingThread" + i);
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
		
		System.out.println((double)totaldiff/(threads*1000000000.0));
		
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
	public static void main(String[] args) throws ClassNotFoundException,SQLException
	{
//		EventNotifier.disableProfilingMatrix();
		queries = 1;
		assignedRate = 0.0;
		querystandard = -1;
		globalQueriesPerType = new int[QUERYTYPES];
		recordsPerQueryType = new int[QUERYTYPES];
		warmupTest();
		
		for (int i = 0; i<QUERYTYPES; ++i) {
			globalQueriesPerType[i] = 0;
			recordsPerQueryType[i] = 0;
		}						
		
		int threads = 1;
		if (args.length >= 2) {
			queries = Integer.parseInt(args[0]);
			threads = Integer.parseInt(args[1]);
			
			
			if (args.length >= 3) {
				querystandard = Integer.parseInt(args[2]);
//				assignedRate = Double.parseDouble(args[2]);
			}
		}
		
		
		if (args.length > 3) {
			usingVT = true;
		}
		try {
			File file = new File("/data/gclog");
			PrintStream str = new PrintStream(file);
			str.println("\n\n\n\n\n----------------------arxi mainTest -------------------\n\n\n\n\n");
			str.close();
		//System.out.println("\n\n\n\n\n----------------------arxi mainTest -------------------\n\n\n\n\n");
		} catch(Exception ie) {
			System.out.println("\n\n\n\n\n---------------------- skata -------------------\n\n\n\n\n");
		}
		
//		long start = System.nanoTime();	
		mainDest(threads);
//		long diff = System.nanoTime();	
		
//		diff -= start;
		
//		System.out.println("Total time for " + (queries * threads) + " random queries: " +

//		if(!usingVT) {
//			System.out.println((double)diff/1000000000.0 + "\n");
//		}
		int sum = 0; 
		for (int i = 0; i<QUERYTYPES-2; ++i) {
			sum += recordsPerQueryType[i];
		}
		
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
		switch (selection) {
		case 1: return "SELECT c_fname,c_lname FROM customer WHERE c_id = " + getRandomNumber(MAX_CUSTOMERS);
		case 2: return "SELECT * FROM item,author WHERE item.i_a_id = author.a_id AND i_id = " + getRandomNumber(MAX_ITEMS);
		case 3: return "SELECT * FROM customer, address, country WHERE customer.c_addr_id = address.addr_id AND address.addr_co_id = country.co_id AND customer.c_uname LIKE '%" + getRandomLetters(1) + "%' limit 50";// + getRandomNumber(1000);
		case 4: return "SELECT * FROM item, author WHERE item.i_a_id = author.a_id AND item.i_subject LIKE '%"+ getRandomLetters(1) +"%' ORDER BY item.i_title limit 50";
		case 5: return "SELECT * FROM item, author WHERE item.i_a_id = author.a_id AND item.i_title LIKE '%"+getRandomLetters(1)+"%' ORDER BY item.i_title limit 50";
		case 6: return "SELECT * FROM author, item WHERE author.a_lname LIKE '%"+getRandomLetters(1)+"%' AND item.i_a_id = author.a_id ORDER BY item.i_title limit 50"; 
		case 7: return "SELECT i_id, i_title, a_fname, a_lname FROM item, author WHERE item.i_a_id = author.a_id AND item.i_subject LIKE '%"+getRandomLetters(1)+"%' ORDER BY item.i_pub_date DESC,item.i_title limit 50"; 
		case 8: return "SELECT i_id, i_title, a_fname, a_lname FROM item, author, order_line WHERE item.i_id = order_line.ol_i_id AND item.i_a_id = author.a_id AND order_line.ol_o_id > (SELECT MAX(o_id)-3333 FROM orders) AND item.i_subject LIKE '%"+getRandomLetters(1) +"%' GROUP BY i_id, i_title, a_fname, a_lname ORDER BY SUM(ol_qty) DESC limit 50";
		case 9: return "SELECT J.i_id,J.i_thumbnail from item I, item J where (I.i_related1 = J.i_id or I.i_related2 = J.i_id or I.i_related3 = J.i_id or I.i_related4 = J.i_id or I.i_related5 = J.i_id) and I.i_id = " + getRandomNumber(MAX_ITEMS);
		case 10: return "SELECT c_passwd FROM customer WHERE c_id = " + getRandomNumber(MAX_CUSTOMERS);
		case 11: return "SELECT ol_i_id FROM orders, order_line WHERE orders.o_id = order_line.ol_o_id AND NOT (order_line.ol_i_id = "+ getRandomNumber(MAX_ITEMS) +") AND orders.o_c_id IN (SELECT o_c_id FROM orders, order_line WHERE orders.o_id = order_line.ol_o_id AND orders.o_id > (SELECT MAX(o_id)-10000 FROM orders) AND order_line.ol_i_id = " + getRandomNumber(MAX_ITEMS) +") GROUP BY ol_i_id ORDER BY SUM(ol_qty) DESC limit 50"; 
		case 12: return "SELECT c_uname FROM customer WHERE c_id = " + getRandomNumber(MAX_CUSTOMERS);
		case 13: return "UPDATE item SET i_related1 = "+ getRandomNumber(MAX_ITEMS) +", i_related2 = "+ getRandomNumber(MAX_ITEMS) +", i_related3 = "+ getRandomNumber(MAX_ITEMS) +", i_related4 = "+ getRandomNumber(MAX_ITEMS) +", i_related5 = "+ getRandomNumber(MAX_ITEMS) +" WHERE i_id = "+ getRandomNumber(MAX_ITEMS);
		case 14: return "UPDATE item SET i_cost = "+getRandomFloat()+", i_image = 'img1/image_1.gif', i_thumbnail = 'img4/thumb_4.gif', i_pub_date = CURRENT_DATE WHERE i_id = " + getRandomNumber(MAX_ITEMS);
		default: return "SELECT i_related1 FROM item where i_id = " + getRandomNumber(MAX_ITEMS);
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



