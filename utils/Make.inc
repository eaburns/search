utils/%.o: incl/utils.hpp

utils/image.o: utils/image.hpp

LDFLAGS+=-lboost_filesystem

BINS+=\
	utils/test\

utils/test: $(OBJS) $(HDRS) utils/test_geom.cc utils/test.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o utils/test\
		utils/test_geom.cc utils/test.cc\
		