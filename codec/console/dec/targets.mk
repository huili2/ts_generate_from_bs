TESTCONSOLE_SRCDIR=codec/console/dec
TESTCONSOLE_CPP_SRCS=\
	$(TESTCONSOLE_SRCDIR)/src/testConsole.cpp\

TESTCONSOLE_OBJS += $(TESTCONSOLE_CPP_SRCS:.cpp=.$(OBJ))

OBJS += $(TESTCONSOLE_OBJS)

$(TESTCONSOLE_SRCDIR)/%.$(OBJ): $(TESTCONSOLE_SRCDIR)/%.cpp
	$(QUIET_CXX)$(CXX) $(CFLAGS) $(CXXFLAGS) $(INCLUDES) $(TESTCONSOLE_CFLAGS) $(TESTCONSOLE_INCLUDES) -c $(CXX_O) $<

testConsole$(EXEEXT): $(TESTCONSOLE_OBJS) $(TESTCONSOLE_DEPS)
	$(QUIET_CXX)$(CXX) $(CXX_LINK_O) $(TESTCONSOLE_OBJS) $(TESTCONSOLE_LDFLAGS) $(LDFLAGS)

binaries: testConsole$(EXEEXT)
BINARIES += testConsole$(EXEEXT)
