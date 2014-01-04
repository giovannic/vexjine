/*
 * This class is used to determine the I/O features of VTF that are enabled and working correctly
 */

package vtf_tests;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

import virtualtime.Accelerate;

public class ValidateIo {
	public static void printBuffer(byte[] buffer) {
		synchronized (buffer) {
			System.out.print(Thread.currentThread().getName() + ": ");
			for (int i =0 ; i<buffer.length; i++) {
				System.out.print(buffer[i] + " ");
			}
			System.out.println();
		}
	}
	
	public static void main(String[] args) {
		int size = 32;
		byte[] buffer = new byte[size];
		for (int i =0  ; i<size; i++) {
			buffer[i] = 'A';
		}
		
		Thread cpuThread = new Thread(new ValidateIoCpuThread(buffer), "CpuThread");
		Thread ioThread = new Thread(new ValidateIoIoThread(buffer), "IoThread");
		cpuThread.start();
		ioThread.start();
		try {
			cpuThread.join();
			ioThread.join();
		} catch (InterruptedException ie) {
			ie.printStackTrace();
		}
		
	}
}


class ValidateIoCpuThread implements Runnable {
	byte[] buffer;
	public ValidateIoCpuThread(byte[] b) {
		super();
		buffer = b;
	}
	
	double apply() {
		double temp =0 ;	
		temp += Math.cos(0);
		return temp;
	}
	
	public void run() {
		double temp =0 ;
		
		for (long i = 0; i<2000000; i++) {
			temp += apply();
		}
		temp += 1;
		
		ValidateIo.printBuffer(buffer);
		
	}
}




class ValidateIoIoThread implements Runnable {
	byte[] buffer;
	public ValidateIoIoThread(byte[] b) {
		super();
		buffer = b;
	}
	
	double apply() {
		double temp =0 ;
		for (long i = 0; i<10; i++) {
			temp += Math.cos(i);
		}
		return temp;
	}
	
	@virtualtime.Accelerate(speedup = 1000000.0)
	private void readToBufferFromIo(InputStream f) throws IOException {
		f.read(buffer);
	}
	
	public void run() {
		try {
			InputStream f = new FileInputStream(getClass().getClassLoader().getResource(".").getPath() + "../data/ioInput.txt");

			int blocksToRead = 100;
			byte b[] = new byte[256 * 512]; 

			while (blocksToRead-- > 0) {
				f.read(b);
			}

			readToBufferFromIo(f);
			f.close();
						
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		ValidateIo.printBuffer(buffer);

	}
}





