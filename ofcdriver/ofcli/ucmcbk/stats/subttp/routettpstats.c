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
 * File name: route_ttp_tats.c
 * Author: 
 * Date:  
 * Description: Arp lim Statistics.
 * 
 */

#ifdef OF_CM_SUPPORT

#include "cbcmninc.h"
#include "cmgutil.h"
#include "appids.h"
#include "secappl.h"
#include "urcu-call-rcu.h"
#include "cntlr_lock.h"
#include "cntlr_list.h"
#include "cntlr_types.h"
#include "openflow.h"
#include "of_utilities.h"
#include "of_msgapi.h"
#include "ip4_fwd_subttp.h"

int32_t route_ttp_stats_appl_init (void);

int32_t route_ttp_stats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** result_iv_pairs_pp);

int32_t route_ttp_stats_getparams (struct ipv4_unicast_subttp_stats *stats, struct cm_array_of_iv_pairs * result_iv_pairs_p);

struct cm_dm_app_callbacks route_ttp_stats_callbacks = 
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  route_ttp_stats_getexactrec,
  NULL,
  NULL
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Function Name:
 * Description:
 * Input:
 * Output:
 * Result:
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t route_ttp_stats_appl_init (void)
{
  int32_t return_value;

  return_value= cm_register_app_callbacks (CM_ON_DIRECTOR_DATAPATH_SUBTTP_ROUTE_STATS_ROUTESTATS_APPL_ID,
      &route_ttp_stats_callbacks);
  if(return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT("Route sub ttp stats CBK registration failed");
    return OF_FAILURE;
  }

  return OF_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Function Name:
 * Description:
 * Input:
 * Output:
 * Result:
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t route_ttp_stats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** result_iv_pairs_pp)
{
  int32_t return_value = OF_FAILURE;
  uint32_t  dpath_id, rec_count=0;
  struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  struct ipv4_unicast_subttp_stats route_ttp_stats;

  if ((route_ttp_stats_setmandparams (pKeysArr, &dpath_id)) !=
      OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    return OF_FAILURE;
  }

  return_value = get_ipv4_unicast_subttp_stats(&route_ttp_stats);
  if (return_value == OF_SUCCESS)
  {
    result_iv_pairs_p =
             (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
    if (result_iv_pairs_p == NULL)
    {
      CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
      return OF_FAILURE;
    }
    route_ttp_stats_getparams(&route_ttp_stats, &result_iv_pairs_p[rec_count]); 
    rec_count ++;
    *result_iv_pairs_pp = result_iv_pairs_p;
    CM_CBK_DEBUG_PRINT ("Route ttp stats response");
    return OF_SUCCESS;
  }
  *result_iv_pairs_pp = result_iv_pairs_p;
  CM_CBK_DEBUG_PRINT ("Route ttp stats response Failed");
  return OF_FAILURE;
}

int32_t route_ttp_stats_setmandparams (struct cm_array_of_iv_pairs *pMandParams,	
	                                uint32_t *dpath_id)
{
	uint32_t uiMandParamCnt;

	CM_CBK_DEBUG_PRINT ("Entered");
	for (uiMandParamCnt = 0; uiMandParamCnt < pMandParams->count_ui;
			uiMandParamCnt++)
	{
		switch (pMandParams->iv_pairs[uiMandParamCnt].id_ui)
		{

			case CM_DM_NAMESPACE_NAME_ID:
				*dpath_id=of_atoi((char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
				break;

		}
	}

	CM_CBK_PRINT_IVPAIR_ARRAY (pMandParams);
	return OF_SUCCESS;
}

int32_t route_ttp_stats_getparams (struct ipv4_unicast_subttp_stats *stats, struct cm_array_of_iv_pairs * result_iv_pairs_p)
{
  char string[64];
  int32_t index = 0;

#define CM_ROUTESTATS_CHILD_COUNT 20

  CM_CBK_DEBUG_PRINT ("Entered");
  result_iv_pairs_p->iv_pairs = (struct cm_iv_pair*)of_calloc(CM_ROUTESTATS_CHILD_COUNT, sizeof(struct cm_iv_pair));
  if(!result_iv_pairs_p->iv_pairs)
  {
    CM_CBK_DEBUG_PRINT (" Memory allocation failed");
    return OF_FAILURE;
  }

  of_memset(string, 0, sizeof(string));
  of_itoa (stats->nf_add_req_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_ROUTESUBTTPSTATS_NF_ADD_REQ_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_itoa (stats->nf_mod_req_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_ROUTESUBTTPSTATS_NF_MOD_REQ_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa ((stats->nf_del_req_count), string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_NF_DEL_REQ_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa ((stats->rt_flowmod_add_count), string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_FLOWMOD_ADD_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_flowmod_mod_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_FLOWMOD_MOD_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_flowmod_del_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_FLOWMOD_DEL_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_flowmod_add_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_FLOWMOD_ADD_FAIL_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_flowmod_fail_mod_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_FLOWMOD_MOD_FAIL_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_flowmod_del_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_FLOWMOD_DEL_FAIL_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_db_add_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_DB_ADD_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_db_mod_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_DB_MOD_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_db_del_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_DB_DEL_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_db_add_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_DB_ADD_FAIL_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_db_mod_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_DB_MOD_FAIL_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_db_del_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_DB_DEL_FAIL_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->rt_db_add_count - stats->rt_db_del_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_RT_DB_ACTIVE_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->pkt_from_dp_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESUBTTPSTATS_PKT_FROM_DP_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  result_iv_pairs_p->count_ui = index;

  CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
  return OF_SUCCESS;
}

#endif /*OF_CM_SUPPORT */
