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
* File name: queuestats.c
* Author:
* Date: 
* Description: Queue Statistics.
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

int32_t of_queue_appl_ucmcbk_init (void);

int32_t of_queue_getfirstnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pArrayofIvPairArr);

int32_t of_queue_getnextnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs * pPrevRecordKey, uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pNextNRecordData_p);

int32_t of_queue_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** pIvPairArr);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * *   U T I L I T Y  F U N C T I O N S         * * * * * * * * * * *
* * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */
int32_t of_queue_ucm_setmandparams (struct cm_array_of_iv_pairs *
    pMandParams,
    struct dprm_datapath_general_info *datapath_info,
    struct ofl_queue_config* stats_info,
    struct cm_app_result ** ppResult);

int32_t of_queue_ucm_getparams (struct ofl_queue_config *stats_info,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);

struct cm_dm_app_callbacks of_queue_ucm_callbacks = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  of_queue_getfirstnrecs,
  of_queue_getnextnrecs,
  of_queue_getexactrec,
  NULL,
  NULL
};
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * Switch  UCM Init * * * * * * *  * * * * * * * * * * *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32_t of_queue_appl_ucmcbk_init (void)
{
  CM_CBK_DEBUG_PRINT ("Entry");
  cm_register_app_callbacks (CM_ON_DIRECTOR_DATAPATH_STATS_FLOWSTATS_APPL_ID,
           &of_queue_ucm_callbacks);
  return OF_SUCCESS;
}


int32_t of_queue_getfirstnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pArrayofIvPairArr)
{
  int32_t return_value = OF_FAILURE;
  struct cm_app_result *stats_result = NULL;
  struct dprm_datapath_general_info datapath_info={};
  struct ofl_queue_config stats_info;
  uint64_t datapath_handle;

  of_memset (&stats_info, 0, sizeof (struct ofl_queue_config));

  if ((of_queue_ucm_setmandparams (pKeysArr,&datapath_info, &stats_info, &stats_result)) !=
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

  return_value=of_queue_send_get_queue_config_request(datapath_handle, OFPP_ANY); 
  if ( return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("queue failed");
    return OF_FAILURE;
  } 
  CM_CBK_DEBUG_PRINT ("flow stats will be sent as multipart response");
  return CM_CNTRL_MULTIPART_RESPONSE;
}


int32_t of_queue_getnextnrecs (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs *pPrevRecordKey, uint32_t * pCount,
    struct cm_array_of_iv_pairs ** pNextNRecordData_p)
{

  CM_CBK_DEBUG_PRINT ("Entered");
  *pCount=0;
  return OF_FAILURE;

}

int32_t of_queue_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** pIvPairArr)
{
  int32_t return_value = OF_FAILURE;
  struct cm_app_result *stats_result = NULL;
  struct dprm_datapath_general_info datapath_info={};
  struct ofl_queue_config stats_info;
  uint64_t datapath_handle;

  of_memset (&stats_info, 0, sizeof (struct ofl_queue_config));

  if ((of_queue_ucm_setmandparams (pKeysArr,&datapath_info, &stats_info, &stats_result)) !=
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

  return_value=of_queue_send_get_queue_config_request(datapath_handle, stats_info.port); 
  if ( return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("queue failed");
    return OF_FAILURE;
  } 
  CM_CBK_DEBUG_PRINT ("queue stats will be sent as multipart response");
  return CM_CNTRL_MULTIPART_RESPONSE;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
* * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */

int32_t of_queue_ucm_setmandparams (struct cm_array_of_iv_pairs *
    pMandParams,
    struct dprm_datapath_general_info *datapath_info,
    struct ofl_queue_config* stats_info,struct cm_app_result ** ppResult)
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

      case CM_DM_QUEUESTATS_PORT_ID: 
        stats_info->port = of_atoi((char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
        CM_CBK_DEBUG_PRINT ("Port ID: %d",stats_info->port);
        break;
    }
  }
  CM_CBK_PRINT_IVPAIR_ARRAY (pMandParams);
  return OF_SUCCESS;
}

int32_t of_queue_ucm_getparams (struct ofl_queue_config *stats_info,
    struct cm_array_of_iv_pairs * result_iv_pairs_p)
{

  char string[1024];
  int32_t index = 0, count_ui=500;
  struct ofl_queue *queue_entry_p;

  result_iv_pairs_p->iv_pairs = (struct cm_iv_pair*)of_calloc(count_ui, sizeof(struct cm_iv_pair));
  if(!result_iv_pairs_p->iv_pairs)
  {
    CM_CBK_DEBUG_PRINT (" Memory allocation failed ");
    return OF_FAILURE;
  }

  of_memset(string, 0, sizeof(string));
  of_itoa(stats_info->port, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_QUEUESTATS_PORT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_sprintf(string,"%d",OF_LIST_COUNT(stats_info->queue_list));
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_QUEUESTATS_QUEUE_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  OF_LIST_SCAN(stats_info->queue_list, queue_entry_p, struct ofl_queue *, OF_QUEUE_LISTNODE_OFFSET)
  {
    of_memset(string, 0, sizeof(string));
    of_itoa(queue_entry_p->ofq_id, string);
    FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_QUEUESTATS_OPENFLOW_QUEUE_ID, CM_DATA_TYPE_STRING, string);
    index++;
   
    of_memset(string, 0, sizeof(string));
    of_itoa(queue_entry_p->cq_id, string);
    FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_QUEUESTATS_CLASS_QUEUE_ID, CM_DATA_TYPE_STRING, string);
    index++;

  }

  result_iv_pairs_p->count_ui = index;
  return OF_SUCCESS;
}


#endif /* OF_CM_SUPPORT */
