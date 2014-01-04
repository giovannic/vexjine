package vtf_tests.demo.tests;

import vtf_tests.demo.ModelDemo;
import junit.framework.Test;

import com.clarkware.junitperf.TimedTest;

public class TestCacheImplOnlyWithInsert extends TestDemoWithInsert {
    public static Test suite() {
        return new TimedTest(new ModelDemo("testClientAndCacheImplDBserverModelWithInsertSucceeding"), maxElapsedTime);
    }

}