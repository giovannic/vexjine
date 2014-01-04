package vtf_tests;
import java.io.*; 
class ParallelIoDemo { 
	public static int threads = 1;
	
	public static void main(String args[]) throws Exception {
		long start = System.nanoTime();
		
		if (args.length == 1) {
			Thread cpuThread = new Thread(new CpuThread(Integer.parseInt(args[0])),"CpuThread");
			Thread ioThread = new Thread(new IoThread(),"IoThread");	
			cpuThread.start();
			ioThread.start();
			cpuThread.join();
			ioThread.join();
			
		} else { 
			threads = Integer.parseInt(args[2]);
			if (args[1].equals("read_diff_file") && threads > 32) {
				System.err.println("Error: Supporting up to 32 parallel I/O threads. Aborting!");
				System.exit(-1);
			}
			Thread cpuThread = new Thread(new CpuThread(Integer.parseInt(args[0])),"CpuThread");
			Thread[] ioThreads = new Thread[threads];
			for (int i = 0; i<threads; i++) {
				if (args[1].equals("read_diff_file")) {
					// all threads reading from different files
					ioThreads[i] = new Thread(new IoThread(i),"IoThread"+i);
				} else if (args[1].equals("read_same_file")) {
					// all threads reading from the same file
					ioThreads[i] = new Thread(new IoThread(),"IoThread"+i);
				} else {
					throw new IllegalArgumentException("Arguement 1 should be read_same_file or read_diff_file instead of " + args[1]);
				}
			}
			cpuThread.start();
			for (int i = 0; i<threads; i++) {
				ioThreads[i].start();
			}
			cpuThread.join();
			for (int i = 0; i<threads; i++) {
				ioThreads[i].join();
			}
		}
			
			
		
		
		
		long end = System.nanoTime();
		System.out.println((double)(end-start)/1e9);
	} 
}


class IoThread implements Runnable {

	private int id;
	public static int blocksPerRead = 256;
	static {
		try {
			blocksPerRead = Integer.parseInt(System.getProperty("bpr"));
		} catch (Exception ie) {
			blocksPerRead = 256;
		}
	}
	public IoThread() {
		id = -1;
	}

	public IoThread(int id) {
		this.id = id;
	}
	
	//@virtualtime.Accelerate(speedup = 0.5)
	private void tryReading(FileInputStream f, String s) throws IOException {

		long start = System.nanoTime();
		int blocksToRead = 1024;
		byte b[] = new byte[blocksPerRead*512]; 
		while (blocksToRead-- > 0) {
//			if (blocksToRead % 75 == 0) {
//				System.out.println(blocksToRead +" remaining in file " + s + " at " + (double)(System.nanoTime() - start)/1e9);
//			}		
			f.read(b);
		}
//		System.out.println("io thread " + Thread.currentThread().getName() + " finished");
	}
	
	public void run() {
		String[] largeFiles = {getClass().getClassLoader().getResource(".").getPath() + "../data/ioInput.txt",getClass().getClassLoader().getResource(".").getPath() + "../data/ioInput.txt",getClass().getClassLoader().getResource(".").getPath() + "../data/ioInput.txt"};

		try {

			FileInputStream f = null;
			String s = new String();
			if (id != -1 && id < largeFiles.length){
				s = largeFiles[id];
			} else {
				s = getClass().getClassLoader().getResource(".").getPath() + "../data/ioInput.txt";
			}
			f = new FileInputStream(s);
			tryReading(f, s);
			f.close(); 

		} catch (IOException ie) {
			ie.printStackTrace();
		}


	}
}


class CpuThread implements Runnable {
	private long myLimit;
	
	CpuThread(long _myLimit) {
		myLimit = _myLimit;		 
	}

	private double myLoops() {
		double temp = 1;
		for (int i =0 ; i < 3; i++) {
			temp += methodInner();	
		}
		return temp;
	}

	private double methodInner() {
		double temp = 1;
		for (int i =0 ; i < myLimit; i++) {
			temp += (temp / (i+1));	
		}
		return temp;
	}

	public void run() {
		long start = System.nanoTime();
		double temp = 0.0;
		for (int i =0; i<IoThread.blocksPerRead; i++) {
		temp += myLoops();
		temp += 1;
		}
//		System.out.println("cpu thread " + Thread.currentThread().getName() + " finished with " +temp + " at " + (double)(System.nanoTime() - start)/1e9);
	}
}

