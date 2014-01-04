import java.io.*; 
class FileInputStreamDemo { 
	public static void main(String args[]) throws Exception { 

		int size; 
		InputStream f = new FileInputStream("/homes/nb605/Documents/Azureus Downloads/Despicable Me/despicable.me.dvdrip.xvid-imbt.avi");

//		System.out.println("Total Available Bytes: " + 	(size = f.available())/1024/1024); 


		byte b[] = new byte[4096*32]; 
		int blocksToRead = Integer.parseInt(args[0]);
		long remainingSum = 0;
		while (blocksToRead-- > 0) {
//			f.read(b);
//			remainingSum += Math.random();
//			remainingSum += f.available();
			f.skip(4096*32);

		}

		f.close(); 

	} 
}

