rdb: rdb/pathfor rdb/withattrs rdb/dfcmplt

EVERYTHING+=rdb

rdb/pathfor: rdb/pathfor.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/pathfor

rdb/withattrs: rdb/withattrs.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/withattrs

rdb/dfcmplt: rdb/dfcmplt.cc utils/utils.a
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
CLEAN+=rdb/dfcmplt
