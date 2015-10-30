/*
** Netwrok Service resource manager tenant source file 
* Copyright (c) 2012, 2013  Freescale.
*  
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
* 
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

/*
 *
 * File name: chainsetzone.c
 * Author:
 * Date: 
 * Description: 
*/
#include "cmincld.h"
#include "cbcmninc.h"
#include "cmgutil.h"
#include "cntrucminc.h"
#include "nsrm.h"

int32_t nsrm_chainsetzone_init (void);

int32_t nsrm_chainsetzone_addrec (void * config_trans_p,
                        struct cm_array_of_iv_pairs * pMandParams,
                        struct cm_array_of_iv_pairs * pOptParams,
                        struct cm_app_result ** result_p);

int32_t nsrm_chainsetzone_modrec (void * config_trans_p,
                        struct cm_array_of_iv_pairs * pMandParams,
                        struct cm_array_of_iv_pairs * pOptParams,
                        struct cm_app_result ** result_p);


int32_t nsrm_chainsetzone_delrec (void * config_transaction_p,
                        struct cm_array_of_iv_pairs * keys_arr_p,
                        struct cm_app_result ** result_p);


int32_t nsrm_chainsetzone_getfirstnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
                        uint32_t * count_p,
                        struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p);



int32_t nsrm_chainsetzone_getnextnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
                        struct cm_array_of_iv_pairs *prev_record_key_p,  uint32_t * count_p,
                        struct cm_array_of_iv_pairs ** next_n_record_data_p);



int32_t nsrm_chainsetzone_getexactrec(struct cm_array_of_iv_pairs * key_iv_pairs_p,
                                        struct cm_array_of_iv_pairs ** pIvPairArr);

int32_t nsrm_chainsetzone_verifycfg (struct cm_array_of_iv_pairs *key_iv_pairs_p,
                        uint32_t command_id, struct cm_app_result ** result_p);


int32_t nsrm_chainsetzone_ucm_setmandparams (struct cm_array_of_iv_pairs *pMandParams,
                        struct nsrm_nschainset_object_key *nshcainset_object_key_p,
                        struct nsrm_zone_direction_rule_key* appl_category_info,
                        struct nsrm_zone_direction_rule_config_info* nsrm_chainsetzone_config_info);


int32_t nsrm_chainsetzone_ucm_setoptparams (struct cm_array_of_iv_pairs *pOptParams,
                                  struct nsrm_zone_direction_rule_key* appl_chainsetzone_info,
                        struct nsrm_zone_direction_rule_config_info* nsrm_chainsetzone_config_info);

int32_t nsrm_chainsetzone_ucm_getparams (struct nsrm_zone_rule *appl_chainsetzone_info,
                                   struct cm_array_of_iv_pairs * result_iv_pairs_p);

int32_t nsrm_chainsetzone_ucm_validatemandparams(struct cm_array_of_iv_pairs *mand_iv_pairs_p,
                                                 struct cm_app_result **  presult_p);


struct cm_dm_app_callbacks nsrm_chainsetzone_callbacks =
{
  NULL,
  nsrm_chainsetzone_addrec,
  nsrm_chainsetzone_modrec,
  nsrm_chainsetzone_delrec,
  NULL,
  nsrm_chainsetzone_getfirstnrecs,
  nsrm_chainsetzone_getnextnrecs,
  nsrm_chainsetzone_getexactrec,
  nsrm_chainsetzone_verifycfg,
  NULL
};

int32_t nsrm_chainsetzone_init (void)
{
  CM_CBK_DEBUG_PRINT ("Entry");

  if (cm_register_app_callbacks (CM_ON_DIRECTOR_NSRM_CHAINSET_CHAINSETZONERULE_APPL_ID, 
               &nsrm_chainsetzone_callbacks) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Failed to initialize callbacks");
    return OF_FAILURE;
  }

  return OF_SUCCESS;
}

int32_t nsrm_chainsetzone_addrec (void * config_trans_p,
                        struct cm_array_of_iv_pairs * pMandParams,
                        struct cm_array_of_iv_pairs * pOptParams,
                        struct cm_app_result ** result_p)
{
   struct cm_app_result *nsrm_chainsetzone_result = NULL;
   int32_t return_value = OF_FAILURE;
   struct nsrm_zone_direction_rule_key *nsrm_chainsetzone_key_info;
   struct nsrm_zone_direction_rule_config_info *nsrm_chainsetzone_config_info;
   struct   nsrm_nschainset_object_key*     nschainset_object_key_p;
   
   CM_CBK_DEBUG_PRINT ("Entered(nsrm_chainsetzone_addrec)");
   nschainset_object_key_p = (struct nsrm_nschainset_object_key *)of_calloc(1,sizeof(struct nsrm_nschainset_object_key));
   nsrm_chainsetzone_key_info = (struct nsrm_zone_direction_rule_key*) 
                    of_calloc(1, sizeof(struct nsrm_zone_direction_rule_key));
   nsrm_chainsetzone_config_info = (struct nsrm_zone_direction_rule_config_info *) 
                    of_calloc(NSRM_MAX_ZONE_RULE_CONFIG_PARAMETERS, sizeof(struct nsrm_zone_direction_rule_config_info));
   if ((nsrm_chainsetzone_ucm_setmandparams (pMandParams,nschainset_object_key_p, nsrm_chainsetzone_key_info, 
                  nsrm_chainsetzone_config_info)) != OF_SUCCESS)
   {
     CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
     fill_app_result_struct (&nsrm_chainsetzone_result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
     *result_p=nsrm_chainsetzone_result;
     of_free(nsrm_chainsetzone_key_info->name_p);
     of_free(nsrm_chainsetzone_key_info->tenant_name_p);
     of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
     of_free(nsrm_chainsetzone_key_info);
     return OF_FAILURE;
   }

  CM_CBK_DEBUG_PRINT("after setmand");
  CM_CBK_DEBUG_PRINT("after setmand");
  if ((nsrm_chainsetzone_ucm_setoptparams (pOptParams, nsrm_chainsetzone_key_info,
                  nsrm_chainsetzone_config_info)) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Optional Parameters Failed");
    fill_app_result_struct (&nsrm_chainsetzone_result, NULL, CM_GLU_SET_OPT_PARAM_FAILED);
    *result_p=nsrm_chainsetzone_result;
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }

  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT("name : %s",nsrm_chainsetzone_key_info->name_p);
  return_value = nsrm_add_zone_to_nschainset(nsrm_chainsetzone_key_info,
                                             2, nsrm_chainsetzone_config_info);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("nsrm chainsetzone add  Failed");
    fill_app_result_struct (&nsrm_chainsetzone_result, NULL, CM_GLU_ZONE_RULE_ADD_FAILED);
    *result_p = nsrm_chainsetzone_result;
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }
  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT ("NSRM chainsetzone added successfully");
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
  return OF_SUCCESS;
}

int32_t nsrm_chainsetzone_modrec (void * config_trans_p,
                        struct cm_array_of_iv_pairs * pMandParams,
                        struct cm_array_of_iv_pairs * pOptParams,
                        struct cm_app_result ** result_p)
{
   struct cm_app_result *nsrm_chainsetzone_result = NULL;
   int32_t return_value = OF_FAILURE;
   struct nsrm_zone_direction_rule_key *nsrm_chainsetzone_key_info = NULL;
   struct nsrm_zone_direction_rule_config_info *nsrm_chainsetzone_config_info = NULL;
   struct   nsrm_nschainset_object_key*     nschainset_object_key_p = NULL;
   
   nschainset_object_key_p = (struct nsrm_nschainset_object_key *)of_calloc(1,sizeof(struct nsrm_nschainset_object_key));

   CM_CBK_DEBUG_PRINT ("Entered(nsrm_category_addrec)");

  nsrm_chainsetzone_key_info = (struct nsrm_zone_direction_rule_key*) 
                         of_calloc(1, sizeof(struct nsrm_zone_direction_rule_key));
  nsrm_chainsetzone_config_info = (struct nsrm_zone_direction_rule_config_info *) 
                          of_calloc(NSRM_MAX_ZONE_RULE_CONFIG_PARAMETERS, sizeof(struct nsrm_zone_direction_rule_config_info));
  if ((nsrm_chainsetzone_ucm_setmandparams (pMandParams,nschainset_object_key_p, nsrm_chainsetzone_key_info,
                       nsrm_chainsetzone_config_info)) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    fill_app_result_struct (&nsrm_chainsetzone_result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
    *result_p=nsrm_chainsetzone_result;
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }
 if ((nsrm_chainsetzone_ucm_setoptparams (pOptParams, nsrm_chainsetzone_key_info,
                  nsrm_chainsetzone_config_info)) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Optional Parameters Failed");
    fill_app_result_struct (&nsrm_chainsetzone_result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
    *result_p=nsrm_chainsetzone_result;
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }

  return_value = nsrm_modify_zone(nsrm_chainsetzone_key_info, 2, 
                                  nsrm_chainsetzone_config_info);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("nsrm chainsetzone modify  Failed");
    fill_app_result_struct (&nsrm_chainsetzone_result, NULL, CM_GLU_ZONE_RULE_MOD_FAILED);
    *result_p = nsrm_chainsetzone_result;
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }
  CM_CBK_DEBUG_PRINT ("NSRM chainsetzone modified successfully");
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
  return OF_SUCCESS;
}

int32_t nsrm_chainsetzone_delrec (void * config_transaction_p,
                        struct cm_array_of_iv_pairs * keys_arr_p,
                        struct cm_app_result ** result_p)
{

   struct cm_app_result *nsrm_chainsetzone_result = NULL;
  int32_t return_value = OF_FAILURE;
   struct nsrm_zone_direction_rule_key *nsrm_chainsetzone_key_info;
   struct nsrm_zone_direction_rule_config_info *nsrm_chainsetzone_config_info;
   struct   nsrm_nschainset_object_key*     nschainset_object_key_p;

   nschainset_object_key_p = (struct nsrm_nschainset_object_key *)of_calloc(1,sizeof(struct nsrm_nschainset_object_key));


  CM_CBK_DEBUG_PRINT ("Entered");
  nsrm_chainsetzone_key_info = (struct nsrm_zone_direction_rule_key*) of_calloc(1, sizeof(struct nsrm_nschain_selection_rule_key));
  nsrm_chainsetzone_config_info = (struct nsrm_zone_direction_rule_config_info *) 
                          of_calloc(1, sizeof(struct nsrm_zone_direction_rule_config_info));
  if ((nsrm_chainsetzone_ucm_setmandparams (keys_arr_p,nschainset_object_key_p, nsrm_chainsetzone_key_info,
                     nsrm_chainsetzone_config_info)) !=OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    fill_app_result_struct (&nsrm_chainsetzone_result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
    *result_p = nsrm_chainsetzone_result;
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }
  return_value = nsrm_del_zone_from_nschainset(nsrm_chainsetzone_key_info);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("nsrm chainsetzone add  Failed");
    fill_app_result_struct (&nsrm_chainsetzone_result, NULL, CM_GLU_ZONE_RULE_DEL_FAILED);
    *result_p = nsrm_chainsetzone_result;
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }
  CM_CBK_DEBUG_PRINT ("NSRM Appliance delete successfully");
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
  return OF_SUCCESS;

}

int32_t nsrm_chainsetzone_getfirstnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
                        uint32_t * count_p,
                        struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p)
{
  struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  int32_t return_value = OF_FAILURE;
  uint32_t uiRecCount = 0;
  int32_t uiRequestedCount = 1, uiReturnedCnt=0;
  struct nsrm_zone_rule* nsrm_chainsetzone_config_info;
  struct nsrm_zone_direction_rule_key *nsrm_chainsetzone_key_info;
  struct   nsrm_zone_direction_rule_config_info*   nsrm_srvmap_config_info;
  struct   nsrm_nschainset_object_key*     nschainset_object_key_p;

  CM_CBK_DEBUG_PRINT ("Entered");
  nschainset_object_key_p = (struct nsrm_nschainset_object_key *) 
          of_calloc(1, sizeof(struct nsrm_nschainset_object_key));
  nsrm_chainsetzone_key_info = (struct nsrm_zone_direction_rule_key*) of_calloc(1, 
                   sizeof(struct nsrm_zone_direction_rule_key));
  nsrm_srvmap_config_info = (struct nsrm_zone_direction_rule_config_info *) 
                          of_calloc(NSRM_MAX_ZONE_RULE_CONFIG_PARAMETERS, sizeof(struct nsrm_zone_direction_rule_config_info));

  if ((nsrm_chainsetzone_ucm_setmandparams (keys_arr_p,nschainset_object_key_p,
                                            nsrm_chainsetzone_key_info, 
                                            nsrm_srvmap_config_info)) != OF_SUCCESS)
  {
     CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
     of_free(nsrm_chainsetzone_key_info->name_p);
     of_free(nsrm_chainsetzone_key_info->tenant_name_p);
     of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
     of_free(nsrm_chainsetzone_key_info);
     return OF_FAILURE;
  }

  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT("tenant name : %s",nsrm_chainsetzone_key_info->tenant_name_p);
  CM_CBK_DEBUG_PRINT("chainset name : %s",nsrm_chainsetzone_key_info->nschainset_object_name_p);
  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT("debug");
  if ((nsrm_chainsetzone_key_info->nschainset_object_name_p == NULL) ||
        (nschainset_object_key_p->name_p == NULL))
  {
     CM_CBK_DEBUG_PRINT ("Parent table key is missing, not proceed further");
     of_free(nsrm_chainsetzone_key_info->name_p);
     of_free(nsrm_chainsetzone_key_info->tenant_name_p);
     of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
     of_free(nsrm_chainsetzone_key_info);
     return OF_FAILURE;
  }
  nsrm_chainsetzone_config_info = (struct nsrm_zone_rule *)of_calloc(1,sizeof(struct nsrm_zone_rule));

  nsrm_chainsetzone_config_info->info = (struct nsrm_zone_direction_rule_config_info *)
                          of_calloc(NSRM_MAX_ZONE_RULE_CONFIG_PARAMETERS, sizeof(struct nsrm_zone_direction_rule_config_info));
  nsrm_chainsetzone_config_info->key.name_p = (char*)of_calloc(1,NSRM_MAX_NAME_LENGTH); 
  nsrm_chainsetzone_config_info->key.tenant_name_p = (char*)of_calloc(1,NSRM_MAX_NAME_LENGTH); 
  nsrm_chainsetzone_config_info->key.nschainset_object_name_p = (char*)of_calloc(1,NSRM_MAX_NAME_LENGTH); 
 
  return_value = nsrm_get_first_zone_from_nschainset_object(nschainset_object_key_p,
                   uiRequestedCount, &uiReturnedCnt, nsrm_chainsetzone_config_info);
  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT("debug");
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Get first record failed for NSRM chainsetzone  Table");
    return OF_FAILURE;
  }

  result_iv_pairs_p = (struct cm_array_of_iv_pairs *) of_calloc(1, sizeof (struct cm_array_of_iv_pairs));
  if (result_iv_pairs_p == NULL)
  {
    CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
     return OF_FAILURE;
  }
  nsrm_chainsetzone_ucm_getparams (nsrm_chainsetzone_config_info, &result_iv_pairs_p[uiRecCount]);
  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT("debug");
  uiRecCount++;
  *array_of_iv_pair_arr_p = result_iv_pairs_p;
  return OF_SUCCESS;
}


int32_t nsrm_chainsetzone_getnextnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
                        struct cm_array_of_iv_pairs *prev_record_key_p,  uint32_t * count_p,
                        struct cm_array_of_iv_pairs ** next_n_record_data_p)
{
  struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  int32_t return_value = OF_FAILURE;
  uint32_t uiRecCount = 0;
  int32_t uiRequestedCount = 1, uiReturnedCnt=0;
  struct nsrm_zone_direction_rule_key *nsrm_chainsetzone_key_info = NULL;
  struct nsrm_zone_rule* nsrm_chainsetzone_config_info = NULL;
  struct   nsrm_zone_direction_rule_config_info*   nsrm_srvmap_config_info = NULL;
  struct   nsrm_nschainset_object_key*     nschainset_object_key_p;

  CM_CBK_DEBUG_PRINT ("Entered");
  nschainset_object_key_p = (struct nsrm_nschainset_object_key*) 
          of_calloc(1, sizeof(struct nsrm_nschainset_object_key));
  nsrm_chainsetzone_key_info = (struct nsrm_zone_direction_rule_key*) of_calloc(1, 
                   sizeof(struct nsrm_zone_direction_rule_key));
  nsrm_srvmap_config_info = (struct nsrm_zone_direction_rule_config_info *) 
                          of_calloc(1, sizeof(struct nsrm_zone_direction_rule_config_info));
  nsrm_chainsetzone_ucm_setmandparams (keys_arr_p, nschainset_object_key_p,nsrm_chainsetzone_key_info, 
                                         nsrm_srvmap_config_info);
  if ((nsrm_chainsetzone_ucm_setmandparams (prev_record_key_p,nschainset_object_key_p, nsrm_chainsetzone_key_info, 
                                       nsrm_srvmap_config_info)) != OF_SUCCESS)
  {
     CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
     of_free(nsrm_chainsetzone_key_info->name_p);
     of_free(nsrm_chainsetzone_key_info->tenant_name_p);
     of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
     of_free(nsrm_chainsetzone_key_info);
     return OF_FAILURE;
  }

  nsrm_chainsetzone_config_info = (struct nsrm_zone_rule *)of_calloc(1,sizeof(struct  nsrm_zone_rule));

  nsrm_chainsetzone_config_info->info = (struct nsrm_zone_direction_rule_config_info *)
                          of_calloc(NSRM_MAX_ZONE_RULE_CONFIG_PARAMETERS, sizeof(struct nsrm_zone_direction_rule_config_info));
  nsrm_chainsetzone_config_info->key.name_p = (char*)of_calloc(1,NSRM_MAX_NAME_LENGTH);
  nsrm_chainsetzone_config_info->key.tenant_name_p = (char*)of_calloc(1,NSRM_MAX_NAME_LENGTH);
  nsrm_chainsetzone_config_info->key.nschainset_object_name_p = (char*)of_calloc(1,NSRM_MAX_NAME_LENGTH);

  return_value = nsrm_get_next_zone_from_nschainset_object (nschainset_object_key_p,
                        nsrm_chainsetzone_key_info, uiRequestedCount, &uiReturnedCnt, 
                        nsrm_chainsetzone_config_info);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Get next record failed for NSRM appl  Table");
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }

  result_iv_pairs_p =(struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
  if (result_iv_pairs_p == NULL)
  {
    CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }
  nsrm_chainsetzone_ucm_getparams (nsrm_chainsetzone_config_info, &result_iv_pairs_p[uiRecCount]);
  uiRecCount++;
  *next_n_record_data_p = result_iv_pairs_p;
  *count_p = uiRecCount;
  of_free(nsrm_chainsetzone_key_info->name_p);
  of_free(nsrm_chainsetzone_key_info->tenant_name_p);
  of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
  of_free(nsrm_chainsetzone_key_info);
  return OF_SUCCESS;
}

int32_t nsrm_chainsetzone_getexactrec(struct cm_array_of_iv_pairs * key_iv_pairs_p,
    struct cm_array_of_iv_pairs ** pIvPairArr)

{
  struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  struct nsrm_zone_direction_rule_key *nsrm_chainsetzone_key_info;
  uint32_t uiRecCount = 0;
  int32_t iRes = OF_FAILURE;
  struct nsrm_zone_rule *chainsetzone_object_config_info;
  struct nsrm_zone_direction_rule_config_info*   nsrm_srvmap_config_info;
  struct   nsrm_nschainset_object_key*     nschainset_object_key_p;

  CM_CBK_DEBUG_PRINT ("Entered");
  nschainset_object_key_p = (struct nsrm_nschainset_object_key*) 
          of_calloc(1, sizeof(struct nsrm_nschainset_object_key));
  nsrm_chainsetzone_key_info = (struct nsrm_zone_direction_rule_key*) 
          of_calloc(1, sizeof(struct nsrm_zone_direction_rule_key));
  nsrm_srvmap_config_info = (struct nsrm_zone_direction_rule_config_info *) 
	  of_calloc(NSRM_MAX_ZONE_RULE_CONFIG_PARAMETERS, sizeof(struct nsrm_zone_direction_rule_config_info));
  if((nsrm_chainsetzone_ucm_setmandparams(key_iv_pairs_p,nschainset_object_key_p, nsrm_chainsetzone_key_info, 
                   nsrm_srvmap_config_info)) !=OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }
 
 
  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT("debug");

   chainsetzone_object_config_info = (struct nsrm_zone_rule *)of_calloc(1,sizeof(struct nsrm_zone_rule));
    chainsetzone_object_config_info->key.name_p = (char *) of_calloc(1,NSRM_MAX_NAME_LENGTH);
    chainsetzone_object_config_info->key.tenant_name_p = (char *) of_calloc(1,NSRM_MAX_NAME_LENGTH);
    chainsetzone_object_config_info->key.nschainset_object_name_p = (char *) of_calloc(1,NSRM_MAX_NAME_LENGTH);
    chainsetzone_object_config_info->info = (struct nsrm_zone_direction_rule_config_info*) of_calloc(NSRM_MAX_ZONE_RULE_CONFIG_PARAMETERS,sizeof(struct nsrm_nschain_selection_rule_config_info));
  

  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT("debug");
  iRes = nsrm_get_exact_zone_from_nschainset_object(nschainset_object_key_p,
                       nsrm_chainsetzone_key_info, chainsetzone_object_config_info);
  CM_CBK_DEBUG_PRINT("debug");
  CM_CBK_DEBUG_PRINT("debug");
  if (iRes != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Error: Appliacne doesn't exists with name %s",nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }

  CM_CBK_DEBUG_PRINT ("Exact matching record found");
  result_iv_pairs_p =
    (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));

  if (result_iv_pairs_p == NULL)
  {
    CM_CBK_DEBUG_PRINT ("Failed to allocate memory for result_iv_pairs_p");
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
    return OF_FAILURE;
  }
  nsrm_chainsetzone_ucm_getparams(chainsetzone_object_config_info, &result_iv_pairs_p[uiRecCount]);
  *pIvPairArr = result_iv_pairs_p;
                                                                                       
    of_free(nsrm_chainsetzone_key_info->name_p);
    of_free(nsrm_chainsetzone_key_info->tenant_name_p);
    of_free(nsrm_chainsetzone_key_info->nschainset_object_name_p);
    of_free(nsrm_chainsetzone_key_info);
  return OF_SUCCESS;
}

int32_t nsrm_chainsetzone_ucm_getparams (struct nsrm_zone_rule *appl_chainsetzone_info,
                                   struct cm_array_of_iv_pairs * result_iv_pairs_p)
{
   uint32_t uindex_i = 0;
   char buf[128] = "";
   CM_CBK_DEBUG_PRINT ("Entered");
#define CM_APPLIANCE_CHILD_COUNT 3

   result_iv_pairs_p->iv_pairs =(struct cm_iv_pair *) of_calloc (CM_APPLIANCE_CHILD_COUNT, sizeof (struct cm_iv_pair));
   if (result_iv_pairs_p->iv_pairs == NULL)
   {
     CM_CBK_DEBUG_PRINT("Memory allocation failed for result_iv_pairs_p->iv_pairs");
     return OF_FAILURE;
   }

   FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i], CM_DM_CHAINSETZONERULE_ZONE_NAME_ID,
		   CM_DATA_TYPE_STRING, appl_chainsetzone_info->key.name_p);
   CM_CBK_DEBUG_PRINT("name : %s",result_iv_pairs_p->iv_pairs[uindex_i].value_p);
   uindex_i++;
   
   of_memset(buf, 0, sizeof(buf));
   of_itoa (appl_chainsetzone_info->info[0].value.zone_direction_e, buf);
   FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i],CM_DM_CHAINSETZONERULE_ZONE_DIRECTION_ID,
                    CM_DATA_TYPE_STRING,buf);
   uindex_i++;

   result_iv_pairs_p->count_ui = uindex_i;
   CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);

   return OF_SUCCESS;

}

int32_t nsrm_chainsetzone_ucm_setmandparams (struct cm_array_of_iv_pairs *pMandParams,
                                  struct nsrm_nschainset_object_key* nschainset_object_key_p,
                                  struct nsrm_zone_direction_rule_key* appl_chainsetzone_info,
                                  struct nsrm_zone_direction_rule_config_info* nsrm_chainsetzone_config_info)
{
  uint32_t uiMandParamCnt;

  CM_CBK_DEBUG_PRINT ("Entered");

  for (uiMandParamCnt = 0; uiMandParamCnt < pMandParams->count_ui;uiMandParamCnt++)
  {
    switch (pMandParams->iv_pairs[uiMandParamCnt].id_ui)
    {
      case CM_DM_CHAINSETZONERULE_ZONE_NAME_ID:
         appl_chainsetzone_info->name_p = (char *) of_calloc(1, NSRM_MAX_NAME_LENGTH);
          CM_CBK_DEBUG_PRINT ("name_p:%s",(char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
          of_strncpy (appl_chainsetzone_info->name_p,(char *) pMandParams->iv_pairs[uiMandParamCnt].value_p,
                 pMandParams->iv_pairs[uiMandParamCnt].value_length);
      break;

     // case CM_DM_CHAINSELRULE_SELTENANT_ID:
        case CM_DM_CHAINSET_TENANT_ID:
          appl_chainsetzone_info->tenant_name_p = (char *) of_calloc(1, NSRM_MAX_NAME_LENGTH);
          nschainset_object_key_p->tenant_name_p = (char *) of_calloc(1, NSRM_MAX_NAME_LENGTH);
          of_strncpy (appl_chainsetzone_info->tenant_name_p,(char *) pMandParams->iv_pairs[uiMandParamCnt].value_p,pMandParams->iv_pairs[uiMandParamCnt].value_length);
          CM_CBK_DEBUG_PRINT ("tenant_name_p:%s",(char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
          of_strncpy (nschainset_object_key_p->tenant_name_p,(char *) pMandParams->iv_pairs[uiMandParamCnt].value_p,pMandParams->iv_pairs[uiMandParamCnt].value_length);
      break;

      //case CM_DM_CHAINSELRULE_CHAINSET_NAME_ID:
      case CM_DM_CHAINSET_NAME_ID:
          appl_chainsetzone_info->nschainset_object_name_p = (char *) of_calloc(1, NSRM_MAX_NAME_LENGTH);
          nschainset_object_key_p->name_p = (char *) of_calloc(1, NSRM_MAX_NAME_LENGTH);
          CM_CBK_DEBUG_PRINT ("nschainset_object_name_p:%s",(char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
          of_strncpy (appl_chainsetzone_info->nschainset_object_name_p,
                 (char *) pMandParams->iv_pairs[uiMandParamCnt].value_p,
                 pMandParams->iv_pairs[uiMandParamCnt].value_length);
          
          of_strncpy (nschainset_object_key_p->name_p,
                 (char *) pMandParams->iv_pairs[uiMandParamCnt].value_p,
                 pMandParams->iv_pairs[uiMandParamCnt].value_length);

      break;

      case CM_DM_CHAINSETZONERULE_ZONE_DIRECTION_ID:
         nsrm_chainsetzone_config_info[0].value.zone_direction_e = of_atoi(pMandParams->iv_pairs[uiMandParamCnt].value_p);
      break;

     }
  }
  return OF_SUCCESS;
}

int32_t nsrm_chainsetzone_ucm_setoptparams (struct cm_array_of_iv_pairs *pOptParams,
                                  struct nsrm_zone_direction_rule_key* appl_chainsetzone_info,
                                  struct nsrm_zone_direction_rule_config_info* nsrm_chainsetzone_config_info)
{
  
  uint32_t uiOptParamCnt;
  char     *src_mac , *dst_mac;

  CM_CBK_DEBUG_PRINT ("Entered");
  for (uiOptParamCnt = 0; uiOptParamCnt < pOptParams->count_ui; uiOptParamCnt++)
  {
    switch (pOptParams->iv_pairs[uiOptParamCnt].id_ui)
    {

    }
  }
     CM_CBK_PRINT_IVPAIR_ARRAY (pOptParams);
     return OF_SUCCESS;
}

int32_t nsrm_chainsetzone_verifycfg (struct cm_array_of_iv_pairs *key_iv_pairs_p,
                        uint32_t command_id, struct cm_app_result ** result_p)
{
  struct cm_app_result *of_chainsetzone_result = NULL;
  int32_t return_value = OF_FAILURE;

  return OF_SUCCESS;
  CM_CBK_DEBUG_PRINT ("Entered");
  return_value = nsrm_chainsetzone_ucm_validatemandparams(key_iv_pairs_p, &of_chainsetzone_result);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Validate Mandatory Parameters Failed");
    return OF_FAILURE;
  }

  *result_p = of_chainsetzone_result;
  return OF_SUCCESS;

}

int32_t nsrm_chainsetzone_ucm_validatemandparams(struct cm_array_of_iv_pairs *mand_iv_pairs_p,
                                                 struct cm_app_result **  presult_p)
{
  uint32_t count;
  struct cm_app_result *of_chainsetzone_result = NULL;


  CM_CBK_DEBUG_PRINT ("Entered");
  for (count = 0; count < mand_iv_pairs_p->count_ui;count++)
  {
    switch (mand_iv_pairs_p->iv_pairs[count].id_ui)
    {
      case CM_DM_CHAINSETZONERULE_ZONE_NAME_ID:
        if (mand_iv_pairs_p->iv_pairs[count].value_p == NULL)
        {
          CM_CBK_DEBUG_PRINT ("Appl category name is NULL");
          fill_app_result_struct (&of_chainsetzone_result, NULL, CM_GLU_ZONE_RULE_NAME_NULL);
          *presult_p = of_chainsetzone_result;
          return OF_FAILURE;
        }
        break;

       case CM_DM_CHAINSET_TENANT_ID: 
       if (mand_iv_pairs_p->iv_pairs[count].value_p == NULL)
        {
          CM_CBK_DEBUG_PRINT ("Appl category name is NULL");
          fill_app_result_struct (&of_chainsetzone_result, NULL, CM_GLU_TENANT_ID_INVALID);
          *presult_p = of_chainsetzone_result;
          return OF_FAILURE;
        }
        break;


     }
   }
  CM_CBK_PRINT_IVPAIR_ARRAY (mand_iv_pairs_p);
  return OF_SUCCESS;
}
