


.SUFFIXES: .out .c .cc .o .h 


.cc.o: 
	$(CPP) $(CPPFLAGS) -c $< -o $@  $(INCLUDE) $(LIB)
