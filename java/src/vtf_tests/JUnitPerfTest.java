package vtf_tests;
import com.clarkware.junitperf.*;
import junit.framework.Test;

public class JUnitPerfTest {
    public static Test suite532() {
        long maxElapsedTime = 164000;
        return new TimedTest(new ServerClientTest("testModelledServerMM1"), maxElapsedTime);
    }

    public static Test suite() {
        long maxElapsedTime = 16000;
        return new TimedTest(new ServerClientTest("testModelsBothMM1"), maxElapsedTime);
    }
}
