BINS+=\
	gridpath/scenario_solver\
	gridpath/gridpath_solver\

GRIDSRC=\
	gridpath/gridmap.cc\
	gridpath/gridpath.cc\
	gridpath/scenario.cc\

GRIDHDR=\
	gridpath/gridmap.hpp\
	gridpath/gridpath.hpp\
	gridpath/scenario.hpp\

gridpath/*.o: $(GRIDHDR)

gridpath/scenario_solver: $(OBJS) $(HDRS) $(GRIDHDR) $(GRIDSRC) gridpath/scenario_main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(GRIDSRC)\
		gridpath/scenario_main.cc -o gridpath/scenario_solver

gridpath/gridpath_solver: $(OBJS) $(HDRS) $(GRIDHDR) $(GRIDSRC) gridpath/gridpath_main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(GRIDSRC)\
		gridpath/gridpath_main.cc -o gridpath/gridpath_solver