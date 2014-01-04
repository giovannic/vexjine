package virtualtime.statistics;

public interface InstrumentationStatsRecord {
	public void addIoPoint();
	public void setProfiled();
	public void addSyncPrimitive();
	public void addInstruction();
	
	public boolean isProfiled();
	public int getInstructions();
	public int getIoPoints();
	public int getSyncPrimitives();
	
	public void add(InstrumentationStatsRecord record);
	public void clearFlags();
	public void setRetransformed();
	public boolean isRetransformed();
}
