/*
 * Class used to profile the duration of the instrumentation code during runtime
 * 
 * The lost time is calculated by "executing" empty-body dummy methods.
 * We would expect that the simulation time of these methods is 0.
 * However, due to instrumentation/timer access overheads this is not the case.
 * To compensate for that we decrease the average total simulation time gathered,
 * up to that point, after each profiling run.
 */

package virtualtime;

abstract public class InstrumentationProfiler {
		
	protected static int WARMUP_ITERATIONS = 20;
	protected static int DEFAULT_ITERATIONS = 1000;
	
	protected VexInstrumentedBlock vexInstrumentedBlock;
	InstrumentationProfiler(VexInstrumentedBlock vib) {
		vexInstrumentedBlock = vib;
	}
	
	protected long warmup() {
		int iterations = DEFAULT_ITERATIONS;
		int warmupIterations = WARMUP_ITERATIONS;

		long start = 0;
		for (int i =0 ; i<warmupIterations; i++) {
			vexInstrumentedBlock.execute();		
		}
		for (int i =0 ; i<(warmupIterations*iterations); ++i) {
			start += EventNotifier._getVtfTime();
		}
		return start;
	}
	
	abstract public long profile();
}


class AverageInstrumentationProfiler extends InstrumentationProfiler {

	AverageInstrumentationProfiler(VexInstrumentedBlock vib) {
		super(vib);
	}

	public long profile() {
		warmup();

		long start1 = EventNotifier._getVtfTime();		
		for (int i =0 ; i<DEFAULT_ITERATIONS; ++i) {
			vexInstrumentedBlock.execute();
		}
		
		long end = EventNotifier._getVtfTime();
//		System.out.println((end-start1) + " and " + ((double)(end-start1)/(vexInstrumentedBlock.getIterationsPerBlock()*DEFAULT_ITERATIONS)));
		return (end-start1)/(vexInstrumentedBlock.getIterationsPerBlock()*DEFAULT_ITERATIONS);
	}
}

class MedianInstrumentationProfiler extends InstrumentationProfiler {
	protected int possibleValues = 512;
	protected int[] profiledValues;
	
	MedianInstrumentationProfiler(VexInstrumentedBlock vib) {
		super(vib);
		profiledValues = new int[possibleValues];
	}

	private void logResult(int newValue, int measurementsUntilNow) {
		while (newValue >= possibleValues) {
			if (profiledValues[possibleValues-1] > 1) {
				possibleValues *= 2;		// increase possible values on overflow
				int[] values2 = new int[possibleValues];
				System.out.println("increasing possiblevalues to " + possibleValues);
				System.arraycopy(profiledValues, 0, values2, 0, possibleValues/2);
				profiledValues = values2;
			} else {
				newValue = possibleValues - 1;
				break;
			}
		}
		++profiledValues[newValue];
	}
	
	public long profile() {
		long diff, start = 0;
		warmup();				
		for (int i=0 ; i<DEFAULT_ITERATIONS; ++i) {
			start = EventNotifier._getVtfTime();
			vexInstrumentedBlock.execute();
			diff = EventNotifier._getVtfTime() - start;
			logResult((int)(diff / vexInstrumentedBlock.getIterationsPerBlock()), i);
		}
	
		return getResultsMedian(DEFAULT_ITERATIONS);
		
	}	
	
	
	private int getResultsMedian(int iterations) {
		int sumVals =0 ;
		for (int i =0 ; i<possibleValues; i++) {
			if (sumVals < (iterations)/2) {
				sumVals += profiledValues[i];
			} else {	
				return (i-1); 
			}
		}
		return possibleValues;
	}
}



	
class MinInstrumentationProfiler extends InstrumentationProfiler {

	MinInstrumentationProfiler(VexInstrumentedBlock vib) {
		super(vib);
	}

	public long profile() {
		warmup();

		long start1all=System.nanoTime();
		long minimum = Long.MAX_VALUE;
		for (int i =0 ; i<DEFAULT_ITERATIONS; ++i) {
			long start1 = EventNotifier._getVtfTime();//EventNotifier._getVtfTime();
			vexInstrumentedBlock.execute();
			long diff = EventNotifier._getVtfTime() - start1;//System.nanoTime();//EventNotifier._getVtfTime();
			if (diff < minimum) {
				minimum = diff;
			}
		}	
		long endall =System.nanoTime();//System.nanoTime();//EventNotifier._getVtfTime();
		
//		System.out.println("The total execution overhead per method instrumentation is " + (minimum/vexInstrumentedBlock.getIterationsPerBlock()) + " and was calculated in " + (double)(endall-start1all)/1e9 + " seconds");
		return (minimum/vexInstrumentedBlock.getIterationsPerBlock());//(end-start1)/(iterationsPerBlock*DEFAULT_ITERATIONS);
	}

}

class TotalOverHeadMinimumInstrumentationProfiler extends InstrumentationProfiler {

	TotalOverHeadMinimumInstrumentationProfiler(VexInstrumentedBlock vib) {
		super(vib);
	}

	/**
	 * Method used to time the entire code of VEX, not the instrumentation delays
	 * @return total execution overhead per method instrumentation
	 */
	public long profile() {

		warmup();

		long start1all=System.nanoTime();
		long minimum = Long.MAX_VALUE;
		for (int i =0 ; i<DEFAULT_ITERATIONS; ++i) {
			long start1 = System.nanoTime();//EventNotifier._getVtfTime();
			vexInstrumentedBlock.execute();
			long diff = System.nanoTime() - start1;//System.nanoTime();//EventNotifier._getVtfTime();
			if (diff < minimum) {
				minimum = diff;
			}
		}	
		long endall =System.nanoTime();//System.nanoTime();//EventNotifier._getVtfTime();

//		System.out.println("The total execution overhead per method instrumentation is " + (minimum/vexInstrumentedBlock.getIterationsPerBlock()) + " and was calculated in " + (double)(endall-start1all)/1e9 + " seconds");
		return minimum;//(end-start1)/(iterationsPerBlock*DEFAULT_ITERATIONS);

	}

}

class InstrumentationProfilerFactory {
	static InstrumentationProfiler createInstrumentationProfiler(String profilingType, String vexExecutionBlock) throws IllegalArgumentException {
		VexInstrumentedBlock vexInstrumentedBlock = null;
		if (vexExecutionBlock.equals("method")) {
			vexInstrumentedBlock = new MethodExecutionBlock();
		} else if (vexExecutionBlock.equals("io")) {
			vexInstrumentedBlock = new IoMethodExecutionBlock();
		} else if (vexExecutionBlock.equals("ip")) {
			vexInstrumentedBlock = new InteractionPointExecutionBlock();
		} else if (vexExecutionBlock.equals("yield")) {
			vexInstrumentedBlock = new YieldExecutionBlock();
		} else {
			throw new IllegalArgumentException("Execution blocks supported: method, io, ip and yield");
		}
		
		if (profilingType.equals("avg")) {
			return new AverageInstrumentationProfiler(vexInstrumentedBlock);
		} else if (profilingType.equals("median")) {
			return new MedianInstrumentationProfiler(vexInstrumentedBlock);

		} else if (profilingType.equals("min")) {
			return new MinInstrumentationProfiler(vexInstrumentedBlock);
			
		} else if (profilingType.equals("totalMin")) {
			return new TotalOverHeadMinimumInstrumentationProfiler(vexInstrumentedBlock);
			
		} else {
			throw new IllegalArgumentException("Profiling types supported: avg, median, min and totalMin");
		}
		
		
	}
	
}


abstract class VexInstrumentedBlock {
	int iterationsPerBlock;
	abstract public void execute();
 
	abstract public String getLabel();
	
	public int getIterationsPerBlock() {
		return iterationsPerBlock;
	}
}

/***
 * Profile method durations
 * 
 * @author nb605
 */
class MethodExecutionBlock extends VexInstrumentedBlock {
	
	private static int INSTRUMENTATION_OVERHEAD_PROFILING_METHOD_ID = 50;
	MethodExecutionBlock() {
		EventNotifier.registerMethod("vtfProfilingTest", INSTRUMENTATION_OVERHEAD_PROFILING_METHOD_ID, 0);
		iterationsPerBlock = 1000;
	}

	public void execute() {
		for (int i =0 ; i<iterationsPerBlock; ++i) {
			EventNotifier._afterMethodEntry(INSTRUMENTATION_OVERHEAD_PROFILING_METHOD_ID);
			EventNotifier._beforeMethodExit(INSTRUMENTATION_OVERHEAD_PROFILING_METHOD_ID);
		}
	}
	
	public String getLabel() {
		return "Method profiler";
	}
}

/***
 * Profile I/O method durations
 * 
 * @author nb605
 */
class IoMethodExecutionBlock extends VexInstrumentedBlock {
	
	IoMethodExecutionBlock() {
		iterationsPerBlock = 1000;
	}

	public void execute() {
		for (int i =0 ; i< iterationsPerBlock ; ++i) {
			Thread curThread = Thread.currentThread();
			curThread.setVtfIoInvocationPoint(1);
			EventNotifier.afterIoMethodEntry(curThread, 1);
			EventNotifier.beforeIoMethodExit(curThread, 1);
		}
	}

	public String getLabel() {
		return "I/O method profiler";
	}
}

/***
 * Profile IP durations
 * 
 * @author nb605
 */
class InteractionPointExecutionBlock extends VexInstrumentedBlock {
	
	InteractionPointExecutionBlock() {
		iterationsPerBlock = 1000;
	}

	public void execute() {
		for (int i =0 ; i<iterationsPerBlock; ++i) {
			EventNotifier._interactionPoint();
		}
	}

	public String getLabel() {
		return "IP profiler";
	}
}


/***
 * Used to profile the duration of Thread.yield() -> essentially the real time duration sched_yield
 * which is then added to the time of the simulated Thread.yield() which replaces the original call in
 * the virtual time execution.
 * 
 * @author nb605
 */
class YieldExecutionBlock extends VexInstrumentedBlock {
	
	YieldExecutionBlock() {
		iterationsPerBlock = 100;
	}

	public void execute() {
		for (int j = 0; j<10; j++) {
			Thread.yield();Thread.yield();Thread.yield();Thread.yield();Thread.yield();Thread.yield();Thread.yield();Thread.yield();Thread.yield();Thread.yield();
		}
	}

	public String getLabel() {
		return "Yield profiler";
	}
}

