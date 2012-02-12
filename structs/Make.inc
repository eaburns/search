STRUCTSOBJS:=\
	structs/stn.o\

include $(STRUCTSOBJS:.o=.d)

structs: structs/structs.a structs/test

EVERYTHING+=structs

TESTS+=.structs_test

.structs_test: structs/binheap.hpp ./structs/test
	./structs/test ".*test" && touch $@

CLEAN+=.structs_test

structs/test:\
	structs/test_htable.cc\
	structs/test_intpq.cc\
	structs/test_binheap.cc\
	structs/test_stn.cc\
	structs/test.cc\
	utils/utils.a\
	structs/structs.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

structs/structs.a: $(STRUCTSOBJS)
	$(AR) rsc $@ $(STRUCTSOBJS)

CLEAN+=$(STRUCTSOBJS) structs/test structs/structs.a
