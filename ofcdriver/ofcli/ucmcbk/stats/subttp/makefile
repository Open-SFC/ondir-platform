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

SHARED_LIB = $(OF_LIB_DIR)/mgmt
#
# Program Name
#

TARGET_LIB = $(SHARED_LIB)/s21-of-cntrl-ttp-stats.so
TARGET = $(MODSET_CBK_OBJ_DIR)/ucmttpstats.o

#
# Objects
#

OBJS += $(MODSET_CBK_OBJ_DIR)/subttpcbk.o
OBJS += $(MODSET_CBK_OBJ_DIR)/fwconnttpstats.o
OBJS += $(MODSET_CBK_OBJ_DIR)/arpttpstats.o
OBJS += $(MODSET_CBK_OBJ_DIR)/fwfwdttpstats.o
OBJS += $(MODSET_CBK_OBJ_DIR)/routettpstats.o
#OBJS += $(MODSET_CBK_OBJ_DIR)/aggregatestats.o

$(MODSET_CBK_OBJ_DIR)/%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)
 
all : $(TARGET)

ifneq (,$(findstring CNTRL_INFRA_LIB_SUPPORT,$(CFLAGS)))
all: $(TARGET) $(TARGET_LIB)
else
all: $(TARGET)
endif

$(TARGET): $(OBJS) 
	$(LD) $(LDOPTS) $(OBJS) -o $@

$(TARGET_LIB) : $(TARGET)
	$(CC) -shared -o $@ $(TARGET) -lm

clean:
	$(RM) $(OBJS) $(TARGET) $(TARGET_LIB)

