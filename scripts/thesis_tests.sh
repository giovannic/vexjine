#!/bin/bash
#unittests=1
#chapter3=1
#chapter4=1
#chapter6=1
chapter7=1

topdir=`pwd`

if [ -z "$1" ]; then
testdir=$topdir/src/tests/thesis_tests_`date +%Y-%m-%d_%H%M%S`
mkdir $testdir

uname -a > $testdir/hostinfo
cat /proc/cpuinfo >> $testdir/hostinfo

else 

testdir=$1

fi


#################### UNIT TESTS ####################
if [ -n "$unittests" ];then
	make clean &> /dev/null
	make nologpapi &> /dev/null
	cd ./java/
	./vtest 			&> "$testdir"/unit_cpu1_timeslot100
	./vtest timeslot=10 		&> "$testdir"/unit_cpu1_timeslot10
	./vtest cpus=2 			&> "$testdir"/unit_cpu2_timeslot100
	./vtest cpus=2,timeslot=10 	&> "$testdir"/unit_cpu2_timeslot10
	./java/disable_cpu1 &> /dev/null
	./utest				&> "$testdir"/unit_tests_various
	cd ..
fi

#################### CHAPTER 3 ####################
if [ -n "$chapter3" ];then 
	./java/disable_cpu1 &> /dev/null
	cd ./src/tests/
	echo "Real scheduling"  &> "$testdir"/chapter3_sequence_verification
	./verifySequence R 5 >> "$testdir"/chapter3_sequence_verification
	echo >> "$testdir"/chapter3_sequence_verification
	echo >> "$testdir"/chapter3_sequence_verification
	echo >> "$testdir"/chapter3_sequence_verification
	echo "VEX scheduling"  >> "$testdir"/chapter3_sequence_verification
	./verifySequence timeslot=10 5 >> "$testdir"/chapter3_sequence_verification

	./testVexCoreWithRandomProgram 1024 500 output_dir=$testdir/results 10 &> "$testdir"/chapter3_profile_accuracy.csv
	sleep 30
	./testCompensationAll &> "$testdir"/chapter3_overhead.csv
	sleep 30
	./testPageFault &> "$testdir"/chapter3_page_fault.csv
	sleep 30

	java -version &> /dev/null							#warmup up pc after crazy swapping
	./testVexCoreWithRandomProgram 1 200 output_dir=$testdir/results 1 &> /dev/null #warmup up pc after crazy swapping
	ls -lstrh &> /dev/null								#warmup up pc after crazy swapping
	cd $topdir
fi

#################### CHAPTER 4 ####################
if [ -n "$chapter4" ];then 

	./java/disable_cpu1 &> /dev/null
	cd ./java

	make clean &> /dev/null
	make papirelease &> /dev/null

	#### create random files list from /usr/
	./createRandomFileCatalogue /usr/ 10000 > vexIoTestsExtFileCatalogue
	cp vexIoTestsExtFileCatalogue "$testdir"/chapter4_vexIoTestsExtFileCatalogue

	#### get file sizes histogram
	ls -lstr `cat vexIoTestsExtFileCatalogue` |awk '{print $1}' | sort -g | uniq -c | awk '{print $2" "$1}' > "$testdir"/chapter4_file_sizes

	if [ -f "/data/vtf_io_stats.csv" ];then
		rm /data/vtf_io_stats.csv
	fi

	#### get observed I/O durations
	 ./vtftest -F -v -uioheavy -s 2  -A adaptiveProfiling=10000 -o "no_scheduling,iostats=all,iolog=ext"
	cat /data/vtf_io_stats.csv  | perl -lne 'print $1 if /Real_Samples,(.*)/' | tr "," "\n" | sort -g | awk '{print log($1)/log(10)}' | 		uniq -c -w 1 |  awk '{print $2" "$1}' | grep -viE inf > "$testdir"/chapter4_histogram_of_observed_io

	#### general global various prediction methdods and iodisregard
	./vtftest -F  -uioheavy -s 2 -A adaptiveProfiling=10000 -O -o "iodisregard,iolevel=global,iobuffer=1" > "$testdir"/chapter4_results_diskio_no_iorecognition
	./iotest 2 2 flushing 0 10 0 0 >> "$testdir"/chapter4_results_diskio_frommem_no_iorecognition

	#### iothreshold effect
	./iotest 2 2 both 0 10 0 0 &> "$testdir"/chapter4_results_diskio_frommem_iothreshold

	#### iolevel effect
	./iotest 3 3 both 0 10 0 2 &> "$testdir"/chapter4_results_diskio_fromdisk_iothreshold_prediction_levels

	#### markov effect
	./iotest 2 3 both 12 16 0 2 &> "$testdir"/chapter4_results_diskio_mem_and_disk_iothreshold_markov

	#### only use max prediction :0 0
	./iotestexclusion 2 2 both 0 0 2 2 &> "$testdir"/chapter4_results_diskio_mem_exclusion_test

	#### only use max prediction :0 0
	./iotestscaling 2 2 both 0 0 2 2 &> "$testdir"/chapter4_results_diskio_mem_scaling_test

	#### derby DB tests
	./iotest 13 13 flushing 0 4 2 2 &> "$testdir"/chapter4_results_derby_nothreshold

	#### mysql tests
	./iotest 12 12 flushing 0 4 2 2 &> "$testdir"/chapter4_results_mysql_nothreshold
	./iotest 12 12 both 0 4 2 2 &> "$testdir"/chapter4_results_mysql_threshold

	cd $topdir	
fi


#################### CHAPTER 6 ####################
if [ -n "$chapter6" ];then 
	./java/disable_cpu1 &> /dev/null
	cd ./java

if [ 1 = 0 ]; then
	make clean &> /dev/null
	make papirelease &> /dev/null

	ant -Dtimeslot=10 mm1 &> "$testdir"/chapter6_results_mm1
fi
	#JGF SINGLE
	echo "========= VEX full profiling ========" > "$testdir"/chapter6_jgfsingle
	./vtftest -ujgfsingle -f 7 -i 3 -a >> "$testdir"/chapter6_jgfsingle
	echo "========= VEX adaptive profiling ========" >> "$testdir"/chapter6_jgfsingle
	./vtftest -ujgfsingle -f 7 -i 3 -a -A "adaptiveProfiling=10000" >> "$testdir"/chapter6_jgfsingle
	echo "========= VEX adaptive profiling -Xint 10k ========" >> "$testdir"/chapter6_jgfsingle
	./vtftest -ujgfsingle -f 7 -i 3 -a -j " -Xint " -A "adaptiveProfiling=10000" >> "$testdir"/chapter6_jgfsingle
	echo "========= VEX adaptive profiling -Xint 1M ========" >> "$testdir"/chapter6_jgfsingle
	./vtftest -ujgfsingle -f 7 -i 3 -a -j " -Xint " -A "adaptiveProfiling=1000000" >> "$testdir"/chapter6_jgfsingle


	#JGF MULTI
	echo "========= VEX adaptive profiling ABC ========" > "$testdir"/chapter6_jgfmulti8
	./vtftest -ujgf8 -i 3 -a -t 19 -A "adaptiveProfiling=10000" >> "$testdir"/chapter6_jgfmulti8
	echo "========= VEX adaptive profiling Stress ========" >> "$testdir"/chapter6_jgfmulti8
	./vtftest -ujgf8 -i 3 -a -f 22 -t 25 -A "adaptiveProfiling=10000" >> "$testdir"/chapter6_jgfmulti8

	#JGF THREAD AND TIME SCALING
	./vtftest -ujgfscaling -s 2 -A "adaptiveProfiling=10000" -O -i 10 -a &> "$testdir"/chapter6_jgf_sparsematmult_scaling
	./vtftest -ujgfscaling -s 2 -A "adaptiveProfiling=10000,adaptedMethods=adajgfsparsematmult" -i 3 -a -v >> "$testdir"/chapter6_jgf_sparsematmult_scaling
	./vtftest -ujgfscaling -s 2 -A "adaptiveProfiling=10000,adaptedMethods=adajgfsparsematmultslow" -i 3 -a -v >> "$testdir"/chapter6_jgf_sparsematmult_scaling


	#SPECjvm2008 MASSIVE
	cd ..
	./jinechapter &> "$testdir"/chapter6_SPECjvm2008_noalt_jvms

fi



#################### CHAPTER 7 ####################
if [ -n "$chapter7" ];then 
	./java/enable_cpu1 &> /dev/null
	cd ./java
	make clean &> /dev/null
	make papirelease &> /dev/null

	./testscalabily &> "$testdir"/chapter7_testscalability
	./testscalabilityscaled &> "$testdir"/chapter7_testscalability_scaled

	./vtftest -ujgf8 -i 3 -a -t 19 -o "cpus=2,timeslot=10" -A "adaptiveProfiling=10000" &> "$testdir"/chapter7_jgf_multi


	echo "======== SPEC MULTI LOOSE =======" &> "$testdir"/chapter7_spec_multi
	./vtftest -uspecregress4 -i 3 -a -o "cpus=2,timeslot=10" -A "adaptiveProfiling=10000" >> "$testdir"/chapter7_spec_multi
	echo "======== SPEC MULTI SPEX =======" &> "$testdir"/chapter7_spec_multi
	./vtftest -uspecregress4 -i 3 -a -o "cpus=2,timeslot=10,spex" -A "adaptiveProfiling=10000" >> "$testdir"/chapter7_spec_multi
fi
