GRIDSRC=\
	gridnav/gridmap.cc\
	gridnav/gridnav.cc\

GRIDHDR=\
	gridnav/gridmap.hpp\
	gridnav/gridnav.hpp\
	gridnav/closedlist.hpp\

gridnav/*.o: $(GRIDHDR)

BINS+=\
	gridnav/scenario_solver\
	gridnav/gridnav_solver\
	gridnav/2poly\

gridnav/scenario_solver: $(OBJS) $(HDRS) $(GRIDHDR) $(GRIDSRC)\
		gridnav/scenario.hpp gridnav/scenario.cc gridnav/scenario_main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(GRIDSRC)\
		gridnav/scenario.cc gridnav/scenario_main.cc -o gridnav/scenario_solver

gridnav/gridnav_solver: $(OBJS) $(HDRS) $(GRIDHDR) $(GRIDSRC) gridnav/gridnav_main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(GRIDSRC)\
		gridnav/gridnav_main.cc -o gridnav/gridnav_solver

gridnav/2poly: $(OBJS) $(HDRS) $(GRIDHDR) $(GRIDSRC) gridnav/2poly.cc visnav/visgraph.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(GRIDSRC)\
		visnav/visgraph.cc gridnav/2poly.cc -o gridnav/2poly