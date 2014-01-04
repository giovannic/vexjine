package vtf_tests.demo;

public interface CacheImpl {

	public void add(int key, Object obj);
	public Object retrieve(int key);
	public void invalidate(int key);
	public void prefill();
}
