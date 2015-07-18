
#controlle image name
cntrl_image_name = ondirector-install

#image installation directory 
ondir_platform_inst_dir = /usr/local/ondir_platform/2.0

#working directory
of_dir = $(PWD)
cntrl_dir = ${of_dir}

#For callbacks
ofcli_dir = ${cntrl_dir}/ofcli

#For Python UCM Bindings
python_ucm_dir = ${cntrl_dir}/python-ucm

#For cminfra
cminfra_dir = ${cntrl_dir}/../cminfra
cntrl_cminfra_obj_dir =${cntrl_dir}/build/ucmobj
cntrl_cminfra_lib_dir =${cntrl_dir}/build/ucmobj/$(LIB_BOARD_DIR)

#controller directories
#========================
ext_dir = ${of_dir}/ext
l3extutils_dir = ${cntrl_dir}/l3extutils
ofproto_dir = ${cntrl_dir}/ofproto
utils_dir = ${cntrl_dir}/utils
ofxml_dir = ${cntrl_dir}/xml

#sub-ttps directory
#====================
sub_ttp_dir = $(cntrl_dir)/../cmn-sub-ttps

#linux-intergrations directory
#====================
lx_integration_dir = $(cntrl_dir)/../linux-integration
api_dispatcher_dir = $(cntrl_dir)/../linux-integration/api-dispatcher

#sd-router-app direcotry
#===========================
#sd_router_app_dir = $(cntrl_dir)/../sdrouter-app
#linux_iptables_dir = $(sd_router_app_dir)/linux-integration/ext/iptables-1.4.21

#sample-app direcotry
#===========================
#sample_app_dir = $(cntrl_dir)/../sample-app
tsc_app_dir = $(cntrl_dir)/../tsc-app

#cbench-app direcotry
#===========================
#cbench_app_dir = $(cntrl_dir)/../cbench


CM_DM_DIR= $(ofcli_dir)
CM_INFRA_DIR= $(cminfra_dir)

ifeq ($(SDK_VERSION), V1.6)
  CFLAGS += -DCONFIG_SDK_VER_1_6
endif
ifeq ($(SDK_VERSION), V1.5)
  CFLAGS += -DCONFIG_SDK_VER_1_5
endif
ifeq ($(SDK_VERSION), V1.4)
  CFLAGS += -DCONFIG_SDK_VER_1_4
endif
ifeq ($(SDK_VERSION), V1.3)
  CFLAGS += -DCONFIG_SDK_VER_1_3
endif
ifeq ($(TARGET_PLATFORM),p4080ds)
  CFLAGS += -DCONFIG_BOARD_P4080
endif
ifeq ($(TARGET_PLATFORM),p2041rdb)
  CFLAGS += -DCONFIG_BOARD_P2041
endif
ifeq ($(TARGET_PLATFORM),t4240rdb)
  CFLAGS += -DCONFIG_BOARD_T4240
endif

#controller includes
INCLUDE += -I $(cntrl_dir)/include\
          -I ${l3extutils_dir}/include\
          -I $(ofproto_dir)/src/include\
          -I $(tsc_app_dir)/src/include\
          -I $(cntrl_dir)/ofcli/ucmcbk/includes/global\
          -I ${ext_dir}/openssl-1.0.0/include\
          -I ${ext_dir}/openssl-1.0.0/include/openssl\
          -I ${ext_dir}/urcu\
          -I ${ext_dir}/futex\

#cleaner approach:
INCLUDE += -I $(SYSROOT_PATH)/usr/include
INCLUDE += -I $(SYSROOT_USR_LIB_PATH)
#INCLUDE += -I $(SYSROOT_EXTN_PATH)
INCLUDE += -I $(ofproto_dir)/src/include

CFLAGS += $(INCLUDE) -Wall -D_GNU_SOURCE -g -O
OF_NCURSES_LIB = -L$(OF_LIB_DIR) -lncurses

BIN_DIR = $(cntrl_dir)/build/obj
OSPKG_BIN_DIR = $(cntrl_dir)/build/bin
OF_LIB_DIR = ${cntrl_dir}/build/$(LIB_BOARD_DIR)
#OF_IPT_LX_DIR = ${cntrl_dir}/../sdrouter-app/linux-integration/ext/iptables-1.4.21

#export of_dir cntrl_dir ext_dir cminfra_dir l3extutils_dir ofcli_dir ofproto_dir utils_dir OF_LIB_DIR OF_NCURSES_LIB OSPKG_BIN_DIR OPENSSL_CRYPTO_LIB OPENSSL_SSL_LIB cntrl_cminfra_obj_dir cntrl_cminfra_lib_dir BIN_DIR cntrl_image_name sub_ttp_dir sd_router_app_dir
export of_dir cntrl_dir ext_dir cminfra_dir l3extutils_dir ofcli_dir ofproto_dir utils_dir OF_LIB_DIR OF_NCURSES_LIB OSPKG_BIN_DIR OPENSSL_CRYPTO_LIB OPENSSL_SSL_LIB cntrl_cminfra_obj_dir cntrl_cminfra_lib_dir BIN_DIR cntrl_image_name tsc_app_dir 

LIBS += -L $(OF_LIB_DIR) -lurcu-mb -lurcu-common -lurcu
LIBS += -L $(OF_LIB_DIR)/apps
LIBS += -lpthread
LIBS += -lrt
LIBS += -ldl
LIBS += -rdynamic
LIBS += -lcrypto
LIBS += -lssl 

MGMT_FLAGS= -DCNTRL_OFCLI_SUPPORT

APP_FLAGS = -DFSLX_COMMON_AND_L3_EXT_SUPPORT
APP_FLAGS += -DNICIRA_EXT_SUPPORT
#APP_FLAGS += -DOPENFLOW_IPSEC_SUPPORT
#APP_FLAGS += -DOPENFLOW_IPSEC_DEBUG_SUPPORT
#APP_FLAGS += -DOPENFLOW_FIREWALL_SUPPORT

#APP_FLAGS += -DCNTRL_CBENCH_SUPPORT
#APP_FLAGS += -DCNTRL_APP_THREAD_SUPPORT
#APP_FLAGS += -DCNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT
#APP_FLAGS += -DCNTRL_IPV4_MULTICAST_SUPPORT
#APP_FLAGS += -DCNTRL_IPV4_UNICAST_SUPPORT
#APP_FLAGS += -DCNTRL_IPV4_FIREWALL_SUPPORT
#APP_FLAGS += -DCNTRL_FW4_CONNECTION_TEMPLATES_SUPPORT
#APP_FLAGS += -DCNTRL_IPV4_FWD_PBR_SUPPORT
#APP_FLAGS += -DCNTRL_ARP_APP_SUPPORT

#APP_FLAGS += -DFSLX_QOS_SUPPORT

#APP_FLAGS += -DOF_CONTROLLER_CONNTRACK_SUPPORT
#APP_FLAGS += -DOF_CONTROLLER_IPT_ACL_SUPPORT
CFLAGS += $(MGMT_FLAGS)
CFLAGS += $(APP_FLAGS)
CFLAGS += -g
CFLAGS += -DCONFIG_VQA_PSP_KERNEL_PATCH
#CFLAGS += -DSYSLOG_SERVER_SUPPORT
CFLAGS += -DCNTLR_CRM_SUPPORT
CFLAGS += -DCNTLR_NSRM_SUPPORT
CFLAGS += -DOF_XML_SUPPORT
CFLAGS += -fPIC
INFRA_FLAGS += -DCNTRL_INFRA_LIB_SUPPORT
CFLAGS += $(INFRA_FLAGS)
ifneq (,$(findstring CNTRL_OFCLI_SUPPORT,$(CFLAGS)))
MGMT_LIB=-lcntrlucmcbk
endif
export MGMT_LIB


