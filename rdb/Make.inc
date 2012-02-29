LDFLAGS+=\
	/usr/lib/libboost_filesystem.a\
	/usr/lib/libboost_filesystem-mt.a\
	/usr/lib/libboost_system.a\
	/usr/lib/libboost_system-mt.a\

RDBOBJS:=\
	rdb/rdb.o\

include $(UTILOBJS:.o=.d)

CLEAN+=$(UTILOBJS)

rdb: rdb/pathfor rdb/withattrs rdb/dfcmplt

EVERYTHING+=rdb

TESTS+=.rdb_test

.rdb_test: ./rdb/test
	./rdb/test ".*test" && touch $@
CLEAN+=.rdb_test

rdb/test: rdb/test.cc rdb/test_rdb.cc $(RDBOBJS) utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/test

rdb/pathfor: rdb/pathfor.cc $(RDBOBJS) utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/pathfor

rdb/withattrs: rdb/withattrs.cc $(RDBOBJS) utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/withattrs

rdb/dfcmplt: rdb/dfcmplt.cc $(RDBOBJS) utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/dfcmplt
