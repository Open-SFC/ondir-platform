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
* File name: bindstats.c
* Author: G Atmaram <B38856@freescale.com>
* Date:   03/13/2013
* Description: Flow Statistics.
* 
 */

#ifdef OPENFLOW_IPSEC_SUPPORT


#include "cbcmninc.h"
#include "cmgutil.h"
#include "cntrucminc.h"
#include "of_utilities.h"
#include "of_multipart.h"
#include "nf_infra_api.h"
#include "fsl_nfapi_ipsec.h"

int32_t ipsec_sa_stats_appl_ucmcbk_init (void);

int32_t ipsec_sa_stats_addrec (void * config_trans_p,
    struct cm_array_of_iv_pairs * pMandParams,
    struct cm_array_of_iv_pairs * pOptParams,
    struct cm_app_result ** pResult);

int32_t ipsec_sa_stats_setrec (void * config_trans_p,
    struct cm_array_of_iv_pairs * pMandParams,
    struct cm_array_of_iv_pairs * pOptParams,
    struct cm_app_result ** pResult);

int32_t ipsec_sa_stats_delrec (void * pConfigTransction,
    struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_app_result ** pResult);

int32_t ipsec_sa_stats_getfirstnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pArrayofIvPairArr);

int32_t ipsec_sa_stats_getnextnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs * pPrevRecordKey, uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pNextNRecordData_p);

int32_t ipsec_sa_stats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** pIvPairArr);

int32_t ipsec_sa_stats_verifycfg (struct cm_array_of_iv_pairs * pKeysArr,
    uint32_t command_id, struct cm_app_result ** pResult);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * V L A N  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
* * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */
int32_t ipsec_sa_stats_ucm_validatemandparams (struct cm_array_of_iv_pairs *
    pMandParams,
    struct cm_app_result **ppResult);

int32_t ipsec_sa_stats_ucm_validateoptparams (struct cm_array_of_iv_pairs *
    pOptParams,
    struct cm_app_result ** ppResult);

int32_t ipsec_sa_stats_ucm_setmandparams (struct cm_array_of_iv_pairs *
    pMandParams,struct dprm_datapath_general_info *datapath_info,
    struct cm_app_result ** ppResult);

int32_t ipsec_sa_stats_ucm_setoptparams (struct cm_array_of_iv_pairs *
    pOptParams,
    struct ipsec_sa_get_inargs *in_info,
    struct ipsec_sa_get_outargs  *out_info,
    struct cm_app_result ** ppResult);


int32_t ipsec_sa_stats_ucm_getparams (struct ipsec_sa_get_inargs *in_info,
                                          struct ipsec_sa_get_outargs *out_info,
                                          struct cm_array_of_iv_pairs  *result_iv_pairs_p);

struct cm_dm_app_callbacks ipsec_sa_stats_ucm_callbacks = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  ipsec_sa_stats_getfirstnrecs,
  ipsec_sa_stats_getnextnrecs,
  ipsec_sa_stats_getexactrec,
  NULL,
  NULL
};
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * Switch  UCM Init * * * * * * *  * * * * * * * * * * *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32_t ipsec_sa_stats_appl_ucmcbk_init (void)
{
  CM_CBK_DEBUG_PRINT ("Entry");
  cm_register_app_callbacks (CM_DM_STATS_SASTATS_ID,&ipsec_sa_stats_ucm_callbacks);
  return OF_SUCCESS;
}



int32_t ipsec_sa_stats_getfirstnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pArrayofIvPairArr)
{
  struct   cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  int32_t  return_value = OF_FAILURE;
  uint32_t uiRecCount = 0;
  struct   dprm_datapath_general_info datapath_info = {};
  struct   cm_app_result *stats_result = NULL;
  uint64_t datapath_handle;
  struct ipsec_sa_get_inargs  in;
  struct ipsec_sa_get_outargs out;
  nsid_t nsid;

  of_memset (&in, 0, sizeof (struct ipsec_sa_get_inargs));
  of_memset (&out, 0, sizeof (struct ipsec_sa_get_outargs));

  out.sa_params = (struct ipsec_sa *) calloc (1, sizeof(struct ipsec_sa));
  if(!out.sa_params)
    return OF_FAILURE;

  if ((ipsec_sa_stats_ucm_setmandparams (pKeysArr,&datapath_info, &stats_result)) !=
      OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    return OF_FAILURE;
  }
  if ((ipsec_sa_stats_ucm_setoptparams (pKeysArr,&in,&out,&stats_result)) !=
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

  in.operation = IPSEC_SA_GET_FIRST;
  if(in.dir == 0)
  {
    in.dir = IPSEC_INBOUND;
  }
 //manish return_value = of_nem_get_nsid_from_dpid_and_dp_ns_id(datapath_info.dpid,0,&nsid);
  //get nsid from dpid before calling this API
 //manish return_value = ipsec_sa_get(nsid,  API_CTRL_FLAG_NO_RESP_EXPECTED,
         //                      &in, &out, NULL);
  if ( return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("ipsec sa stats failed");
    return OF_FAILURE;
  } 
  CM_CBK_DEBUG_PRINT ("ipsec sa stats will be sent as multipart response");
  return CM_CNTRL_MULTIPART_RESPONSE;
  result_iv_pairs_p = (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));

 if (result_iv_pairs_p == NULL)
  {
    CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
    return OF_FAILURE;
  }
  ipsec_sa_stats_ucm_getparams (&in,&out, &result_iv_pairs_p[uiRecCount]);
  uiRecCount++;
  //manish *next_n_record_data_p = result_iv_pairs_p;
  *pCount = uiRecCount;
  return OF_SUCCESS;
 
  CM_CBK_DEBUG_PRINT ("ipsec sa stats will be sent as multipart response");
  return CM_CNTRL_MULTIPART_RESPONSE;
}




int32_t ipsec_sa_stats_getnextnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs *pPrevRecordKey, uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pNextNRecordData_p)
{

  struct   cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  int32_t  return_value = OF_FAILURE;
  uint32_t uiRecCount = 0;
  struct   cm_app_result *stats_result = NULL;
  struct   dprm_datapath_general_info datapath_info={};
  struct   dprm_port_runtime_info runtime_info;
  uint64_t datapath_handle;
  uint64_t port_handle;
  struct ipsec_sa_get_inargs  in;
  struct ipsec_sa_get_outargs out;
  nsid_t nsid;
  CM_CBK_DEBUG_PRINT ("Entered");
  *pCount=0;
  

  of_memset (&in, 0, sizeof (struct ipsec_sa_get_inargs));
  of_memset (&out, 0, sizeof (struct ipsec_sa_get_outargs));

  out.sa_params = (struct ipsec_sa *) calloc (1, sizeof(struct ipsec_sa));
  if(!out.sa_params)
    return OF_FAILURE;

  if ((ipsec_sa_stats_ucm_setmandparams (pPrevRecordKey,&datapath_info, &stats_result)) !=
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

  in.operation = IPSEC_SA_GET_NEXT;
  if(in.dir == 0)
  {
    in.dir = IPSEC_INBOUND;
  }
 //manish return_value = of_nem_get_nsid_from_dpid_and_dp_ns_id(datapath_info.dpid,0,&nsid);
  //get nsid from dpid before calling this API
//manish  return_value = ipsec_sa_get(nsid,  API_CTRL_FLAG_NO_RESP_EXPECTED,
       //                        &in, &out, NULL);
  if ( return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("ipsec sa stats failed");
    return OF_FAILURE;
  }
  result_iv_pairs_p = (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));

 if (result_iv_pairs_p == NULL)
  {
    CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
    return OF_FAILURE;
  }
  ipsec_sa_stats_ucm_getparams (&in,&out, &result_iv_pairs_p[uiRecCount]);
  uiRecCount++;
  *pNextNRecordData_p = result_iv_pairs_p;
  *pCount = uiRecCount;
  return OF_SUCCESS;
 
  CM_CBK_DEBUG_PRINT ("ipsec sa stats will be sent as multipart response");
  return CM_CNTRL_MULTIPART_RESPONSE;
}




int32_t ipsec_sa_stats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** pIvPairArr)
{
  struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  struct cm_app_result *stats_result = NULL;
  int32_t return_value = OF_FAILURE;

  CM_CBK_DEBUG_PRINT ("Entered");
  return return_value;

}

int32_t ipsec_sa_stats_verifycfg (struct cm_array_of_iv_pairs * pKeysArr,
    uint32_t command_id, struct cm_app_result ** pResult)
{
  struct cm_app_result *stats_result = NULL;
  int32_t return_value = OF_FAILURE;
  struct ofi_bind_entry_info stats_info = { };
  struct ipsec_sa_get_inargs in = {};
  struct ipsec_sa_get_outargs out = {};
  

  CM_CBK_DEBUG_PRINT ("Entered");
  of_memset (&stats_info, 0, sizeof (struct ofi_bind_entry_info));

  return_value = ipsec_sa_stats_ucm_validatemandparams (pKeysArr, &stats_result);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Validate Mandatory Parameters Failed");
    return OF_FAILURE;
  }

  if (datapath_ucm_validateoptparams (pKeysArr, &stats_result)
      != OF_SUCCESS)
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

int32_t ipsec_sa_stats_ucm_validatemandparams (struct cm_array_of_iv_pairs *
    pMandParams,	struct cm_app_result **ppResult)
{
  uint32_t uiMandParamCnt;
  struct cm_app_result *stats_result = NULL;
  uint32_t uiPortId;


  CM_CBK_DEBUG_PRINT ("Entered");
  for (uiMandParamCnt = 0; uiMandParamCnt < pMandParams->count_ui;
      uiMandParamCnt++)
  {
    switch (pMandParams->iv_pairs[uiMandParamCnt].id_ui)
    {
        default :     
        break;

    }
  }
  CM_CBK_PRINT_IVPAIR_ARRAY (pMandParams);
  return OF_SUCCESS;
}

int32_t ipsec_sa_stats_ucm_validateoptparams (struct cm_array_of_iv_pairs *
    pOptParams,
  //manish check the name
  //  struct cm_app_result ** ppResult)
    struct cm_app_result ** presult_p)
{
  uint32_t uiOptParamCnt, uiTableCnt;
  struct cm_app_result *of_sa_result = NULL;  

  CM_CBK_DEBUG_PRINT ("Entered");
  for (uiOptParamCnt = 0; uiOptParamCnt < pOptParams->count_ui; uiOptParamCnt++)
  {
    switch (pOptParams->iv_pairs[uiOptParamCnt].id_ui)
    {
      case  CM_DM_SASTATS_DIRECTION_ID:
        if (pOptParams->iv_pairs[uiOptParamCnt].value_p == NULL)
        {
          CM_CBK_DEBUG_PRINT (" Direction is NULL");
          fill_app_result_struct (&of_sa_result, NULL, CM_GLU_IPSEC_DIRECTION_NULL);
          *presult_p = of_sa_result;
          return OF_FAILURE;
        }
        if(pOptParams->iv_pairs[uiOptParamCnt].value_p != IPSEC_INBOUND || 
           pOptParams->iv_pairs[uiOptParamCnt].value_p != IPSEC_OUTBOUND)
        {
           CM_CBK_DEBUG_PRINT (" Direction is NULL");
           return OF_FAILURE;
        }
        break;
     case CM_DM_SASTATS_SPI_ID:
       if (pOptParams->iv_pairs[uiOptParamCnt].value_p == NULL)
        {
          CM_CBK_DEBUG_PRINT (" Policy ID is NULL");
          fill_app_result_struct (&of_sa_result, NULL, CM_GLU_SA_SPI_ID_NULL);
          *presult_p = of_sa_result;
          return OF_FAILURE;
        }
        break;

        case CM_DM_SASTATS_DEST_IP_ID:
          if (pOptParams->iv_pairs[uiOptParamCnt].value_p == NULL)
          {
            CM_CBK_DEBUG_PRINT (" Dest IP is NULL");
            fill_app_result_struct (&of_sa_result, NULL, CM_GLU_SA_DEST_IP_ID_NULL);
            *presult_p = of_sa_result;
            return OF_FAILURE;
          }
          break;

      case CM_DM_SASTATS_PROTOCOL_ID:
        if (pOptParams->iv_pairs[uiOptParamCnt].value_p == NULL)
        {
          CM_CBK_DEBUG_PRINT (" Protocol is NULL");
          fill_app_result_struct (&of_sa_result, NULL, CM_GLU_SA_PROTOCOL_ID_NULL);
          *presult_p = of_sa_result;
          return OF_FAILURE;
        }
//manish commented due to undclared
/*
        if(pOptParams->iv_pairs[uiOptParamCnt].value_p != AH || 
           pOptParams->iv_pairs[uiOptParamCnt].value_p != ESP)
        {
           CM_CBK_DEBUG_PRINT (" Protcol is invalid");
           return OF_FAILURE;
        }
*/

        break;

   
      default:
        break;
    }
  }
  CM_CBK_PRINT_IVPAIR_ARRAY (pOptParams);
  return OF_SUCCESS;
}

int32_t ipsec_sa_stats_ucm_setmandparams (struct cm_array_of_iv_pairs  *pMandParams,
                                              struct dprm_datapath_general_info *datapath_info,
                                              struct cm_app_result ** ppResult)
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
    }
  }
  CM_CBK_PRINT_IVPAIR_ARRAY (pMandParams);
  return OF_SUCCESS;
}

int32_t ipsec_sa_stats_ucm_setoptparams (struct cm_array_of_iv_pairs *
    pOptParams,
    struct ipsec_sa_get_inargs *in_info,
    struct ipsec_sa_get_outargs *out_info,
    struct cm_app_result ** ppResult)
{
  uint32_t uiOptParamCnt;

  CM_CBK_DEBUG_PRINT ("Entered");

  for (uiOptParamCnt = 0; uiOptParamCnt < pOptParams->count_ui; uiOptParamCnt++)
  {
    switch (pOptParams->iv_pairs[uiOptParamCnt].id_ui)
    {
      case CM_DM_SASTATS_DIRECTION_ID:
        CM_CBK_DEBUG_PRINT ("direction:%s",(char *) pOptParams->iv_pairs[uiOptParamCnt].value_p);
        if(!strcmp((char *) pOptParams->iv_pairs[uiOptParamCnt].value_p,"IPSEC_INBOUND"))
        {
          in_info->dir = IPSEC_INBOUND;
        }
        else if(!strcmp((char *) pOptParams->iv_pairs[uiOptParamCnt].value_p,"IPSEC_OUTBOUND"))
        {
          in_info->dir = IPSEC_OUTBOUND;
        }
        break;

      case CM_DM_SASTATS_SPI_ID:
        CM_CBK_DEBUG_PRINT ("sa_id:%s",(char *) pOptParams->iv_pairs[uiOptParamCnt].value_p);
        //manish in_info->sa_id = of_atoi((char *) pOptParams->iv_pairs[uiOptParamCnt].value_p);
        break;

      case CM_DM_SASTATS_DEST_IP_ID:
        CM_CBK_DEBUG_PRINT ("dest_ip:%s",(char *) pOptParams->iv_pairs[uiOptParamCnt].value_p);
       if(cm_val_and_conv_aton((char *)pOptParams->iv_pairs[uiOptParamCnt].value_p,&(in_info->dest_ip))!=OF_SUCCESS)

        break;

      case CM_DM_SASTATS_PROTOCOL_ID:
        CM_CBK_DEBUG_PRINT ("protocol:%s",(char *) pOptParams->iv_pairs[uiOptParamCnt].value_p);
        if(!strcmp((char *) pOptParams->iv_pairs[uiOptParamCnt].value_p,"AH"))
        {
          //manish in_info->protocol = AH;
        }
        else if(!strcmp((char *) pOptParams->iv_pairs[uiOptParamCnt].value_p,"ESP"))
        {
          //manish in_info->protocol = ESP;
        }

        break;

      default:
        break;
    }
  }

  CM_CBK_PRINT_IVPAIR_ARRAY (pOptParams);
  return OF_SUCCESS;
}


int32_t ipsec_sa_stats_ucm_getparams (struct ipsec_sa_get_inargs *in_info,
                                          struct ipsec_sa_get_outargs *out_info,
                                          struct cm_array_of_iv_pairs  *result_iv_pairs_p)
{

  char string[1024];
  char tmp_string[1024];
  char buf_port[40];
  int32_t index = 0, count_ui, iIndex;
  uint8_t bind_obj_id;  

#define CM_SASTATS_CHILD_COUNT 6
  count_ui = CM_SASTATS_CHILD_COUNT;

  result_iv_pairs_p->iv_pairs = (struct cm_iv_pair*)of_calloc(count_ui, sizeof(struct cm_iv_pair));
  if(!result_iv_pairs_p->iv_pairs)
  {
    CM_CBK_DEBUG_PRINT (" Memory allocation failed ");
    return OF_FAILURE;
  }
  of_memset(string, 0, sizeof(string));
  sprintf(string,"%d",in_info->dir);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_SASTATS_DIRECTION_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  //sprintf(string,"%d",in_info->sa_id);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_SASTATS_SPI_ID,CM_DATA_TYPE_STRING, string);
  index++;

  result_iv_pairs_p->count_ui = index;
  return OF_SUCCESS;
}


#endif /* OF_CM_SUPPORT */
