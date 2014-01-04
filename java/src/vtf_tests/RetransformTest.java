package vtf_tests;

import java.io.File;
import java.lang.instrument.Instrumentation;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;

import virtualtime.ClassTransformer;

public class RetransformTest {
	private static void reloadClass(int i) {
			
	}
	
	public static void main(String[] args) {
		System.out.println(Thread.currentThread().getName());
		for (int i =0; i <10; i++) { 
//			reloadClass(i);
//			RetransformTestClass.printMessage(i);
		}
	}
}