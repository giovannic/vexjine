package vtf_tests.demo.tests;

import junit.framework.Test;
import vtf_tests.demo.ModelDemo;

import com.clarkware.junitperf.TimedTest;

public class TestEmpty extends TestDemo {

    public static Test suite() {
        return new TimedTest(new ModelDemo("testEmpty"), maxElapsedTime);
    }
}
