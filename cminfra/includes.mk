CM_INCLD_DIR  = $(CM_DIR)/include

INCLUDES = \
		-I $(CM_INCLD_DIR)/common \
		-I $(CM_INCLD_DIR)/infra \
		-I $(CM_INCLD_DIR)/infra/dm \
		-I $(CM_INCLD_DIR)/infra/dm/include \
		-I $(cminfra_dir)/infra/dm/include \
		-I $(cminfra_dir)/ldsv/include \
		-I $(cminfra_dir)/include/common \
		-I $(cminfra_dir)/infra/je/include \
		-I $(cminfra_dir)/infra/transport/include \
		-I $(cminfra_dir)/cli/include \
		-I $(cminfra_dir)/utils/dslib/include \
                -I $(cminfra_dir)/utils/netutil/include\
                -I $(cminfra_dir)/utils/timer/include\
                -I $(cminfra_dir)/utils/auth/include\
                -I $(cminfra_dir)/lxos/include\
		-I $(CM_INCLD_DIR)/infra/je\
		-I $(CM_INCLD_DIR)/infra/transport \
		-I $(CM_INCLD_DIR)/lxos \
		-I $(CM_INCLD_DIR)/utils \
		-I $(CM_INCLD_DIR)/utils/common \
		-I $(CM_INCLD_DIR)/utils/dslib \
		-I $(CM_INCLD_DIR)/utils/netutil \
		-I $(CM_INCLD_DIR)/utils/auth\
		-I $(cminfra_dir)/http/cfg \
		-I $(cminfra_dir)/http/include \
		-I $(cminfra_dir)/http/include/lx \
		-I $(cminfra_dir)/http/include/httpscrt \
		-I $(ext_dir)/openssl-1.0.0/include/ \
		-I $(CM_INCLD_DIR)/utils/timerlib 

#		-I $(ext_dir)/openssl-1.0.0/include/openssl \

    INCLUDES += \
  		-I$(SYSROOT_PATH)/usr/include \
  		-I$(SYSROOT_USR_LIB_PATH)\
#  		-I$(SYSROOT_EXTN_PATH)

CFLAGS += $(INCLUDES)
