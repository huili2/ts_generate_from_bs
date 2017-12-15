DECODER_SRCDIR=codec/decoder
DECODER_CPP_SRCS=\
	$(DECODER_SRCDIR)/core/src/au_parser.cpp\
	$(DECODER_SRCDIR)/core/src/bit_stream.cpp\
	$(DECODER_SRCDIR)/core/src/cabac_decoder.cpp\
	$(DECODER_SRCDIR)/core/src/decode_slice.cpp\
	$(DECODER_SRCDIR)/core/src/decoder.cpp\
	$(DECODER_SRCDIR)/core/src/decoder_core.cpp\
	$(DECODER_SRCDIR)/core/src/decoder_data_tables.cpp\
	$(DECODER_SRCDIR)/core/src/fmo.cpp\
	$(DECODER_SRCDIR)/core/src/manage_dec_ref.cpp\
	$(DECODER_SRCDIR)/core/src/memmgr_nal_unit.cpp\
	$(DECODER_SRCDIR)/core/src/mv_pred.cpp\
	$(DECODER_SRCDIR)/core/src/parse_mb_syn_cabac.cpp\
	$(DECODER_SRCDIR)/core/src/parse_mb_syn_cavlc.cpp\
	$(DECODER_SRCDIR)/core/src/pic_queue.cpp\
	$(DECODER_SRCDIR)/plus/src/PlusExt.cpp\

DECODER_OBJS += $(DECODER_CPP_SRCS:.cpp=.$(OBJ))

OBJS += $(DECODER_OBJS)

$(DECODER_SRCDIR)/%.$(OBJ): $(DECODER_SRCDIR)/%.cpp
	$(QUIET_CXX)$(CXX) $(CFLAGS) $(CXXFLAGS) $(INCLUDES) $(DECODER_CFLAGS) $(DECODER_INCLUDES) -c $(CXX_O) $<

$(LIBPREFIX)decoder.$(LIBSUFFIX): $(DECODER_OBJS)
	$(QUIET)rm -f $@
	$(QUIET_AR)$(AR) $(AR_OPTS) $+

libraries: $(LIBPREFIX)decoder.$(LIBSUFFIX)
LIBRARIES += $(LIBPREFIX)decoder.$(LIBSUFFIX)
