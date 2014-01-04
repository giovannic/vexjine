include Makefile.inc

all:
	cd cinqs/ ; $(MAKE) $(MAKECMDGOALS)
ifeq "$(LIBPERFCTR_PRESENT)" "yes"
	cd perfctr/ ; $(MAKE) $(MAKECMDGOALS)
endif
ifeq "$(COMPILE_PAPIPROFILER)" "yes"
	cd profiler/ ; $(MAKE) $(MAKECMDGOALS)
endif
	cd vex/ ; $(MAKE) $(MAKECMDGOALS)
	cd jine/ ; $(MAKE) $(MAKECMDGOALS) &
	cd tests/ ; $(MAKE) $(MAKECMDGOALS)
	cd ../java/bin/vtf_tests/forwardleap/utils/ ; $(MAKE) $(MAKECMDGOALS)
	cd ../java/ ; ant create_static_classes; ant


clean:
	cd cinqs/ ; $(MAKE) clean
ifeq "$(LIBPERFCTR_PRESENT)" "yes"
	cd perfctr/ ; $(MAKE) clean
endif
ifeq "$(COMPILE_PAPIPROFILER)" "yes"
	cd profiler/ ; $(MAKE) clean
endif
	cd vex/ ; $(MAKE) clean
	cd jine/ ; $(MAKE) clean
	cd tests/ ; $(MAKE) clean

cleanobj:
	rm `find . | grep '\.o' | grep -viE svn | grep -viE out`
