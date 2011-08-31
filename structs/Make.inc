BINS+=\
	structs/struct_test\

structs/struct_test: $(OBJS) $(HDRS)\
	structs/test_htable.cc\
	structs/test.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS)\
		structs/test.cc\
		structs/test_htable.cc\
		-o structs/struct_test