package vtf_tests.demo;

public class ArrayCache implements CacheImpl {
	public ArrayCache(int size) {
		this.size = size;
		keys = new int[size];
		storage = new Object[size];
		for (int i=0; i<size; i++) {
			storage[i] = null;
			keys[i] = 0;
		}
		totalElements = 0;
	}
	
	
	@Override
	public void add(int key, Object obj) {
		synchronized(this) {
			boolean alreadyAdded = false;
			for (int i = 0; i<Math.min(totalElements, size); i++) {
				if (keys[i] == key) {
					alreadyAdded = true;
				}
			}
			if (!alreadyAdded) {
				keys[totalElements % size] = key;
				storage[totalElements % size] = obj;
				++totalElements;
			}
		}
	}

	@Override
	public Object retrieve(int key) {
		Object toReturn = null;
		synchronized(this) {
//			System.out.println("+++++++++++= SEARCHING FOR " + (char)key + " ++++++++++++++");
//			for (int i = 0; i<Math.min(totalElements, size); i++) {
//				System.out.println((char)keys[i] + " "+ storage[i]);
//			}

			for (int i = 0; i<Math.min(totalElements, size); i++) {
				if (keys[i] == key) {
					toReturn = storage[i];
					break;
				}
			}

//			if (toReturn == null) {
//				System.out.println("not found " + (char)key);
//			} else {
//				System.out.println("found " + (char)key);
//			}

		}

		return toReturn;
	}

	public void prefill() {
		int lowercaseFill = (size < 26)? size : 26;
		for (int i = 0; i<lowercaseFill; i++) {
			keys[i] = 65 + i;
			storage[i] = 53;
		}
		for (int i = 26; i<size; i++) {
			keys[i] = 71 + i;
			storage[i] = 53;
		}
		totalElements = size;
	}
	protected int size;
	protected int totalElements;
	protected Object[] storage;
	protected int[] keys;
	
	@Override
	public void invalidate(int key) {
		synchronized(this) {
			for (int i = 0; i<Math.min(totalElements, size); i++) {
				if (keys[i] == key) {
					keys[i] = 0;
					storage[i] = null;
					break;
				}
			}
		}
	}
}
