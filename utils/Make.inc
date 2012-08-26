utils:\
	utils/utils.a\
	utils/test

UTILOBJS:=\
	utils/fatal.o\
	utils/time.o\
	utils/datafile.o\
	utils/testing.o\
	utils/hash.o\
	utils/rand.o\
	utils/encode.o\
	utils/geom2d.o\
	utils/rank.o\
	utils/math.o\
	utils/mem.o\
	utils/fs.o\
	utils/str.o\

include $(UTILOBJS:.o=.d)

CLEAN+=$(UTILOBJS)

EVERYTHING+=utils

TESTS+=.utils_test

.utils_test: ./utils/test
	./utils/test ".*test" && touch $@
CLEAN+=.utils_test

utils/utils.a:\
	$(UTILOBJS)
	@ echo $@
	@ $(AR) rsc $@ $(UTILOBJS)

CLEAN+=utils/utils.a

utils/test:\
	utils/test_geom.cc\
	utils/test_encode.cc\
	utils/bench_ops.cc\
	utils/test_misc.cc\
	utils/test_fs.cc\
	utils/test.cc\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

CLEAN+=utils/test
