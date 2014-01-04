package vtf_tests.serverClientTest.tests;
import com.clarkware.junitperf.*;
import junit.framework.Test;
import vtf_tests.*;

public class TimeTestModelledServerMM1 {
	
    public static Test suite() {
        long maxElapsedTime = 164000;
        return new TimedTest(new ServerClientTest("testModelledServerMM1"), maxElapsedTime);
    }
    
}
