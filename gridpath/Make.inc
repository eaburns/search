BINS+=\
	gridpath/gridpathmd_solver\

GRIDSRC=\
	gridpath/gridmap.cc\
	gridpath/gridpath.cc\
	gridpath/scenario.cc\

GRIDHDR=\
	gridpath/gridmap.hpp\
	gridpath/gridpath.hpp\
	gridpath/scenario.hpp\

gridpath/*.o: $(GRIDHDR)

gridpath/gridpathmd_solver: $(OBJS) $(HDRS) $(GRIDHDR) $(GRIDSRC) gridpath/main.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(GRIDSRC)\
		gridpath/main.cc -o gridpath/gridpathmd_solver