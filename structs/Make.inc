structs:\
	structs/structs.a\
	structs/test

STRUCTSOBJS:=\
	structs/stn.o\

include $(STRUCTSOBJS:.o=.d)

EVERYTHING+=structs

TESTS+=.structs_test

.structs_test: ./structs/test
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
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

structs/structs.a:\
	$(STRUCTSOBJS)
	@echo $@
	@$(AR) rsc $@ $(STRUCTSOBJS)

CLEAN+=$(STRUCTSOBJS) structs/test structs/structs.a
