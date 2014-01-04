import java.io.*; 
import java.util.Random;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URL;

class UrlDemo { 

	public static void main(String args[]) throws Exception { 
//		String url = "http://www.doc.ic.ac.uk/~nb605/jrockit";
		String url = "http://www.doc.ic.ac.uk/~nb605/queueing.tar.gz";
//"http://en.wikipedia.org/wiki/Greek_history";
		InputStream is = null;
//"http://jmeter-tips.blogspot.com/";
//			"http://www.joedog.org/index/siege-manual";
		try {
			URL u = new URL(url);
			is = u.openStream();         // throws an IOException

			//return new BufferedReader(new InputStreamReader(new DataInputStream(new BufferedInputStream(is))));
			BufferedReader f = new BufferedReader(new InputStreamReader(is));

//			char b[] = new char[32*4096]; 
//			char b[] = new char[128 * 1024]; 
			char b[] = new char[64]; 
	//		char b = 127;
			int blocksToRead = Integer.parseInt(args[0]);

			while (blocksToRead-- > 0) {
				f.read(b);
//				System.out.println(b);/
			}

			f.close();

		} catch (Exception ie) {


		}
 

	} 
}

