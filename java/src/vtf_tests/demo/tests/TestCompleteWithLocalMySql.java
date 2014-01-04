package vtf_tests.demo.tests;

import junit.framework.Test;
import vtf_tests.demo.MySqlLocalCompleteDemo;

import com.clarkware.junitperf.TimedTest;

public class TestCompleteWithLocalMySql extends TestDemo {

    public static Test suite() {
        return new TimedTest(new MySqlLocalCompleteDemo("testCompleteWithMySql"), maxElapsedTime);
    }
}
