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
# Date:   03/15/2013
# Description:

UCMCBK_DIR= $(ofcli_dir)/ucmcbk

include $(UCMCBK_DIR)/rules.mod

#
# Program Name
#

SHARED_LIB = $(OF_LIB_DIR)/mgmt

TARGET_LIB = $(SHARED_LIB)/s01-of-cntrl-nem-mgmt.so
TARGET = $(MODSET_CBK_OBJ_DIR)/ucmnemcbk.o

#
# Objects
#

$(MODSET_CBK_OBJ_DIR)/%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)
 
OBJS = $(MODSET_CBK_OBJ_DIR)/ns2nsidcbk.o
OBJS += $(MODSET_CBK_OBJ_DIR)/nsid2nscbk.o
OBJS += $(MODSET_CBK_OBJ_DIR)/dp2nsidcbk.o
OBJS += $(MODSET_CBK_OBJ_DIR)/nsid2dpcbk.o
OBJS += $(MODSET_CBK_OBJ_DIR)/dp2pethcbk.o
OBJS += $(MODSET_CBK_OBJ_DIR)/peth2dpcbk.o
OBJS += $(MODSET_CBK_OBJ_DIR)/nemcbkinit.o

all : $(TARGET)

ifneq (,$(findstring CNTRL_INFRA_LIB_SUPPORT,$(CFLAGS)))
#all: $(TARGET) $(TARGET_LIB)
all: $(TARGET) 
else
all: $(TARGET)
endif

$(TARGET): $(OBJS) 
	$(LD) $(LDOPTS) $(OBJS) -o $@

$(TARGET_LIB) : $(TARGET)
	$(CC) -shared -o $@ $(TARGET) -lm

clean:
	$(RM) $(OBJS) $(TARGET) $(TARGET_LIB)

