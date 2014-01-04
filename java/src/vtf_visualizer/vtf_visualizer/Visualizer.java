package vtf_visualizer;

import java.applet.Applet;

import java.awt.*;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.*;
import java.net.*;
import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.Vector;

//No need to extend JApplet, since we don't add any components;
//we just paint.
public class Visualizer extends Applet {
	
    VisualizerCanvas graph;
    
    //HashMap<Long,String> threadNames;	// we want to maintain sequence
    Vector<Long> threadIds;
    Vector<String> threadNames;
    Vector<VtfEventRecord> events;
    
    boolean showTimeGrid;
    boolean urlFileMode;
    
    int showProgress;
    int last_event_shown;
    int progressInteval;
    
    public void init() {
       
        setBackground(Color.gray);
        setLayout(new BorderLayout(50,50));
       
        String eventsFilename = getParameter("events_file");

        urlFileMode = false;
        String urlFileModeParam = getParameter("url_file");
        if (urlFileModeParam.equals("true")) {
        	urlFileMode = true;
        } else {
        	urlFileMode = false;
        }
        
        readEvents(eventsFilename);
        graph = new VisualizerCanvas(threadIds, threadNames, events);
        
        
        showTimeGrid = false;
        String timeGrid = getParameter("time_grid");
        if (timeGrid.equals("true")) {
        	showTimeGrid = true;
        } 
        
        showProgress = 0;
        progressInteval = 0;
        String progress = getParameter("show_progress");
        if (progress.equals("true")) {
         	 showProgress = 1;
         	 progress = getParameter("progress_interval");
         	 progressInteval = Integer.parseInt(progress);
        } else if (progress.equals("onclick")) {
        	showProgress = 2;
        	graph.setMouseListener();
        }

        boolean showAbbrev = false;
        String abbrev = getParameter("show_abbrev");
        if (abbrev.equals("true")) {
         	 showAbbrev = true;
        } else {
        	 showAbbrev = false;
        }
        
        int timelineDiff = 0;
        abbrev = getParameter("distance_from_right_end");
        if (!abbrev.equals("false")) {
        	 timelineDiff = Integer.parseInt(abbrev);
        }        
        graph.setOptions(showTimeGrid, showProgress, showAbbrev, progressInteval, timelineDiff);
        add(graph, BorderLayout.CENTER);        
    }
    
    public void start() {
        //addItem("starting... ");
        
    }
    public void stop() {
        //addItem("stopping... ");
    }

    public void destroy() {
        //addItem("preparing for unloading...");
    }

    public void readEvents(String filename) {
    	threadIds = new Vector<Long>();
    	threadNames = new Vector<String>();
    	events = new Vector<VtfEventRecord>();

    	BufferedReader bufRdr = null;
		try {
			if (urlFileMode) {
				URL urlFile = new URL(filename);
				bufRdr = new BufferedReader(new InputStreamReader(urlFile.openStream()));
			} else {
//				URL urlFile = new URL(filename);
//				bufRdr = new BufferedReader(new InputStreamReader(urlFile.openStream()));				
				File file = new File(filename);
				bufRdr = new BufferedReader(new FileReader(file));
			}
		} catch (IOException e1) {
			System.out.println(filename + " " + urlFileMode);
			e1.printStackTrace();
		}
    	
    	String line = null;
    	
    	int threads = 0;	
    	int events_count = 0;
    	Long thread_id;
    	Long timestamp;
    	int thread_action;
    	int thread_state;
    	
    	String thread_name;
    	//read each line of text file
    	try {
			
    		// Reading threads ids - thread names
    		while((line = bufRdr.readLine()) != null) {
				if (line.equals("")) {
					//addItem("Finished finding threads");
					break;
				}
				
				
				StringTokenizer st = new StringTokenizer(line,",");
				thread_id = Long.parseLong(st.nextToken());
				thread_name = st.nextToken();
				threadIds.add(thread_id);
				if (thread_name.length() > 22) {
					thread_name = thread_name.substring(0, 13) + "..." + thread_name.substring(thread_name.length()-6); 
				}
				
				threadNames.add(thread_name);
				threads++;
			}
    		//addItem("... Exiting with " + threads + " threads read");	
    		
    		// Read events
    		while((line = bufRdr.readLine()) != null) {				
				StringTokenizer st = new StringTokenizer(line,",");
				thread_id = Long.parseLong(st.nextToken());
				thread_action = Integer.parseInt(st.nextToken());
				timestamp = Long.parseLong(st.nextToken());
				thread_state = Integer.parseInt(st.nextToken());
				
				if (thread_action == VtfEventRecord.METHOD_ENTER || thread_action == VtfEventRecord.METHOD_EXIT) {
					//events.add(new VtfEventRecord(thread_id, thread_action, thread_state, timestamp));
					events.add(new VtfEventRecord(thread_id, thread_action, thread_state, timestamp, st.nextToken()));
				} else if (thread_action == VtfEventRecord.METHOD_IO_ENTER || thread_action == VtfEventRecord.METHOD_IO_EXIT) {
					events.add(new VtfEventRecord(thread_id, thread_action, thread_state, timestamp));
//					events.add(new VtfEventRecord(thread_id, thread_action, thread_state, timestamp, st.nextToken()));
				} else {
					events.add(new VtfEventRecord(thread_id, thread_action, thread_state, timestamp));
				}
				
				events_count++;
			}			
    		//addItem("... and " + events_count + " events read");
			bufRdr.close();
		} catch (IOException e) {
			System.out.println(filename + " " + urlFileMode);
			e.printStackTrace();
		}
		
    	//close the file	
    }
    
}


class VtfEventRecord {
	// Event actions
	static final int THREAD_START = 1;
	static final int THREAD_END = 2;
	static final int METHOD_ENTER = 3;
	static final int METHOD_EXIT = 4;
	static final int SUSPEND = 5;
	static final int RESUME = 6;
	static final int WAIT = 7;
	static final int WAITING_RELEASED = 8;
	static final int WAITING_TIMEOUT = 9;
	static final int METHOD_IO_ENTER = 10;
	static final int METHOD_IO_EXIT = 11;
	static final int SUSPEND_SELF = 12;
	static final int GC_STARTED = 13;
	static final int GC_FINISHED = 14;
	static final int IO_PREDICTION = 15;
	static final int INTERNAL_SOCKET_READ = 16;
	static final int INTERNAL_SOCKET_READ_NULL_WAIT = 17;
	static final int INTERNAL_SOCKET_READ_RESUME  	= 18;
	static final int INTERNAL_SOCKET_WRITE 			= 19;
	static final int INTERNAL_SOCKET_WRITE_FULL_WAIT= 20;
	static final int INTERNAL_SOCKET_WRITE_RESUME  	= 21;
	static final int INTERNAL_SOCKET_WRITE_RESUME_READ = 22;
	static final int SET_NATIVE_WAITING = 23;
	
	// Thread states
	static final int RUNNING = 1;
	static final int WAITING = 2;
	static final int SUSPENDED = 3;
	static final int LEARNING_IO = 4;
	static final int IN_IO = 5;
	static final int ZOMBIE = 6;
	static final int IN_NATIVE = 7;
	static final int WAITING_INTERNAL_SOCKET_READ = 8;
	
	long thread_id;
	int action;
	int thread_state;
	long timestamp;
	String methodName;
	VtfEventRecord(long tId, int act, int tState, long timeStamp) {
		thread_id = tId;
		thread_state = tState;
		action = act;
		timestamp = timeStamp;
		methodName = null;
	}

	VtfEventRecord(long tId, int act, int tState, long timeStamp, String method) {
		thread_id = tId;
		thread_state = tState;
		action = act;
		timestamp = timeStamp;
		methodName = method;
	}
}

class VisualizerCanvas extends Canvas implements MouseListener {

	// Canvas information 
	int width,height;
	long maxTime;
	int beginningPoint;
	int timeLineOffset;
	
	// Vtf visualization related information
	int time_categories;
	int[] timerCategories;
	int timerCategoriesCounter;
	
	Vector<Long> threadIds;
	Vector<String> threadNames;
	
	HashMap<Long,VtfThreadInfo> threadsToInfo;
	HashMap<Point, Boolean> pointToEvent;	//event point - is method?
	Vector<VtfEventRecord> events;
	
	Point currentPoint;
	int threads;
	
	// Options
	boolean showTimeGrid;// show time grid
	int showProgress;	// show progress: 1: in presentation, 2: onclick
	boolean showAbbrev;	// show abbreviations of events
	int progressInteval;// interval of milliseconds between consecutive events
	int timelineDiff;
	
	boolean zoomMode;
	int zoomFromX;
	int zoomToX;
	long zoomFromTime;
	
	// Counter of events
	int eventsToShow;
	long showUntilTime;	
	boolean focusOnEvent; // is set true when a user clicks on a particular event
	int eventsToFocus;
	
	VisualizerCanvas(Vector<Long> threadIdsRead, Vector<String> threadNamesRead, Vector<VtfEventRecord> eventsRead) {
	   // Constructor.
	   setBackground(Color.white);	   
	   events = eventsRead;
	   threadIds = threadIdsRead;
	   threadNames = threadNamesRead;
	   threads = threadNames.size();
	   threadsToInfo= new HashMap<Long, VtfThreadInfo>();
	   maxTime =0;
	   timerCategories = new int[80];
	   timerCategoriesCounter = 0;
	   eventsToShow = -1;	//all unless changed
	   currentPoint = null;
	   pointToEvent = new HashMap<Point, Boolean>();
	   focusOnEvent = false;
	   progressInteval = 0;
	   showAbbrev = false;
	   showUntilTime = Long.MAX_VALUE;
	   eventsToFocus = 0; 
	   
	   timelineDiff = 10;
	   
		zoomFromX = 0;
		zoomToX = 0;
		zoomMode = false;
		zoomFromTime =0 ;
		
	}
	

	public void setOptions(boolean _showTimeGrid, int _showProgress, boolean _showAbbrev, int _progressInteval, int _timelineDiff) {
		showTimeGrid = _showTimeGrid;
		showProgress = _showProgress;
		showAbbrev = _showAbbrev;
		progressInteval = _progressInteval;
		timelineDiff = _timelineDiff;
	}
	
	public void setMouseListener() {
		addMouseListener(this);
		eventsToShow = 0;
	}
	private int getNextTimeCategoryYoffset() {
		return timerCategories[(timerCategoriesCounter++) % time_categories];
	}
	private int getTimeCategoryYoffset(int index) {
		return timerCategories[index];
	}
	
	// Apply different ordering according to total number of events
	private int getTimeCategoryYoffset(int index, int totalEvents) {
		if (totalEvents == 1) {
			return timerCategories[0];
		} else {
			switch(index) {
				case 0: return timerCategories[2];
				case 1: return timerCategories[0];
				case 2: return timerCategories[1];
				default: return timerCategories[index];
			}
		}
	}
	

	public void clearFunction() {
	      // Set the canvas to draw no graph at all.
	   repaint();
	}

	private int getXofTimestamp(long timestamp) {
		
		int startPoint, endPoint;
		if (zoomMode) {
			startPoint = beginningPoint;
			endPoint = width;
		} else {
			startPoint = beginningPoint;
			endPoint = width-timelineDiff;
		}
		if (zoomMode && zoomFromTime != 0) {
			if (timestamp <= zoomFromTime) {
				return beginningPoint;
			} else if (timestamp >= maxTime) {
				return width;
			} else {
				return (int)((timestamp-zoomFromTime) * (endPoint - startPoint) / (maxTime-zoomFromTime)) + startPoint;
			}
		} else {
			return (int)((timestamp) * (endPoint - startPoint) / (maxTime)) + startPoint;
		}		
		//return (int)(timestamp * (endPoint - startPoint) / maxTime) + startPoint; 
	}
	private long getTimestampFromX(int x) {
		int startPoint, endPoint;
		if (zoomMode) {
			startPoint = beginningPoint;
			endPoint = width;
		} else {
			startPoint = beginningPoint;
			endPoint = width-timelineDiff;
		}
		//return (long)(maxTime * (x - startPoint) / (endPoint - startPoint) );
		if (zoomMode && zoomFromTime != 0) {
			return (long)((maxTime-zoomFromTime) * (x - startPoint) / (endPoint - startPoint) ) + zoomFromTime;
		} else {
			return (long)((maxTime) * (x - startPoint) / (endPoint - startPoint) );
		}		
	}	
	/*
	private int getXofTimestamp(long timestamp) {
		
		int startPoint, endPoint;
		//if (zoomMode) {
			//startPoint = zoomFromX;
			//endPoint = zoomToX;
		//} else {
			startPoint = beginningPoint;
			endPoint = width-timelineDiff;
		//}
	
		if (zoomFromTime != 0) {
			return (int)((timestamp-zoomFromTime) * (endPoint - startPoint) / (maxTime-zoomFromTime)) + startPoint;
		} else {
			return (int)((timestamp) * (endPoint - startPoint) / (maxTime)) + startPoint;
		}
	}
	private long getTimestampFromX(int x) {
		int startPoint, endPoint;
	
			startPoint = beginningPoint;
			endPoint = width-timelineDiff;
	
		if (zoomFromTime != 0) {
			return (long)((maxTime-zoomFromTime) * (x - startPoint) / (endPoint - startPoint) );
		} else {
			return (long)((maxTime) * (x - startPoint) / (endPoint - startPoint) );
		}
	}	
	 */
	private int getCharactersOffset(String s) {
		return 60 * s.length() / 8;
	}
	
	public void paint(Graphics g) {
		int aliveThreads = 0;
		int allThreads = 0;
		int maximumThreads = 0;
		
		int ioThreads = 0;
		int maximumIoThreads = 0;
		int avgIoThreadsSum=0, avgIoThreadsCount =0;
		int ioPredictionsMade = 0;
		if (zoomFromX != zoomToX) {
			zoomMode = true;	
		}
		
		width = getSize().width;
		height = getSize().height;
			
		if (showAbbrev) {
			time_categories = 5;
		} else {
			time_categories = 80;
		}
		timerCategories[0] = 15;
//		int starting = -8;
//		for (int i = 1; i < 4; i++) {
//			timerCategories[i] = starting;
//			starting-=14;
//		}
		timerCategories[1] = -10;
		timerCategories[2] = 30;
		int starting = -24;
		for (int i = 3; i < time_categories; i++) {
			timerCategories[i] = starting;
			starting -= 14;
		}

		int offset = height-100;
		int step = (offset - 10)/threads;
		
		// Find maximum time - if not in zoom mode - otherwise use the current maxTime defined when you zoomed
		int length = events.size();
		if (!zoomMode) {
			
			maxTime = 0;
			for (int i = length-1 ; i>=0; i--) {
				if (events.elementAt(i).timestamp > maxTime) {
					maxTime = events.elementAt(i).timestamp; 
				}
			}
		}
		
		// Find maximum thread name length to show the start of the lines correctly
		int maximumThreadNameLength = 0;
		
	    Iterator<String> iterator = threadNames.iterator();
	    String threadName; 
	    	    
	    while (iterator.hasNext()) {	    	
	    	threadName = (String)iterator.next();
	    	if (threadName.length() > maximumThreadNameLength) {
	    		maximumThreadNameLength = threadName.length();
	    	}
	    }
	    
	    Long threadId;
	    // Thread lines
	    beginningPoint = 60*(maximumThreadNameLength)/8;
	    for (int i = 0; i < threads; i++) {
	    	    	
	    	g.setColor(Color.BLACK);
	    	threadId = threadIds.elementAt(i);
	    	threadName = threadNames.elementAt(i);
	    	g.drawString(threadName, 2, offset);
	   
	    	g.setColor(Color.LIGHT_GRAY);
			for (int j = 1300; j < width; j+=(width+100)) {
				g.drawString(threadName, j, offset);	
			}
			g.drawLine(beginningPoint, offset-5, width, offset-5);
			
			threadsToInfo.put(threadId, new VtfThreadInfo(threadName, offset-5));
			offset -= step;
	    }

	    // Enter system thread
	    threadId = (long)0;
	    threadsToInfo.put(threadId, new VtfThreadInfo("System", -200));
	    
	    // Time line
	    g.setColor(Color.BLUE);
	    timeLineOffset = height-50;
		g.drawLine(beginningPoint, timeLineOffset, width, timeLineOffset);
		
		if (showProgress == 1) {
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		
		//---------------------------------------------------------
		//int length = events.size();
		VtfThreadInfo info = null;
		VtfEventRecord event = null;

		timerCategoriesCounter =0;
		String letter = new String();
		String actionDescription = new String();
		
		int labelPositionY; 
		pointToEvent.clear();
		int limit = 0;
		boolean withTimeLimit = false;
		if (showUntilTime < Long.MAX_VALUE) {
			limit = length;
			withTimeLimit = true;
		} else {
			limit = (length > eventsToShow && eventsToShow != -1)?eventsToShow:length;
		}
		boolean isMethodEvent;
		long lastTime = 0;
		
		int startingEvent = 0;
		// Get event to start from
		if (zoomMode) {
			do {
				event = events.elementAt(startingEvent++);
				if (withTimeLimit) {
					eventsToShow++;
				}				
			} while (event.timestamp < zoomFromTime);
			startingEvent--;
			if (withTimeLimit) {
				eventsToShow--;
			}
		}
		for (int i = startingEvent; i <limit && lastTime < showUntilTime; i++) {
			
			if (withTimeLimit) {
				eventsToShow++;
			}
			
			g.setColor(Color.BLACK);	
			event = events.elementAt(i);
			threadId = event.thread_id;
			info = threadsToInfo.get(threadId);
					
			isMethodEvent = false;
			
			if (event.action == VtfEventRecord.THREAD_START) {
				info.state =  event.thread_state; //VtfThreadInfo.RUNNING;
				letter = "B";
				actionDescription = "was spawned";
				aliveThreads++;
				allThreads++;
				
			} else if (event.action == VtfEventRecord.THREAD_END) {
				info.state = event.thread_state; //VtfThreadInfo.ZOMBIE;
				g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				letter = "Z";
				actionDescription = "terminated execution";
				aliveThreads--;
				
			} else if (event.action == VtfEventRecord.METHOD_ENTER) {
				info.state = event.thread_state; //VtfThreadInfo.RUNNING;
				//g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				g.drawOval(getXofTimestamp(event.timestamp), info.offset -4, 8, 8);
				actionDescription = "entered method " + event.methodName;
				isMethodEvent = true;
			} else if (event.action == VtfEventRecord.METHOD_EXIT) {
				info.state = event.thread_state; //VtfThreadInfo.RUNNING;
				g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				g.drawRect(getXofTimestamp(event.timestamp), info.offset -4, 8, 8);
				actionDescription = "exited method " + event.methodName;
				isMethodEvent = true;
				
			} else if (event.action == VtfEventRecord.SUSPEND) {
				info.state = event.thread_state; //VtfThreadInfo.SUSPENDED;
				g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				letter = "S";
				actionDescription = "was suspended";
			} else if (event.action == VtfEventRecord.RESUME) {
				info.state = event.thread_state; //VtfThreadInfo.RUNNING;
				letter = "R";
				actionDescription = "was resumed";
				
			} else if (event.action == VtfEventRecord.WAIT) {
				info.state = event.thread_state; //VtfThreadInfo.WAITING;
				g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				letter = "W";
				actionDescription = "started waiting";
			} else if (event.action == VtfEventRecord.WAITING_RELEASED) {
				info.state = event.thread_state; //VtfThreadInfo.RUNNING;
				letter = "I";
				actionDescription = "was interrupted";
			} else if (event.action == VtfEventRecord.WAITING_RELEASED) {
				info.state = event.thread_state; //VtfThreadInfo.RUNNING;
				letter = "O";
				actionDescription = "has timed-out";								
			} else if (event.action == VtfEventRecord.SET_NATIVE_WAITING) {
				info.state = event.thread_state; //VtfThreadInfo.NATIVE_WAITING;
				g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				letter = "W";
				actionDescription = "started NATIVE waiting";
			} else if (event.action == VtfEventRecord.METHOD_IO_ENTER) {
				info.state = event.thread_state; //VtfThreadInfo.RUNNING;
				//g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				g.drawOval(getXofTimestamp(event.timestamp), info.offset -4, 8, 8);
				actionDescription = "entered I/O method " + event.methodName;
				isMethodEvent = true;
				info.ioStartingTime = event.timestamp;
				ioThreads++;
			} else if (event.action == VtfEventRecord.METHOD_IO_EXIT) {
				info.state = event.thread_state; //VtfThreadInfo.RUNNING;
				g.drawRect(getXofTimestamp(event.timestamp), info.offset -4, 8, 8);
				g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				
				actionDescription = "exited I/O method " + event.methodName;
				info.ioStartingTime = event.timestamp;
				isMethodEvent = true;
				letter = "R";				

				ioThreads--;
			} else if (event.action == VtfEventRecord.SUSPEND_SELF) {
				info.state = event.thread_state; //VtfThreadInfo.SUSPENDED;
				g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				letter = "S";
				actionDescription = "suspended itself";				
			} else if (event.action == VtfEventRecord.GC_STARTED) {
				actionDescription = "started GC";
			} else if (event.action == VtfEventRecord.GC_FINISHED) {
				actionDescription = "finished GC";
				
			    g.setColor(Color.RED);
				g.fillRect(getXofTimestamp(info.lastTime), 0, getXofTimestamp(event.timestamp)-getXofTimestamp(info.lastTime), height-60);
				
				offset = height-100;
			    for (int ti = 0; ti < threads; ti++) {
			    	g.setColor(Color.LIGHT_GRAY);
					g.drawLine(beginningPoint, offset-5, width, offset-5);
					offset -= step;
			    }				
			} else if (event.action == VtfEventRecord.IO_PREDICTION) {
				info.state = event.thread_state; //VtfThreadInfo.IN_IO;
				actionDescription = "predicted I/O duration at " +((Object)(info.ioStartingTime/1000000.0)).toString()+ " to time point";
				ioPredictionsMade++;
			    g.setColor(Color.GREEN);
				g.drawOval(getXofTimestamp(info.ioStartingTime), info.offset-5, getXofTimestamp(event.timestamp)-getXofTimestamp(info.ioStartingTime), 10);
				
			} else if (event.action == VtfEventRecord.INTERNAL_SOCKET_READ) {
				actionDescription = "started internal socket read";
			} else if (event.action == VtfEventRecord.INTERNAL_SOCKET_READ_NULL_WAIT) {
				actionDescription = "waits for internal socket read";
				info.state = VtfThreadInfo.WAITING;
				g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				letter = "W";				
			} else if (event.action == VtfEventRecord.INTERNAL_SOCKET_READ_RESUME) {
				actionDescription = "was resumed started internal socket read";
				letter = "R";
				
			} else if (event.action == VtfEventRecord.INTERNAL_SOCKET_WRITE) {
				actionDescription = "started writing to internal socket";
			} else if (event.action == VtfEventRecord.INTERNAL_SOCKET_WRITE_FULL_WAIT) {
				actionDescription = "waits to write to full buffer";
				info.state = VtfThreadInfo.WAITING;
				g.drawLine(getXofTimestamp(info.lastTime), info.offset, getXofTimestamp(event.timestamp), info.offset);
				letter = "W";				
			} else if (event.action == VtfEventRecord.INTERNAL_SOCKET_WRITE_RESUME) {
				actionDescription = "started internal socket read";
				letter = "R";				
			} else if (event.action == VtfEventRecord.INTERNAL_SOCKET_WRITE_RESUME_READ) {
				actionDescription = "will notify thread waiting to read on socket";
			}
			
			
			if (isMethodEvent) {				 
				pointToEvent.put(new Point(getXofTimestamp(event.timestamp)+4,info.offset), isMethodEvent);
			} else {
				pointToEvent.put(new Point(getXofTimestamp(event.timestamp),info.offset), isMethodEvent);
			}
			
			// Show single event
			if (focusOnEvent) {
				String tempString;
				if ((!isMethodEvent && Math.abs(getXofTimestamp(event.timestamp)-currentPoint.x) < 6 &&  Math.abs(info.offset-currentPoint.y) < 6) ||
					  isMethodEvent && Math.abs(getXofTimestamp(event.timestamp)+4-currentPoint.x) < 6 &&  Math.abs(info.offset-currentPoint.y) < 6) {
					int diff = 0;
					int positionX = getXofTimestamp(event.timestamp);
					tempString = (((Object)(event.timestamp/1000000.0)).toString()) + ": " + actionDescription;
					
					// Use special handling to show the events in the order they take place - bottom up
					
					labelPositionY = info.offset+getTimeCategoryYoffset(info.timerIndex, eventsToFocus);
					
					// take care that the message does not get out of bounds in the right corner
					if (getCharactersOffset(tempString) + positionX - 30 + 8*info.timerIndex> width) {
						diff = getCharactersOffset(tempString) + positionX - 30 - width + 8*info.timerIndex;
					}
					g.drawString(tempString, positionX - 30 - diff + 8*info.timerIndex, labelPositionY);
					g.drawLine(positionX, info.offset-5, positionX, info.offset+5);
					info.timerIndex = ++info.timerIndex % time_categories;
				}					
				
			} else if (!isMethodEvent && showAbbrev){
				// Show all events abbreviated
				labelPositionY = info.offset+getTimeCategoryYoffset(info.timerIndex);	
				g.drawString(letter + "-" + ((Object)(event.timestamp/1000000.0)).toString(), getXofTimestamp(event.timestamp), labelPositionY);
				info.timerIndex = ++info.timerIndex % time_categories;
			}
			
			
			
			// Show grid
			g.setColor(Color.LIGHT_GRAY);
			if (showTimeGrid) {
				g.drawLine(getXofTimestamp(event.timestamp), 0, getXofTimestamp(event.timestamp), height);
			}
				
			
			if (event.action != VtfEventRecord.INTERNAL_SOCKET_WRITE_RESUME_READ) {
				info.lastTime = event.timestamp;
			}
			lastTime = info.lastTime;

			// Wait for slide-show
			if (showProgress == 1) {
				try {
					Thread.sleep(progressInteval);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			
			// Print the last event on the timeline for the mouse version
			if ((showProgress == 2 && i == eventsToShow-1 && (!withTimeLimit || lastTime >= showUntilTime)) || (showTimeGrid && showProgress == 1)) {
				g.setColor(Color.BLUE);
				int diff = 0;
				String tempString = info.threadName + " " + actionDescription + " at " +  (((Object)(event.timestamp/1000000.0)).toString()) + " msec";
				int positionX = getXofTimestamp(event.timestamp);
				if (getCharactersOffset(tempString) + positionX - 30 + 8*info.timerIndex> width) {
					diff = getCharactersOffset(tempString) + positionX - 30 - width;
				}				
				g.drawLine(positionX, height-45, getXofTimestamp(event.timestamp), height-55);
				g.drawString(tempString, getXofTimestamp(event.timestamp) - 30 - diff, height-30);
				
				g.setColor(Color.LIGHT_GRAY);
				g.drawLine(getXofTimestamp(event.timestamp), 0, getXofTimestamp(event.timestamp), height-60);
				

				// Print thread states
				offset = height-100;
				VtfThreadInfo tinfo;
			    for (int ti = 0; ti < threads; ti++) {
			    	threadId = threadIds.elementAt(ti);			    	
			    	threadName = threadNames.elementAt(ti);
			    	tinfo = threadsToInfo.get(threadId);
			    	
			    	g.setColor(VtfThreadInfo.getStateColor(tinfo.state));
			    	g.drawString(threadName + " (" + VtfThreadInfo.getStateName(tinfo.state) + ")", 2, offset);
					offset -= step;
			    }		
			}			
			//g.drawString(((Object)(event.timestamp/1000000.0)).toString(), getXofTimestamp(event.timestamp), getNextTimeCategoryYoffset());
			
			if (maximumThreads < aliveThreads) {
				maximumThreads = aliveThreads;
			}
			if (maximumIoThreads < ioThreads) {
				maximumIoThreads = ioThreads;
			}
			if (ioThreads > 1) {
				avgIoThreadsSum += ioThreads;
				avgIoThreadsCount++;
			}
		}
		
		// Display it in the end to update correctly if we are using until-a-time-point view
		g.setColor(Color.BLUE);
		if (withTimeLimit) {
			g.drawString("Until time: " + showUntilTime, 5, 15);
		} else {
			g.drawString("Events #: " + startingEvent + " - " + eventsToShow, 5, 15);
		}
		g.drawString("Live threads:" + aliveThreads + "/" + allThreads + " [max: " +maximumThreads + "]", 5, 28);
		g.drawString("I/O threads:" + ioThreads  + " [max: " +maximumIoThreads + ", avg: " + (Math.round((double)avgIoThreadsSum/(double)avgIoThreadsCount))+((ioPredictionsMade>0)?", pred: " +ioPredictionsMade:"")+ "]", 5, 41);
		
		if (zoomMode) {
			g.drawString("Zooming (Click to zoom out)", 5, height-33);
			g.drawString("[" + (((Object)(zoomFromTime/1000000.0)).toString().substring(0,8) + " - " +  ((Object)(maxTime /1000000.0)).toString().substring(0,8))+ "ms]", 5, height-20);
		}
	}


	  // This method will be called when the mouse has been clicked.
    public void mouseClicked (MouseEvent me) {

    	currentPoint = me.getPoint();
    	if (zoomMode && me.getButton() == MouseEvent.BUTTON1 && currentPoint.x < beginningPoint && currentPoint.y > height-40 && currentPoint.y < height-16) {
    		zoomFromX = 0;
    		zoomToX = 0;
    		zoomMode = false;
    	}
    	// Use current cursor coordinates to decide whether to focus on single, show all up-to-a point, or show next 
    	
    	focusOnEvent = false;
    	
    	showUntilTime = Long.MAX_VALUE;
    	if (Math.abs(currentPoint.y-timeLineOffset) < 6) {
    		showUntilTime = getTimestampFromX(currentPoint.x);
    		eventsToShow = 0;
    	} else {
	    	
	    	Iterator<Point> iter = pointToEvent.keySet().iterator();
	    	Point p;
	    	eventsToFocus = 0;
	    	boolean flag = true;
	    	while (iter.hasNext() && flag) {
	    		p = (Point)iter.next();
	    		
	    		if (Math.abs(p.x-currentPoint.x) < 6 &&  Math.abs(p.y-currentPoint.y) < 6) {
	    			focusOnEvent = true;
	    		} else {
	    			focusOnEvent = false;
	    		}
	    		
	    		if (focusOnEvent || eventsToFocus>0) {
	    			eventsToFocus++;
	    			if (focusOnEvent == false) {
	    				flag = false;
	    				focusOnEvent = true;
	    			}
	    		}
	    	}
	    	
    	}
    	
    	if (!focusOnEvent && showUntilTime == Long.MAX_VALUE) {
    		int count;
    		if (me.getClickCount() == 2) {
    			count = 10;
    		} else {
    			count = 1;
    		}
    		
    		if (me.getButton() == MouseEvent.BUTTON1) {
    			
    			eventsToShow+=count;	
    			
    		} else {
    			eventsToShow-=count;
    			if (eventsToShow < 0) {
    				eventsToShow = 0;	
    			}
    		}
    		
    	}
    	repaint();

    } 
    
    public void mouseEntered (MouseEvent me) {}
    
    public void mousePressed (MouseEvent me) {
    	if (!zoomMode && me.getPoint().x >= beginningPoint && me.getPoint().y < height-75 && me.getModifiersEx() != 0) {
    		
    		zoomFromX = me.getPoint().x;
    		zoomFromTime = getTimestampFromX(zoomFromX);
    	} else {
    		zoomFromX = 0;
    		zoomToX = 0; 
    	}
    }
    public void mouseReleased (MouseEvent me) {
    	if (!zoomMode && me.getPoint().x >= beginningPoint && me.getPoint().y < height-75 && me.getModifiersEx() != 0) {
	    	zoomToX = me.getPoint().x;
	    	if (zoomToX != zoomFromX) {
	    		maxTime = getTimestampFromX(zoomToX);
	    	}
	    	repaint();
    	} else {
    		zoomFromX = 0;
    		zoomToX = 0;
    	}
    } 
    public void mouseExited (MouseEvent me) {} 	
	
}  // end class VisualizerCanvas



class VtfThreadInfo {
	int offset;
	long lastTime;
	int state;
	int timerIndex;
	String threadName;
	long ioStartingTime;
	
	
	static final int RUNNING = 1;
	static final int WAITING = 2;
	static final int SUSPENDED = 3;
	static final int LEARNING_IO = 4;
	static final int IN_IO = 5;
	static final int ZOMBIE = 6;
	static final int IN_NATIVE = 7;
	static final int WAITING_INTERNAL_SOCKET_READ = 8;
	static final int REGISTERING = 9;
	static final int NATIVE_WAITING = 10;

	
	static Color getStateColor(int _state) {
		switch (_state) {
			case RUNNING: return Color.BLACK;
			case WAITING: case NATIVE_WAITING: return Color.RED;
			case SUSPENDED: return Color.MAGENTA;
			case IN_IO: return Color.GREEN;
			case ZOMBIE: return Color.LIGHT_GRAY;
			
		}
		return Color.LIGHT_GRAY;
	}
	
	static String getStateName(int _state) {
		switch (_state) {
			case RUNNING: return "RUNNING";
			case WAITING: return "WAITING";
			case SUSPENDED: return "SUSPENDED";
			case IN_IO: return "PRED I/O";
			case ZOMBIE: return "TERMINATED";
			case REGISTERING: return "REGISTERING";
			case NATIVE_WAITING: return "NATIVE WAITING";			
		}
		return "UNBORN";
	}
		
	VtfThreadInfo(String tName, int offSet) {
		threadName = tName;
		offset = offSet;
		lastTime = 0;
		state = 0;
		timerIndex = 0;
		ioStartingTime = 0;
	}
	
}
