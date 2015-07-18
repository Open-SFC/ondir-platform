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

/* File  :      dp2pethcbk.c
 * Author:      Atmaram G <atmaram.g@freescale.com>
 * Date:   08/23/2013
 * Description: A network map_entry is logical mapping between network elements.This file provides datapath ID to Namespace  mapping APIs. It also provides APIs to getfirst/getnext/getexact/delete map_entrys. NEM maintains  separate hash table and mempool list for map_entry maintanance for this database.
 */
#ifdef OF_CM_SUPPORT
#ifdef CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT


#include "cbcmninc.h"
#include "cmgutil.h"
#include "cntrucminc.h"
#include "cntrucminc.h"

int32_t of_nem_dpid2peth_appl_ucmcbk_init (void);

int32_t of_nem_dpid2peth_getfirstnrecs (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      uint32_t * count_p,
      struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p);

int32_t of_nem_dpid2peth_getnextnrecs (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      struct cm_array_of_iv_pairs * prev_record_key_p, uint32_t * count_p,
      struct cm_array_of_iv_pairs ** next_n_record_data_p);

int32_t of_nem_dpid2peth_getexactrec (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      struct cm_array_of_iv_pairs ** pIvPairArr);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * DPID2PETH  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
 * * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */
int32_t of_nem_dpid2peth_ucm_validatemandparams (struct cm_array_of_iv_pairs *
      mand_iv_pairs_p,
      struct cm_app_result **
      presult_p);

int32_t of_nem_dpid2peth_ucm_validateoptparams (struct cm_array_of_iv_pairs *
      opt_iv_pairs_p,
      struct cm_app_result ** presult_p);


int32_t of_nem_dpid2peth_ucm_setmandparams (struct cm_array_of_iv_pairs *
      mand_iv_pairs_p,
      struct of_nem_dp_peth_info * nem_map_info,
      struct cm_app_result ** presult_p);

int32_t of_nem_dpid2peth_ucm_setoptparams (struct cm_array_of_iv_pairs *
      opt_iv_pairs_p,
      struct of_nem_dp_peth_info * nem_map_info,
      struct cm_app_result ** presult_p);


int32_t of_nem_dpid2peth_ucm_getparams (struct of_nem_dp_peth_info *nem_map_info,
      struct cm_array_of_iv_pairs * result_iv_pairs_p);



struct cm_dm_app_callbacks of_nem_dpid2peth_ucm_callbacks = {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   of_nem_dpid2peth_getfirstnrecs,
   of_nem_dpid2peth_getnextnrecs,
   of_nem_dpid2peth_getexactrec,
   NULL,
   NULL
};
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * Namespace  UCM Init * * * * * * *  * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32_t of_nem_dpid2peth_appl_ucmcbk_init (void)
{
   CM_CBK_DEBUG_PRINT ("NEM - DPID2 PETH Entry");
   cm_register_app_callbacks (CM_ON_DIRECTOR_NEM_DPID2PETH_APPL_ID, &of_nem_dpid2peth_ucm_callbacks);
   return OF_SUCCESS;
}

int32_t of_nem_dpid2peth_getfirstnrecs (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      uint32_t * count_p,
      struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p)
{

   struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
   struct of_nem_dp_peth_info nem_map_info;
   uint64_t nem_db_entry_handle;
   int32_t ret_val = OF_FAILURE;
   uint32_t uiRecCount = 0;

   CM_CBK_DEBUG_PRINT ("entry");

   of_memset (&nem_map_info, 0, sizeof (struct of_nem_dp_peth_info));
   ret_val=of_nem_dpid_2_peth_db_get_first_map_entry(&nem_map_info,  &nem_db_entry_handle);
   if (ret_val != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Get first record failed for DPID to Peth");
      return OF_FAILURE;
   }
   result_iv_pairs_p =
      (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
   if (result_iv_pairs_p == NULL)
   {
      CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
      return OF_FAILURE;
   }

   of_nem_dpid2peth_ucm_getparams (&nem_map_info, &result_iv_pairs_p[uiRecCount]);
   uiRecCount++;
   *array_of_iv_pair_arr_p = result_iv_pairs_p;
   *count_p = uiRecCount;
   return OF_SUCCESS;
}


int32_t of_nem_dpid2peth_getnextnrecs (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      struct cm_array_of_iv_pairs *prev_record_key_p, uint32_t * count_p,
      struct cm_array_of_iv_pairs ** next_n_record_data_p)
{

   struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
   struct cm_app_result *app_result_p = NULL;
   struct of_nem_dp_peth_info nem_map_info;
   uint64_t nem_db_entry_handle;
   int32_t ret_val = OF_FAILURE;
   uint32_t uiRecCount = 0;

   CM_CBK_DEBUG_PRINT ("Entered");
   of_memset (&nem_map_info, 0, sizeof (struct of_nem_dp_peth_info));

   if ((of_nem_dpid2peth_ucm_setmandparams (prev_record_key_p, &nem_map_info, &app_result_p)) !=
         OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
      return OF_FAILURE;
   }

   ret_val=of_nem_dpid_2_peth_db_get_map_entry_handle(nem_map_info.dp_id, nem_map_info.port_id, &nem_db_entry_handle);
   if (ret_val != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Error: nem entry doesn't exists with dpid  %llx port_id %d",nem_map_info.dp_id, nem_map_info.port_id);
      return OF_FAILURE;
   }

   of_memset (&nem_map_info, 0, sizeof (struct of_nem_dp_peth_info));
   ret_val=of_nem_dpid_2_peth_db_get_next_map_entry(&nem_map_info, &nem_db_entry_handle);
   if (ret_val != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("failed");
      *count_p = 0;
      return OF_FAILURE;
   }
   result_iv_pairs_p =
      (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));

   if (result_iv_pairs_p == NULL)
   {
      CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
      return OF_FAILURE;
   }
   CM_CBK_DEBUG_PRINT ("Next Nsid record is : %d ",
         nem_map_info.ns_id);
   of_nem_dpid2peth_ucm_getparams (&nem_map_info, &result_iv_pairs_p[uiRecCount]);
   uiRecCount++;
   CM_CBK_DEBUG_PRINT ("Number of records requested were %d ", *count_p);
   CM_CBK_DEBUG_PRINT ("Number of records found are %d", uiRecCount+1);
   *next_n_record_data_p = result_iv_pairs_p;
   *count_p = uiRecCount;
   return OF_SUCCESS;
}

int32_t of_nem_dpid2peth_getexactrec (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      struct cm_array_of_iv_pairs ** pIvPairArr)
{

   struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
   struct cm_app_result *of_nem_result = NULL;
   struct of_nem_dp_peth_info nem_map_info;
   uint64_t nem_db_entry_handle;
   int32_t ret_val = OF_FAILURE;

   CM_CBK_DEBUG_PRINT ("Entered");
   of_memset (&nem_map_info, 0, sizeof (struct of_nem_dp_peth_info));

   if ((of_nem_dpid2peth_ucm_setmandparams (key_iv_pairs_p, &nem_map_info, &of_nem_result)) !=
         OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
      return OF_FAILURE;
   }

   ret_val=of_nem_dpid_2_peth_db_get_map_entry_handle(nem_map_info.dp_id, nem_map_info.port_id, &nem_db_entry_handle);
   if (ret_val != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Error: nem entry doesn't exists with dpid %llx",nem_map_info.dp_id);
      return OF_FAILURE;
   }

   of_memset (&nem_map_info, 0, sizeof (struct of_nem_dp_peth_info));
   ret_val=of_nem_dpid_2_peth_db_get_exact_map_entry(nem_db_entry_handle, &nem_map_info);
   if (ret_val != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Unable to find the matching record ");
      return OF_FAILURE;
   }

   CM_CBK_DEBUG_PRINT ("Exact matching record found");
   result_iv_pairs_p =
      (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));

   if (result_iv_pairs_p == NULL)
   {
      CM_CBK_DEBUG_PRINT ("Failed to allocate memory for result_iv_pairs_p");
      return OF_FAILURE;
   }
   of_nem_dpid2peth_ucm_getparams (&nem_map_info, result_iv_pairs_p);
   *pIvPairArr = result_iv_pairs_p;
   return OF_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * DPID2PETH  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
 * * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */

int32_t of_nem_dpid2peth_ucm_validatemandparams (struct cm_array_of_iv_pairs *
      mand_iv_pairs_p,
      struct cm_app_result **  presult_p)
{
   uint32_t count;


   CM_CBK_DEBUG_PRINT ("Entered");
   for (count = 0; count < mand_iv_pairs_p->count_ui;
         count++)
   {
      switch (mand_iv_pairs_p->iv_pairs[count].id_ui)
      {
         case CM_DM_DPID2PETH_NSID_ID:
            break;
         default:
            break;
      }
   }
   CM_CBK_PRINT_IVPAIR_ARRAY (mand_iv_pairs_p);
   return OF_SUCCESS;
}

int32_t of_nem_dpid2peth_ucm_validateoptparams (struct cm_array_of_iv_pairs *
      opt_iv_pairs_p,
      struct cm_app_result ** presult_p)
{

   CM_CBK_DEBUG_PRINT ("Entered");
   return OF_SUCCESS;
}

int32_t of_nem_dpid2peth_ucm_setmandparams (struct cm_array_of_iv_pairs *
      mand_iv_pairs_p,
      struct of_nem_dp_peth_info * nem_map_info,
      struct cm_app_result ** presult_p)
{
   uint32_t count;
   int32_t ns_id;
   int64_t dp_id;

   CM_CBK_DEBUG_PRINT ("Entered");
   for (count = 0; count < mand_iv_pairs_p->count_ui;
         count++)
   {
      switch (mand_iv_pairs_p->iv_pairs[count].id_ui)
      {
         case CM_DM_DPID2PETH_NSID_ID:
            ns_id=of_atoi((char *) mand_iv_pairs_p->iv_pairs[count].value_p);
            nem_map_info->ns_id=ns_id;
            break;
         case CM_DM_DPID2PETH_PORTNAME_ID:
            strcpy(nem_map_info->port_name, mand_iv_pairs_p->iv_pairs[count].value_p);
            break;
         case CM_DM_DPID2PETH_PETHNAME_ID:
            strcpy(nem_map_info->peth_name, mand_iv_pairs_p->iv_pairs[count].value_p);
            break;
         case CM_DM_DPID2PETH_DPID_ID:
            dp_id=charTo64bitNum((char *) mand_iv_pairs_p->iv_pairs[count].value_p);
            CM_CBK_DEBUG_PRINT("dpid is: %llx", dp_id);
            nem_map_info->dp_id=dp_id;
            break;
         default:
            break;

      }
   }
   CM_CBK_PRINT_IVPAIR_ARRAY (mand_iv_pairs_p);
   return OF_SUCCESS;
}

int32_t of_nem_dpid2peth_ucm_setoptparams (struct cm_array_of_iv_pairs *
      opt_iv_pairs_p,
      struct of_nem_dp_peth_info * nem_map_info,
      struct cm_app_result ** presult_p)
{
   uint32_t count;

   CM_CBK_DEBUG_PRINT ("Entered");

   for (count = 0; count < opt_iv_pairs_p->count_ui; count++)
   {
      switch (opt_iv_pairs_p->iv_pairs[count].id_ui)
      {
         default:
            break;
      }
   }

   CM_CBK_PRINT_IVPAIR_ARRAY (opt_iv_pairs_p);
   return OF_SUCCESS;
}



int32_t of_nem_dpid2peth_ucm_getparams (struct of_nem_dp_peth_info *nem_map_info,
      struct cm_array_of_iv_pairs * result_iv_pairs_p)
{
   uint32_t index_ui = 0;
   char buf[NEM_MAX_IFACE_NAME_LEN] = "";
#define CM_DPID2PETH_CHILD_COUNT 4

   result_iv_pairs_p->iv_pairs =
      (struct cm_iv_pair *) of_calloc (CM_DPID2PETH_CHILD_COUNT, sizeof (struct cm_iv_pair));
   if (result_iv_pairs_p->iv_pairs == NULL)
   {
      CM_CBK_DEBUG_PRINT
         ("Memory allocation failed for result_iv_pairs_p->iv_pairs");
      return OF_FAILURE;
   }

   /*DpId*/
   sprintf(buf,"%llx",nem_map_info->dp_id);
   FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[index_ui],CM_DM_DPID2PETH_DPID_ID,
         CM_DATA_TYPE_STRING, buf);
   index_ui++;

#if 0
   /*port name*/
   strcpy (buf,nem_map_info->port_name);
   FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[index_ui],CM_DM_DPID2PETH_PORTNAME_ID,
         CM_DATA_TYPE_STRING, buf);
   index_ui++;
#endif

   /*PortId*/
   of_itoa (nem_map_info->port_id, buf);
   FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[index_ui],CM_DM_DPID2PETH_PORTID_ID,
         CM_DATA_TYPE_STRING, buf);
   index_ui++;

   /*peth name*/
   strcpy (buf,nem_map_info->peth_name);
   FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[index_ui],CM_DM_DPID2PETH_PETHNAME_ID,
         CM_DATA_TYPE_STRING, buf);
   index_ui++;

   /*NsId*/
   of_itoa (nem_map_info->ns_id, buf);
   FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[index_ui],CM_DM_DPID2PETH_NSID_ID,
         CM_DATA_TYPE_STRING, buf);
   index_ui++;


   result_iv_pairs_p->count_ui = index_ui;
   CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
   return OF_SUCCESS;
}


#endif /*CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT */
#endif /* OF_CM_SUPPORT */
