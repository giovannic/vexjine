package vtf_tests;

import java.io.BufferedReader;

import java.io.FileNotFoundException;
import java.io.RandomAccessFile;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Random;

import vtf_tests.demo.loggers.DurationLogger;
import vtf_tests.demo.loggers.EmptyLogger;
import vtf_tests.demo.loggers.LoggerInterface;
import vtf_tests.serverClientTest.ThinkingBehaviour;
import vtf_tests.serverClientTest.thinking.BusyWaitThinkingBehaviour;
import vtf_tests.serverClientTest.thinking.SleepingThinkingBehaviour;
import vtf_tests.serverClientTest.tools.Exp;

public class LocalFileServer implements Runnable {
	public static String filesCatalogueFilename;
	public static boolean loadCatalogueFromMemory = false;
	public static boolean loadCatalogueFromDisk = false;
	public static int iterations;
	public static int bufferSize = 128*1024;
	
	public static LoggerInterface readFileResponseTime = new EmptyLogger();
	public static LoggerInterface readDirResponseTime = new EmptyLogger();
	public static LoggerInterface thinkTime = new EmptyLogger();
	static {
		try {
			if (System.getProperty("log").equals("true")) {
				readFileResponseTime = new DurationLogger("File read");
				//readDirResponseTime = new DurationLogger("Directory read");
				//thinkTime = new DurationLogger("Think time");
			}
		} catch (Exception e) {
			
		}
	}
	
	public static ThinkingBehaviour thinkingBetweenRequests = new NoThinking();
	static {
		try {
			if (System.getProperty("think") != null) {
				thinkingBetweenRequests = new SleepingThinkingBehaviour(new Exp(Double.parseDouble(System.getProperty("think"))));
			} else if (System.getProperty("busywait") != null) {
				thinkingBetweenRequests = new BusyWaitThinkingBehaviour(new Exp(Double.parseDouble(System.getProperty("think"))));
			}
		} catch (Exception e) {
			
		}
	}
	
	protected long getCheckSumOfFile(String filename, byte[] cs_buffer) {
		if (filename == null) {
			return -1;
		}
		FileInputStream fis = null;
		try {
//			System.out.println(filename);
			fis = new FileInputStream(new File(filename));
			
			int bytesRead = 0;
			long checkSum = 0;
			long startTime = readFileResponseTime.onEventStart();
			
			while ((bytesRead = fis.read(cs_buffer)) != -1) {
				readFileResponseTime.onEventEnd(startTime);
				for (int k = 0; k<bufferSize; k++) {
					checkSum += cs_buffer[k];
				}
//				thinkingBetweenRequests.think();
				startTime = readFileResponseTime.onEventStart();
			}
			
			for (int k = 0; k<bytesRead; k++) {
				checkSum += cs_buffer[k];
			}
			
			return checkSum;
		} catch (IOException ie){
			System.out.println(ie.getMessage());
		} finally {
			if (fis != null) {
				try {
					fis.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
//catch (InterruptedException ier) {
//			System.out.println(ier.getMessage());
//		}

		return 0;
	}


	@Override
	public void run() {
		CatalogueParser fileCatalogueParser = null;
		
		if (loadCatalogueFromDisk) {	
			fileCatalogueParser = new DiskResidentFileCatalogueParser(filesCatalogueFilename);
		} else { 
			fileCatalogueParser = new MemoryLoadedFileCatalogueParser(filesCatalogueFilename);
		}
		
		byte[] cs_buffer = new byte[bufferSize];
		FileWriter fwriter = null;
		try {

			int allFiles = fileCatalogueParser.getFilesInCatalogueCount();
			fwriter = new FileWriter(new File("localFileServerResults/checksums_thread_" + Thread.currentThread().getId()));
			fwriter.write("Found " + allFiles + " files in catalogue " + filesCatalogueFilename + "\n");
			
			for (int i = 0; i<iterations; i++) {
				long startFileProcessTime = System.nanoTime();
				String filename = fileCatalogueParser.getRandomFileInCatalogueLine();
				long checkSum = getCheckSumOfFile(filename, cs_buffer);
				long endFileProcessTime = System.nanoTime();
				fwriter.write((endFileProcessTime - startFileProcessTime) + " ns to find the checksum of " + filename + " equal to " + checkSum + "\n");
			}
			
		} catch (IOException ie){
			System.out.println(ie.getMessage());		
		} finally {
			// 
			// Don't forget to close the stream when we finish reading 
			// the file.
			//
			try {
				if (fwriter != null) {
					fwriter.close();
				}
			} catch (IOException ie) {
				
			}
		}

		
	}
	
	public static void main(String[] args) {
	
		if (args.length != 4) {
			System.err.println("Syntax error: java vtf_tests.LocalFileServer <files_catalogue_filename> <catalogue loaded in: [disk|memory]> <iterations> <# threadsCount>");
			System.exit(-1);
		}
		loadCatalogueFromMemory = false;
		loadCatalogueFromDisk = false;
		
		filesCatalogueFilename = args[0];
		int threadsCount = 1;
		iterations = 1000;
		try {
			threadsCount = Integer.parseInt(args[3]);
			iterations   = Integer.parseInt(args[2]);
		} catch (NumberFormatException ne) {
			ne.printStackTrace();
			threadsCount = 1;
			iterations   = 1000;
		}
		
		if (args[1].equals("disk")) {
			loadCatalogueFromDisk = true;
		} else if (args[1].equals("memory")) {
			loadCatalogueFromMemory = true;
		} else {
			throw new IllegalArgumentException("Wrong 'catalogue loaded in' argument: acceptable values are 'disk' and 'memory'");
		}
		new File("localFileServerResults").mkdirs();		// all statically instrumented

		Thread[] threads = new Thread[threadsCount];
		
		long start = System.nanoTime();
		for (int i = 0; i<threadsCount; i++) {
			threads[i] = new Thread(new LocalFileServer(), "ioThread" + i);
			threads[i].start();
		}
		
		for (int i = 0; i<threadsCount; i++) {
			try {
				threads[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		long end = System.nanoTime();
//		System.out.println("Completed parsing " + iterations + " files from " + filesCatalogueFilename + " from " + threadsCount + " parallel threads in " + (double)(end-start)/(double)1e9 + " seconds");
		System.out.println((double)(end-start)/(double)1e9);
		
		readFileResponseTime.print("/data/io_logfile");
		thinkTime.print("/data/io_thinktime");
		readDirResponseTime.print("/data/io_dirlogfile");
	}


}


interface CatalogueParser {	
	abstract public int getFilesInCatalogueCount();
	abstract public String getRandomFileInCatalogueLine(); 
}

class DiskResidentFileCatalogueParser implements CatalogueParser {
	private RandomAccessFile input;
	private int filesCount;
	private Random rand;
	public DiskResidentFileCatalogueParser(String filesCatalogueFilename) {
		rand = new Random(123);
		try {
			input = new RandomAccessFile(new File(filesCatalogueFilename), "r");
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
				
		filesCount = 0;
		try {
			while(input.readLine() != null) {
				++filesCount;		
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	@Override
	public int getFilesInCatalogueCount() {
		return filesCount;
	}

	@Override
	public String getRandomFileInCatalogueLine() {
		int line = rand.nextInt(filesCount);
		try {
			input.seek(0);
			int count = 0;
			String file = "";
			long startTime = LocalFileServer.readDirResponseTime.onEventStart();
			while ((file = input.readLine()) != null && count++ < line) {
				LocalFileServer.readDirResponseTime.onEventEnd(startTime);
				startTime = LocalFileServer.readDirResponseTime.onEventStart();
			}
//				System.out.println(line + " " + file);
			return file;
		} catch (IOException e) {
			e.printStackTrace();
			return "";
		}
	}

	protected void finalize() {
		try {
			input.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}


class MemoryLoadedFileCatalogueParser implements CatalogueParser {
	
	private ArrayList<String> filesCatalogue;
	private Random rand;
	private String filesCatalogueFilename;
	
	public MemoryLoadedFileCatalogueParser(String filesCatalogueFilename) {
		rand = new Random(123);
		filesCatalogue = new ArrayList<String>();
		this.filesCatalogueFilename = filesCatalogueFilename;
	}

	@Override
	public int getFilesInCatalogueCount() {
		BufferedReader input = null;

		try {
			input = new BufferedReader(new FileReader(filesCatalogueFilename));
			String file;
			while ((file = input.readLine()) != null) {
				filesCatalogue.add(new String(file));
			}
			
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (input != null) {
				try {
					input.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		return filesCatalogue.size();
	}

	@Override
	public String getRandomFileInCatalogueLine() {
		int line = rand.nextInt(filesCatalogue.size());
		return filesCatalogue.get(line);
	}

}

class NoThinking implements ThinkingBehaviour {
	public void think() {
		
	}
}
