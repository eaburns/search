rdb:\
	rdb/rdb.a\
	rdb/pathfor\
	rdb/withattrs\
	rdb/dfcmplt\
	rdb/pathattrs\
	rdb/addattrs

RDBOBJS:=\
	rdb/rdb.o\

include $(RDBOBJS:.o=.d)

CLEAN+=$(RDBOBJS)

EVERYTHING+=rdb

TESTS+=.rdb_test

.rdb_test: ./rdb/test
	./rdb/test ".*test" && touch $@
CLEAN+=.rdb_test

rdb/rdb.a: $(RDBOBJS)
	@echo $@
	@$(AR) rsc $@ $(RDBOBJS)
CLEAN+=rdb/rdb.a

rdb/test:\
	rdb/test.cc\
	rdb/test_rdb.cc\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/test

rdb/pathfor:\
	rdb/pathfor.cc\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/pathfor

rdb/withattrs:\
	rdb/withattrs.cc\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/withattrs

rdb/dfcmplt:\
	rdb/dfcmplt.cc\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/dfcmplt

rdb/pathattrs:\
	rdb/pathattrs.cc\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/pathattrs

rdb/addattrs:\
	rdb/addattrs.cc\
	rdb/rdb.a\
	utils/utils.a
	@echo $@
	@$(CXX) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/addattrs
