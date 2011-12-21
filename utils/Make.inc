LDFLAGS+=\
	/usr/lib/libboost_filesystem.a\
	/usr/lib/libboost_filesystem-mt.a\
	/usr/lib/libboost_system.a\
	/usr/lib/libboost_system-mt.a\

UTILOBJS:=\
	utils/fatal.o\
	utils/time.o\
	utils/datafile.o\
	utils/rdb.o\
	utils/testing.o\
	utils/hash.o\
	utils/rand.o\
	utils/encode.o\
	utils/image.o\
	utils/geom.o\
	utils/rank.o\
	utils/math.o\
	utils/mem.o\

include $(UTILOBJS:.o=.d)

CLEAN+=$(UTILOBJS)

utils: utils/utils.a utils/test utils/pathfor utils/withattrs

EVERYTHING+=utils

TESTS+=.utils_test

.utils_test: ./utils/test
	./utils/test ".*test" && touch $@
CLEAN+=.utils_test

utils/utils.a: $(UTILOBJS)
	$(AR) rsc $@ $(UTILOBJS)
CLEAN+=utils.a

utils/test: utils/test_geom.cc utils/test_rdb.cc utils/test.cc\
	utils/test_encode.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=utils/test

utils/pathfor: utils/pathfor.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=utils/pathfor

utils/withattrs: utils/withattrs.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=utils/withattrs