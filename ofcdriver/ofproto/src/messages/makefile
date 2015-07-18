include $(cntrl_dir)/rules.mk

$(BIN_DIR)/%.o: %.c 
	$(CC) -c -Wall -o $@ $< $(CFLAGS)

TARGET = $(BIN_DIR)/messages.o

all: $(TARGET) 

OBJS = 	$(BIN_DIR)/of_cntlr_msg.o\
 	$(BIN_DIR)/of_symmetric_msgapi.o\
 	$(BIN_DIR)/of_cntlr_2_sw_msgapi.o\
 	$(BIN_DIR)/of_async_msgapi.o\
 	$(BIN_DIR)/of_msg_utils.o\
 	$(BIN_DIR)/of_msgreq.o\
 	$(BIN_DIR)/of_group.o\
 	$(BIN_DIR)/of_queue.o\
 	$(BIN_DIR)/of_meter.o\
 	$(BIN_DIR)/of_flow.o\
 	$(BIN_DIR)/of_tmpl_utils.o
 	#$(BIN_DIR)/of_multipart.o

$(TARGET) : $(OBJS) 
	$(LD) -r $(OBJS) -o $(TARGET) 

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
       


