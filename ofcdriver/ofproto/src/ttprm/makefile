# File name:  makefile
# Author: Freescale Semiconductor    
# Description:

include $(cntrl_dir)/rules.mk

$(BIN_DIR)/%.o: %.c 
	$(CC) -c -Werror -o $@ $< $(CFLAGS)

TARGET = $(BIN_DIR)/ttprm.o

all: $(TARGET) 


OBJS = 	$(BIN_DIR)/ttprm_api.o\
	$(BIN_DIR)/ttprm_init.o\
	$(BIN_DIR)/ttprm_cbk.o
	



$(TARGET) : $(OBJS) 
	$(LD) -r $(OBJS) -o $(TARGET) 

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
       

#OBJS = 	$(BIN_DIR)/crmapi.o\

