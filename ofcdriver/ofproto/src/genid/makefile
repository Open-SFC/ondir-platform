# File name:  makefile
# Author: Freescale Semiconductor    
# Description:

include $(cntrl_dir)/rules.mk

$(BIN_DIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

TARGET = $(BIN_DIR)/genid_app.o

all: $(TARGET)


OBJS =  $(BIN_DIR)/genid.o


$(TARGET) : $(OBJS)
	$(LD) -r $(OBJS) -o $(TARGET) 

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)

