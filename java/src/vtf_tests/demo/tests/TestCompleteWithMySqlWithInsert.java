package vtf_tests.demo.tests;

import vtf_tests.demo.MySqlCompleteDemoWithInsert;
import junit.framework.Test;

import com.clarkware.junitperf.TimedTest;

public class TestCompleteWithMySqlWithInsert extends TestDemoWithInsert {
    public static Test suite() {
        return new TimedTest(new MySqlCompleteDemoWithInsert("testCompleteWithInsertWithMySql"), maxElapsedTime);
    }
}