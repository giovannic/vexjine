package queueing;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.util.Collections;
import java.util.Vector;

public class ServerProfiler {

	private static long getMedian() {

		Server server = new Server(400000);
		
		Vector results = new Vector(10);
		long start =0 , end =0;
		
		for (int i = 100; i<500; i++) {
			server.doWork(Math.PI/2, i);	
		}		
		System.out.println("Warmup finished");
		
		int count = 0;
		double temp = 0;
		
		for (int i = 100000; i<4*51200000; i=i*2) {
		//for (int i = 10000; i<110000; i=i*2) {
			
			start = System.nanoTime();
			server.doWork(Math.PI/2, i);
			end = System.nanoTime();
			
			++count;
//			System.out.println(end-start);

			results.add((end-start)/i);
		}

		Collections.sort(results);
		for (int i = 0; i<results.size(); i++) {
			System.out.println(results.elementAt(i));
		}
		return (Long)results.elementAt(4);
		
	}
	/**
	 * @param args
	 */
	public static void main(String[] args) {

		double slowdownFactor = 1.333;
		int printFastEvery = 4;
		double speedupFactor = 0.8;
		
		//double[] test_values = {40000, 133333, 200000, 250000, 325000, 400000, 444444, 500000, 571429, 666667, 800000, 1000000, 1111111, 1200000, 1400000, 1600000, 1866667, 2100000, 2222222, 2400000, 2666667};
		//double[] test_values = {40000, 133333, 200000, 250000, 325000, 400000, 444444, 500000, 571429, 666667, 800000, 1000000, 1200000, 1400000, 1600000, 1866667, 2100000, 2222222, 2400000, 2666667};
		
		//double[] test_values = {40000,133333,200000,250000,333333,400000,444444,500000,571429,666667,800000,1000000,1176471,1428571,1818182,2000000,2222222,2500000,2857143,3333333,3636364,4000000};
		//int[] test_values = {};
		int[] test_values = {40000,133333,200000,400000,500000,666667,1000000,1333333,2000000,2500000,3333333,4000000,4444444,4545455,4651163,4761905,5000000,5263158,5555556,5882353,6250000,6666667,7142857,7692308,8333333,9090909,10000000};
		//int[] test_values = {40000,133333,200000,250000,333333,400000,444444,500000,571429,666667,800000,1000000,1176471,1428571,1818182,2000000,2222222,2500000,2857143,3333333,3636364,4000000,4444444,4545455,4651163,4761905};
		

		int values = 27;
		for (int i =0 ; i<values; i++) {
			System.out.println(test_values[i] + " " + (1000000000 / test_values[i]));
		}

//	Irmabunt	
//		double timePerIteration = 66.0/ 1000000000.0;;//(double)(getMedian()) / 1000000000.0;

		double timePerIteration = (double)(getMedian()) / 1000000000.0;
		double[] normalResults = new double[values];
		double[] fastResults = new double[values];
		double[] slowResults = new double[values];

		double mi, lambda = 2.5, response_time;
		
		System.out.println("Max service rate " + (1/(2.5*timePerIteration)));
		System.out.println("Max service rate for 2sec max " + (1  /(((1/(2*1000000000)) + 2.5)*timePerIteration)  ));

		
		try {
			FileOutputStream outParamfast = new FileOutputStream("/homes/nb605/VTF/java/vtfTester/files/mm1fastparameters");
			PrintStream foutparamfast = new PrintStream(outParamfast);
			
			FileOutputStream outfast = new FileOutputStream("/homes/nb605/VTF/java/vtfTester/files/mm1fastreal");
			PrintStream fouttimesfast = new PrintStream(outfast);


			FileOutputStream outParamspeedup = new FileOutputStream("/homes/nb605/VTF/java/vtfTester/files/mm1parameters_speedup");
			PrintStream foutParamsfast = new PrintStream(outParamspeedup);
			
			FileOutputStream outParamslow = new FileOutputStream("/homes/nb605/VTF/java/vtfTester/files/mm1parameters_slowdown");
			PrintStream foutParamsslow = new PrintStream(outParamslow);

			FileOutputStream outParam = new FileOutputStream("/homes/nb605/VTF/java/vtfTester/files/mm1parameters");
			PrintStream fout2 = new PrintStream(outParam);
			
			FileOutputStream out = new FileOutputStream("/homes/nb605/VTF/java/vtfTester/files/mm1real");
			PrintStream fout = new PrintStream(out);

			FileOutputStream outspeedup = new FileOutputStream("/homes/nb605/VTF/java/vtfTester/files/mm1real_speedup");
			PrintStream foutfast = new PrintStream(outspeedup);

			FileOutputStream outslow = new FileOutputStream("/homes/nb605/VTF/java/vtfTester/files/mm1real_slowdown");
			PrintStream foutslow = new PrintStream(outslow);

			for (int i =0 ; i<values; i++) {
				System.out.println("average service time for " + test_values[i] + " = " + (double)(test_values[i]*timePerIteration));
				mi = 1.0 / (timePerIteration * test_values[i]);
				normalResults[i] = 1.0 / (mi - lambda);

				if (normalResults[i] > 0.0 && normalResults[i] < 5.0) {
					fout2.println((int)test_values[i]);
					fout.println(normalResults[i]);

					
					if (i % printFastEvery == 0) {
						foutparamfast.println((int)test_values[i]);
						fouttimesfast.println(normalResults[i]);
					}
				} 


				mi = 1.0 / (timePerIteration * (test_values[i]*speedupFactor));
				fastResults[i] = 1.0 / (mi - lambda);
				if (fastResults[i] > 0.0 && fastResults[i] < 5) {
					foutParamsfast.println((int)test_values[i]);
					foutfast.println(fastResults[i]);
				}				

				mi = 1.0 / (timePerIteration * (test_values[i]*slowdownFactor));
				slowResults[i] = 1.0 / (mi - lambda);
				if (slowResults[i] > 0.0 && slowResults[i] < 5) {
					foutParamsslow.println((int)test_values[i]);
					foutslow.println(slowResults[i]);
				}				


			}

			for (int i =0 ; i<values; i++) {
				System.out.print((int)test_values[i] + "\t");
				if (slowResults[i] > 0) {
					System.out.print(slowResults[i] + "\t");
				} else { System.out.print("\t\t\t"); }

				if (normalResults[i] > 0) {
					System.out.print(normalResults[i] + "\t");
				} else {System.out.print("\t\t\t"); }

				if (fastResults[i] > 0) {
					System.out.print(fastResults[i] + "\t");
				}

				System.out.println();

			}
			foutParamsslow.close();outParamslow.close();foutParamsfast.close();outParamspeedup.close();
			foutslow.close();
			outslow.close();

			foutfast.close();
			outspeedup.close();
			fout.close();
			out.close();
			fout2.close();
			outParam.close();
			
			outParamfast.close();
			foutparamfast.close();
			outfast.close();
			fouttimesfast.close();
			
		} catch (IOException io) {
			io.printStackTrace();
		}

	}

}
