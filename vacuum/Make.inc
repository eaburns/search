vacuum:\
	vacuum/vacuum_solver\

CLEAN+=\
	vacuum/vacuum_solver

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
