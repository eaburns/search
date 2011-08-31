BINS+=\
	structs/struct_test\

structs/struct_test: $(OBJS) $(HDRS) structs/test.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) structs/test.cc -o structs/struct_test