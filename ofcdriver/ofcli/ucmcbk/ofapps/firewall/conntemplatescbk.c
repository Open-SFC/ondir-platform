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
 * File name: fw4_conntemplates.c
 * Author: Varun  <b36461@freescale.com> 
 * Date:   10/13/2014
 * Description: 
 * 
 */
#ifdef CNTRL_IPV4_FIREWALL_SUPPORT
#ifdef CNTRL_FW4_CONNECTION_TEMPLATES_SUPPORT

#include "cbcmninc.h"
#include "cmgutil.h"
#include "fwnat_incld.h"
#include "fw4_conntemplates_api.h"

/******************************************************************/
int32_t of_fw4_conntemplates_appl_ucmcbk_init();

int32_t of_fw4_conntemplates_addrec(void * config_trans_p,
                        struct cm_array_of_iv_pairs * pMandParams,
                        struct cm_array_of_iv_pairs * pOptParams,
                        struct cm_app_result ** result_p);

int32_t of_fw4_conntemplates_delrec(void * config_transaction_p,
                          struct cm_array_of_iv_pairs * keys_arr_p,
                          struct cm_app_result ** result_p);

int32_t of_fw4_conntemplates_getfirstrec(
                       struct cm_array_of_iv_pairs * keys_arr_p,
                       uint32_t * count_p,
                       struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p);

int32_t of_fw4_conntemplates_getnextrec(
                       struct cm_array_of_iv_pairs * keys_arr_p,
                       struct cm_array_of_iv_pairs * prev_record_key_p,
                       uint32_t * count_p,
                       struct cm_array_of_iv_pairs ** next_n_record_data_p);

int32_t of_fw4_conntemplates_getexactrec(
                       struct cm_array_of_iv_pairs * keys_arr_p,
                       struct cm_array_of_iv_pairs ** pIvPairArr);

int32_t of_fw4_conntemplates_verifycfg(
                       struct cm_array_of_iv_pairs * keys_arr_p,
                       uint32_t command_id, 
                       struct cm_app_result ** result_p);

/************************ *********************************************/
int32_t of_fw4_conntemplates_validatemandparams (
                               struct cm_array_of_iv_pairs *pMandParams,
                               struct cm_app_result **presult_p);

int32_t of_fw4_conntemplates_validateoptparams (
                        struct cm_array_of_iv_pairs *pOptParams,
                        struct cm_app_result ** presult_p);

int32_t of_fw4_conntemplates_setmandparams (
                        struct cm_array_of_iv_pairs *pMandParams,
                        struct fw4_conn_templates_config *config_info_p,
                        struct cm_app_result ** presult_p);

int32_t of_fw4_conn_add_recemplates_setoptparams (
                        struct cm_array_of_iv_pairs *pOptParams,
                        struct fw4_conn_templates_config *config_info_p,
                        struct cm_app_result ** presult_p);

int32_t of_fw4_conntemplates_getparams (
                        struct fw4_conn_templates_config *config_info_p,
                        struct cm_array_of_iv_pairs * result_iv_pairs_p);


struct cm_dm_app_callbacks of_fw4_conntemplates_callbacks =
{
   NULL,
   of_fw4_conntemplates_addrec,
   NULL,
   of_fw4_conntemplates_delrec,
   NULL,
   of_fw4_conntemplates_getfirstrec,
   of_fw4_conntemplates_getnextrec,
   of_fw4_conntemplates_getexactrec,
   of_fw4_conntemplates_verifycfg,
   NULL
}; 

int32_t of_fw4_conntemplates_appl_ucmcbk_init()
{
  if(cm_register_app_callbacks(
                     CM_ON_DIRECTOR_NAMESPACE_APPS_CONNTEMPLATES_APPL_ID, 
                     &of_fw4_conntemplates_callbacks)!=OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("fw4 connection templates cli cbks init failed");
    return OF_FAILURE; 
  }
  return OF_SUCCESS;
}

int32_t of_fw4_conntemplates_setmandparams (
                        struct cm_array_of_iv_pairs      *pMandParams,
                        struct fw4_conn_templates_config *config_info_p,
                        struct cm_app_result ** presult_p)
{
  uint32_t uiMandParamCnt;
  char ns_name[30];
  int32_t status;

  CM_CBK_DEBUG_PRINT ("Entered");
  for (uiMandParamCnt = 0;
       uiMandParamCnt < pMandParams->count_ui; uiMandParamCnt++)
  {
    switch (pMandParams->iv_pairs[uiMandParamCnt].id_ui)
    {
      case CM_DM_CONNTEMPLATES_DESTPORT_ID:
        config_info_p->dst_port = of_atoi((char *) \
        pMandParams->iv_pairs[uiMandParamCnt].value_p);
        CM_CBK_DEBUG_PRINT ("dest_port:%d",config_info_p->dst_port);
        break;


      case CM_DM_NAMESPACE_NAME_ID:
	{
	  strcpy(ns_name,(char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
	  status = of_nem_get_nsid_from_namespace(ns_name, 
                                    &(config_info_p->ns_id));
	  if(status == OF_SUCCESS)
	  {
	    CM_CBK_DEBUG_PRINT("Conversion from nsid to string success");	   
            CM_CBK_DEBUG_PRINT ("ns_name:%s,ns_id:%d",
                               ns_name,config_info_p->ns_id);
   	  }
	}
	break;

      case CM_DM_CONNTEMPLATES_PROTOCOL_ID:

        if(!strcasecmp((char *) pMandParams->iv_pairs[uiMandParamCnt].value_p,
                 "tcp"))
        {
          config_info_p->protocol = CM_IPPROTO_TCP;
        }
        else if(!strcasecmp((char *) pMandParams->iv_pairs[uiMandParamCnt].value_p,
                 "udp"))
        {
         config_info_p->protocol = CM_IPPROTO_UDP; 
        }
        else
        {
          CM_CBK_DEBUG_PRINT("Invalid protocol");
          return OF_FAILURE;
        }
        break;

      default:
        break;
    }
  }
  return OF_SUCCESS;
}

int32_t of_fw4_conntemplates_setoptparams (
                        struct cm_array_of_iv_pairs      *pOptParams,
                        struct fw4_conn_templates_config *config_info_p,
                        struct cm_app_result ** presult_p)
{
  uint32_t uiOptParamCnt;

  CM_CBK_DEBUG_PRINT ("Entered");

  for (uiOptParamCnt = 0;
       uiOptParamCnt < pOptParams->count_ui; uiOptParamCnt++)
  {
    switch (pOptParams->iv_pairs[uiOptParamCnt].id_ui)
    {

      case CM_DM_CONNTEMPLATES_SOURCEIP_ID:
        CM_CBK_DEBUG_PRINT ("src_ip:%s",\
        (char *)pOptParams->iv_pairs[uiOptParamCnt].value_p);

        if(cm_val_and_conv_aton(
               (char *)pOptParams->iv_pairs[uiOptParamCnt].value_p,\
               &(config_info_p->src_ip))!=OF_SUCCESS)
        {
          return OF_FAILURE;
        }
        break;

      case CM_DM_CONNTEMPLATES_DESTIP_ID:
        CM_CBK_DEBUG_PRINT ("dest_ip:%s",
        (char *)pOptParams->iv_pairs[uiOptParamCnt].value_p);

        if(cm_val_and_conv_aton(
                 (char *)pOptParams->iv_pairs[uiOptParamCnt].value_p,\
                             &(config_info_p->dst_ip))!=OF_SUCCESS)
        {
          return OF_FAILURE;
        }
        break;

      case CM_DM_CONNTEMPLATES_SOURCEPORT_ID:
        CM_CBK_DEBUG_PRINT ("src_port:%s",
        (char *)pOptParams->iv_pairs[uiOptParamCnt].value_p);

        config_info_p->src_port = of_atoi((char *) \
        pOptParams->iv_pairs[uiOptParamCnt].value_p);
        break;

      default:
        break;
    }
  }
  return OF_SUCCESS;
}

int32_t of_fw4_conntemplates_verifycfg (
                        struct cm_array_of_iv_pairs *key_iv_pairs_p,
                        uint32_t command_id, 
                        struct cm_app_result ** result_p)
{
  struct cm_app_result *result = NULL;
  int32_t return_value = OF_FAILURE;

  CM_CBK_DEBUG_PRINT ("Entered");

  return_value = of_fw4_conntemplates_validatemandparams(key_iv_pairs_p, 
                                                      &result);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Validate Mandatory Parameters Failed");
    return OF_FAILURE;
  }
  *result_p = result;
  return OF_SUCCESS;
}

int32_t of_fw4_conntemplates_validatemandparams(
       struct cm_array_of_iv_pairs *mand_iv_pairs_p,
       struct cm_app_result **  presult_p)
{
  uint32_t count;
  struct cm_app_result *result = NULL;


  CM_CBK_DEBUG_PRINT ("Entered");
  for (count = 0; count < mand_iv_pairs_p->count_ui;
      count++)
  {
    switch (mand_iv_pairs_p->iv_pairs[count].id_ui)
    {
      case CM_DM_CONNTEMPLATES_PROTOCOL_ID:
        if (mand_iv_pairs_p->iv_pairs[count].value_p == NULL)
        {
          CM_CBK_DEBUG_PRINT ("protocol is NULL");
          fill_app_result_struct (&result, NULL, CM_GLU_CONNTEMPLATE_PROTO_NULL);
          *presult_p = result;
          return OF_FAILURE;
        }
        break;

      case CM_DM_CONNTEMPLATES_DESTPORT_ID:
        if (mand_iv_pairs_p->iv_pairs[count].value_p == NULL)
        {
          CM_CBK_DEBUG_PRINT ("destination port  is NULL");
          fill_app_result_struct (&result, NULL, CM_GLU_CONNTEMPLATE_DSTPORT_NULL);
          *presult_p = result;
          return OF_FAILURE;
        }
        break;
    }
  }
  CM_CBK_PRINT_IVPAIR_ARRAY (mand_iv_pairs_p);
  return OF_SUCCESS;
}



/******************************************************************************
******************************************************************************/
int32_t of_fw4_conntemplates_addrec (void * config_trans_p,
                        struct cm_array_of_iv_pairs * pMandParams,
                        struct cm_array_of_iv_pairs * pOptParams,
                        struct cm_app_result ** result_p)
{
  struct cm_app_result *result = NULL;
  int32_t               return_value = OF_FAILURE;
  struct fw4_conn_templates_config config_info={};
  uint64_t                    conn_template_handle;
  
  CM_CBK_DEBUG_PRINT ("Entered");
  
  if((of_fw4_conntemplates_setmandparams(
                               pMandParams, 
                               &config_info, 
                               &result)) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT("Set Mandatory Parameters Failed");
    fill_app_result_struct(&result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
    *result_p = result;
    return OF_FAILURE;
  }

  if((of_fw4_conntemplates_setoptparams(
                               pOptParams, 
                               &config_info, 
                               &result)) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT("Set Optional Parameters Failed");
    fill_app_result_struct(&result, NULL, CM_GLU_SET_OPT_PARAM_FAILED);
    *result_p = result;
    return OF_FAILURE;
  }

  return_value = fw4_add_conn_templates(
                                  &config_info,  
                                  &conn_template_handle);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("connection template  add record Failed");
    fill_app_result_struct (&result, NULL, CM_GLU_CONN_TEMPLATE_ADD_FAILED);
    *result_p = result;
    return OF_FAILURE;
  }
  CM_CBK_DEBUG_PRINT ("connection template record added successfully");
  return OF_SUCCESS;
}

/******************************************************************************
******************************************************************************/
int32_t of_fw4_conntemplates_delrec(void * config_transaction_p,
                        struct cm_array_of_iv_pairs * keys_arr_p,
                        struct cm_app_result ** result_p)
{
  struct cm_app_result *result = NULL;
  int32_t return_value = OF_FAILURE;

  struct fw4_conn_templates_config  config_info={};

  CM_CBK_DEBUG_PRINT ("Entered");
  if ((of_fw4_conntemplates_setmandparams(
                                keys_arr_p, 
                                &config_info, 
                                &result)) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    fill_app_result_struct (&result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
    *result_p = result;
    return OF_FAILURE;
  }

  if((of_fw4_conntemplates_setoptparams(
                               keys_arr_p, 
                               &config_info, 
                               &result)) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT("Set Optional Parameters Failed");
    fill_app_result_struct(&result, NULL, CM_GLU_SET_OPT_PARAM_FAILED);
    *result_p = result;
    return OF_FAILURE;
  }


  return_value = fw4_del_conn_templates(&config_info);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("crm tenant add  Failed");
    fill_app_result_struct (&result, NULL, CM_GLU_CONN_TEMPLATE_DEL_FAILED);
    *result_p = result;
    return OF_FAILURE;
  }
  CM_CBK_DEBUG_PRINT ("connection template record delete successfully");
  return OF_SUCCESS;
}

/******************************************************************************
******************************************************************************/
int32_t of_fw4_conntemplates_getfirstrec (
                              struct cm_array_of_iv_pairs * keys_arr_p,
                              uint32_t * count_p,
                 struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p)
{
  struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  struct cm_app_result        *result = NULL;
  int32_t return_value = OF_FAILURE;
  uint32_t uiRecCount = 0;

  struct  fw4_conn_templates_config config_info={};
  uint64_t conn_template_handle;

  CM_CBK_DEBUG_PRINT ("Entered");

  if ((of_fw4_conntemplates_setmandparams(
                                keys_arr_p, 
                                &config_info, 
                                &result)) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    fill_app_result_struct (&result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
    return OF_FAILURE;
  }

  return_value = fw4_get_first_conn_template(&config_info,  
                             &conn_template_handle);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Get first record failed for connection template.");
    return OF_FAILURE;
  }

  result_iv_pairs_p = (struct cm_array_of_iv_pairs *) of_calloc (1, \
                             sizeof (struct cm_array_of_iv_pairs));
  if (result_iv_pairs_p == NULL)
  {
    CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
    return OF_FAILURE;
  }
  of_fw4_conntemplates_getparams (&config_info, 
                                  &result_iv_pairs_p[uiRecCount]);
  uiRecCount++;
  *array_of_iv_pair_arr_p = result_iv_pairs_p;
  *count_p = uiRecCount;
  return OF_SUCCESS;
}

int32_t of_fw4_conntemplates_getnextrec(
                        struct cm_array_of_iv_pairs * keys_arr_p,
                        struct cm_array_of_iv_pairs *prev_record_key_p, 
                        uint32_t * count_p,
                        struct cm_array_of_iv_pairs ** next_n_record_data_p)
{
  struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  struct cm_app_result        *result = NULL;
  int32_t return_value = OF_FAILURE;
  uint32_t uiRecCount = 0;

  struct fw4_conn_templates_config config_info={};
  uint64_t conn_template_handle;

  CM_CBK_DEBUG_PRINT ("Entered");


  if ((of_fw4_conntemplates_setmandparams(keys_arr_p, 
                                         &config_info, 
                                         &result)) != OF_SUCCESS)
  {
     CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
      return OF_FAILURE;
  }



  if ((of_fw4_conntemplates_setmandparams(prev_record_key_p, 
                                         &config_info, 
                                         &result)) != OF_SUCCESS)
  {
     CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
      return OF_FAILURE;
  }

  if((of_fw4_conntemplates_setoptparams(prev_record_key_p, 
                               &config_info, 
                               &result)) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT("Set Optional Parameters Failed");
    return OF_FAILURE;
  }



  return_value = fw4_get_exact_conn_template(&config_info, 
                                   &conn_template_handle);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Error: template doesn't exists");
    return OF_FAILURE;
  }

  return_value = fw4_get_next_conn_template(&config_info,  
                                  &conn_template_handle);

  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("failed");
    *count_p = 0;
    return OF_FAILURE;
  }

  result_iv_pairs_p = (struct cm_array_of_iv_pairs *) 
   of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
  if (result_iv_pairs_p == NULL)
  {
    CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
     return OF_FAILURE;
  }
  of_fw4_conntemplates_getparams(&config_info, &result_iv_pairs_p[uiRecCount]);
  uiRecCount++;
  *next_n_record_data_p = result_iv_pairs_p;
  *count_p = uiRecCount;
  return OF_SUCCESS;
}



int32_t of_fw4_conntemplates_getparams(
  struct fw4_conn_templates_config *config_info_p,
  struct cm_array_of_iv_pairs      *result_iv_pairs_p)
{
  int32_t          iCount = 0;
  char string[200]="";

#define CM_FW4_CONN_TEMPLATES_CHILD_COUNT  6
  CM_CBK_DEBUG_PRINT("Entered \n\r");

  result_iv_pairs_p->iv_pairs =
    (struct cm_iv_pair *) of_calloc (CM_FW4_CONN_TEMPLATES_CHILD_COUNT,
        sizeof (struct cm_iv_pair));

  if (result_iv_pairs_p->iv_pairs == NULL)
  {
    CM_CBK_DEBUG_PRINT ("Memory allocation failed for "
        "result_iv_pairs_p->iv_pairs");
    return OF_FAILURE;
  }

  /* 1. DP Id */
  memset(string, 0, sizeof(string));
  sprintf (string,"%d", config_info_p->ns_id);
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
                   CM_DM_NAMESPACE_NAME_ID,
                   CM_DATA_TYPE_STRING, 
                   string);
  iCount++;

  /* 2. source ip */
  memset(string, 0, sizeof(string));
  if(config_info_p->src_ip)
  {
    cm_inet_ntoal(config_info_p->src_ip, string);
    FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
                     CM_DM_CONNTEMPLATES_SOURCEIP_ID,
                     CM_DATA_TYPE_IPADDR,
                     string);
  }
  else
  {
    FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
                     CM_DM_CONNTEMPLATES_SOURCEIP_ID,
                     CM_DATA_TYPE_STRING,
                     "ANY");
  }
  iCount++;

  /* 3. Dest ip */
  memset(string, 0, sizeof(string));
  if(config_info_p->dst_ip)
  {
    cm_inet_ntoal(config_info_p->dst_ip, string);
    FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
                     CM_DM_CONNTEMPLATES_DESTIP_ID,
                     CM_DATA_TYPE_IPADDR,
                     string);
  }
  else
  {  
     FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
                     CM_DM_CONNTEMPLATES_DESTIP_ID,
                     CM_DATA_TYPE_STRING,
                     "ANY");

  }
  iCount++;

  /* 4. source Port */
  memset(string, 0, sizeof(string));
  if(config_info_p->src_port)
  {
    of_itoa(config_info_p->dst_port, string);
    FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
                     CM_DM_CONNTEMPLATES_SOURCEPORT_ID,
                     CM_DATA_TYPE_STRING,
                     string);
  }
  else
  {
    FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
                     CM_DM_CONNTEMPLATES_SOURCEPORT_ID,
                     CM_DATA_TYPE_STRING,
                     "ANY");
  }
  iCount++;



  /* 5 Dest Port */
  memset(string, 0, sizeof(string));
  of_itoa(config_info_p->dst_port, string);
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_CONNTEMPLATES_DESTPORT_ID,
      CM_DATA_TYPE_STRING,
      string);
  iCount++;


 
  /** 6 Protocol **/
  memset(string, 0, sizeof(string));
  if(config_info_p->protocol == CM_IPPROTO_TCP)
  {
    FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
                     CM_DM_CONNTEMPLATES_PROTOCOL_ID,
                     CM_DATA_TYPE_STRING,
                     "tcp");
  }
  else if(config_info_p->protocol == CM_IPPROTO_UDP)
  {
    FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
                     CM_DM_CONNTEMPLATES_PROTOCOL_ID,
                     CM_DATA_TYPE_STRING,
                     "udp"); 
  }  
  iCount++;

 // CM_CBK_DEBUG_PRINT("Filled number of params = %d \n\r",
   //   iCount);
  result_iv_pairs_p->count_ui = iCount;
  return OF_SUCCESS;
}

int32_t of_fw4_conntemplates_getexactrec(
     struct cm_array_of_iv_pairs * key_iv_pairs_p,
     struct cm_array_of_iv_pairs ** pIvPairArr)
{

  struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  struct cm_app_result *result = NULL;
  struct fw4_conn_templates_config  config_info={};
  int32_t return_value = OF_FAILURE;
  uint64_t conn_template_handle;

  CM_CBK_DEBUG_PRINT ("Entered");
  of_memset (&config_info, 0, sizeof(config_info));

  if ((of_fw4_conntemplates_setmandparams(key_iv_pairs_p, 
                                    &config_info, 
                                    &result)) != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT("Set Mandatory Parameters Failed");
    return OF_FAILURE;
  }

  return_value = fw4_get_exact_conn_template(&config_info, 
                                   &conn_template_handle);
  if (return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Error: template doesn't exists");
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

  of_fw4_conntemplates_getparams(&config_info, 
                                 result_iv_pairs_p);
  *pIvPairArr = result_iv_pairs_p;
  return OF_SUCCESS;
}
#endif
#endif 
