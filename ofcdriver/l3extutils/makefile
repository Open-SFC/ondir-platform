include $(cntrl_dir)/rules.mk


$(BIN_DIR)/%.o: %.c 
	$(CC) -c -Werror -o $@ $< $(CFLAGS)

SHARED_LIB = $(OF_LIB_DIR)/apps

ifneq (,$(findstring FSLX_COMMON_AND_L3_EXT_SUPPORT,$(CFLAGS)))
TARGET1 = $(SHARED_LIB)/s01-l3ext-utils.so
endif
TARGET2 = $(BIN_DIR)/l3ext-utils-target.o
all: $(TARGET1) $(TARGET2)

OBJS =  $(BIN_DIR)/of_l3ext_utils.o \
        $(BIN_DIR)/nicira_ext.o \
        $(BIN_DIR)/ipsec_of_extn.o \
        $(BIN_DIR)/qos_of_ext.o

ifneq (,$(findstring  FSLX_COMMON_AND_L3_EXT_SUPPORT,$(CFLAGS)))
$(TARGET1) : $(OBJS)
	$(CC) -shared -o $@ $(OBJS) -lm
endif
$(TARGET2) : $(OBJS)
	$(LD) -r $(OBJS) -o $(TARGET2)
clean:
	$(RM) $(TARGET1)
	$(RM) $(TARGET2)
	$(RM) $(OBJS)       


