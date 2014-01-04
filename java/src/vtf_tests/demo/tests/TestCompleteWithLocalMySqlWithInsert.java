package vtf_tests.demo.tests;

import vtf_tests.demo.MySqlLocalCompleteDemoWithInsert;
import junit.framework.Test;

import com.clarkware.junitperf.TimedTest;

public class TestCompleteWithLocalMySqlWithInsert extends TestDemoWithInsert {
    public static Test suite() {
        return new TimedTest(new MySqlLocalCompleteDemoWithInsert("testCompleteWithInsertWithMySql"), maxElapsedTime);
    }
}