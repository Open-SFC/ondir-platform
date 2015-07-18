
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

include rules.mk

all: build

makecli:
	make -w -C $(ofcli_dir)
makecminfra:
	make -w -C $(cminfra_dir) -f makefile.cntrl
makeutils:
	make -w -C $(utils_dir)
makel3utils:
	make -w -C $(l3extutils_dir)
makexml: 
	make -w -C $(ofxml_dir)
makeofproto: 
	make -w -C $(ofproto_dir)
makelxintegration:
#	make -w -C $(lx_integration_dir)
makesubttp: 
#ifneq ($(shell readlink -e $(sub_ttp_dir)),)
#	make -w -C $(sub_ttp_dir)
#endif
makesdrouterapp: 
#	make -w -C $(sd_router_app_dir)
makesampleapp:
#	make -w -C $(sample_app_dir)        
makecbenchapp:
#	make -w -C $(cbench_app_dir)
makelinuxiptables:
#	(cd $(linux_iptables_dir); ./configure --host=$(TARGET_HOST) --prefix=/usr/local; make; mkdir ipt-lx; make install DESTDIR=$(OF_IPT_LX_DIR)/ipt-lx; cd -)

makepythonucmbindings:
	make -w -C $(python_ucm_dir)

build: makecli makecminfra makeutils makel3utils makexml makeofproto makelxintegration makesubttp makepythonucmbindings

clean:
	make -C $(l3extutils_dir) clean
	make -C $(ofproto_dir) clean
	make -C $(utils_dir) clean
	make -C $(ofcli_dir) clean
	make -C $(ofxml_dir) clean
	make -C $(cminfra_dir) -f makefile.cntrl clean
#	make -w -C $(lx_integration_dir) clean
ifneq ($(shell readlink -e $(sub_ttp_dir)),)
#	make -w -C $(sub_ttp_dir) clean
endif
#	make -w -C $(sd_router_app_dir) clean
#	make -w -C $(sample_app_dir) clean
#	make -w -C $(cbench_app_dir) clean
#	(cd $(linux_iptables_dir); make clean; rm -rf ipt-lx; cd -)

cleanlxintegration:
	make -w -C $(lx_integration_dir) clean
image:
	mkdir $(cntrl_image_name)
	mkdir $(cntrl_image_name)/bin
	mkdir $(cntrl_image_name)/bin/lib
	mkdir $(cntrl_image_name)/conf
	mkdir $(cntrl_image_name)/app-conf
	mkdir $(cntrl_image_name)/certs
	mkdir $(cntrl_image_name)/html
	#mkdir $(cntrl_image_name)/ipt-lx
	cp ondirector-start.sh $(cntrl_image_name)/
	cp ondirector-install.sh $(cntrl_image_name)/
	cp ondirector-stop.sh $(cntrl_image_name)/
	cp -rf build/bin/*  $(cntrl_image_name)/bin/
	cp -rf $(OF_LIB_DIR)/*  $(cntrl_image_name)/bin/lib
#	cp -rf $(OF_IPT_LX_DIR)/ipt-lx/*  $(cntrl_image_name)/ipt-lx
	cp -rf $(cntrl_cminfra_lib_dir)/*  $(cntrl_image_name)/bin/lib
	cp -rf certs  $(cntrl_image_name)/
	cp -rf conf $(cntrl_image_name)/
	cp -rf ofcli/system/html/* $(cntrl_image_name)/html/
	tar czf ondirector-image-$(TARGET_PLATFORM).tar.gz $(cntrl_image_name)
	rm -rf $(cntrl_image_name)

imageinstall:
	mkdir $(cntrl_image_name)
	mkdir $(cntrl_image_name)/bin
	mkdir $(cntrl_image_name)/bin/lib
	mkdir $(cntrl_image_name)/conf
	mkdir $(cntrl_image_name)/app-conf
	mkdir $(cntrl_image_name)/certs
	mkdir $(cntrl_image_name)/html
	#mkdir $(cntrl_image_name)/ipt-lx
	cp ond-of-start.sh $(cntrl_image_name)/
	cp ond-of-stop.sh $(cntrl_image_name)/
	cp -rf build/bin/*  $(cntrl_image_name)/bin/
	cp -rf $(OF_LIB_DIR)/*  $(cntrl_image_name)/bin/lib
#	cp -rf $(OF_IPT_LX_DIR)/ipt-lx/*  $(cntrl_image_name)/ipt-lx
	cp -rf $(cntrl_cminfra_lib_dir)/*  $(cntrl_image_name)/bin/lib
	cp -rf certs  $(cntrl_image_name)/
	cp -rf conf $(cntrl_image_name)/
	cp -rf ofcli/system/html/* $(cntrl_image_name)/html/
	tar czf ondirector-image-$(TARGET_PLATFORM).tar.gz $(cntrl_image_name)
	sudo rm -rf $(ondir_platform_inst_dir)
	sudo mkdir -p $(ondir_platform_inst_dir)
	sudo cp -rf $(cntrl_image_name)/*  $(ondir_platform_inst_dir)
	sudo cp -rf $(cntrl_image_name)/bin/lib  $(ondir_platform_inst_dir)
	sudo rm -rf $(cntrl_image_name)/bin/lib
	rm -rf $(cntrl_image_name)
	
hdrinstall:
	sudo rm -rf $(ondir_platform_inst_dir)/include
	sudo mkdir  $(ondir_platform_inst_dir)/include
	sudo cp -f include/cntlr_error.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/cntlr_event.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/cntlr_system.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/cntrlappcmn.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/crmapi.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/dprm.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/mchash.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/memapi.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/nsrm.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/of_msgapi.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/of_multipart.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/oftype.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/of_utilities.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/timerapi.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/tsc_cli.h  $(ondir_platform_inst_dir)/include
	sudo cp -f include/ttprmgdef.h  $(ondir_platform_inst_dir)/include
	sudo mkdir -p $(ondir_platform_inst_dir)/include/cminfra/include/common
	sudo cp -f $(cminfra_dir)/include/common/cmdefs.h  $(ondir_platform_inst_dir)/include/cminfra/include/common
	sudo cp -f $(cminfra_dir)/include/common/cmerror.h  $(ondir_platform_inst_dir)/include/cminfra/include/common
	sudo cp -f $(cminfra_dir)/include/common/cmincld.h  $(ondir_platform_inst_dir)/include/cminfra/include/common
	sudo cp -f $(cminfra_dir)/include/common/cmmath.h  $(ondir_platform_inst_dir)/include/cminfra/include/common
	sudo cp -f $(cminfra_dir)/include/common/cmtype.h  $(ondir_platform_inst_dir)/include/cminfra/include/common
	sudo mkdir -p $(ondir_platform_inst_dir)/include/cminfra/include/lxos
	sudo cp -f $(cminfra_dir)/include/lxos/cmatmic.h  $(ondir_platform_inst_dir)/include/cminfra/include/lxos
	sudo cp -f $(cminfra_dir)/include/lxos/cmoslck.h  $(ondir_platform_inst_dir)/include/cminfra/include/lxos
	sudo cp -f $(cminfra_dir)/include/lxos/cmsock.h  $(ondir_platform_inst_dir)/include/cminfra/include/lxos
	sudo cp -f $(cminfra_dir)/include/lxos/lxoswrp.h  $(ondir_platform_inst_dir)/include/cminfra/include/lxos
	sudo mkdir -p $(ondir_platform_inst_dir)/include/cminfra/include/infra/dm
	sudo cp -f $(cminfra_dir)/include/infra/dm/dmgdef.h  $(ondir_platform_inst_dir)/include/cminfra/include/infra/dm
	sudo cp -f $(cminfra_dir)/include/infra/dm/dmgif.h  $(ondir_platform_inst_dir)/include/cminfra/include/infra/dm
	sudo mkdir -p $(ondir_platform_inst_dir)/include/cminfra/include/infra/je
	sudo cp -f $(cminfra_dir)/include/infra/je/jegdef.h  $(ondir_platform_inst_dir)/include/cminfra/include/infra/je
	sudo mkdir -p $(ondir_platform_inst_dir)/include/cminfra/include/infra/transport
	sudo cp -f $(cminfra_dir)/include/infra/transport/cmtransgdf.h  $(ondir_platform_inst_dir)/include/cminfra/include/infra/transport
	sudo mkdir -p $(ondir_platform_inst_dir)/include/cminfra/include/utils/dslib
	sudo cp -f $(cminfra_dir)/include/utils/dslib/cmdll.h  $(ondir_platform_inst_dir)/include/cminfra/include/utils/dslib
	sudo cp -f $(cminfra_dir)/include/utils/dslib/cmsll.h  $(ondir_platform_inst_dir)/include/cminfra/include/utils/dslib
	sudo cp -f $(cminfra_dir)/include/utils/dslib/cmmempol.h  $(ondir_platform_inst_dir)/include/cminfra/include/utils/dslib
	sudo mkdir -p $(ondir_platform_inst_dir)/include/cminfra/include/utils/netutil
	sudo cp -f $(cminfra_dir)/include/utils/netutil/cmnet.h  $(ondir_platform_inst_dir)/include/cminfra/include/utils/netutil
	sudo cp -f $(cminfra_dir)/include/utils/netutil/mbufutil.h  $(ondir_platform_inst_dir)/include/cminfra/include/utils/netutil
	sudo mkdir -p $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/cntlr_epoll.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/cntlr_list.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/cntlr_lock.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/cntlr_tls.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/cntlr_transprt.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/cntlr_types.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/cntrl_queue.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/controller.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/crmldef.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/crm_port_api.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/crm_vm_api.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/crm_vn_api.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/dc_vs_ttp.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/dprmldef.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/nsrmldef.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/oflog.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/of_nem.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/of_view.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/openflow.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo cp -f $(ofproto_dir)/src/include/port_status_mgr.h  $(ondir_platform_inst_dir)/include/ofproto/src/include
	sudo mkdir -p $(ondir_platform_inst_dir)/include/ucmcbk/cntlrucm/include
	sudo cp -f $(ofcli_dir)/ucmcbk/cntlrucm/include/cntrucminc.h  $(ondir_platform_inst_dir)/include/ucmcbk/cntlrucm/include/
	sudo cp -f $(ofcli_dir)/ucmcbk/cntlrucm/include/cntrucmssl.h  $(ondir_platform_inst_dir)/include/ucmcbk/cntlrucm/include/
	sudo mkdir -p $(ondir_platform_inst_dir)/include/ucmcbk/comm/include
	sudo cp -f $(ofcli_dir)/ucmcbk/comm/include/cmcntrlr.h  $(ondir_platform_inst_dir)/include/ucmcbk/comm/include/
	sudo mkdir -p $(ondir_platform_inst_dir)/include/ucmcbk/crm/include
	sudo cp -f $(ofcli_dir)/ucmcbk/crm/include/crmcntrucmcbk.h  $(ondir_platform_inst_dir)/include/ucmcbk/crm/include/
	sudo mkdir -p $(ondir_platform_inst_dir)/include/ucmcbk/dprm/include
	sudo cp -f $(ofcli_dir)/ucmcbk/dprm/include/cntrucmcbk.h  $(ondir_platform_inst_dir)/include/ucmcbk/dprm/include/
	sudo mkdir -p $(ondir_platform_inst_dir)/include/ucmcbk/includes/generated
	sudo cp -f $(ofcli_dir)/ucmcbk/includes/generated/appids.h  $(ondir_platform_inst_dir)/include/ucmcbk/includes/generated/
	sudo cp -f $(ofcli_dir)/ucmcbk/includes/generated/secappl.h  $(ondir_platform_inst_dir)/include/ucmcbk/includes/generated/
	sudo mkdir -p $(ondir_platform_inst_dir)/include/ucmcbk/includes/global
	sudo cp -f $(ofcli_dir)/ucmcbk/includes/global/cbcmninc.h  $(ondir_platform_inst_dir)/include/ucmcbk/includes/global/
	sudo cp -f $(ofcli_dir)/ucmcbk/includes/global/cbkerr.h  $(ondir_platform_inst_dir)/include/ucmcbk/includes/global/
	sudo cp -f $(ofcli_dir)/ucmcbk/includes/global/cbkldef.h  $(ondir_platform_inst_dir)/include/ucmcbk/includes/global/
	sudo cp -f $(ofcli_dir)/ucmcbk/includes/global/cmgutil.h  $(ondir_platform_inst_dir)/include/ucmcbk/includes/global/
	sudo cp -f $(ofcli_dir)/ucmcbk/includes/global/secappreg.h  $(ondir_platform_inst_dir)/include/ucmcbk/includes/global/
	sudo cp -f $(ofcli_dir)/ucmcbk/includes/global/cmreg.h  $(ondir_platform_inst_dir)/include/ucmcbk/includes/global/
	sudo mkdir -p $(ondir_platform_inst_dir)/include/third_party_opensrc/openssl-1.0.0/include/openssl
	sudo cp -f ext/openssl-1.0.0/include/openssl/ossl_typ.h  $(ondir_platform_inst_dir)/include/third_party_opensrc/openssl-1.0.0/include/openssl
	sudo mkdir -p $(ondir_platform_inst_dir)/include/third_party_opensrc/futex
	sudo cp -f ext/futex/usersem.h  $(ondir_platform_inst_dir)/include/third_party_opensrc/futex
	sudo cp -f ext/futex/i386.h  $(ondir_platform_inst_dir)/include/third_party_opensrc/futex
