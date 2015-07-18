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
 * File name: nsid2nscbk.c
 * Author: G Atmaram <B38856@freescale.com>
 * Date:   08/23/2013
 * Description: 
 * 
 */

#ifdef OF_CM_SUPPORT
#ifdef CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT


#include "cbcmninc.h"
#include "cmgutil.h"
#include "cntrucminc.h"
#include "cntrucminc.h"

int32_t of_nem_nsid2ns_appl_ucmcbk_init (void);

int32_t of_nem_nsid2ns_getfirstnrecs (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      uint32_t * count_p,
      struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p);

int32_t of_nem_nsid2ns_getnextnrecs (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      struct cm_array_of_iv_pairs * prev_record_key_p, uint32_t * count_p,
      struct cm_array_of_iv_pairs ** next_n_record_data_p);

int32_t of_nem_nsid2ns_getexactrec (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      struct cm_array_of_iv_pairs ** pIvPairArr);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * NSID2NS  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
 * * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */
int32_t of_nem_nsid2ns_ucm_validatemandparams (struct cm_array_of_iv_pairs *
      mand_iv_pairs_p,
      struct cm_app_result **
      presult_p);

int32_t of_nem_nsid2ns_ucm_validateoptparams (struct cm_array_of_iv_pairs *
      opt_iv_pairs_p,
      struct cm_app_result ** presult_p);


int32_t of_nem_nsid2ns_ucm_setmandparams (struct cm_array_of_iv_pairs *
      mand_iv_pairs_p,
      struct of_nem_ns_info * nem_map_info,
      struct cm_app_result ** presult_p);

int32_t of_nem_nsid2ns_ucm_setoptparams (struct cm_array_of_iv_pairs *
      opt_iv_pairs_p,
      struct of_nem_ns_info * nem_map_info,
      struct cm_app_result ** presult_p);


int32_t of_nem_nsid2ns_ucm_getparams (struct of_nem_ns_info *nem_map_info,
      struct cm_array_of_iv_pairs * result_iv_pairs_p);



struct cm_dm_app_callbacks of_nem_nsid2ns_ucm_callbacks = {
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   of_nem_nsid2ns_getfirstnrecs,
   of_nem_nsid2ns_getnextnrecs,
   of_nem_nsid2ns_getexactrec,
   NULL,
   NULL
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * Namespace  UCM Init * * * * * * *  * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int32_t of_nem_nsid2ns_appl_ucmcbk_init (void)
{
   CM_CBK_DEBUG_PRINT ("NEM - NSID 2 NS Entry");
   cm_register_app_callbacks (CM_ON_DIRECTOR_NEM_NSID2NS_APPL_ID, &of_nem_nsid2ns_ucm_callbacks);
   return OF_SUCCESS;
}

int32_t of_nem_nsid2ns_getfirstnrecs (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      uint32_t * count_p,
      struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p)
{

   struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
   struct of_nem_ns_info nem_map_info;
   uint64_t nem_db_entry_handle;
   int32_t ret_val = OF_FAILURE;
   uint32_t uiRecCount = 0;

   CM_CBK_DEBUG_PRINT ("entry");

   of_memset (&nem_map_info, 0, sizeof (struct of_nem_ns_info));
   ret_val=of_nem_nsid_2_ns_db_get_first_map_entry(&nem_map_info,  &nem_db_entry_handle);
   if (ret_val != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Get first record failed for Namespace");
      return OF_FAILURE;
   }
   result_iv_pairs_p =
      (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
   if (result_iv_pairs_p == NULL)
   {
      CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
      return OF_FAILURE;
   }

   of_nem_nsid2ns_ucm_getparams (&nem_map_info, &result_iv_pairs_p[uiRecCount]);
   uiRecCount++;
   *array_of_iv_pair_arr_p = result_iv_pairs_p;
   *count_p = uiRecCount;
   return OF_SUCCESS;
}


int32_t of_nem_nsid2ns_getnextnrecs (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      struct cm_array_of_iv_pairs *prev_record_key_p, uint32_t * count_p,
      struct cm_array_of_iv_pairs ** next_n_record_data_p)
{

   struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
   struct cm_app_result *app_result_p = NULL;
   struct of_nem_ns_info nem_map_info;
   uint64_t nem_db_entry_handle;
   int32_t ret_val = OF_FAILURE;
   uint32_t uiRecCount = 0;

   CM_CBK_DEBUG_PRINT ("Entered");
   of_memset (&nem_map_info, 0, sizeof (struct of_nem_ns_info));

   if ((of_nem_nsid2ns_ucm_setmandparams (prev_record_key_p, &nem_map_info, &app_result_p)) !=
         OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
      return OF_FAILURE;
   }

   ret_val=of_nem_nsid_2_ns_db_get_map_entry_handle(nem_map_info.id, &nem_db_entry_handle);
   if (ret_val != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Error: nem entry does not exist with id  %d",nem_map_info.id);
      return OF_FAILURE;
   }

   of_memset (&nem_map_info, 0, sizeof (struct of_nem_ns_info));
   ret_val=of_nem_nsid_2_ns_db_get_next_map_entry(&nem_map_info, &nem_db_entry_handle);
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
   CM_CBK_DEBUG_PRINT ("Next Namespace record is : %s ",
         nem_map_info.name);
   of_nem_nsid2ns_ucm_getparams (&nem_map_info, &result_iv_pairs_p[uiRecCount]);
   uiRecCount++;
   CM_CBK_DEBUG_PRINT ("Number of records requested were %ld ", *count_p);
   CM_CBK_DEBUG_PRINT ("Number of records found are %ld", uiRecCount+1);
   *next_n_record_data_p = result_iv_pairs_p;
   *count_p = uiRecCount;
   return OF_SUCCESS;
}

int32_t of_nem_nsid2ns_getexactrec (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      struct cm_array_of_iv_pairs ** pIvPairArr)
{

   struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
   struct cm_app_result *of_nem_result = NULL;
   struct of_nem_ns_info nem_map_info;
   uint64_t nem_db_entry_handle;
   int32_t ret_val = OF_FAILURE;

   CM_CBK_DEBUG_PRINT ("Entered");
   of_memset (&nem_map_info, 0, sizeof (struct of_nem_ns_info));

   if ((of_nem_nsid2ns_ucm_setmandparams (key_iv_pairs_p, &nem_map_info, &of_nem_result)) !=
         OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
      return OF_FAILURE;
   }

   ret_val=of_nem_nsid_2_ns_db_get_map_entry_handle(nem_map_info.id, &nem_db_entry_handle);
   if (ret_val != OF_SUCCESS)
   {
      CM_CBK_DEBUG_PRINT ("Error: nem entry does not exist with id %d",nem_map_info.id);
      return OF_FAILURE;
   }

   of_memset (&nem_map_info, 0, sizeof (struct of_nem_ns_info));
   ret_val=of_nem_nsid_2_ns_db_get_exact_map_entry(nem_db_entry_handle, &nem_map_info);
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
   of_nem_nsid2ns_ucm_getparams (&nem_map_info, result_iv_pairs_p);
   *pIvPairArr = result_iv_pairs_p;
   return OF_SUCCESS;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * NSID2NS  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
 * * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */

int32_t of_nem_nsid2ns_ucm_validatemandparams (struct cm_array_of_iv_pairs *
      mand_iv_pairs_p,
      struct cm_app_result **  presult_p)
{
   uint32_t count;
   struct cm_app_result *of_nem_result = NULL;


   CM_CBK_DEBUG_PRINT ("Entered");
   for (count = 0; count < mand_iv_pairs_p->count_ui;
         count++)
   {
      switch (mand_iv_pairs_p->iv_pairs[count].id_ui)
      {

         case CM_DM_NSID2NS_NSNAME_ID:
            if (mand_iv_pairs_p->iv_pairs[count].value_p == NULL)
            {
               CM_CBK_DEBUG_PRINT ("Namespace Name is NULL");
               fill_app_result_struct (&of_nem_result, NULL, CM_GLU_NAMESPACE_NAME_NULL);
               *presult_p = of_nem_result;
               return OF_FAILURE;
            }
            break;

         case CM_DM_NSID2NS_NSID_ID:
            break;
         default:
            break;
      }
   }
   CM_CBK_PRINT_IVPAIR_ARRAY (mand_iv_pairs_p);
   return OF_SUCCESS;
}

int32_t of_nem_nsid2ns_ucm_validateoptparams (struct cm_array_of_iv_pairs *
      opt_iv_pairs_p,
      struct cm_app_result ** presult_p)
{
   uint32_t count;
   struct cm_app_result *of_nem_result = NULL;

   CM_CBK_DEBUG_PRINT ("Entered");
   for (count = 0; count < opt_iv_pairs_p->count_ui; count++)
   {
      switch (opt_iv_pairs_p->iv_pairs[count].id_ui)
      {
         case CM_DM_NSID2NS_NSNAME_ID:
            if (opt_iv_pairs_p->iv_pairs[count].value_p == NULL)
            {
               CM_CBK_DEBUG_PRINT ("Fully Qualified Domain Name (FQDN) is NULL");
               fill_app_result_struct (&of_nem_result, NULL, CM_GLU_NAMESPACE_NAME_NULL);
               *presult_p = of_nem_result;
               return OF_FAILURE;
            }
            break;
         case CM_DM_NSID2NS_NSID_ID:
            break;
         default:
            break;
      }
   }
   CM_CBK_PRINT_IVPAIR_ARRAY (opt_iv_pairs_p);
   return OF_SUCCESS;
}

int32_t of_nem_nsid2ns_ucm_setmandparams (struct cm_array_of_iv_pairs *
      mand_iv_pairs_p,
      struct of_nem_ns_info * nem_map_info,
      struct cm_app_result ** presult_p)
{
   uint32_t count;
   int32_t ns_id;
   CM_CBK_DEBUG_PRINT ("Entered");
   for (count = 0; count < mand_iv_pairs_p->count_ui;
         count++)
   {
      switch (mand_iv_pairs_p->iv_pairs[count].id_ui)
      {

         case CM_DM_NSID2NS_NSNAME_ID:
            of_strncpy (nem_map_info->name,
                  (char *) mand_iv_pairs_p->iv_pairs[count].value_p,
                  mand_iv_pairs_p->iv_pairs[count].value_length);
            break;
         case CM_DM_NSID2NS_NSID_ID:
            ns_id=of_atoi((char *) mand_iv_pairs_p->iv_pairs[count].value_p);
            nem_map_info->id=ns_id;
            break;
         default:
            break;

      }
   }
   CM_CBK_PRINT_IVPAIR_ARRAY (mand_iv_pairs_p);
   return OF_SUCCESS;
}

int32_t of_nem_nsid2ns_ucm_setoptparams (struct cm_array_of_iv_pairs *
      opt_iv_pairs_p,
      struct of_nem_ns_info * nem_map_info,
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



int32_t of_nem_nsid2ns_ucm_getparams (struct of_nem_ns_info *nem_map_info,
      struct cm_array_of_iv_pairs * result_iv_pairs_p)
{
   uint32_t uindex_i = 0;
   char buf[10] = "";

#define CM_NSID2NS_CHILD_COUNT 2

   result_iv_pairs_p->iv_pairs =
      (struct cm_iv_pair *) of_calloc (CM_NSID2NS_CHILD_COUNT, sizeof (struct cm_iv_pair));
   if (result_iv_pairs_p->iv_pairs == NULL)
   {
      CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p->iv_pairs");
      return OF_FAILURE;
   }


   /*NsId*/
   of_itoa (nem_map_info->id, buf);
   FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i],CM_DM_NSID2NS_NSID_ID,
         CM_DATA_TYPE_STRING, buf);
   uindex_i++;

   /* Namespace Interface*/
   FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i], CM_DM_NSID2NS_NSNAME_ID,
         CM_DATA_TYPE_STRING, nem_map_info->name);
   uindex_i++;

   result_iv_pairs_p->count_ui = uindex_i;
   CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
   return OF_SUCCESS;
}


#endif /* CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT */
#endif /* OF_CM_SUPPORT */
