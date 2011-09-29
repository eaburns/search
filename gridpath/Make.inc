BINS+=\
	gridpath/gridpathmd_solver\

GRIDSRC=\
	gridpath/gridmap.cc\
	gridpath/gridpath.cc\

GRIDHDR=\
	gridpath/gridmap.hpp\
	gridpath/gridpath.hpp\

gridpath/gridpathmd_solver: $(OBJS) $(HDRS) $(GRIDHDR) $(GRIDSRC) gridpath/main.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(GRIDSRC)\
		gridpath/main.cc -o gridpath/gridpathmd_solver