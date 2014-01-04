package virtualtime;

public @interface ModelPerformance {
	String jmtModelFilename() default "";
	boolean replaceMethodBody() default false;
	int returnValue() default 0;
	String sourceNodeLabel() default "";
	int customerClass() default 0;
}
