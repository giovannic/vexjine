/**
 * 
 * minimal agent
 * 
 */ 
#include <jvmti.h>
#include <stdio.h>
#include <string.h>
/* -------------------------------------------------------------------- */

static jvmtiEnv *jvmti = NULL;
static jvmtiCapabilities capa;

/**
 * Called when the agent is first loaded...
 * */
JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void *reserved) {
	printf("Agent_OnLoad started...\n");
	
	jvmtiError error;
	jint res;
	
	res = jvm->GetEnv((void **) &jvmti, JVMTI_VERSION_1_0);

	if (res != JNI_OK || jvmti == NULL) {
		printf("ERROR: Unable to access JVMTI Version 1 (0x%x),"
			" is your J2SE a 1.5 or newer version?"
			" JNIEnv's GetEnv() returned %d\n",
		JVMTI_VERSION_1, res);

	}

	(void) memset(&capa, 0, sizeof(jvmtiCapabilities));
	capa.can_set_native_method_prefix = 1;
	error = jvmti->AddCapabilities(&capa);
	error = jvmti->SetNativeMethodPrefix("__vtf_native_prefix_");

	printf("Minimal agent initialized correctly1\n");
	return JNI_OK;

}

/* Agent_OnUnload: This is called immediately before the shared library is
 *   unloaded. This is the last code executed.
 */

JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm) {
	/* Make sure all malloc/calloc/strdup space is freed */
}

