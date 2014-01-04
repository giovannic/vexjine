package vtf_profiler;
import virtualtime.EventNotifier;
import java.util.Random;
public class RandomMethodProfiler {

	private static Random ra = new Random();

	public static double doit(double i0, double i1, double i2, double i3) {
		double temp = 0.0;
		temp += (i2 + (i1+1.0))  + (i3+1.0);
		temp += (i2 / (i2+1.0))  + (i2 + (i1+1.0)) ;
		temp += i0 + (i0 / (i1+1.0)) ;
		temp += i1 + (i1+1.0);
		temp += i0 + (i2 * (i3+1.0)) ;
		temp += i0 * (i0 + (i3+1.0)) ;
		temp += i1 + (i0+1.0);
		temp += i2 * (i2 / (i3+1.0)) ;
		temp += i1 + (i0+1.0);
		temp += i2 / (i1+1.0);
		temp += i3 * (i2+1.0);
		temp += i1 * (i1+1.0);
		temp += i1 + (i2+1.0);
		temp += i0 * (i3+1.0);
		temp += i3 + (i0+1.0);
		temp += i0 + (i3+1.0);
		temp += i0 + (i3 / (i1+1.0)) ;
		temp += i2 * (i3+1.0);
		temp += i3 * (i0+1.0);
		temp += i0 + (i2+1.0);
		temp += (i1 + (i2+1.0))  / (i1+1.0);
		temp += i0 + (i1+1.0);
		temp += i3 + (i3+1.0);
		temp += i3 / (i2 / (i3+1.0)) ;
		temp += ((i0 / (i3+1.0))  / (i1+1.0))  * (i0+1.0);
		temp += i2 + (i2+1.0);
		temp += i0 / (i3+1.0);
		temp += (i0 * (i2+1.0))  * (i0+1.0);
		temp += i0 * (i3+1.0);
		temp += (i2 + (i2+1.0))  * ((i0 + (i1+1.0))  * (i3+1.0)) ;
		temp += i0 * (i1+1.0);
		temp += i2 * (i3+1.0);
		temp += i1 + (i3+1.0);
		temp += i0 + (i1+1.0);
		temp += i3 * (i0+1.0);
		temp += i1 * (i2+1.0);
		temp += i0 + (i3+1.0);
		temp += i3 * (i2+1.0);
		temp += (i1 + (i0+1.0))  + (i0+1.0);
		temp += (i1 / (i3+1.0))  + (i3+1.0);
		temp += i2 + (i1+1.0);
		temp += i3 + (i0+1.0);
		temp += (i3 / (i1+1.0))  + (i0 / (i2+1.0)) ;
		temp += i1 / (i1+1.0);
		temp += i0 + (i0+1.0);
		temp += i2 / (i0+1.0);
		temp += i0 * (i1+1.0);
		temp += i3 / (i2+1.0);
		temp += i1 * (i1 + (i0+1.0)) ;
		temp += i3 * (i0+1.0);
		temp += (i1 / (i0+1.0))  + (i1+1.0);
		temp += i1 + (i3 + (i1+1.0)) ;
		temp += i0 + (i2+1.0);
		temp += i2 + (i1+1.0);
		temp += i3 + (i0+1.0);
		temp += (((i0 + (i3+1.0))  + (i1+1.0))  + (i0+1.0))  + (i3+1.0);
		temp += i1 * (i3+1.0);
		temp += (i1 * (i2 + (i2+1.0)) )  * (i2+1.0);
		temp += i3 + (i3+1.0);
		temp += (i2 * (i0+1.0))  + (i2+1.0);
		temp += i3 + (i0+1.0);
		temp += i2 + (i2+1.0);
		temp += i3 / (i3+1.0);
		temp += i2 + (i2+1.0);
		return temp;
	}

	public static double myLoops(int loops) {
		double temp = 0.0;
		long start = System.nanoTime();
		for(int i = 0; i<loops; i++) {
			temp += doit(95107348, 2041054267, 909595301, 1683142864);
		}
		temp += 1;
		long end = System.nanoTime();
		System.out.println((end-start)/1000000000.0);
		return temp;
	}

	public static void main(String[] argv) {
		int loops = Integer.parseInt(argv[0]);
		RandomMethodProfiler.myLoops(loops);
	}

}
