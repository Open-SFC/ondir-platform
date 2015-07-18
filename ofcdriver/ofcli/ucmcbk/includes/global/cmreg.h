/* 
 *
 * Copyright  2012, 2013  Freescale Semiconductor
 *
 *
 * Licensed under the Apache License, version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
*/

/*
 *
 * File name: ucmreg.h
 * Author: G Atmaram <B38856@freescale.com>
 * Date:   03/25/2013
 * Description: 
 * 
 */

#ifndef __CM_REGINI_H
#define __CM_REGINI_H

#define CM_CBK_REG_DEBUG_PRINT printf("\n%35s(%4d): ",__FUNCTION__,__LINE__),\
                            printf


int32_t of_portstats_ucm_getparams (struct ofi_port_stats_info *port_info_p,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t dprm_datapath_asyncmsgcfg_ucm_getparms (struct ofp_async_config   *of_async_config_msg,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t cntrl_ucm_sendresponse(struct sockaddr_in *peer_address, char *msg, uint32_t msg_len);
int32_t ucm_cntrl_create_conn();
int32_t of_tablestats_ucm_getparams (struct ofi_table_stats_info *table_info_p,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_flowstats_ucm_getparams (struct ofi_flow_entry_info *stats_info,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_aggrstats_ucm_getparams (struct ofi_aggregate_flow_stats *stats_info_p,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_group_ucm_getparams (struct ofi_group_desc_info *group_entry_p,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_group_stats_ucm_getparams (struct ofi_group_stat *group_stats_p,
                struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_group_features_ucm_getparams (struct ofi_group_features_info *group_features_p,
                struct cm_array_of_iv_pairs * result_iv_pairs_p);
 int32_t of_tblfeatures_ucm_getparams (struct ofi_table_features_info *table_info,
                        struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_ipopts_getparams (struct ofl_switch_config *of_ipopts_info,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_swinfo_ucm_getparams (struct ofi_switch_info *swinfo_entry_p,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_meter_ucm_getparams (struct ofi_meter_rec_info *meter_entry_p,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_meter_features_ucm_getparams (struct ofi_meter_features_info *meter_features_p,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_meter_stats_ucm_getparams (struct ofi_meter_stats_info *meter_stats_p,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);
int32_t of_bindstats_ucm_getparams (struct ofi_bind_entry_info *stats_info,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);

int32_t cntrl_ucm_sendresponse(struct sockaddr_in *peer_address, char *msg, uint32_t msg_len);
int32_t ucm_cntrl_app_init(void);
int32_t dprm_datapath_port_appl_ucmcbk_init (void);
int32_t dprm_datapath_dpattribute_appl_ucmcbk_init (void);
int32_t dprm_switch_attribute_appl_ucmcbk_init (void);
int32_t dprm_domain_attribute_appl_ucmcbk_init (void);
int32_t dprm_datapath_port_attribute_appl_ucmcbk_init (void);
int32_t dprm_datapath_channels_appl_cmcbk_init (void);
int32_t dprm_datapath_auxchannels_appl_cmcbk_init (void);
int32_t of_swinfo_appl_cmcbk_init (void);
int32_t of_mempoolstats_appl_ucmcbk_init (void);
int32_t of_portstats_appl_ucmcbk_init (void);
int32_t of_tablestats_appl_ucmcbk_init (void);
int32_t of_flowstats_appl_ucmcbk_init (void);
int32_t of_bindstats_appl_ucmcbk_init (void);
int32_t of_aggrstats_appl_ucmcbk_init (void);
int32_t  of_ipopts_appl_ucmcbk_init(void);  
int32_t	of_cntlrrole_appl_ucmcbk_init(void);
int32_t of_asyncmsgcfg_appl_ucmcbk_init(void);	
int32_t  of_tblfeatures_appl_ucmcbk_init(void);
int32_t  of_group_appl_ucmcbk_init(void);
int32_t  of_bucket_appl_ucmcbk_init(void);
int32_t  of_action_appl_ucmcbk_init(void);
int32_t  of_group_stats_appl_ucmcbk_init(void);
int32_t  of_group_features_appl_ucmcbk_init(void);
int32_t  of_meter_appl_ucmcbk_init(void);
int32_t  of_meter_band_appl_ucmcbk_init(void);
int32_t  of_meter_stats_appl_ucmcbk_init(void);
int32_t  of_meter_features_appl_ucmcbk_init(void);

int32_t  of_ssl_appl_ucmcbk_init(void);
int32_t  of_event_appl_ucmcbk_init(void);
  
int32_t  of_ssl_appl_addnlcacerts_ucmcbk_init(void);
int32_t  of_tracelog_appl_ucmcbk_init(void);

  #if 1
int32_t  of_flow_appl_ucmcbk_init(void);
int32_t  of_flow_match_appl_ucmcbk_init(void);
int32_t  of_flow_instruction_appl_ucmcbk_init(void);
int32_t  of_flow_actionlist_appl_ucmcbk_init(void);
int32_t  of_flow_actionset_appl_ucmcbk_init(void);
  #endif

int32_t of_view_appl_ucmcbk_init (void);
int32_t of_namespace_appl_ucmcbk_init (void);
int32_t  of_protocol_appl_ucmcbk_init(void);//protocol init
int32_t  of_ttp_appl_init(void);
int32_t  of_ttp_tbl_appl_init(void);
int32_t  of_ttp_domain_appl_init(void);
int32_t  of_ttp_match_appl_init(void);

int32_t of_nem_ns2nsid_appl_ucmcbk_init (void);

int32_t of_nem_nsid2ns_appl_ucmcbk_init (void);

int32_t of_nem_dpid2nsid_appl_ucmcbk_init (void);

int32_t of_nem_nsid2dpid_appl_ucmcbk_init (void);

int32_t of_nem_dpid2peth_appl_ucmcbk_init (void);

int32_t of_nem_peth2dpid_appl_ucmcbk_init (void);

int32_t of_ip4_fwd_gw_appl_ucmcbk_init (void);

int32_t of_ip4_fwd_rt_appl_ucmcbk_init (void);

#endif //__CM_REGINI_H
