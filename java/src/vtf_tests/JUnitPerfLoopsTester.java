package vtf_tests;
import com.clarkware.junitperf.*;

import junit.framework.Test;

public class JUnitPerfLoopsTester {

    public static Test suite() {
        long maxElapsedTime = 800;
        Test testCase = new LoopsTest("testNormal");
        Test timedTest = new TimedTest(testCase, maxElapsedTime);
        
        return timedTest;
    }
    
    public static void main(String[] args) {
        junit.textui.TestRunner.run(suite());
    }
}
