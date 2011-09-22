BINS+=\
	structs/test\

structs/test: $(OBJS) $(HDRS)\
	structs/test_htable.cc\
	structs/test_intpq.cc\
	structs/test_binheap.cc\
	structs/test.cc
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS)\
		structs/test.cc\
		structs/test_htable.cc\
		structs/test_intpq.cc\
		structs/test_binheap.cc\
		-o structs/test