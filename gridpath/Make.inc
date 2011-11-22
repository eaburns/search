GRIDSRC=\
	gridpath/gridmap.cc\
	gridpath/gridpath.cc\

GRIDHDR=\
	gridpath/gridmap.hpp\
	gridpath/gridpath.hpp\
	gridpath/closedlist.hpp\

gridpath/*.o: $(GRIDHDR)

BINS+=\
	gridpath/scenario_solver\
	gridpath/gridpath_solver\
	gridpath/2poly\

gridpath/scenario_solver: $(OBJS) $(HDRS) $(GRIDHDR) $(GRIDSRC)\
		gridpath/scenario.hpp gridpath/scenario.cc gridpath/scenario_main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(GRIDSRC)\
		gridpath/scenario.cc gridpath/scenario_main.cc -o gridpath/scenario_solver

gridpath/gridpath_solver: $(OBJS) $(HDRS) $(GRIDHDR) $(GRIDSRC) gridpath/gridpath_main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(GRIDSRC)\
		gridpath/gridpath_main.cc -o gridpath/gridpath_solver

gridpath/2poly: $(OBJS) $(HDRS) $(GRIDHDR) $(GRIDSRC) gridpath/2poly.cc visnav/visgraph.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(GRIDSRC)\
		visnav/visgraph.cc gridpath/2poly.cc -o gridpath/2poly