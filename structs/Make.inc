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

STRUCTSTESTOBJS=\
	structs/test_htable.o\
	structs/test_intpq.o\
	structs/test_binheap.o\
	structs/test_stn.o\
	structs/test_kdtree.o\

structs/test:\
	$(STRUCTSTESTOBJS)\
	structs/test.cc\
	utils/utils.a\
	structs/structs.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@

include $(STRUCTSTESTOBJS:.o=.d)

structs/structs.a:\
	$(STRUCTSOBJS)
	@echo $@
	@$(AR) rsc $@ $(STRUCTSOBJS)

CLEAN+=$(STRUCTSOBJS) structs/test structs/structs.a
