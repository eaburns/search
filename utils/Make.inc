utils/%.o: incl/utils.hpp

utils/image.o: utils/image.hpp

LDFLAGS+=-lboost_filesystem

BINS+=\
	utils/test\
	utils/pathfor/

utils/test: $(OBJS) $(HDRS) utils/test_geom.cc utils/test_rdb.cc utils/test.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o utils/test\
		utils/test_geom.cc\
		utils/test_rdb.cc\
		utils/test.cc

utils/pathfor: $(OBJS) $(HDRS) utils/pathfor.cc
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) utils/pathfor.cc -o utils/pathfor