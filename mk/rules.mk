


.SUFFIXES: .out .c .cc .o .h 


.cc.o: 
	$(CXX) $(CXXFLAGS) -c $< -o $@  $(LIBS)
