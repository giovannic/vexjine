package vtf_tests.serverClientTest;

public class ModelBasedQueueingSim implements Runnable {

	@virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/waiting400_local.jsimg",
    		replaceMethodBody=true)  		
	private static void think() {
		
	}
	
    @virtualtime.ModelPerformance(
    		jmtModelFilename="/homes/nb605/VTF/src/models/mm1_q_local.jsimg",
    		replaceMethodBody=true)  		  		
	private void makeRequest() {
		
	}
	
	public void run() {
		makeRequest();
	}
	
    public static void main(String args[]) {
    	int requests = 400;
    	for (int i=0; i<requests; i++) {
            Thread thd = new Thread(new ModelBasedQueueingSim(), "request " + i);
            thd.start();
            
            think();
    	}	
    }

}
