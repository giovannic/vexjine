package vtf_tests;
import virtualtime.Accelerate;
import virtualtime.EventNotifier;

import virtualtime.EventNotifiervtonly;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.Reader;
import java.io.BufferedReader;
import java.util.Random;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Vector;

public class CpuTest {

	private static final boolean USINGVT = true;

	private static final int ITERATIONS = 10;
	
    public int memConsumer;

    public int myMethod() {
    	
    	memConsumer = memConsumer+1;
    	System.out.println(memConsumer);
    	
    	return memConsumer;
    }
    /*
	 * This worked like a charm. VTslow/VTfast = RTslow/RTfast = CPUslow/CPUfast
	 *@Accelerate(speedup = 1)
	 *
	
	private double myCalcFunc(int loops) {
        double temp = 0.0;
        
  		for(int i = 0; i<loops; i++) {
            temp += (i * (8+i)) / (i+1);      			      			
  		}
  		temp += 1;
        
        return temp;
	}

	/*
	 * This worked terribly: RTfast << RTslow (why? it should be closer since we have I/O)
	 * and VTslow/VTfast = CPUslow/CPUfast
	 * 
	 * Special effort to eliminate caching effects by using only new files at each test
	 *@Accelerate(speedup = 0.75)
	 */
	/*
	private void myIOFunc(int limits) {
		try {
			FileReader fr = new FileReader("/home/nick/Virtual_Time/java/events6.txt");
			FileWriter fw = new FileWriter("/home/nick/Virtual_Time/java/temp11.txt",true);
			int c;
			int count = 0;
			while((c = fr.read()) != -1) {
				fw.write(c);
			}
			fr.close();
			fw.close();
		} catch (IOException ex){
			System.out.println("File not found");
			
		}
		
	}*/

	/*
	 * RTfast = 89	(x3)
	 * RTslow = 112 (x3)
	 * VTslow = 64 (x2)
	 * VTfast = 47 (x2)
	 * 
	 * RTfast/RTslow ~ 79%
	 * VTfast/VTslow ~ 73%
	 * CPUfast/CPUslow = 75%
	 *
	 * Again caching effects have been taken into account to be resolved.
	 *
	 *@Accelerate(speedup = 0.75)
	 */

	public static void myReader() {
		try {
			FileReader fr = new FileReader("/home/nick/apache-tomcat-6.0.18/apache-tomcat-6.0.18/webapps/InternalSocketServlet/warandpeace.txt");
			fr.read();
			fr.read();
			fr.read();
			//System.out.println(c);
			fr.close();		
		} catch (Exception ex){
			System.out.println(ex.getStackTrace().toString());
		}

	}
	
	public static void myWriter() {

					try {

						FileWriter fw = new FileWriter("/home/nick/Virtual_Time/java/temp_to_write.txt",true);
						
						fw.write("o papas o paxys einai kala\n");
						fw.write("o papas o paxys einai kalytera\n");
						fw.write("o papas o paxys einai kallista\n");
						
						fw.close();
					} catch (IOException ex){
						System.out.println("File not found");
						
					}


	}

	private FileReader myFileReader(String file) {
		try {
		    long start, end;
			start = System.nanoTime();
			FileReader fr = new FileReader(file);
			end = System.nanoTime();
			System.out.println(end - start);

			return fr;
		} catch (Exception ex){
			System.out.println(ex.getStackTrace().toString());
			return null;
		}

	}	
	/*
	private void myOnlyReadFuncPrinting(int limits) {
		try {
			int c = 0;
			int count = 0;
			char[] buf = new char[1024];
	        long start, end,s1,e1;

			start = System.nanoTime();
			FileReader fr = new FileReader("/home/nick/apache-tomcat-6.0.18/apache-tomcat-6.0.18/webapps/InternalSocketServlet/warandpeace_temp.txt");
			end = System.nanoTime();
			System.out.println("Only Filereader definition "+ (end - start));

			start = System.nanoTime();
			while(c != -1) {
	

				count++;
				s1 = System.nanoTime();
				c = myRead(fr, buf);

				e1 = System.nanoTime();
				System.out.println(count + ") myReads "+ (e1 - s1));
			}
			end = System.nanoTime();
			System.out.println("all myReads "+ (end - start));
			
			fr.close();
			
		} catch (Exception ex){
			System.out.println("File not found");	
		}
		
	}
	private void myOnlyReadFunc(int limits) {
		try {
			int c = 0;
			int count = 0;
			char[] buf = new char[1024];

			FileReader fr = new FileReader("/home/nick/apache-tomcat-6.0.18/apache-tomcat-6.0.18/webapps/InternalSocketServlet/warandpeace_temp.txt");
			while(c != -1) {
				count++;
				c = myRead(fr, buf);
			}
			fr.close();
			
		} catch (Exception ex){
			System.out.println("File not found");	
		}
		
	}

	public void makeRequest() {	
		
		int size = 8192;
		char[] buf = new char[8192];
		
		try {
			FileReader fr = myFileReader("/home/nick/apache-tomcat-6.0.18/apache-tomcat-6.0.18/webapps/InternalSocketServlet/warandpeace_temp.txt");
						
			myRead(fr, buf);     
	        fr.close();
		} catch (Exception ex){
			System.out.println("File not found");	
		}

	}
	@Accelerate(speedup = 2.75)	

  				if (i % 5000 == 0) {
  					System.out.println("*" + temp);
  				}

*/	
	
	/*
	 * Basic looping method 
	 */	
	public double doitIns(double i) {
		
		//for (int k = 0;k<100;k++) {
			i = (i * (8.0+i)) / (i+1.0);
		//}

		return i;
	}
	
	public double myLoopsIns(long loopCount) {
    	double temp = 0.0;
    	double i;
//long start,dur;
  //  	start = getTime("1");
  		for(i = 0; i<loopCount; i++) {
  				//temp += (i * (8.0+i)) / (i+1.0);//doit(i);
  				//
  				//for(int kk  = 0;kk<1000;kk++) {
  				temp += doitIns(i);
  				//temp += (i * (8.0+i)) / (i+1.0);
  				//}
  				//
  		}
	//	 dur = getTime("1");
		// System.out.println((dur-start)/loopCount);
  		temp += 1;
  		return temp;
	}	
	
	public static double testMethodDepthEffect(int arg, long i) {
		if (arg == 0) {
			double temp = 0.0;
			for (int j =0 ; j<1000; j++) {
				temp += (double) (i * (8+i)) / (j+1);
			}
			return temp;
		} else {
			return testMethodDepthEffect(arg-1, i);
		}
		               
		
	}

	public static double testAccelerationAbsense(int arg, long i) {
		
		return testMethodDepthEffect(arg,i);
		/*
		if (arg == 0) {
			// Use here the method you want
			return (double)(i * (8+i)) / (i+1);
		} else {
			return testMethodDepthEffect(arg-1, i);			
		}
		*/
		
	}
	
	public static double testAccelerationCorrectness(int arg, long i) {
		
		if (arg == 0) {
			// Use here the method you want
			return (double)(i * (8+i)) / (i+1);
		} else {
			return testMethodDepthEffect(arg-1, i);			
		}
		
	}

	public static double testAccelerationCorrectness(int arg, long i, int testingOverloading) {
		
		if (arg == 0) {
			// Use here the method you want
			return (double)(i * (8+i) + testingOverloading) / (i+1);
		} else {
			return testMethodDepthEffect(arg-1, i);			
		}
		
	}
	
	
	/*
	 * Basic looping method 
	 */	
	public static double doit(double i) {
		
		//for (int k = 0;k<100;k++) {
			i = (i * (8.0+i)) / (i+1.0);
		//}

		return i;
	}
	
	public static double myLoops(long loopCount) {
    	double temp = 0.0;
    	double i;
//long start,dur;
  //  	start = getTime("1");
//    	System.out.println("sdgs");
  		for(i = 0; i<loopCount; i++) {
  				//temp += (i * (8.0+i)) / (i+1.0);//doit(i);
  				//
  				//for(int kk  = 0;kk<1000;kk++) {
//  				temp += doit(i);
//  				synchronized(class) {
  					temp += (i * (8.0+i)) / (i+1.0);
//  				}
  			
  				//temp += testAccelerationCorrectness(0, 6);
  				//temp += (i * (8.0+i)) / (i+1.0);
  				//}
  				//
  		}
	//	 dur = getTime("1");
		// System.out.println((dur-start)/loopCount);
  		temp += 1;
  		return temp;
	}
	
	/*
	 * Basic printing writing method 
	 */
	public static void myPrints(long loopCount) {
		for (long j = 0; j<loopCount ; j++) {
	    	System.out.println(j + ",");
	    }
	}	

	
	public static String globalString = ""; 
	
	
	public static void myIOfuncs(long loopCount, int myId, int type) {
	   if (type == 1) {			
    		
        	for (int i = 0; i < 150000; i++) {
	        	System.out.println("Thread " + myId);	            			
	        }
	
	   } else if (type == 2) {
//		   long start = CpuTest.getTime("1");
		   char[] buffer = new char[buffer_size];
//		   long end = CpuTest.getTime("1");
//		   System.out.println(myId+" buffer " + (end-start));
    		try {
long start, end;
start = CpuTest.getTime("1");
    			//FileReader fr = new FileReader("/home/nick/apache-tomcat-6.0.18/apache-tomcat-6.0.18/webapps/InternalSocketServlet/warandpeace"+myId+".txt");
    			FileReader fr = new FileReader("/data/samplefiles/warandpeace"+myId+".txt");
end = CpuTest.getTime("1");
System.out.println(myId+" file open from " + start + " to " + end + ": " + (end-start));    			

			///if (tempBool) {
/*
				while(fr.read(buffer) > 0) {
					System.out.println(buffer);	
				}
*/

			//	tempBool = false;
			//}

//start = CpuTest.getTime("1");
   			fr.close();	
//end = CpuTest.getTime("1");
//System.out.println(myId+" close " + (end-start));    			  			

    		} catch (Exception ex){
    			System.out.println(ex.getStackTrace().toString());
    		}
 
	   } else if (type == 4) {
		 //111111111111111111111111111111111

 		   // Experiment
		   char[] buffer = new char[buffer_size];
		   String oldString, newString;
    	   try {
    			//FileReader fr = new FileReader("/home/nick/apache-tomcat-6.0.18/apache-tomcat-6.0.18/webapps/InternalSocketServlet/warandpeace"+myId+".txt");
    			FileReader fr = new FileReader(myFiles.elementAt(myId));    			
    			//while (fr.read(buffer) != -1);
    			fr.read(buffer);
				fr.close();	
//				fr = new FileReader(myFiles.elementAt(3*myId+1));    			
//    			while (fr.read(buffer) != -1);
//				fr.close();	
//				fr = new FileReader(myFiles.elementAt(3*myId+2));    			
//    			while (fr.read(buffer) != -1);
//				fr.close();					
    		} catch (Exception ex){
    			System.out.println(ex.getMessage());
    			System.out.println(ex.getStackTrace().toString());
    		}   		
		
	   } else {
			
		    char[] buf = new char[1024];
		    String    serverIPname = "localhost";     // server IP name
		    int       serverPort   = 3456;    
		   
		    //String message = new String("A");
		    int message_size = buffer_size;
		    //for (int i = 0; i<message_size; i++) {
		    //message += "A";
		    //}
			
	    	try {
	    		
	    		java.net.Socket sock = new java.net.Socket(serverIPname,serverPort);       			// create socket and connect
	    		java.io.PrintWriter pw   = new java.io.PrintWriter(sock.getOutputStream(), true);  // create reader and writer
	    		java.io.InputStreamReader isr = new java.io.InputStreamReader(sock.getInputStream());
	    		
			pw.println(message_size);        // send msg to the server telling him how long a string it should send
    			while(isr.read(buf) > 0);
	    		//isr.read(buf);
	    		
		        pw.close();                 // close everything
        		isr.close();
		        sock.close();
		        
				//EventNotifier.writePerformanceMeasures("/data/client_results.csv");	
			} catch (Throwable e) {
		        System.out.println("Client error " + e.getMessage());
				e.printStackTrace();
	    	}				
	   }
		
			
	}	
	
	/*
	 * Basic GC-bound method
	 */
	 
	public static void myGc(String items) {
		int item_count = Integer.parseInt(items);
		
		
		for (int i = 0; i<item_count; i++) {
			CpuTest[] test = new CpuTest[1024];
			for (int j=0;j<1024;j++) {
				test[j] = new CpuTest();
				test[j].memConsumer = 3;
			}
			
		}
	}
	
	/*
	 * Basic I/O operations method
	 */
	 
	public static void myIO(String filename) {
		long start, end;
		char[] cbuf = new char[4096]; 
		try {
			//start = System.nanoTime();
			FileReader fr;
			for(int k = 0; k<ITERATIONS; k++) {
				fr = new FileReader(filename);
				fr.read(cbuf);
				fr.close();
			}
			/*
			for(int k = 0; k<cbuf.length; k++) {
				System.out.print(cbuf[k]);		
			}
			*/
			
			//end = System.nanoTime();
			//System.out.println(end - start);
	
		} catch (Exception ex){
			System.out.println(ex.getMessage());
		}	
	}
	/*
	 * Waiting function - make main thread wait (actively or not) before the join of children threads 
	 */
	public static void mainWait(boolean active) {	//false for sleeping, true for active waiting
	// Used for active waiting of the main thread before the joins
	
		if (active) {
			double temp = 0.0;
	
			for(long ii = 0; ii<100000000; ii++) { 
					temp += (ii * (8+ii)) / (ii+1);      			      			
			}
			temp += 1;
		} else {   
		
	
			System.out.println("*****Before********");
			try {   		
				Thread.currentThread().sleep(10000);
			} catch (InterruptedException e) {
		        e.printStackTrace();
		    }
			System.out.println("*****After********");	
		}
		
	}
	
	/*
	 * Function spawning a number of threads defined by application arguments
	 * and performing io operations
	 */	
	public static void myParIO(String[] argv, boolean join) {
		int threads = Integer.parseInt(argv[3]);
		Thread[] thd = new Thread[threads];		
		mode = argv[0];
		//io experiment type for pario(j) system.out, file.read, client
		
		exp_type = argv[2];
		buffer_size = Integer.parseInt(argv[4]);
		shouldJoin = join;

		tempBool = false;
		
	    for(int i =0; i<threads;i++) {     

		if (i == threads/2) {
			tempBool = true;
		}


	        thd[i] = new Thread(new Runnable() {
	        	
	            public void run() {
            		long start = 0, end = 0;
            		//Random r = new Random();
            		int myId = Integer.parseInt(Thread.currentThread().getName().substring(8));
            		
            		if (!shouldJoin) {
            			start = CpuTest.getTime(mode);
            		}
            		
	            	if (exp_type.equals("system.out")) {
	            		CpuTest.myIOfuncs(buffer_size, myId, 1);
	            	} else if (exp_type.equals("file.read")) {
	            		CpuTest.myIOfuncs(buffer_size, myId, 2);
	            		            		
	            	/*
	            	 * Server - client example: The server should be started (see main)
	            	 * Each client sends a String of variable length to the server
	            	 * which responds sending back the same String 
	            	 * one write and one read through sockets is profiled   	
	            	 */
	            	} else if (exp_type.equals("client")) {	            							    
	            		CpuTest.myIOfuncs(buffer_size, myId, 3);	
	            	} else if (exp_type.equals("exp")) {	            							    
	            		CpuTest.myIOfuncs(buffer_size, myId, 4);	
	            	}
	            	
            		if (!shouldJoin) {
            			end = CpuTest.getTime(mode);
            			double diff = ((end - start)/1000000000.0);
            			System.out.println("*" + myId +"*\t"+ diff);
            			//System.out.println("*" + myId +"*\t"+(end - start)+"\t\t"+(start)+" - "+(end));
            		}		            	
	            	
					}}, "ioThread"+i);	
//2222222222222222222222222
		        thd[i].start();
/*
		    try {
		 	thd[i].join();	
		    } catch (Exception ex) {
		    	System.out.println(ex.getMessage());
		    }
*/

	    }
	    

    	//CpuTest.mainWait(0);	//0 for sleeping, 1 for active
	    if (join) {
		    try {
			    for(int i =0; i<threads;i++) {     
			    	thd[i].join();	
			    }
		    } catch (Exception ex) {
		    	ex.printStackTrace(System.out);
		    	System.out.println(ex.getMessage());
		    	
		    }
	    }
	    // else the threads print the times themselves - shouldJoin = false
	    
	}	
	
	/*
	 * Function spawning a number of threads defined by application arguments
	 * and performing loop calculations
	 */
	static boolean toJoin;
	static int myLimit;
	public static void myParLoops(String[] argv, boolean join) {
		int threads = Integer.parseInt(argv[3]);
		Thread[] thd = new Thread[threads];		
		mode = argv[0];
		myLimit=Integer.parseInt(argv[2]);
		toJoin = join;	
		CpuTestParallelThread pt = new CpuTestParallelThread(mode, toJoin, myLimit);
	    for(int i =0; i<threads;i++) {     

	    	//System.out.println(" Starting thread loopThread" + i);
	        thd[i] = new Thread(pt, "loofasdfaspThread" + i);
	        thd[i].start();
	        
	    }
	    
	   
    	//CpuTest.mainWait(0);	//0 for sleeping, 1 for active
	    if (join) {
	    	
		    try {    	
			    for(int i =0; i<threads;i++) {
			    	thd[i].join();
			    }
		    } catch (Exception ex) {
		    	ex.printStackTrace(System.out);
		    	System.out.println("Exception during join " + ex.getMessage());	
		    }
	    }
	    
	}
	

    // The request that is made to the normal Tomcat server
    private static void makeRequest(String host, int port, String path, String threadId) {
		  Socket socket = null;
		  PrintWriter socket_out = null;
		  Reader socket_in = null;
		 
		  char[] buf = new char[1024];
		  
		  int charNo = -1;
		 
		  int count =0;
		  try {
		      socket = new Socket(host, port);          
		  } catch (UnknownHostException e) {
		      System.err.println("UnknownHostException: " + e);
		  } catch (IOException e) {
		      System.err.println("IOException (socket): " + e);
		  }
		  
		 // System.out.println(threadId + " after establishing socket");
		 
		  try {
		      socket_out = new PrintWriter(socket.getOutputStream(), true);
		  } catch (IOException e) {
			  System.out.println("IOException (print-writer): " + e);
		
		  }
		
		  try {
		      socket_in = new InputStreamReader(socket.getInputStream());
		  } catch (IOException e) {
		      System.err.println("IOException (reader): " + e);
		
		  }
		
		  String toSend = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
		  
		  //System.out.println(threadId + " before writing to socket");
		  try {
		  	socket_out.write(toSend);
		  	socket_out.flush();
		  } catch (Exception ex) {
		      System.out.println("Exception on write: " + ex.toString());
		  }
		  
		  //System.out.println(threadId + " before wait for read");
		  try {
		      charNo = socket_in.read(buf);
		  } catch (IOException e) {
		      System.err.println("IOException (on read()): " + e);
		  }
		
		  count++;        
		  while (charNo != -1) {
		      try {
		    	  charNo = socket_in.read(buf);
		      } catch (IOException e) {
		          System.err.println("IOException (on read2()): " + e);
		      }
		      count++;
		
		  }
		  
		  try {
		      socket.close();
		  } catch (IOException e) {
		      System.err.println("IOException (on close): " + e);
		  }

   }    	
	
	public static long getTime(String argv) {

		if( argv.equals("1")) {
			return System.nanoTime();
		} else if( argv.equals("2")) {
			return EventNotifiervtonly.getThreadVirtualTime();
		} else if( argv.equals("3")) {
			return EventNotifier.getThreadVirtualTime();
		} else if( argv.equals("4")) {
			return EventNotifiervtonly.getPapiRealTime();
		} else if( argv.equals("5")) {
			return EventNotifiervtonly.getJvmtiVirtualTime();
		} else if( argv.equals("6")) {
			return EventNotifiervtonly.getJvmtiRealTime();				
		} else { 
			return EventNotifiervtonly.getThreadVirtualTime();
		}
	}
	
	static String mode;
	static String exp_type;
	static int buffer_size;
	static boolean shouldJoin;
	static boolean tempBool;
	static Vector<String> myFiles;
	/**
	 * Main testing function
	 * argv[0]: 1 for Real Time measurement, 2 for Virtual Time from VTF_light, 3 for Virtual Time from VTF
	 * argv[1]: experiment type {loops, io, parloops, pario, parloopsj (with join), parioj (with join)}
	 * argv[2]: io experiment type for pario(j) system.out, file.read, client
	 * argv[3]: #threads 
	 * argv[4]: buffer size for IO experiments
	 * @param argv
	 */
	public static void main(String[] argv) {
		/*
		long start = System.nanoTime();
		CpuTest.myLoops(Integer.parseInt(argv[0]));
		long stop = System.nanoTime();
		System.out.println("System.nanoTime() measurement for myLoops: " + (stop-start));
		*/
		if (argv.length == 0) {
			System.out.println("Syntax: vtf_tests.CpuTest <1:RT|2:VT|3:VT> <loops|io|parloops|pario|parloopsj> <parloopsj-limit> <parloopsj-threads>");
			return;
		}
		
		long start = 0, end;
		long limit;
		try {
			limit = Long.parseLong(argv[2]);
		} catch (NumberFormatException nfe) {
			limit = 0;
		}
		
		for (int i = 0; i<1; i++) {
			
			if (argv[1].equals("loops")) {
				//start = CpuTest.getTime(argv[0]);
				CpuTest.myLoops(limit);
			    //end = CpuTest.getTime(argv[0]);
		       // System.out.println(end - start);
			} else if (argv[1].equals("prints")) {
				if(argv[0].equals("1")) {
					start = CpuTest.getTime(argv[0]);
				}
				CpuTest.myPrints(limit);
				if(argv[0].equals("1")) {
				        end = CpuTest.getTime(argv[0]);
				        System.out.println((end - start));
				}
			} else if (argv[1].equals("io")) {
//333333333333333333333

				myFiles = new Vector<String>();
				try {
				BufferedReader input = new BufferedReader(new FileReader("/data/myrandomfiles"));
				String line = null;
				while((line = input.readLine()) != null) {
					myFiles.add(line);					
				}
				} catch (IOException ie){
					System.out.println(ie.getMessage());
				}
                 
				
				Random r = new Random();
                int myId;
			    int filesToRead = Integer.parseInt(argv[3]);

			    for (int kk =0 ; kk < filesToRead; kk++) {
			    	start = CpuTest.getTime(argv[0]);
				    //myId = r.nextInt(myFiles.size());
			    	myId = kk;
				    CpuTest.myIOfuncs(limit, myId, 4);
				    end = CpuTest.getTime(argv[0]);
				    System.out.println("*" + myId +"*\t"+(end - start));
			    }
			} else if (argv[1].equals("parloops")) {
				//internal measurements and only one iteration
				CpuTest.myParLoops(argv, false);
				
				i = 11;

			} else if (argv[1].equals("pario")) {


				myFiles = new Vector<String>();
				try {
				BufferedReader input = new BufferedReader(new FileReader("/data/myrandomfiles"));
				String line = null;
				while((line = input.readLine()) != null) {
					myFiles.add(line);					
				}
				} catch (IOException ie){
					System.out.println(ie.getMessage());
				}
                 				
				//internal measurements and only one iteration
				CpuTest.myParIO(argv, false);
			
				
				
//				int printsPerIt;
//				try {
//					printsPerIt = Integer.parseInt(argv[3]);
//				} catch (NumberFormatException nfe) {
//					printsPerIt= 0;
//				}
//				
//				try {
//					limit = Long.parseLong(argv[4]);
//				} catch (NumberFormatException nfe) {
//					limit = 0;
//				}
//								
//				System.out.println(limit +"  * " + printsPerIt);
//				buffer_size = 1024;
//				if(argv[0].equals("1")) {
//					start = CpuTest.getTime(argv[0]);
//				}
//				
//				
//				
//				//for(long li = 0 ; li<limit ; li++) {
//					for(int j =0 ; j <printsPerIt; j++) {
//						CpuTest.myIOfuncs(1024, 0 , 1);
//					}
//				//}
//				
//				if(argv[0].equals("1")) {
//					end = CpuTest.getTime(argv[0]);
//					System.out.println((end - start));
//				}				
//				
//				
				i = 11;
			} else if (argv[1].equals("parloopsj")) {
				
				if (argv[0].equals("1")) {
					start = CpuTest.getTime(argv[0]);
				}
				CpuTest.myParLoops(argv, true);

				if (argv[0].equals("1")) {
				    end = CpuTest.getTime(argv[0]);
				    //System.out.println(end - start);
				    System.out.println(((end - start)/1000000000.0));
				}
			} else if (argv[1].equals("parioj")) {

				start = CpuTest.getTime(argv[0]);
				CpuTest.myParIO(argv, true);
				end = CpuTest.getTime(argv[0]);
		        System.out.println((end - start)+ ",");
		        
			} else if (argv[1].equals("gc")) {
				start = CpuTest.getTime(argv[0]);
				CpuTest.myGc(argv[2]);
				end = CpuTest.getTime(argv[0]);
		        System.out.println((end - start));

			} else if (argv[1].equals("server")) {
				//System.out.println("Server started...waiting for clients");
			    int serverPort   = 3456;                 // server port number					
				int threads = Integer.parseInt(argv[3]); 
				try {
					java.net.ServerSocket sock = new java.net.ServerSocket(serverPort);   // create socket and bind to port
					java.net.Socket clientSocket;
					
					while (threads-- > 0) {
						clientSocket = sock.accept();                   // wait for client to connect
						
						//System.out.println("client has connected");	         
						java.io.PrintWriter pw   	= new java.io.PrintWriter(clientSocket.getOutputStream(),true);
						java.io.BufferedReader br   = new java.io.BufferedReader(new java.io.InputStreamReader(clientSocket.getInputStream()));
						String msg = br.readLine();                  	// read msg from client
						int message_size = Integer.parseInt(msg);
		    				String message = new String("A");
						for (i = 0; i<message_size; i++) {
							message += "A";
						}
			
						//System.out.println("Message from the client >" + msg);
						pw.println(message);        			// send msg to client
												      
						br.close();
						//pw.close();                                     // close everything
						
						clientSocket.close();
					}			      	
					
					sock.close();
				
				} catch (Throwable e) {
					System.out.println("Server error " + e.getMessage());
				    e.printStackTrace();
				}
				
			} else if (argv[1].equals("iotrace")) {
				// Used to get the traces of the IO methods - the deepest method should be the one actually performing the I/O
				if (argv[2].equals("System.out"))  {
					if(argv[0].equals("1")) {
						start = CpuTest.getTime(argv[0]);
					}
					
					for(i = 0 ; i< 1; i++) {
						System.out.println("Hello world");
					}
					
					if(argv[0].equals("1")) {
						end = CpuTest.getTime(argv[0]);
						System.out.println((end - start));
					}					
				} else if (argv[2].equals("file.read"))  {

					if(argv[0].equals("1")) {
						start = CpuTest.getTime(argv[0]);
					}

					//CpuTest.myReader();
					CpuTest.myIOfuncs(64, 124, 2);
					
					if(argv[0].equals("1")) {
						end = CpuTest.getTime(argv[0]);
						System.out.println((end - start));
					}
					
				} else if (argv[2].equals("file.write"))  {
					if(argv[0].equals("1")) {
						start = CpuTest.getTime(argv[0]);
					}

					CpuTest.myReader();
					
					if(argv[0].equals("1")) {
						end = CpuTest.getTime(argv[0]);
						System.out.println((end - start));
					}

					
				} else if (argv[2].equals("socket.read")) {
					
				    int serverPort   = 3456;                 // server port number					
					 
					try {
						java.net.ServerSocket sock = new java.net.ServerSocket(serverPort);   // create socket and bind to port
						java.net.Socket clientSocket = sock.accept();                   // wait for client to connect
							
						//System.out.println("client has connected");	         
						//java.io.PrintWriter pw   	= new java.io.PrintWriter(clientSocket.getOutputStream(),true);
						java.io.BufferedReader br   = new java.io.BufferedReader(new java.io.InputStreamReader(clientSocket.getInputStream()));
						String msg = br.readLine();                  	// read msg from client
						br.readLine();                  	// read msg from client
						//System.out.println("Message from the client >" + msg);
						//pw.println(msg);            					// send msg to client
												      
						br.close();
						//pw.close();                                     // close everything
						
						clientSocket.close();
					
						sock.close();
						EventNotifier.writePerformanceMeasures("/data/server_results.csv");
					} catch (Throwable e) {
					
						System.out.println("Server error " + e.getMessage());
					    e.printStackTrace();
					}
					
				} else if (argv[2].equals("socket.write")) {
					java.io.BufferedReader  br   = null;                              // socket input from server
				    
				    char[] buf = new char[1024];
				    String    serverIPname = "localhost";     // server IP name
				    int       serverPort   = 3456;    
				   
				    String message = new String("A");
				    int message_size = 128;
				    for (i = 1; i<message_size; i++) {
				    	message += "A";
				    }
					
			    	try {
			    		
			    		java.net.Socket sock = new java.net.Socket(serverIPname,serverPort);       			// create socket and connect
				          
			    		java.io.PrintWriter pw   = new java.io.PrintWriter(sock.getOutputStream(), true);  // create reader and writer
			    		//java.io.InputStreamReader isr = new java.io.InputStreamReader(sock.getInputStream());
				        //System.out.println("Connected to Server");
			
				        pw.println(message);        // send msg to the server		
			            pw.close();                 // close everything
					          //br.close();
				        //isr.close();
				        sock.close();
				
						EventNotifier.writePerformanceMeasures("/data/client_results.csv");	
					} catch (Throwable e) {
				        System.out.println("Client error " + e.getMessage());
						e.printStackTrace();
			    	}					
					
				} else if (argv[2].equals("apache.tomcat")) {
					if(argv[0].equals("1")) {
						start = CpuTest.getTime(argv[0]);
					}

					makeRequest("localhost", 8080, "/InternalSocketServlet/warandpeace.txt", "requestingThread");
					if(argv[0].equals("1")) {
						end = CpuTest.getTime(argv[0]);
						System.out.println((end - start));
						
					}

				}
			}
			

		}

		//System.out.println(" ");
		
	}
	
}

class CpuTestParallelThread implements Runnable {
	private String mode;
	private boolean toJoin;
	private long myLimit;
	
	CpuTestParallelThread(String _mode, boolean _toJoin, long _myLimit) {
		mode = _mode;
		toJoin = _toJoin;
		myLimit = _myLimit;

	}

	/*
	public double myLoops(long loopCount) {
		double temp = 0.0;
		double i;

		for(i = 0; i<loopCount; i++) {
			temp += (i * (8.0+i)) / (i+1.0);
		}
		temp += 1;
		return temp;
	}


	public double myInnerLoops(long loopCount) {
		double temp = 0.0;
		double i;
		
		long iterations = 1000;
		long innerLoopCount = loopCount / iterations;	// done like this to avoid long native waiting

		for(i = 0; i<iterations; i++) {
			temp += myLoops(innerLoopCount);
		}
		temp += 1;
		return temp;
	}
*/
	
	public double myInnerLoops(long loopCount) {
		double temp = 0.0;
		double i;

		for(i = 0; i<loopCount; i++) {
			temp += (i * (8.0+i)) / (i+1.0);
		}
		temp += 1;
		return temp;
	}
	
	public double myLoops(long loopCount) {
		double temp = 0.0;
		double i;
		
		long iterations = 1000;
		long innerLoopCount = loopCount / iterations;	// done like this to avoid long native waiting
		for(i = 0; i<iterations; i++) {
			temp += myInnerLoops(innerLoopCount);
		}
		temp += 1;
		return temp;
	}


	public long getTime(String argv) {

		if( argv.equals("1")) {
			return System.nanoTime();
		} else if( argv.equals("2")) {
			return EventNotifiervtonly.getThreadVirtualTime();
		} else if( argv.equals("3")) {
			return EventNotifier.getThreadVirtualTime();
		} else if( argv.equals("4")) {
			return EventNotifiervtonly.getPapiRealTime();
		} else if( argv.equals("5")) {
			return EventNotifiervtonly.getJvmtiVirtualTime();
		} else if( argv.equals("6")) {
			return EventNotifiervtonly.getJvmtiRealTime();				
		} else { 
			return EventNotifiervtonly.getThreadVirtualTime();
		}
	}
	


//	public double doitIns(double i) {
//		i = (i * (8.0+i)) / (i+1.0);
//		return i;
//	}
//	
//	public double myLoopsIns(long loopCount) {
//    	double temp = 0.0;
//    	double i;
//  		for(i = 0; i<loopCount; i++) {
//			temp += doitIns(i);
//  		}
//  		temp += 1;
//  		return temp;
//	}	

	
	public void run() {
		if (toJoin) {
			double temp = myLoops(myLimit); //myLoopsIns(myLimit); //
			temp += 1;
		} else {

			long start,end, start_vt=0, end_vt=0;
			if (mode.equals("3")) {
				start_vt = getTime("1");
			}
			start = getTime(mode);
			double result = myLoops(myLimit);
			result += myLoops(myLimit);
			end =getTime(mode);
			
			System.out.println(result + " " + ((end - start)/1000000000.0));// + " " + start + " " + end);
			if (mode.equals("3")) {
				end_vt = getTime("1");
			} 
			result += 1;
		}

	}
}

