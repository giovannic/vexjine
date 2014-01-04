package vtf_tests.demo.tests;

import junit.framework.Test;
import vtf_tests.demo.ModelDemo;
import vtf_tests.demo.MySqlCompleteDemo;

import com.clarkware.junitperf.TimedTest;

public class TestCompleteWithMySql extends TestDemo {

    public static Test suite() {
        return new TimedTest(new MySqlCompleteDemo("testCompleteWithMySql"), maxElapsedTime);
    }
}
