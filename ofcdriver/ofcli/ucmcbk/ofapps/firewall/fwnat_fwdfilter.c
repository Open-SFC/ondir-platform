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
 * File name: fwnat_fwdfilter.c
 * Author: Jothirlatha J <b37228@freescale.com> 
 * Date:   10/13/2014
 * Description: 
 * 
 */
/* File  :      fwnat_fwdfilter.c
 * Author:      Jothirlatha J <jothi.j@freescale.com> 
 * Date:   10/13/2014
 */
#ifdef OF_CONTROLLER_IPT_ACL_SUPPORT
#include "cbcmninc.h"
#include "cmgutil.h"
#include "fwnat_incld.h"
#include "fwnat_ldef.h"
#include "fwnat_listener.h"
#include "fwnat_acl.h"

#define FWNAT_CM_DEBUG(a,b,c...)   

#if 1 
// TODO: to be removed. Move to approriate header file
//

#define MAX_IPT_CHAIN_NAME  1024
#define MAX_IPT_PORT_NAME   255
#define MAX_IPT_NS_NAME_LEN   255

#endif 

int32_t of_fwnat_fwdfilter_ucm_setmandparams (
      struct of_fwnat_cm_filter_inargs * fwnat_inargs,
      struct cm_array_of_iv_pairs *
      mand_iv_pairs_p,
      char   *chainname,
      struct cm_app_result ** presult_p); 
int32_t fwnat_fwdfilter_ucm_getparams ( 
  struct of_fwnat_cm_filter_inargs  *fwnat_inargs, 
  struct cm_array_of_iv_pairs       *result_iv_pairs_p); 
int32_t of_fwnat_fwdfilter_getfirstrec 
     (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      uint32_t * count_p,
      struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p); 
int32_t of_fwnat_fwdfilter_getnextrec (
      struct cm_array_of_iv_pairs * key_iv_pairs_p,
      struct cm_array_of_iv_pairs *prev_record_key_p, 
      uint32_t * count_p,
      struct cm_array_of_iv_pairs ** next_n_record_data_p); 
int32_t of_fwnat_fwdfilter_getexactrec 
        (struct cm_array_of_iv_pairs * key_iv_pairs_p,
         struct cm_array_of_iv_pairs ** pIvPairArr); 

struct cm_dm_app_callbacks of_fwnat_fwdfilter_callbacks =
{
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   of_fwnat_fwdfilter_getfirstrec,
   of_fwnat_fwdfilter_getnextrec,
   of_fwnat_fwdfilter_getexactrec,
   NULL,
   NULL
}; 

int32_t of_fwnat_fwdfilter_appl_ucmcbk_init()
{
   CM_CBK_DEBUG_PRINT ("FWNAT - forward filter Entry");
   cm_register_app_callbacks (CM_ON_DIRECTOR_DATAPATH_FIREWALL_FILTERFORWARD_APPL_ID,
                  &of_fwnat_fwdfilter_callbacks);
   return OF_SUCCESS;
}
int32_t of_fwnat_fwdfilter_getfirstrec 
     (struct cm_array_of_iv_pairs * key_iv_pairs_p,
      uint32_t * count_p,
      struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p)
{
  struct of_fwnat_cm_filter_inargs  fwnat_inargs; 
	struct cm_array_of_iv_pairs      *result_iv_pairs_p = NULL;
  int32_t                           status = OF_FAILURE; 
  uint32_t                          uiRecCount = 0;
  struct cm_app_result              *app_result; 

  FWNAT_CM_DEBUG ("%s:%s:%d Entered. \n\r", __FILE__, __FUNCTION__, 
      __LINE__); 

  *count_p = 0;

  CM_CBK_DEBUG_PRINT ("Entered");
  of_memset (&fwnat_inargs, 0, sizeof(fwnat_inargs)); 

  status = of_fwnat_fwdfilter_ucm_setmandparams (&fwnat_inargs, 
                               key_iv_pairs_p, 
                               fwnat_inargs.chainname, 
                               &app_result); 

  if (status != OF_SUCCESS)
  {
    *array_of_iv_pair_arr_p = result_iv_pairs_p; 
    *count_p                = uiRecCount; 
    return OF_FAILURE; 
  }

  /* This is the funtion call for forward filter chain. 
   * Set the chain name. 
   */ 
  strcpy (fwnat_inargs.tablename,"FILTER"); 
  strcpy (fwnat_inargs.chainname,"FORWARD"); 

  FWNAT_CM_DEBUG ("%s:%s:%d chainname = %s DPID = %llx \n\r", 
        __FILE__, __FUNCTION__, __LINE__, 
        fwnat_inargs.chainname, fwnat_inargs.idpID); 

  status = ip4_fwnat_get_first_record(&fwnat_inargs);  

  if (status != OF_SUCCESS)
  {
    *array_of_iv_pair_arr_p = result_iv_pairs_p; 
    *count_p                = uiRecCount; 
		CM_CBK_DEBUG_PRINT ("Get First FWNAT Failed");
    return OF_FAILURE; 
  }
  
	result_iv_pairs_p =
		(struct cm_array_of_iv_pairs *) of_calloc (1, 
                   sizeof (struct cm_array_of_iv_pairs));
	if (result_iv_pairs_p == NULL)
	{
		CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
		return OF_FAILURE;
	}

  status = fwnat_fwdfilter_ucm_getparams (&fwnat_inargs, 
                                 result_iv_pairs_p); 
  if (status != OF_SUCCESS)
  {
		CM_CBK_DEBUG_PRINT ("fwnat_fwdfilter_ucm_getparams failed");
    return OF_FAILURE; 
  }

  uiRecCount++; 
  *array_of_iv_pair_arr_p = result_iv_pairs_p; 
  *count_p                = uiRecCount; 

  return OF_SUCCESS; 
}

int32_t fwnat_fwdfilter_ucm_getparams ( 
  struct of_fwnat_cm_filter_inargs *fwnat_inargs, 
  struct cm_array_of_iv_pairs      *result_iv_pairs_p)
{

  char             string[MAX_IPT_CHAIN_NAME]; 
  int32_t          iCount = 0; 

#define CM_FILTERFORWARD_CHILD_COUNT  13
  FWNAT_CM_DEBUG ("%s:%s:%d Entered. \n\r", __FILE__, __FUNCTION__, 
      __LINE__); 

  result_iv_pairs_p->iv_pairs =
    (struct cm_iv_pair *) of_calloc (CM_FILTERFORWARD_CHILD_COUNT, 
        sizeof (struct cm_iv_pair));

  if (result_iv_pairs_p->iv_pairs == NULL)
  {
    CM_CBK_DEBUG_PRINT ("Memory allocation failed for "
        "result_iv_pairs_p->iv_pairs");
    return OF_FAILURE;
  }

  /* 1. DP Id */ 
  memset(string, 0, sizeof(string)); 
  sprintf (string,"%llx", fwnat_inargs->idpID);
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_DATAPATH_DATAPATHID_ID,
      CM_DATA_TYPE_STRING, string);
  iCount++;

#if 0
  /* ns name */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_TABLENAME_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->nsname);
  iCount++; 
#endif 
  /* 2. Table name */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_TABLENAME_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->tablename);
  iCount++; 
  /* 3. Chain name */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_CHAINNAME_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->chainname);
  iCount++; 
  /* 4. source ip */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_SOURCEIP_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->sourceip);
  iCount++; 
  /* 5 Dest ip */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_DESTIP_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->destip);
  iCount++; 
  /* 6 source Port */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_SOURCEPORT_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->sourceport);
  iCount++; 
  /* 7 Dest Port */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_DESTPORT_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->destport);
  iCount++; 
  /* 8 Protocol */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_PROTOCOL_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->protocol);
  iCount++; 
  /*  9 Input interface Name  */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_INIFACENAME_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->inifacename);
  iCount++; 
  /* 10 Output interface Name  */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_OUTIFACENAME_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->outifacename);
  iCount++; 
  /* 12 rule Position */ 
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_RULEPOSITION_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->position);
  iCount++; 

  /* Flow Priority */ 
  memset(string, 0, sizeof(string)); 
  sprintf(string,"%llu",fwnat_inargs->flow_priority);
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_FLOW_PRIORITY_ID,
      CM_DATA_TYPE_STRING, string);
  iCount++; 

  /* 11 Action */
  FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[iCount],
      CM_DM_FILTERFORWARD_ACTION_ID,
      CM_DATA_TYPE_STRING, 
      fwnat_inargs->action);
  iCount++; 

  FWNAT_CM_DEBUG ("%s:%s:%d Filled number of params = %d \n\r", 
      __FILE__, __FUNCTION__, __LINE__, iCount); 

  result_iv_pairs_p->count_ui = iCount;
#if 0 
  PrintCmFilterArgs (fwnat_inargs); 
#endif 
  return OF_SUCCESS;  
}


int32_t of_fwnat_fwdfilter_getnextrec(
      struct cm_array_of_iv_pairs * key_iv_pairs_p,
      struct cm_array_of_iv_pairs *prev_record_key_p, 
      uint32_t * count_p,
      struct cm_array_of_iv_pairs ** next_n_record_data_p)
{
  struct of_fwnat_cm_filter_inargs  fwnat_inargs; 
  struct cm_array_of_iv_pairs      *result_iv_pairs_p = NULL;
  struct cm_app_result             *app_result_p = NULL;
  int32_t                           ret_val = OF_FAILURE;
  uint32_t                          uiRecCount = 0;
  int32_t                           status = OF_SUCCESS; 

  CM_CBK_DEBUG_PRINT ("Entered");

  FWNAT_CM_DEBUG ("%s:%s:%d Entered. \n\r", __FILE__, __FUNCTION__, 
      __LINE__); 
  of_memset( &fwnat_inargs, 0 , 
              sizeof(struct of_fwnat_cm_filter_inargs)); 

  if (OF_SUCCESS != of_fwnat_fwdfilter_ucm_setmandparams(
                   &fwnat_inargs, key_iv_pairs_p, 
                   fwnat_inargs.chainname, 
                   &app_result_p))
  {
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    return OF_FAILURE; 
  }

  if (OF_SUCCESS !=  of_fwnat_fwdfilter_ucm_setoptparams 
                 (prev_record_key_p,
                  &fwnat_inargs,
                  &app_result_p))
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    return OF_FAILURE; 
  } 

  status = ip4_fwnat_get_next_record(&fwnat_inargs);  

  if (status != OF_SUCCESS)
  {
		CM_CBK_DEBUG_PRINT ("Get First FWNAT Failed");
    return OF_FAILURE; 
  }
   result_iv_pairs_p =
      (struct cm_array_of_iv_pairs *) of_calloc (1, 
             sizeof (struct cm_array_of_iv_pairs));

   if (result_iv_pairs_p == NULL)
   {
      CM_CBK_DEBUG_PRINT ("Memory allocation failed for "
          "result_iv_pairs_p");
      return OF_FAILURE;
   }

  status = fwnat_fwdfilter_ucm_getparams (&fwnat_inargs, 
               result_iv_pairs_p); 
  uiRecCount++; 

  if (status != OF_SUCCESS)
  {
		CM_CBK_DEBUG_PRINT ("fwnat_fwdfilter_ucm_getparams failed");
    return OF_FAILURE; 
  }

  *next_n_record_data_p = result_iv_pairs_p; 
  *count_p                = uiRecCount; 

  CM_CBK_DEBUG_PRINT ("Exited");

  return OF_SUCCESS; 
}
int32_t of_fwnat_fwdfilter_getexactrec 
        (struct cm_array_of_iv_pairs * key_iv_pairs_p,
         struct cm_array_of_iv_pairs ** pIvPairArr)
{
  struct of_fwnat_cm_filter_inargs  fwnat_inargs; 
  struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
  struct cm_app_result        *app_result_p = NULL;
  int32_t                      ret_val = OF_FAILURE;
  uint32_t                     uiRecCount = 0;
  int32_t                      status = OF_FAILURE; 

  CM_CBK_DEBUG_PRINT ("Entered."); 
  FWNAT_CM_DEBUG ("\n\r%s:%s:%d Entered. \n\r", __FILE__, __FUNCTION__, 
      __LINE__); 

  of_memset( &fwnat_inargs, 0 , 
              sizeof(struct of_fwnat_cm_filter_inargs)); 

  if (OF_SUCCESS != of_fwnat_fwdfilter_ucm_setmandparams(
                   &fwnat_inargs, key_iv_pairs_p, 
                   fwnat_inargs.chainname, 
                   &app_result_p))
  {
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    return OF_FAILURE; 
  }

  FWNAT_CM_DEBUG ("\n\r%s:%s:%d calling "
      "ip4_fwnat_get_exact_record. idpID = %llx \n\r", 
       __FILE__, __FUNCTION__, __LINE__, 
       fwnat_inargs.idpID); 

  status = ip4_fwnat_get_exact_record(&fwnat_inargs);  

  if (status != OF_SUCCESS)
  {
    FWNAT_CM_DEBUG ("\n\r%s:%s:%d "
        "ip4_fwnat_get_exact_record returned FAILURE. \n\r",
        __FILE__, __FUNCTION__, 
        __LINE__); 
    CM_CBK_DEBUG_PRINT ("Get First FWNAT Failed");
    *pIvPairArr = result_iv_pairs_p; 
    return OF_FAILURE; 
  }

  FWNAT_CM_DEBUG ("\n\r%s:%s:%d "
      "ip4_fwnat_get_exact_record returned SUCESS. \n\r",
      __FILE__, __FUNCTION__, 
      __LINE__); 
	result_iv_pairs_p =
		(struct cm_array_of_iv_pairs *) of_calloc (1, 
                        sizeof (struct cm_array_of_iv_pairs));
	if (result_iv_pairs_p == NULL)
	{
		CM_CBK_DEBUG_PRINT ("Failed to allocate memory for "
        "result_iv_pairs_p");
    *pIvPairArr = result_iv_pairs_p; 
		return OF_FAILURE;
	}

  FWNAT_CM_DEBUG ("\n\r%s:%s:%d "
      "nsname = %s  \n\r", __FILE__, __FUNCTION__, 
      __LINE__, fwnat_inargs.nsname); 
  status = fwnat_fwdfilter_ucm_getparams (&fwnat_inargs, 
                                result_iv_pairs_p); 

  uiRecCount++; 

  if (status != OF_SUCCESS)
  {
		CM_CBK_DEBUG_PRINT ("fwnat_fwdfilter_ucm_getparams failed");
    return OF_FAILURE; 
  }

  *pIvPairArr = result_iv_pairs_p; 

  FWNAT_CM_DEBUG ("%s: Exited with SUCCESS.", __FUNCTION__); 
  CM_CBK_DEBUG_PRINT ("Exited with SUCCESS."); 

  return OF_SUCCESS; 
}

int32_t of_fwnat_fwdfilter_ucm_setmandparams (
      struct of_fwnat_cm_filter_inargs * fwnat_inargs,
      struct cm_array_of_iv_pairs *
      mand_iv_pairs_p,
      char   *chainname,
      struct cm_app_result ** presult_p)
{
   uint32_t count;
   int32_t ns_id;
   uint64_t  idpId = 0; 

   CM_CBK_DEBUG_PRINT ("Entered");

   for (count = 0; count < mand_iv_pairs_p->count_ui;
       count++)
   {
     FWNAT_CM_DEBUG ("My mandatory params [%u]  = %s of len = %u \n\r ", 
                  count,
                  mand_iv_pairs_p->iv_pairs[count].value_p, 
                  mand_iv_pairs_p->iv_pairs[count].value_length); 
   }

   strcpy(fwnat_inargs->tablename,"FILTER"); 
   strcpy(fwnat_inargs->chainname,"FORWARD"); 

   for (count = 0; count < mand_iv_pairs_p->count_ui;
         count++)
   {
      switch (mand_iv_pairs_p->iv_pairs[count].id_ui)
      {

         case CM_DM_DATAPATH_DATAPATHID_ID:
           {
             FWNAT_CM_DEBUG ("%s: Datapath ID hit. %s \n\r", 
                 __FUNCTION__, 
                 mand_iv_pairs_p->iv_pairs[count].value_p); 
             fwnat_inargs->idpID = charTo64bitNum((char *) 
                 mand_iv_pairs_p->iv_pairs[count].value_p);
             FWNAT_CM_DEBUG ("%s: Datapath ID hit. %llx \n\r", 
                 __FUNCTION__, fwnat_inargs->idpID);  
           }
            break;
         case CM_DM_FILTERFORWARD_TABLENAME_ID:
           {

             FWNAT_CM_DEBUG ("%s: Table name provided. %s \n\r", 
                 __FUNCTION__, 
                 mand_iv_pairs_p->iv_pairs[count].value_p); 
#if 0
             // ignore this value for now. It is not
             // the table name. 
             strncpy(fwnat_inargs->tablename, 
                   mand_iv_pairs_p->iv_pairs[count].value_p,
                   MAX_IPT_CHAIN_NAME); 
#endif 
           }
            break;
         default:
            FWNAT_CM_DEBUG ("%s: default option hit.  ID not known %u \n\r", 
                   __FUNCTION__, mand_iv_pairs_p->iv_pairs[count].id_ui); 
            break;

      }
   }
   CM_CBK_PRINT_IVPAIR_ARRAY (mand_iv_pairs_p);
   return OF_SUCCESS;
}

int32_t of_fwnat_fwdfilter_ucm_setoptparams (struct cm_array_of_iv_pairs *
      opt_iv_pairs_p,
      struct of_fwnat_cm_filter_inargs * fwnat_cm_filter_inargs,
      struct cm_app_result ** presult_p)
{
  uint32_t count;

  CM_CBK_DEBUG_PRINT ("Entered");

  for (count = 0; count < opt_iv_pairs_p->count_ui; count++)
  {
    switch (opt_iv_pairs_p->iv_pairs[count].id_ui)
    {
      case CM_DM_FILTERFORWARD_RULEPOSITION_ID:
        fwnat_cm_filter_inargs->rulePosition = of_atoi
          ((char *)opt_iv_pairs_p->iv_pairs[count].value_p);
        fwnat_cm_filter_inargs->attrflags |= IP4_FWNAT_RULE_POS_SET; 
        break;
      case CM_DM_FILTERFORWARD_SOURCEIP_ID:
          strcpy(fwnat_cm_filter_inargs->sourceip, 
            ((char *)opt_iv_pairs_p->iv_pairs[count].value_p));
        fwnat_cm_filter_inargs->attrflags |= IP4_FWNAT_SIP_ATTR_SET; 
        break;
      case CM_DM_FILTERFORWARD_DESTIP_ID:
          strcpy(fwnat_cm_filter_inargs->destip, 
            ((char *)opt_iv_pairs_p->iv_pairs[count].value_p));
        fwnat_cm_filter_inargs->attrflags |= IP4_FWNAT_DIP_ATTR_SET; 
        break;
      case CM_DM_FILTERFORWARD_PROTOCOL_ID:
          strcpy(fwnat_cm_filter_inargs->protocol, 
            ((char *)opt_iv_pairs_p->iv_pairs[count].value_p));
        fwnat_cm_filter_inargs->attrflags |= IP4_FWNAT_PROTO_ATTR_SET; 
        break;
      case CM_DM_FILTERFORWARD_SOURCEPORT_ID:
          strcpy(fwnat_cm_filter_inargs->sourceport, 
            ((char *)opt_iv_pairs_p->iv_pairs[count].value_p));
        fwnat_cm_filter_inargs->attrflags |= IP4_FWNAT_SPORT_ATTR_SET; 
        break;
      case CM_DM_FILTERFORWARD_DESTPORT_ID:
          strcpy(fwnat_cm_filter_inargs->destport, 
            ((char *)opt_iv_pairs_p->iv_pairs[count].value_p));
        fwnat_cm_filter_inargs->attrflags |= IP4_FWNAT_DPORT_ATTR_SET; 
        break;
      case CM_DM_FILTERFORWARD_CHAINNAME_ID:
        of_strncpy (fwnat_cm_filter_inargs->chainname,
            (char *) opt_iv_pairs_p->iv_pairs[count].value_p,
            opt_iv_pairs_p->iv_pairs[count].value_length);
        break;
      default:
        break;
    }
  }

  strcpy(fwnat_cm_filter_inargs->tablename,"FILTER"); 
  strcpy(fwnat_cm_filter_inargs->chainname,"FORWARD"); 

  CM_CBK_PRINT_IVPAIR_ARRAY (opt_iv_pairs_p);
  return OF_SUCCESS;
}
#endif 
