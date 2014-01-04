package vtf_tests;

public class ModelSingle {

    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/io_model_10ms.jsimg",
    		replaceMethodBody=true)  			
	public static void doWork() {
		System.out.println("Hello world");
	}
		
	
	public static void main(String[] args) {
		int iterations = 10;
		try {
			iterations = Integer.parseInt(args[0]);
		} catch (Exception ie) {
			iterations = 10;
		}
		
		long start = System.currentTimeMillis();
		for (int i = 0; i<iterations; i++) {
			ModelSingle.doWork();
		}
		long end = System.currentTimeMillis();
		System.out.println(iterations + " iterations in " + (end - start));
	}
}
