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
* File name: flowstats.c
* Author: G Atmaram <B38856@freescale.com>
* Date:   03/13/2013
* Description: Flow Statistics.
* 
 */

#ifdef OF_CM_SUPPORT


#include "cbcmninc.h"
#include "cmgutil.h"
#include "cntrucminc.h"
#include "of_utilities.h"
#include "of_multipart.h"
#include "nicira_ext.h"
#include "cntlr_transprt.h"
#include "fsl_ext.h"
#include "cntrlappcmn.h"
//#include "cmbufdef.h"

int32_t of_flowstats_appl_ucmcbk_init (void);

int32_t of_flowstats_getfirstnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pArrayofIvPairArr);

int32_t of_flowstats_getnextnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs * pPrevRecordKey, uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pNextNRecordData_p);

int32_t of_flowstats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** pIvPairArr);

int32_t of_flowstats_verifycfg (struct cm_array_of_iv_pairs * pKeysArr,
    uint32_t command_id, struct cm_app_result ** pResult);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * V L A N  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
* * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */
int32_t of_flowstats_ucm_validatemandparams (struct cm_array_of_iv_pairs *
    pMandParams,
    struct cm_app_result **ppResult);

int32_t of_flowstats_ucm_setmandparams (struct cm_array_of_iv_pairs *
    pMandParams,
    struct dprm_datapath_general_info *datapath_info,
    struct ofi_flow_entry_info* stats_info,
    struct cm_app_result ** ppResult);

int32_t of_flowstats_ucm_getparams (struct ofi_flow_entry_info *stats_info,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);

struct cm_dm_app_callbacks of_flowstats_ucm_callbacks = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  of_flowstats_getfirstnrecs,
  of_flowstats_getnextnrecs,
  of_flowstats_getexactrec,
  NULL,
  NULL
};
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * Switch  UCM Init * * * * * * *  * * * * * * * * * * *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32_t of_flowstats_appl_ucmcbk_init (void)
{
  CM_CBK_DEBUG_PRINT ("Entry");
  cm_register_app_callbacks (CM_ON_DIRECTOR_DATAPATH_STATS_FLOWSTATS_APPL_ID,
           &of_flowstats_ucm_callbacks);
  return OF_SUCCESS;
}


int32_t of_flowstats_getfirstnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pArrayofIvPairArr)
{
  int32_t return_value = OF_FAILURE;
  struct cm_app_result *stats_result = NULL;
  struct dprm_datapath_general_info datapath_info={};
  struct ofi_flow_entry_info stats_info;
  uint64_t datapath_handle;

  of_memset (&stats_info, 0, sizeof (struct ofi_flow_entry_info));

  if ((of_flowstats_ucm_setmandparams (pKeysArr,&datapath_info, &stats_info, &stats_result)) !=
      OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    return OF_FAILURE;
  }

  return_value=dprm_get_datapath_handle(datapath_info.dpid, &datapath_handle);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Get DPRM Handle failed");
    return OF_FAILURE;
  }

  return_value=cntlr_send_flow_stats_request(datapath_handle); 
  if ( return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("flowstats failed");
    return OF_FAILURE;
  } 
  CM_CBK_DEBUG_PRINT ("flow stats will be sent as multipart response");
  return CM_CNTRL_MULTIPART_RESPONSE;
}


int32_t of_flowstats_getnextnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs *pPrevRecordKey, uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pNextNRecordData_p)
{

  CM_CBK_DEBUG_PRINT ("Entered");
  *pCount=0;
  return OF_FAILURE;

}

int32_t of_flowstats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** pIvPairArr)
{
  int32_t return_value = OF_FAILURE;

  CM_CBK_DEBUG_PRINT ("Entered");
  return return_value;

}

int32_t of_flowstats_verifycfg (struct cm_array_of_iv_pairs * pKeysArr,
    uint32_t command_id, struct cm_app_result ** pResult)
{
  struct cm_app_result *stats_result = NULL;
  int32_t return_value = OF_FAILURE;
  struct ofi_flow_entry_info stats_info = { };

  CM_CBK_DEBUG_PRINT ("Entered");
  of_memset (&stats_info, 0, sizeof (struct ofi_flow_entry_info));

  return_value = of_flowstats_ucm_validatemandparams (pKeysArr, &stats_result);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Validate Mandatory Parameters Failed");
    return OF_FAILURE;
  }

  *pResult = stats_result;
  return OF_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * V L A N  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
* * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */

int32_t of_flowstats_ucm_validatemandparams (struct cm_array_of_iv_pairs *
    pMandParams,	struct cm_app_result **ppResult)
{
  uint32_t uiMandParamCnt;
  struct cm_app_result *stats_result = NULL;


  CM_CBK_DEBUG_PRINT ("Entered");
  for (uiMandParamCnt = 0; uiMandParamCnt < pMandParams->count_ui;
      uiMandParamCnt++)
  {
    switch (pMandParams->iv_pairs[uiMandParamCnt].id_ui)
    {
      case CM_DM_FLOWSTATS_TABLEID_ID:  
        if (pMandParams->iv_pairs[uiMandParamCnt].value_p == NULL)
        {
          CM_CBK_DEBUG_PRINT ("Table ID is NULL");
          fill_app_result_struct (&stats_result, NULL, CM_GLU_TABLE_ID_NULL);
          *ppResult = stats_result;
          return OF_FAILURE;
        }
        break;

    }
  }
  CM_CBK_PRINT_IVPAIR_ARRAY (pMandParams);
  return OF_SUCCESS;
}

int32_t of_flowstats_ucm_setmandparams (struct cm_array_of_iv_pairs *
    pMandParams,
    struct dprm_datapath_general_info *datapath_info,
    struct ofi_flow_entry_info* stats_info,		struct cm_app_result ** ppResult)
{
  uint32_t uiMandParamCnt;
  uint64_t idpId;

  CM_CBK_DEBUG_PRINT ("Entered");
  for (uiMandParamCnt = 0; uiMandParamCnt < pMandParams->count_ui;
      uiMandParamCnt++)
  {
    switch (pMandParams->iv_pairs[uiMandParamCnt].id_ui)
    {
      case CM_DM_DATAPATH_DATAPATHID_ID:
        idpId=charTo64bitNum((char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
        datapath_info->dpid=idpId;
        CM_CBK_DEBUG_PRINT("dpid is: %llx\r\n", idpId);
        break;
#if 0
      case CM_DM_FLOWSTATS_TABLEID_ID: 
        stats_info->table_id = of_atoi((char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
        CM_CBK_DEBUG_PRINT ("Table ID: %d",stats_info->table_id);
        break;
#endif
    }
  }
  CM_CBK_PRINT_IVPAIR_ARRAY (pMandParams);
  return OF_SUCCESS;
}

int32_t of_flowstats_ucm_getparams (struct ofi_flow_entry_info *stats_info,
    struct cm_array_of_iv_pairs * result_iv_pairs_p)
{
  char *instruction_actions[]={
    "Output to switch port",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "None",
    "Copy TTL out",
    "Copy TTL in",
    "None",
    "None",
    "MPLS TTL",
    "Decrement MPLS TTL",
    "Push a new VLAN tag",
    "Pop the outer VLAN tag",
    "Push a new MPLS tag",
    "Pop the outer MPLS tag",
    "Set queue id when outputting to a port",
    "Apply group",
    "IP TTL",
    "Decrement IP TTL",
    "Set a header field using OXM TLV format",
    "Push a new PBB service tag (I-TAG) ",
    "Pop the outer PBB service tag (I-TAG)"
  };

  char *port_type[]={
    "Maximum number of ports",
    "Send the packet out the input port",
    "Submit the packet to the first flow table",
    "Process with normal L2/L3 switching",
    "All physical ports in VLAN",
    "All physical ports",
    "Send to controller",
    "Local openflow port",
    "Wildcard port used only for flow mod"
  };

char *fslx_instrctn_subtype[]= 
{
  "None",
  "Apply actions on create",
  "Apply actions on delete",
  "Wait to complete pkt outs",
  "Jump",
  "Write priority register",
  "Write priority reg from current flow",
  "Re lookup",
  "Re inject to",
  "Apply group",
  "Write group",
  "EXT1",
  "EXT2",
  "EXT3",
  "EXT4"
};

char *of_sd_router_table_names[]=
{
  "In port classification table",
  "Ethernet filtering table",
  "Ethernet classification table",
  "Arp inbound table",
  "IP classification table",
  "IPV4 Firewall Session table",
  "PBR table",
  "Main routing table",
  "IPv4 Firewall Self Filter table",
  "Multicast group table",
  "Multicast routing table",
  "IP4 self classification table",
  "IPSEC packet classification table",
  "IPSEC udp encapsulation natt table",
  "IPSEC inbound sa table",
  "IPSEC inbound policy table",
  "IPv4 Firewall FWD Filter table",
  "IPSEC outbound policy table",
  "IPSEC outbound sa table",
  "ARP resolution table",
  "Qos Filter table",
  "Qos config table"
};

  char string[1024];
  char tmp_string[1024];
  char buf_port[40];
  int32_t index = 0, count_ui;
  uint8_t table_id;  
  uint64_t metadata; 
  uint64_t metadata_mask; 
  int32_t no_of_actions, i;
  struct ofi_action *action_entry_p;
  struct ofi_instruction *instruction_entry_p;
  struct ofi_match_field *match_entry_p;
  char buf[64] = "";

#define CM_FLOWSTATS_CHILD_COUNT 201
  count_ui = CM_FLOWSTATS_CHILD_COUNT;

  result_iv_pairs_p->iv_pairs = (struct cm_iv_pair*)of_calloc(count_ui, sizeof(struct cm_iv_pair));
  if(!result_iv_pairs_p->iv_pairs)
  {
    CM_CBK_DEBUG_PRINT (" Memory allocation failed ");
    return OF_FAILURE;
  }
  of_memset(string, 0, sizeof(string));
  of_itoa(stats_info->table_id, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_TABLEID_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  sprintf(string,"%s", of_sd_router_table_names[stats_info->table_id]);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_TABLENAME_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  sprintf(string,"%d",stats_info->priority);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_PRIORITY_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_itoa(stats_info->alive_sec,string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_ALIVESEC_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_itoa(stats_info->alive_nsec,string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_ALIVENSEC_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa(stats_info->cookie,string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_COOKIE_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  sprintf(string,"%d",stats_info->Idle_timeout);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_IDLETIMEOUT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  sprintf(string,"%d",stats_info->hard_timeout);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_HARDTIMEOUT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats_info->packet_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_PACKETCOUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa(stats_info->byte_count,string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_BYTECOUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;
  
  of_memset(string, 0, sizeof(string));
  of_sprintf(string,"%d",OF_LIST_COUNT(stats_info->match_field_list));
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_MATCHFIELD_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  OF_LIST_SCAN(stats_info->match_field_list, match_entry_p, struct ofi_match_field *, OF_MATCH_LISTNODE_OFFSET)
  {
    of_memset(string, 0, sizeof(string));
if(match_entry_p->match_class == OFPXMC_OPENFLOW_BASIC)
   {
    switch(match_entry_p->field_type)
    {
      case OFPXMT_OFB_IN_PORT:
        of_sprintf(string,"Type=Switch input port Value=%d", match_entry_p->ui32_data); 
        break;

      case OFPXMT_OFB_IN_PHY_PORT:
        of_sprintf(string,"Type=Switch physical input Value=%d",match_entry_p->ui32_data);
        break;

      case OFPXMT_OFB_METADATA:
        of_sprintf(string,"Type=Metadata passed between Value=%llx",match_entry_p->ui64_data);
        break;

      case OFPXMT_OFB_ETH_DST:
        of_sprintf(string,"Type=Ethernet destination address Value=%02x:%02x:%02x:%02x:%02x:%02x",match_entry_p->ui8_data_array[0],match_entry_p->ui8_data_array[1], match_entry_p->ui8_data_array[2],match_entry_p->ui8_data_array[3],match_entry_p->ui8_data_array[4],match_entry_p->ui8_data_array[5]);
        break;

      case OFPXMT_OFB_ETH_SRC:
        of_sprintf(string,"Type=Ethernet source address Value=%02x:%02x:%02x:%02x:%02x:%02x",match_entry_p->ui8_data_array[0],match_entry_p->ui8_data_array[1],match_entry_p->ui8_data_array[2],match_entry_p->ui8_data_array[3],match_entry_p->ui8_data_array[4],match_entry_p->ui8_data_array[5]);
        break;

      case OFPXMT_OFB_ETH_TYPE:
        of_sprintf(string,"Type=Ethernet frame type Value=%x",match_entry_p->ui16_data);
        break;

      case OFPXMT_OFB_VLAN_VID:
        of_sprintf(string,"Type=VLAN id Value=%d",match_entry_p->ui16_data);
        break;

      case OFPXMT_OFB_IP_PROTO:
        of_sprintf(string,"Type=IP protocol Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_IPV4_SRC:
        of_memset(buf, 0, sizeof(buf));
        cm_inet_ntoal(match_entry_p->ui32_data, buf);
        of_sprintf(string,"Type=IPv4 source address Value=%s",buf);
        break;

      case OFPXMT_OFB_IPV4_DST:
        of_memset(buf, 0, sizeof(buf));
        cm_inet_ntoal(match_entry_p->ui32_data, buf);
        of_sprintf(string,"Type=IPv4 destination address Value=%s",buf);
        break;

      case OFPXMT_OFB_TCP_SRC:
        of_sprintf(string,"Type=TCP source port Value=%d",match_entry_p->ui16_data);
        break;

      case OFPXMT_OFB_TCP_DST:
        of_sprintf(string,"Type=TCP destination port Value=%d",match_entry_p->ui16_data);
        break;

      case OFPXMT_OFB_UDP_SRC:
        of_sprintf(string,"Type=UDP source port Value=%d",match_entry_p->ui16_data);
        break;

      case OFPXMT_OFB_UDP_DST:
        of_sprintf(string,"Type=UDP destination port Value=%d",match_entry_p->ui16_data);
        break;

      case OFPXMT_OFB_SCTP_SRC:
        of_sprintf(string,"Type=SCTP source port Value=%d",match_entry_p->ui16_data);
        break;

      case OFPXMT_OFB_SCTP_DST:
        of_sprintf(string,"Type=SCTP destination port Value=%d",match_entry_p->ui16_data);
        break;

      case OFPXMT_OFB_ICMPV4_TYPE:
        of_sprintf(string,"Type=ICMP type Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_ICMPV4_CODE:
        of_sprintf(string,"Type=ICMP code Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_ARP_OP:
        of_sprintf(string,"Type=ARP opcode Value=%d",match_entry_p->ui16_data);
        break;

      case OFPXMT_OFB_MPLS_LABEL:
        of_sprintf(string,"Type=MPLS label Value=%d",match_entry_p->ui32_data);
        break;

      case OFPXMT_OFB_MPLS_TC:
        of_sprintf(string,"Type=MPLS TC Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_MPLS_BOS:
        of_sprintf(string,"Type=MPLS BoS bit Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_PBB_ISID:
        of_sprintf(string,"Type=PBB I-SID Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_TUNNEL_ID:
        of_sprintf(string,"Type=Logical Port Metadata Value=%llx",match_entry_p->ui64_data);
        break;

      case OFPXMT_OFB_VLAN_PCP:
        of_sprintf(string,"Type=VLAN priority Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_ARP_SPA:
        of_memset(buf, 0, sizeof(buf));
        cm_inet_ntoal(match_entry_p->ui32_data, buf);
	of_sprintf(string,"Type=ARP source IPv4 address Value=%s",buf);
	break;      

      case OFPXMT_OFB_ARP_TPA:
        of_memset(buf, 0, sizeof(buf));
        cm_inet_ntoal(match_entry_p->ui32_data, buf);
	of_sprintf(string," ARP target IPv4 address:= %s",buf);
	break;


      case OFPXMT_OFB_ARP_THA:
	of_sprintf(string,"Type=ARP target hardware address Value=%x",match_entry_p->ui32_data);
	break;

      case OFPXMT_OFB_IP_DSCP:
        of_sprintf(string,"Type=IP DSCP (6 bits in ToS field) Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_IP_ECN:
        of_sprintf(string,"Type=IP ECN (2 bits in ToS field) Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_IPV6_SRC:
        of_sprintf(string,"Type=IPv6 source address");
        break;
  
      case OFPXMT_OFB_IPV6_DST:
        of_sprintf(string,"Type=IPv6 destination address");
        break;

      case OFPXMT_OFB_ICMPV6_TYPE:
        of_sprintf(string,"Type=ICMPv6 type Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_ICMPV6_CODE:
        of_sprintf(string,"Type=ICMPv6 code Value=%d",match_entry_p->ui8_data);
        break;

      case OFPXMT_OFB_IPV6_FLABEL:
        of_sprintf(string,"Type=IPv6 Flow Label Value=%x",match_entry_p->ui32_data);
        break;

      case OFPXMT_OFB_IPV6_ND_TARGET:
        of_sprintf(string,"Type=Target address for ND");
        break;

      case OFPXMT_OFB_IPV6_ND_SLL:
        of_sprintf(string,"Type=Source link-layer for ND");
        break;

      case OFPXMT_OFB_IPV6_ND_TLL:
        of_sprintf(string,"Type=Target link-layer for ND");
        break;

      case OFPXMT_OFB_IPV6_EXTHDR:
        of_sprintf(string,"Type=IPv6 Extension Header pseudo-field");
        break;

      default:
        of_sprintf(string,"Type=UNKNOWN");
        break;

    }
   }
   else if(match_entry_p->match_class == OFPXMC_EXPERIMENTER)
    {
        of_sprintf(string,"Class=Experimenter class VendorId= %d",match_entry_p->ui32_data);
      
    }
     else if(match_entry_p->match_class == OFPXMC_NXM_1)
       {
         of_sprintf(string,"Class=Backward compatibility with NXM REG=%d Value= %llX", match_entry_p->field_type,match_entry_p->ui64_data); 
       }
       else
       {
        of_sprintf(string,"Class=UNKNOWN");
       }
    //For diplaying the mask value
    OF_LOG_MSG(OF_LOG_MOD,OF_LOG_DEBUG,"maskset is %d  value  is %x",match_entry_p->is_mask_set,match_entry_p->mask);

    if(match_entry_p->is_mask_set == 1){
    if(match_entry_p->mask_len == 4){
        of_sprintf(string,"%s MASK = %x",string,ntohl(*(uint32_t*)(match_entry_p->mask)));
    }
    else if((match_entry_p->mask_len == 6)){
        of_sprintf(string,"%s  MASK=%02x:%02x:%02x:%02x:%02x:%02x",string,match_entry_p->mask[0],match_entry_p->mask[1],match_entry_p->mask[2],match_entry_p->mask[3],match_entry_p->mask[4],match_entry_p->mask[5]);
    }
    else if(match_entry_p->mask_len == 8){
        of_sprintf(string,"%s  MASK=%x%x%x%x%x%x%x%x",string,match_entry_p->mask[0],match_entry_p->mask[1],match_entry_p->mask[2],match_entry_p->mask[3],match_entry_p->mask[4],match_entry_p->mask[5],match_entry_p->mask[6],match_entry_p->mask[7]);
    }
    else if(match_entry_p->mask_len == 16){
        of_sprintf(string,"%s MASK IS SET",string);
    }
    }


    FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_MATCHFIELDINFO_ID,CM_DATA_TYPE_STRING, string);
    index++;
  }

  of_memset(string, 0, sizeof(string));
  of_sprintf(string,"%d",OF_LIST_COUNT(stats_info->instruction_list));
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_INSTRUCTION_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  OF_LIST_SCAN(stats_info->instruction_list, instruction_entry_p, struct ofi_instruction *,OF_INSTRUCTION_LISTNODE_OFFSET )
  {
    of_memset(string, 0, sizeof(string));
    switch(instruction_entry_p->type)
    {
      case  OFPIT_GOTO_TABLE:
        table_id = instruction_entry_p->table_id;
        of_sprintf(string,"Type=Goto TableID=%d", table_id); 
        break;

      case OFPIT_WRITE_METADATA:
        metadata = instruction_entry_p->metadata;
        metadata_mask = instruction_entry_p->metadata_mask;
        of_sprintf(string,"Type=Write Metadata=%llx MetadataMask=%llx", metadata,metadata_mask);
        break;

      case OFPIT_APPLY_ACTIONS:
        of_sprintf(string,"Type=Apply actions ");
        break;

      case OFPIT_WRITE_ACTIONS:
        of_sprintf(string,"Type=Write actions ");
        break;
      case OFPIT_CLEAR_ACTIONS:
        of_sprintf(string,"Type=Clear actions ");
        break;
      case OFPIT_METER:
        of_sprintf(string,"Meter");
        break;

      case OFPIT_EXPERIMENTER:
	switch(instruction_entry_p->subtype)
	{
		case FSLXIT_APPLY_ACTIONS_ON_CREATE:
			{
				of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype]);
			}
			break;
		case FSLXIT_APPLY_ACTIONS_ON_DELETE:
			{
				of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype]);
			}
			break;
		case FSLXIT_WAIT_TO_COMPLETE_PKT_OUTS:
			{
				if(instruction_entry_p->value1 == 1)
					of_sprintf(string,"Type=EXPERIMENTER VendorID=%x SubType=%s State=FSLXPS_WAIT", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype]);
				else if(instruction_entry_p->value1 == 2)
					of_sprintf(string,"Type=EXPERIMENTER VendorID=%x SubType=%s State=FSLXPS_READY", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype]);
				else
					of_sprintf(string,"Type=EXPERIMENTER VendorID=%x SubType=%s State=%d (unknown)", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype],instruction_entry_p->value1);
			}
			break;
		case FSLXIT_WRITE_PRIORITY_REGISTER:
			{
				of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s PriorityValue %d", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype],instruction_entry_p->value1);
			}
			break;
		case FSLXIT_WRITE_PRIORITY_REG_FROM_CURRENT_FLOW:
			{
				of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype]);
			}
			break;
		case FSLXIT_RE_LOOKUP:
			{
				if(instruction_entry_p->value1 == 0)
					of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s FSLX_PRIO_REG_NONE", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype]);
				else if(instruction_entry_p->value1 == 1)
					of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s FSLX_PRIO_REG_SAME", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype]);
				else if(instruction_entry_p->value1 == 2)
					of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s FSLX_PRIO_REG_NEXT", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype]);
				else
					of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s UNKNOWN PRIORITY REG TYPE", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype]);
			}
			break;
		case FSLXIT_RE_INJECT_TO:
			{
				if(instruction_entry_p->value1 == 0)
					of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s FSLX_PRIO_REG_NONE TableId %d", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype],instruction_entry_p->table_id);
				else if(instruction_entry_p->value1 == 1)
					of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s FSLX_PRIO_REG_SAME TableId %d", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype],instruction_entry_p->table_id);
				else if(instruction_entry_p->value1 == 2)
					of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s FSLX_PRIO_REG_NEXT TableId %d", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype],instruction_entry_p->table_id);
				else
					of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s UNKNOWN_PRIORITY_REG_TYPE TableId %d", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype],instruction_entry_p->table_id);
			}
			break;
		case FSLXIT_APPLY_GROUP:
			{
				of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s GroupId= %d", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype],instruction_entry_p->group_id);
			}
			break;
		case FSLXIT_WRITE_GROUP:
			{
				of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s GroupId= %d", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype],instruction_entry_p->group_id);
			}
			break;
		case FSLXIT_WRITE_PRIORITY_REG_FROM_NEXT_FLOW:
			{
				of_sprintf(string,"Type=EXPERIMENTER Vendor=%x SubType=%s", instruction_entry_p->vendorid,fslx_instrctn_subtype[instruction_entry_p->subtype]);
			}
			break;
		default:
			of_sprintf(string,"Type=EXPERIMENTER SubType=UNKNOWN");
			break;

	}//switch
	break;

    }
    FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_INSTRUCTIONINFO_ID,CM_DATA_TYPE_STRING, string);
    index++;

    of_memset(string, 0, sizeof(string));
    of_sprintf(string,"%d",OF_LIST_COUNT(instruction_entry_p->action_list));
    FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_ACTION_COUNT_ID,CM_DATA_TYPE_STRING, string);
    index++;

    no_of_actions=OF_LIST_COUNT(instruction_entry_p->action_list);
    i=0;  
    if ( no_of_actions == 0 )
    {
      of_memset(string, 0, sizeof(string));
      of_sprintf(string,"%s", " - ");
      FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_FLOWSTATS_ACTIONINFO_ID,CM_DATA_TYPE_STRING, string);
      index++;
    }
    else
    {
      
      OF_LIST_SCAN(instruction_entry_p->action_list, action_entry_p, struct ofi_action *, OF_ACTION_LISTNODE_OFFSET )
      {
        i++;
            of_memset(string, 0, sizeof(string));  
        of_memset(tmp_string, 0, sizeof(tmp_string));
        of_memset(buf_port, 0, sizeof(buf_port));
        if(action_entry_p->type != OFPAT_EXPERIMENTER)
          of_sprintf(string,"Type=%s",instruction_actions[action_entry_p->type]);
        switch(action_entry_p->type)
        {
          case   OFPAT_OUTPUT:  /* Output to switch port. */
            {
              of_sprintf(buf_port,"%lx", (long unsigned int) action_entry_p->port_no);
              if ( (strcmp(buf_port, "ffffff00") == 0))
                of_sprintf( tmp_string," port=%s max_len=%d", port_type[0],action_entry_p->max_len);
              else if ( (strcmp(buf_port, "fffffff8") == 0))
                of_sprintf( tmp_string," port=%s max_len=%d", port_type[1],action_entry_p->max_len);
              else if (strcmp(buf_port, "fffffff9") == 0) 
                of_sprintf( tmp_string," port=%s max_len=%d", port_type[2],action_entry_p->max_len);
              else if (strcmp(buf_port, "fffffffa") == 0)
                of_sprintf( tmp_string," port=%s max_len=%d", port_type[3],action_entry_p->max_len);
              else if (strcmp(buf_port, "fffffffb") == 0)
                of_sprintf( tmp_string," port=%s max_len=%d", port_type[4],action_entry_p->max_len);
              else if (strcmp(buf_port, "fffffffc") == 0)
                of_sprintf( tmp_string," port=%s max_len=%d", port_type[5],action_entry_p->max_len);
              else if (strcmp(buf_port, "fffffffd") == 0)
                of_sprintf( tmp_string," port=%s max_len=%d", port_type[6],action_entry_p->max_len);
              else if (strcmp(buf_port, "fffffffe") == 0)
                of_sprintf( tmp_string," port=%s max_len=%d", port_type[7],action_entry_p->max_len);
              else if (strcmp(buf_port, "ffffffff") == 0)
                of_sprintf( tmp_string," port=%s max_len=%d", port_type[8],action_entry_p->max_len);
              else
                of_sprintf( tmp_string," port=%lu max_len=%u",(long unsigned int) action_entry_p->port_no,action_entry_p->max_len);
            }
            break;
          case   OFPAT_SET_MPLS_TTL : /* MPLS TTL */
          case   OFPAT_SET_NW_TTL : /* IP TTL. */
            {
              of_sprintf(tmp_string," ttl=%d ",action_entry_p->ttl);
            }
            break;
          case   OFPAT_PUSH_VLAN  : /* Push a new VLAN tag */
          case   OFPAT_PUSH_MPLS: /* Push a new MPLS tag */
          case   OFPAT_POP_MPLS : /* Pop the outer MPLS tag */
          case   OFPAT_PUSH_PBB: /* Push a new PBB service tag (I-TAG) */
            {
              of_sprintf(tmp_string," ether_type=%d ",action_entry_p->ether_type);
            }
            break;
          case   OFPAT_SET_QUEUE: /* Set queue id when outputting to a port */
            {
              of_sprintf(tmp_string," queue_id=%d ",action_entry_p->queue_id);
            }
            break;
          case   OFPAT_GROUP : /* Apply group. */
            {
              of_sprintf(tmp_string," group_id=%d ",action_entry_p->group_id);
            }
            break;
          case   OFPAT_COPY_TTL_OUT: /* Copy TTL "outwards" -- from next-to-outermost    to outermost */
          case   OFPAT_COPY_TTL_IN  : /* Copy TTL "inwards" -- from outermost to    next-to-outermost */
          case   OFPAT_DEC_MPLS_TTL : /* Decrement MPLS TTL */
          case   OFPAT_DEC_NW_TTL: /* Decrement IP TTL. */    
          case   OFPAT_POP_PBB: /* Pop the outer PBB service tag (I-TAG) */
            {
              of_sprintf(tmp_string,"%s", " - ");
            }
            break;
          case OFPAT_SET_FIELD: 
            {
              OF_LOG_MSG(OF_LOG_MOD,OF_LOG_DEBUG,"field type is %d",action_entry_p->setfield_type);
              switch(action_entry_p->setfield_type){
                case OFPXMT_OFB_IN_PORT:
                  of_sprintf(tmp_string," Switch input port= %d",action_entry_p->ui32_data);
                  break;
                case OFPXMT_OFB_IN_PHY_PORT:
                  of_sprintf(tmp_string," Switch physical input port:= %d",action_entry_p->ui32_data);
                  break;
                case OFPXMT_OFB_METADATA: 
                  of_sprintf(tmp_string," Metadata passed between tables:= %d",(int) action_entry_p->ui64_data);
                  break;
                case OFPXMT_OFB_ETH_DST:
                  of_sprintf(tmp_string," Ethernet destination address Value=%02x:%02x:%02x:%02x:%02x:%02x",action_entry_p->ui8_data_array[0],action_entry_p->ui8_data_array[1], action_entry_p->ui8_data_array[2],action_entry_p->ui8_data_array[3],action_entry_p->ui8_data_array[4],action_entry_p->ui8_data_array[5]);
                  break;
                case OFPXMT_OFB_ETH_SRC:
                  of_sprintf(tmp_string," Ethernet source address Value=%02x:%02x:%02x:%02x:%02x:%02x",action_entry_p->ui8_data_array[0],action_entry_p->ui8_data_array[1], action_entry_p->ui8_data_array[2],action_entry_p->ui8_data_array[3],action_entry_p->ui8_data_array[4],action_entry_p->ui8_data_array[5]);
                  break;
                case OFPXMT_OFB_ETH_TYPE:
                  of_sprintf(tmp_string," Ethernet frame type := %d",action_entry_p->ui16_data);
                  break;
                case OFPXMT_OFB_VLAN_VID:
                  of_sprintf(tmp_string," VLAN id:= %d",action_entry_p->ui16_data);
                  break;
                case OFPXMT_OFB_IP_PROTO:
                  of_sprintf(tmp_string," IP protocol:= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_IPV4_SRC:
                  of_sprintf(tmp_string," IPv4 source address:= %x",action_entry_p->ui32_data);
                  break;
                case OFPXMT_OFB_IPV4_DST:
                  of_sprintf(tmp_string," IPv4 destination address:= %x",action_entry_p->ui32_data);
                  break;
                case OFPXMT_OFB_TCP_SRC:
                  of_sprintf(tmp_string," TCP source port:= %d",action_entry_p->ui16_data);
                  break;
                case OFPXMT_OFB_TCP_DST:
                  of_sprintf(tmp_string," TCP destination port:= %d",action_entry_p->ui16_data);
                  break;
                case OFPXMT_OFB_UDP_SRC:
                  of_sprintf(tmp_string," UDP source port:= %d",action_entry_p->ui16_data);
                  break;
                case OFPXMT_OFB_UDP_DST:
                  of_sprintf(tmp_string," UDP destination port := %d",action_entry_p->ui16_data);
                  break;
                case OFPXMT_OFB_SCTP_SRC:
                  of_sprintf(tmp_string," SCTP source port:= %d",action_entry_p->ui16_data);
                  break;
                case OFPXMT_OFB_SCTP_DST:
                  of_sprintf(tmp_string," SCTP destination port:= %d",action_entry_p->ui16_data);
                  break;
                case OFPXMT_OFB_ICMPV4_TYPE:
                  of_sprintf(tmp_string," ICMP type:= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_ICMPV4_CODE:
                  of_sprintf(tmp_string," ICMP code:= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_ARP_OP:
                  of_sprintf(tmp_string," ARP opcode:= %d",action_entry_p->ui16_data);
                  break;
                case OFPXMT_OFB_MPLS_LABEL:
                  of_sprintf(tmp_string," MPLS label:= %d",action_entry_p->ui32_data);
                  break;
                case OFPXMT_OFB_MPLS_TC:
                  of_sprintf(tmp_string," MPLS TC:= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_MPLS_BOS:
                  of_sprintf(tmp_string," MPLS BoS bit:= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_PBB_ISID:
                  of_sprintf(tmp_string," PBB I-SID:= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_TUNNEL_ID:
                  of_sprintf(tmp_string," Logical Port Metadata:= %llx",action_entry_p->ui64_data);
                  break;
                case OFPXMT_OFB_VLAN_PCP:
                  of_sprintf(tmp_string," VLAN priority:= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_ARP_SPA:
                  of_sprintf(tmp_string," ARP source IPv4 address:= %x",action_entry_p->ui32_data);
                  break;
                case OFPXMT_OFB_ARP_TPA:
                  of_sprintf(tmp_string," ARP target IPv4 address:= %x",action_entry_p->ui32_data);
                  break;
                case OFPXMT_OFB_ARP_SHA:
                  of_sprintf(tmp_string," ARP source hardware address");
                  break;
                case OFPXMT_OFB_ARP_THA:
                  of_sprintf(tmp_string," ARP target hardware address");
                  break;
                case OFPXMT_OFB_IP_DSCP:
                  of_sprintf(tmp_string," IP DSCP (6 bits in ToS field):= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_IP_ECN:
                  of_sprintf(tmp_string," IP ECN (2 bits in ToS field):= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_IPV6_SRC:
                  of_sprintf(tmp_string," IPv6 source address");
                  break;
                case OFPXMT_OFB_IPV6_DST:
                  of_sprintf(tmp_string," IPv6 destination address");
                  break;
                case OFPXMT_OFB_IPV6_FLABEL:
                  of_sprintf(tmp_string," IPv6 Flow Label:= %d",action_entry_p->ui32_data);
                  break;
                case OFPXMT_OFB_ICMPV6_TYPE:
                  of_sprintf(tmp_string," ICMPv6 type:= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_ICMPV6_CODE:
                  of_sprintf(tmp_string," ICMPv6 code:= %d",action_entry_p->ui8_data);
                  break;
                case OFPXMT_OFB_IPV6_ND_TARGET:
                  of_sprintf(tmp_string," Target address for ND");
                  break;
                case OFPXMT_OFB_IPV6_ND_SLL:
                  of_sprintf(tmp_string," Source link-layer for ND");
                  break;
                case OFPXMT_OFB_IPV6_ND_TLL:
                  of_sprintf(tmp_string," Target link-layer for ND");
                  break;
                case OFPXMT_OFB_IPV6_EXTHDR:
                  of_sprintf(tmp_string," IPv6 Extension Header pseudo-field= %d",action_entry_p->ui16_data);
                  break;
              }


            }
            break;

           case OFPAT_EXPERIMENTER:
            {
		    switch (action_entry_p->sub_type)
		    {
				 case FSLXAT_ARP_RESPONSE:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Arp Response TargetMac= %llX", action_entry_p->vendorid,action_entry_p->ui64_data);
                                    }
                                    break;
                            case FSLXAT_IP_FRAGMENT:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=IP Fragment MTU= %d", action_entry_p->vendorid,action_entry_p->ui32_data);
                                    }
                                    break;
                            case FSLXAT_IP_REASM:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=IP Reasm",action_entry_p->vendorid);
                                    }
                                    break;
                            case FSLXAT_DROP_PACKET:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Drop Packet",action_entry_p->vendorid);
                                    }
                                    break;
                            case FSLXAT_SEND_ICMPV4_DEST_UNREACHABLE:
                                    {
                                            if(action_entry_p->ui8_data != FSLX_ICMPV4_DR_EC_DFBIT_SET)
                                                    of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Send ICMPV4 Dest Unreachable Code= %d",action_entry_p->vendorid, action_entry_p->ui8_data);
                                            else
                                                    of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Send ICMPV4 Dest Unreachable Code= %d NextHopMtu %d", action_entry_p->vendorid,action_entry_p->ui8_data,action_entry_p->ui16_data);
                                    }
                                    break;
                            case FSLXAT_SEND_ICMPV4_TIME_EXCEED:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Send ICMPV4 Time Exceed Code %d",action_entry_p->vendorid, action_entry_p->ui8_data);
                                    }
                                    break;
case FSLXAT_SET_PHY_PORT_FIELD_CNTXT:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Set PHY Port Field Cntxt PhyInPort %d",action_entry_p->vendorid, action_entry_p->ui32_data);
                                    }
                                    break;
                            case FSLXAT_SET_META_DATA_FIELD_CNTXT:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Set Metadata Field Cntxt MetaData= %llX",action_entry_p->vendorid, action_entry_p->ui64_data);
                                    }
                                    break;
                            case FSLXAT_SET_TUNNEL_ID_FIELD_CNTXT:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Set tunnel id field cntxt Tunnel_id= %llX", action_entry_p->vendorid,action_entry_p->ui64_data);
                                    }
                                    break;
                            case FSLXAT_START_TABLE:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Start table table_id= %d",action_entry_p->vendorid, action_entry_p->ui8_data);
                                    }
                                    break;
                            case FSLXAT_WRITE_METADATA_FROM_PKT:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Write metadata from pkt Field_id= %d Mask= %llX", action_entry_p->vendorid,action_entry_p->ui32_data,action_entry_p->ui64_data);
                                    }
                                    break;
                            case FSLXAT_WRITE_META_DATA:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Write MetaData= %llX",action_entry_p->vendorid, action_entry_p->ui64_data);
                                    }
                                    break;
                           case FSLXAT_DETACH_BIND_OBJ:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Detach BindObjId= %d", action_entry_p->vendorid,action_entry_p->ui32_data);
                                            break;
                                    }
                            case FSLXAT_SEND_APP_STATE_INFO:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Send app state info Type= %d Method= %d Count= %d", action_entry_p->vendorid,action_entry_p->ui8_data,action_entry_p->ui16_data,(int) action_entry_p->ui64_data);
                                            break;
                                    }

                            case FSLXAT_BIND_OBJECT_APPLY_ACTIONS:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Bind object apply actions BindObjId= %d",action_entry_p->vendorid, action_entry_p->ui32_data);
                                            break;
                                    }

                            case FSLXAT_SET_APP_STATE_INFO:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Set app state info",action_entry_p->vendorid);
                                            /* struct fslx_action_set_app_state_info */
                                            break;
                                    }

                            case FSLXAT_CREATE_SA_CNTXT:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Create sa cntxt",action_entry_p->vendorid);
                                            /* struct fslx_create_ipsec_sa */
                                            break;
                                    }
                            case FSLXAT_DELETE_SA_CNTXT:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Delete sa cntxt",action_entry_p->vendorid);
                                            /* struct fslx_delete_ipsec_sa */
                                            break;
                                    }
                            case FSLXAT_IPSEC_ENCAP:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Ipsec encap",action_entry_p->vendorid);
                                            /* struct fslx_ipsec_encap */
                                            break;
                                    }

                            case FSLXAT_IPSEC_DECAP:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Ipsec decap",action_entry_p->vendorid);
                                            /* struct fslx_ipsec_decap */
                                            break;
                                    }

                            case FSLXAT_POP_NATT_UDP_HEADER:
                                    {
                                            of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Pop natt udp header",action_entry_p->vendorid);
                                            break;
                                    }

			    case NXAST_REG_LOAD:
				   // of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType= NXAST_REG_LOAD", action_entry_p->vendorid);
					 of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType= nxast reg load-%d NBITS= %d OFFSET= %d  VALUE= %lld", action_entry_p->vendorid,action_entry_p->ui32_data,action_entry_p->ui8_data,action_entry_p->ui16_data,action_entry_p->ui64_data);

				    break;   
			    case NXAST_COPY_FIELD:
				    of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=nxast copy field", action_entry_p->vendorid);
				    break;   
			    default:
				    of_sprintf(tmp_string,"Type=EXPERIMENTER Vendor=%x SubType=Unknown", action_entry_p->vendorid);
				    break;   
		    }
              		
             }
            break;

          default:
            of_sprintf(tmp_string,"%s", " - ");
            break;
        }
        strcat(string, tmp_string);
        if ( i == no_of_actions)
        {
          //strcat(string, "\n");
        }
        FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FLOWSTATS_ACTIONINFO_ID,CM_DATA_TYPE_STRING, string);
        index++;
      }
    }
  }

  result_iv_pairs_p->count_ui = index;
  return OF_SUCCESS;
}


#endif /* OF_CM_SUPPORT */
