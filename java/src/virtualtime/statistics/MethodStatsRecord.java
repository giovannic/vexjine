package virtualtime.statistics;

public class MethodStatsRecord implements InstrumentationStatsRecord {
	public MethodStatsRecord() {
		instructions = 0;
		ioPoints = 0;
		syncPrimitives = 0;
		profiled = false;
		retransformed = false;
	}
	
	@Override
	public void addInstruction() {
		++instructions;		
	}

	@Override
	public void addIoPoint() {
		++ioPoints;		
	}

	@Override
	public void addSyncPrimitive() {
		++syncPrimitives;		
	}

	@Override
	public void setProfiled() {
		profiled = true;		
	}

	@Override
	public boolean isProfiled() {
		return profiled;
	}
	

	private int instructions;
	private int ioPoints;
	private int syncPrimitives;
	private boolean profiled;
	boolean retransformed;
	
	@Override
	public void add(InstrumentationStatsRecord record) {
		instructions += record.getInstructions();
		ioPoints += record.getIoPoints();
		syncPrimitives += record.getSyncPrimitives();
	}

	@Override
	public int getInstructions() {
		return instructions;
	}

	@Override
	public int getIoPoints() {
		return ioPoints;
	}

	@Override
	public int getSyncPrimitives() {
		return syncPrimitives;
	}

	@Override
	public void clearFlags() {
		profiled = false;
		retransformed = false;
	}

	@Override
	public void setRetransformed() {
		retransformed = true;
	}
	
	public boolean isRetransformed() {
		return retransformed;
	}

}
