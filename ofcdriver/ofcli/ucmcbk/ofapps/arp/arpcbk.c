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
 * File name: arpcbk.c
 * Author:  <B36289@freescale.com>
 * Date:   05/15/2013
 * Description: 
 * 
 */

#ifdef OF_CM_SUPPORT

/******************************************************************************
 * * * * * * * * * * * * Include Files * * * * * * * * * * * * * * * * * *
 *******************************************************************************/
#include "cbcmninc.h"
#include "cmgutil.h"
#include "cntrucminc.h"
#include "cntrl_queue.h"
#include "cntlr_tls.h"
#include "cntlr_transprt.h"
#include "of_arp.h"

/******************************************************************************
 * * * * * * * * * * * * Global Variables * * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

int32_t of_arp_appl_ucmcbk_init (void);

/******************************************************************************
 * * * * * * * * * * * * Function Prototypes * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

/** Need to edit/write add, del, modify, verify functions **/
int32_t arp_addrec (void * config_trans_p,
		struct cm_array_of_iv_pairs * mand_params,
		struct cm_array_of_iv_pairs * opt_params,
		struct cm_app_result ** result_p);

int32_t arp_modrec (void * config_trans_p,
		struct cm_array_of_iv_pairs * mand_params,
		struct cm_array_of_iv_pairs * opt_params,
		struct cm_app_result ** result_p);

int32_t arp_delrec (void * config_transaction_p,
		struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_app_result ** result_p);

int32_t arp_verifycfg (struct cm_array_of_iv_pairs * keys_arr_p,
		uint32_t command_id, struct cm_app_result ** result_p);

int32_t arp_getfirstnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
		uint32_t * count_p,
		struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p);

int32_t arp_getnextnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_array_of_iv_pairs * prev_record_key_p, uint32_t * count_p,
		struct cm_array_of_iv_pairs ** next_n_record_data_p);

int32_t arp_getexactrec (struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_array_of_iv_pairs ** pIvPairArr);

extern char *net_ntoa (uint32_t nta, char * buf);
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * P R O T O C O L  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
 * * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */
int32_t arp_ucm_set_mand_params (struct cm_array_of_iv_pairs *
		params,
		uint32_t * nsid,arp_record_t * arp_record_info,
		struct cm_app_result ** presult_p);

int32_t arp_ucm_set_opt_params (struct cm_array_of_iv_pairs *
		params,
		arp_record_t * arp_record_info,
		struct cm_app_result ** presult_p);


int32_t arp_ucm_getparams ( arp_record_t *arp_record_info,
		struct cm_array_of_iv_pairs * result_iv_pairs_p);

int32_t arp_ucm_validateoptparams (struct cm_array_of_iv_pairs *
		params,
		struct cm_app_result ** presult_p);


struct cm_dm_app_callbacks arp_ucm_callbacks = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	arp_getfirstnrecs,
	arp_getnextnrecs,
	arp_getexactrec,
	NULL,
	NULL
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * Protocol  UCM Init * * * * * * *  * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_init()
{
	//int32_t status=OF_SUCCESS;
	int32_t retval=OF_SUCCESS;

	CM_CBK_DEBUG_PRINT("Loading  ARP CBK");

	do
	{
		/** Initialize ARP CBK */
		retval=of_arp_appl_ucmcbk_init();
		if(retval != OF_SUCCESS)
		{
			CM_CBK_DEBUG_PRINT( "ARP CBK initialization failed");
			//status= OF_FAILURE;
			break;
		}

	}while(0);

	return;
}
#endif

int32_t of_arp_appl_ucmcbk_init (void)
{
	CM_CBK_DEBUG_PRINT ("Entry");
	cm_register_app_callbacks (CM_ON_DIRECTOR_NAMESPACE_APPS_ARP_APPL_ID,&arp_ucm_callbacks);
	return OF_SUCCESS;
}

/******************************************************************************
 * * * * * * * * * * * * Function Definitions * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

int32_t arp_addrec (void * config_trans_p,
		struct cm_array_of_iv_pairs * mand_params,
		struct cm_array_of_iv_pairs * opt_params,
		struct cm_app_result ** result_p)
{
	struct cm_app_result *arp_result = NULL;
	int32_t return_value = OF_FAILURE;
	arp_record_t arp_record_info = { };
	uint32_t ns_id;

	CM_CBK_DEBUG_PRINT ("Entered");

	of_memset (&arp_record_info, 0, sizeof (arp_record_t));

	if ((arp_ucm_set_mand_params (mand_params, &ns_id, &arp_record_info, &arp_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		//		fill_app_result_struct (&arp_result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
		*result_p=arp_result;
		return OF_FAILURE;
	}

	return_value = arp_ucm_set_opt_params (opt_params, &arp_record_info, &arp_result);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Optional Parameters Failed");
		//		fill_app_result_struct (&arp_result, NULL, CM_GLU_SET_OPT_PARAM_FAILED);
		*result_p = arp_result;
		return OF_FAILURE;
	}
	/* later extend it to the UCM -- Hard coding here*/
	// strcpy(protocol_info.path_name,"/path/to/name" );
	// protocol_info.transport_type = 6;

	//	return_value=cntlr_add_arp_record(&arp_record_info);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("add arp record info Failed");
		//		fill_app_result_struct (&arp_result, NULL, CM_GLU_DOMAIN_ADD_FAILED);
		*result_p = arp_result;
		return OF_FAILURE;
	}

	CM_CBK_DEBUG_PRINT ("arp record added successfully");
	return OF_SUCCESS;
}

int32_t arp_modrec (void * config_trans_p,
		struct cm_array_of_iv_pairs * mand_params,
		struct cm_array_of_iv_pairs * opt_params,
		struct cm_app_result ** result_p)
{
	struct cm_app_result *arp_result = NULL;
	int32_t return_value = OF_FAILURE;
	arp_record_t arp_record_info = { };
	uint32_t ns_id;

	CM_CBK_DEBUG_PRINT ("Entered");

	of_memset (&arp_record_info, 0, sizeof (arp_record_t));

	if ((arp_ucm_set_mand_params (mand_params, &ns_id, &arp_record_info, &arp_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		//		fill_app_result_struct (&arp_result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
		*result_p=arp_result;
		return OF_FAILURE;
	}

	return_value = arp_ucm_set_opt_params (opt_params, &arp_record_info, &arp_result);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Optional Parameters Failed");
		//		fill_app_result_struct (&arp_result, NULL, CM_GLU_SET_OPT_PARAM_FAILED);
		*result_p = arp_result;
		return OF_FAILURE;
	}


	//return_value=cntlr_modify_arp_record(&arp_record_info);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("arp record Modification Failed");
		//		fill_app_result_struct (&arp_result, NULL, CM_GLU_DOMAIN_ADD_FAILED);
		*result_p = arp_result;
		return OF_FAILURE;
	}


	CM_CBK_DEBUG_PRINT ("arp record Modifed succesfully");
	return OF_SUCCESS;
}

int32_t arp_delrec (void * config_transaction_p,
		struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_app_result ** result_p)
{
	struct cm_app_result *arp_result = NULL;
	int32_t return_value = OF_FAILURE;
	arp_record_t arp_record_info = { };
	uint32_t ns_id;

	CM_CBK_DEBUG_PRINT ("Entered");
	of_memset (&arp_record_info, 0, sizeof (arp_record_t));

	if ((arp_ucm_set_mand_params (keys_arr_p, &ns_id, &arp_record_info, &arp_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		//		fill_app_result_struct (&arp_result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
		*result_p = arp_result;
		return OF_FAILURE;
	}

//	return_value=cntlr_delete_arp_record(&arp_record_info);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Delete Table record failed");
		//		fill_app_result_struct (&arp_result, NULL, CM_GLU_DATAPATH_DELETE_FAILED);
		*result_p = arp_result;
		return OF_FAILURE;
	}

	CM_CBK_DEBUG_PRINT ("arp record deleted succesfully");
	return OF_SUCCESS;
}

int32_t arp_getfirstnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
		uint32_t * count_p,
		struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p)
{
	struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
	int32_t return_value = OF_FAILURE;
	uint32_t uiRecCount = 0;
	struct cm_app_result *arp_result = NULL;
	arp_record_t arp_record_info = { };
	uint32_t ns_id;
	CM_CBK_DEBUG_PRINT ("Entered");
	of_memset (&arp_record_info, 0, sizeof (arp_record_t));
	if ((arp_ucm_set_mand_params (keys_arr_p, &ns_id, &arp_record_info, &arp_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		return OF_FAILURE;
	}

	return_value=arp_get_first_record(ns_id,&arp_record_info);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Get firstnrecords failed for arp Table ns_id %d",ns_id);
		return OF_FAILURE;
	}

	result_iv_pairs_p =
		(struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
	if (result_iv_pairs_p == NULL)
	{
		CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
		return OF_FAILURE;
	}
	arp_ucm_getparams (&arp_record_info, &result_iv_pairs_p[uiRecCount]);
	uiRecCount++;
	*array_of_iv_pair_arr_p = result_iv_pairs_p;
	*count_p = uiRecCount;
	CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
	return OF_SUCCESS;
}


int32_t arp_getnextnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_array_of_iv_pairs *prev_record_key_p, uint32_t * count_p,
		struct cm_array_of_iv_pairs ** next_n_record_data_p)
{

	struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
	struct cm_app_result *arp_result = NULL;
	int32_t return_value = OF_FAILURE;
	uint32_t uiRecCount = 0;
	arp_record_t arp_record_info={};
	uint32_t ns_id;
	CM_CBK_DEBUG_PRINT ("Entered");
	of_memset (&arp_record_info, 0, sizeof (arp_record_t));
	result_iv_pairs_p =
		(struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
	if (result_iv_pairs_p == NULL)
	{
		CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
		return OF_FAILURE;
	}

	if ((arp_ucm_set_mand_params (keys_arr_p, &ns_id, &arp_record_info,&arp_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		return OF_FAILURE;
	}
	if ((arp_ucm_set_mand_params (prev_record_key_p, &ns_id, &arp_record_info, &arp_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		return OF_FAILURE;
	}

	return_value=arp_get_next_record(ns_id,&arp_record_info);
	if (return_value == OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Next arp  record is ");
		arp_ucm_getparams (&arp_record_info, &result_iv_pairs_p[uiRecCount]);
		uiRecCount++;
	}

	CM_CBK_DEBUG_PRINT ("Number of records requested were %u ", *count_p);
	CM_CBK_DEBUG_PRINT ("Number of records found are %u", uiRecCount+1);
	*next_n_record_data_p = result_iv_pairs_p;
	*count_p = uiRecCount;
	CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
	return OF_SUCCESS;
}

int32_t arp_getexactrec (struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_array_of_iv_pairs ** pIvPairArr)
{
	struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
	struct cm_app_result *arp_result = NULL;
	int32_t return_value = OF_FAILURE;
	arp_record_t arp_record_info={};
	uint32_t ns_id;
	CM_CBK_DEBUG_PRINT ("Entered arp_getexact fn successfully");
	result_iv_pairs_p =
		(struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
	if (result_iv_pairs_p == NULL)
	{
		CM_CBK_DEBUG_PRINT ("Failed to allocate memory for result_iv_pairs_p");
		return OF_FAILURE;
	}

	if ((arp_ucm_set_mand_params (keys_arr_p, &ns_id,&arp_record_info, &arp_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		return OF_FAILURE;
	}


	of_memset (&arp_record_info, 0, sizeof (arp_record_t));
	return_value=arp_get_exact_record(ns_id,&arp_record_info);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Error: get dprm table handle failed");
		return OF_FAILURE;
	}

	CM_CBK_DEBUG_PRINT ("Exact matching record found");
	arp_ucm_getparams (&arp_record_info, result_iv_pairs_p);
	*pIvPairArr = result_iv_pairs_p;
	CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
	return return_value;
}

int32_t arp_verifycfg (struct cm_array_of_iv_pairs * keys_arr_p,
		uint32_t command_id, struct cm_app_result ** result_p)
{
	struct cm_app_result *arp_result = NULL;
	int32_t return_value = OF_FAILURE;
	arp_record_t arp_record_info = { };

	CM_CBK_DEBUG_PRINT ("Entered");
	of_memset (&arp_record_info, 0, sizeof (arp_record_t));

//	return_value = arp_ucm_validatemandparams (keys_arr_p, &arp_result);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Validate Mandatory Parameters Failed");
		*result_p = arp_result;
		return OF_FAILURE;
	}

	if (arp_ucm_validateoptparams (keys_arr_p, &arp_result)
			!= OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Validate Optional Parameters Failed");
		*result_p = arp_result;
		return OF_FAILURE;
	}
	*result_p = arp_result;
	return OF_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * P R O T O C O L  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
 * * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */
int32_t arp_ucm_validatemandparams (struct cm_array_of_iv_pairs *
		params,   struct cm_app_result **presult_p)
{
	uint32_t count;
	struct cm_app_result *arp_result = NULL;
	CM_CBK_DEBUG_PRINT ("Entered");
	for (count = 0; count < params->count_ui;
			count++)
	{
		switch (params->iv_pairs[count].id_ui)
		{
			case CM_DM_ARP_IPADDRESS_ID:
				if (params->iv_pairs[count].value_p == NULL)
				{
					CM_CBK_DEBUG_PRINT ("ip addredd is NULL");
					//	fill_app_result_struct (&arp_result, NULL, CM_GLU_DOMAIN_NAME_NULL);
					*presult_p = arp_result;
					return OF_FAILURE;
				}
			//	of_strncpy (dest_ipaddr,
			//			(uint32_t *) params->iv_pairs[count].value_p,
			//			params->iv_pairs[count].value_length);
				break;

			case CM_DM_ARP_HWADDRESS_ID: 
				if (params->iv_pairs[count].value_p == NULL)
				{
					CM_CBK_DEBUG_PRINT ("hardware address is NULL");
					//	fill_app_result_struct (&arp_result, NULL, CM_GLU_DATAPATH_ID_NULL);
					*presult_p = arp_result;
					return OF_FAILURE;
				}
				//	id=of_atoi(params->iv_pairs[count].value_p);
				//			fill_app_result_struct (&arp_result, NULL, CM_GLU_CNTLR_XPRT_INVALID_PROTOCOL_ID);
				*presult_p = arp_result;
				return OF_FAILURE;

		}
		break;
	}
CM_CBK_PRINT_IVPAIR_ARRAY (params);
//*presult_p = arp_result;
return OF_SUCCESS;
}


int32_t arp_ucm_validateoptparams (struct cm_array_of_iv_pairs *
		params,
		struct cm_app_result ** presult_p)
{

	CM_CBK_DEBUG_PRINT ("Entered");
	return OF_SUCCESS;
}

int32_t arp_ucm_set_mand_params (struct cm_array_of_iv_pairs *
		params,uint32_t *ns_id_p,arp_record_t *arp_record_info,struct cm_app_result ** presult_p)
{
	uint32_t count;
	uint32_t nsid;
	char ns_name[5];
//	uint32_t dst_ipaddr;
	int32_t status = OF_FAILURE;
	CM_CBK_DEBUG_PRINT ("Entered");
	for (count = 0; count < params->count_ui;
			count++)
	{
		switch (params->iv_pairs[count].id_ui)
		{
			case CM_DM_NAMESPACE_NAME_ID:
				{
					strcpy(ns_name,(char *) params->iv_pairs[count].value_p);
					status = of_nem_get_nsid_from_namespace(ns_name, &nsid);
					if(status == OF_SUCCESS)
					{
						CM_CBK_DEBUG_PRINT("Conversion from nsid to string success");
						*ns_id_p = nsid;   
					}
				}
				break;
			case CM_DM_ARP_IPADDRESS_ID:
				{
					CM_CBK_DEBUG_PRINT("ip address %s",(char *) params->iv_pairs[count].value_p);
					cm_val_and_conv_aton((char *)params->iv_pairs[count].value_p,&arp_record_info->dest_ipaddr);
					CM_CBK_DEBUG_PRINT("ip address 0x%x",arp_record_info->dest_ipaddr);
				}
				break;
		}
	}
	CM_CBK_PRINT_IVPAIR_ARRAY (params);
	return OF_SUCCESS;
}

int32_t arp_ucm_set_opt_params (struct cm_array_of_iv_pairs *
		params,
		arp_record_t * arp_record_info,
		struct cm_app_result ** presult_p)
{
	uint32_t count;

	CM_CBK_DEBUG_PRINT ("Entered");

	for (count = 0; count < params->count_ui; count++)
	{
		switch (params->iv_pairs[count].id_ui)
		{
			break;
			default:
			break;
		}
	}

	CM_CBK_PRINT_IVPAIR_ARRAY (params);
	return OF_SUCCESS;
}

int32_t arp_ucm_getparams (arp_record_t *arp_record_info,
		struct cm_array_of_iv_pairs * result_iv_pairs_p)
{
	uint32_t uindex_i = 0;
	char buf[64] = "";

	char *arp_states[]={
		"FREE",
		"INCOMPLETE",
		"PROBE",
		"STALE",
		"REACHABLE",
	};

#define CM_ARP_CHILD_COUNT 5

	result_iv_pairs_p->iv_pairs =
		(struct cm_iv_pair *) of_calloc (CM_ARP_CHILD_COUNT, sizeof (struct cm_iv_pair));
	if (result_iv_pairs_p->iv_pairs == NULL)
	{
		CM_CBK_DEBUG_PRINT
			("Memory allocation failed for result_iv_pairs_p->iv_pairs");
		return OF_FAILURE;
	}

	/*Address*/
	net_ntoa(arp_record_info->dest_ipaddr,buf);
//	printf("ip_address %x", arp_record_info->dest_ipaddr);
	FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i],CM_DM_ARP_IPADDRESS_ID,
			CM_DATA_TYPE_STRING, buf);
	uindex_i++;

	/* HW Addrees */
	if(arp_record_info->dst_hwaddr != NULL)
		sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",arp_record_info->dst_hwaddr[0],arp_record_info->dst_hwaddr[1],arp_record_info->dst_hwaddr[2],arp_record_info->dst_hwaddr[3],arp_record_info->dst_hwaddr[4],arp_record_info->dst_hwaddr[5]);
	FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i],CM_DM_ARP_HWADDRESS_ID,
			CM_DATA_TYPE_STRING, buf);
	uindex_i++;

	/* Flags */
	if (arp_record_info->flags == 1)
		strcpy(buf,"DYNAMIC");
	else
		if (arp_record_info->flags == 0)
			strcpy(buf,"STATIC");
	FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i],CM_DM_ARP_FLAGS_ID,
			CM_DATA_TYPE_STRING, buf);
	uindex_i++;

	/*State*/
	strcpy(buf,arp_states[arp_record_info->state]);
	FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i],CM_DM_ARP_STATE_ID,
			CM_DATA_TYPE_STRING, buf);
	uindex_i++;


	/* Iface */
	sprintf(buf,"peth_%d",arp_record_info->if_index);
	FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i], CM_DM_ARP_IFACE_ID,
			CM_DATA_TYPE_STRING, buf);
	uindex_i++;

	result_iv_pairs_p->count_ui = uindex_i;
	CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
	return OF_SUCCESS;
}

#endif /* OF_CM_SUPPORT */
