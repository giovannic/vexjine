import java.io.*; 
import java.util.Random;

class FileOutputStreamDemo { 
	public static void main(String args[]) throws Exception { 

		int size; 
		FileOutputStream f = new FileOutputStream("largetestfile.avi");
//		FileOutputStream f = new FileOutputStream("/homes/nb605/Documents/Azureus Downloads/Despicable Me/despicable.me.dvdrip.xvid-imbt.avi", "r");

//		System.out.println("Total Available Bytes: " + 	(size = f.available())/1024/1024); 

		byte b[] = new byte[32*4096]; 
//		byte b[] = new byte[128]; 
//		byte b = 127;
		int blocksToRead = Integer.parseInt(args[0]);
		int totalBlockReads = blocksToRead;
		long remainingSum = 0;
		Random rand = new Random();
		while (blocksToRead-- > 0) {
//			rand.nextBytes(b);
			f.write(b);
//			remainingSum += Math.random();
//			remainingSum += f.available();

//			f.seek(2 *(totalBlockReads - blocksToRead) * 4096*32);

		}

		f.close(); 

	} 
}

