COMMON_SRCDIR=codec/common
COMMON_CPP_SRCS=\
	$(COMMON_SRCDIR)/src/common_tables.cpp\
	$(COMMON_SRCDIR)/src/crt_util_safe_x.cpp\
	$(COMMON_SRCDIR)/src/memory_align.cpp\
	$(COMMON_SRCDIR)/src/utils.cpp\
	$(COMMON_SRCDIR)/src/PlusTrace.cpp\
	$(COMMON_SRCDIR)/src/WelsTaskThread.cpp\
	$(COMMON_SRCDIR)/src/WelsThread.cpp\
	$(COMMON_SRCDIR)/src/WelsThreadLib.cpp\
	$(COMMON_SRCDIR)/src/WelsThreadPool.cpp\

COMMON_OBJS += $(COMMON_CPP_SRCS:.cpp=.$(OBJ))

OBJS += $(COMMON_OBJS)

$(COMMON_SRCDIR)/%.$(OBJ): $(COMMON_SRCDIR)/%.cpp
	$(QUIET_CXX)$(CXX) $(CFLAGS) $(CXXFLAGS) $(INCLUDES) $(COMMON_CFLAGS) $(COMMON_INCLUDES) -c $(CXX_O) $<

$(LIBPREFIX)common.$(LIBSUFFIX): $(COMMON_OBJS)
	$(QUIET)rm -f $@
	$(QUIET_AR)$(AR) $(AR_OPTS) $+

libraries: $(LIBPREFIX)common.$(LIBSUFFIX)
LIBRARIES += $(LIBPREFIX)common.$(LIBSUFFIX)
