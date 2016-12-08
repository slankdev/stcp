


.SUFFIXES: .out .c .cc .o .h
.cc.o:
	@echo " CXX $@"
	@$(CXX) $(CXXFLAGS) -c -g $< -o $@


$(TARGET): $(OBJS)
	@echo " LD $@"
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)


