# Copyright  2012, 2013  Freescale Semiconductor
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#     Unless required by applicable law or agreed to in writing, software
#     distributed under the License is distributed on an "AS IS" BASIS,
#     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#     See the License for the specific language governing permissions and
#     limitations under the License.

# File name:  makefile
# Author: Freescale Semiconductor    
# Description:

include $(cntrl_dir)/rules.mk

$(BIN_DIR)/%.o: %.c 
	$(CC) -Werror -c -o $@ $< $(CFLAGS)

TARGET = $(BIN_DIR)/crm.o

all: $(TARGET) 


OBJS = 	$(BIN_DIR)/crm_vm_api.o\
 	$(BIN_DIR)/crm_vn_api.o\
 	$(BIN_DIR)/crm_port_api.o\
 	$(BIN_DIR)/crm_view_api.o\
 	$(BIN_DIR)/crm_tenant_api.o\
 	$(BIN_DIR)/crm_subnet_api.o\
 	$(BIN_DIR)/crmapi.o\
 	$(BIN_DIR)/crm_attribute.o\
 	$(BIN_DIR)/crm_view_init.o

$(TARGET) : $(OBJS) 
	$(LD) -r $(OBJS) -o $(TARGET) 

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
       

#OBJS = 	$(BIN_DIR)/crmapi.o\

