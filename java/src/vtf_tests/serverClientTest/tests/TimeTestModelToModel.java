package vtf_tests.serverClientTest.tests;

import junit.framework.Test;
import vtf_tests.ServerClientTest;

import com.clarkware.junitperf.TimedTest;

public class TimeTestModelToModel {
    public static Test suite() {
        long maxElapsedTime = 16000;
        return new TimedTest(new ServerClientTest("testModelsBothMM1"), maxElapsedTime);
    }
}
