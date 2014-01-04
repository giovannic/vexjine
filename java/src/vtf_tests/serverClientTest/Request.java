package vtf_tests.serverClientTest;

import java.util.Random;

public class Request {
	public Request() {
		serviced = false;
	}
	public Request(Random r) {
		int asciiValue = r.nextInt(52);
		if (asciiValue < 26) {
			character = (char)(asciiValue + 65);	// upper case
		} else{
			character = (char)(asciiValue + 71);		// lower case
		}
		
	}

	public void setResult(int res) {
		result = res;
	}
	
	public char getChar() {
		return character;
	}
	
	public int getResult() {
		return result;
	}
	
	public void setServiced(boolean s) {
		serviced = s;
	}
	
	public boolean isServiced() {
		return serviced;
	}
	
	protected boolean serviced;
	protected char character;
	protected int result;
}
