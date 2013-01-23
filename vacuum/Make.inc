vacuum:\
	vacuum/vacuum_solver\
	vacuum/vacuum_cs730\

CLEAN+=\
	vacuum/vacuum_solver\
	vacuum/vacuum_cs730\

EVERYTHING+=vacuum

VACUUMOBJS=\
	vacuum/vacuum.o\

include $(VACUUMOBJS:.o=.d)
CLEAN+=$(VACUUMOBJS)

vacuum/vacuum_solver:\
	$(VACUUMOBJS)\
	gridnav/gridmap.o\
	vacuum/main.cc\
	vacuum/main.d\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) -static $(VACUUMOBJS)\
		vacuum/main.cc\
		gridnav/gridmap.o\
		utils/utils.a\
		structs/structs.a\
		search/search.a\
		$(LDFLAGS) -DNCAKES=$*  -o $@

include vacuum/main.d

vacuum/vacuum_cs730:\
	$(VACUUMOBJS)\
	gridnav/gridmap.o\
	vacuum/main_cs730.cc\
	vacuum/main.d\
	utils/utils.a\
	structs/structs.a\
	search/search.a
	@echo $@
	@$(CXX) -static $(VACUUMOBJS)\
		vacuum/main_cs730.cc\
		gridnav/gridmap.o\
		utils/utils.a\
		structs/structs.a\
		search/search.a\
		$(LDFLAGS) -DNCAKES=$*  -o $@

include vacuum/main_cs730.d
