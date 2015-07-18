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
 * File name: mcastcbk.c
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
#include "ip4_mfe_api.h"
#include "of_mcast.h"

/******************************************************************************
 * * * * * * * * * * * * Global Variables * * * * * * * * * * * * * * * * * * *
 *******************************************************************************/
#define MAX_BUF_SIZE    64

int32_t of_mcast_appl_ucmcbk_init (void);

/******************************************************************************
 * * * * * * * * * * * * Function Prototypes * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

/** Need to edit/write add, del, modify, verify functions **/
int32_t mcast_addrec (void * config_trans_p,
		struct cm_array_of_iv_pairs * mand_params,
		struct cm_array_of_iv_pairs * opt_params,
		struct cm_app_result ** result_p);

int32_t mcast_modrec (void * config_trans_p,
		struct cm_array_of_iv_pairs * mand_params,
		struct cm_array_of_iv_pairs * opt_params,
		struct cm_app_result ** result_p);

int32_t mcast_delrec (void * config_transaction_p,
		struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_app_result ** result_p);

int32_t mcast_verifycfg (struct cm_array_of_iv_pairs * keys_arr_p,
		uint32_t command_id, struct cm_app_result ** result_p);

int32_t mcast_getfirstnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
		uint32_t * count_p,
		struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p);

int32_t mcast_getnextnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_array_of_iv_pairs * prev_record_key_p, uint32_t * count_p,
		struct cm_array_of_iv_pairs ** next_n_record_data_p);

int32_t mcast_getexactrec (struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_array_of_iv_pairs ** pIvPairArr);

extern char *net_ntoa (uint32_t nta, char * buf);
extern int32_t ip4_mcfwd_get_iface_id_by_vif_id(uint32_t ns_id, uint32_t vif_id, uint32_t *link_id_p);
extern int32_t iface_db_get_ifname_from_ifid(uint32_t ns_id, int32_t ifid, char *if_name);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * P R O T O C O L  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
 * * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */
int32_t mcast_ucm_set_mand_params (struct cm_array_of_iv_pairs *
		params,
		uint32_t * nsid,struct of_ip4_mcfwd_mfe_info * mcast_record_info,
		struct cm_app_result ** presult_p);

int32_t mcast_ucm_set_opt_params (struct cm_array_of_iv_pairs *
		params,
		struct of_ip4_mcfwd_mfe_info * mcast_record_info,
		struct cm_app_result ** presult_p);
int32_t mcast_ucm_validateoptparams (struct cm_array_of_iv_pairs *
                params,
                struct cm_app_result ** presult_p);


int32_t mcast_ucm_getparams ( struct of_ip4_mcfwd_mfe_info *mcast_record_info,
		struct cm_array_of_iv_pairs * result_iv_pairs_p,uint32_t ns_id);


struct cm_dm_app_callbacks mcast_ucm_callbacks = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	mcast_getfirstnrecs,
	mcast_getnextnrecs,
	mcast_getexactrec,
	NULL,
	NULL
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * Protocol  UCM Init * * * * * * *  * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_init()
{
	int32_t retval=OF_SUCCESS;

	CM_CBK_DEBUG_PRINT("Loading  Multicast CBK");

	do
	{
		/** Initialize MCAST CBK */
		retval=of_mcast_appl_ucmcbk_init();
		if(retval != OF_SUCCESS)
		{
			CM_CBK_DEBUG_PRINT( "Multicast CBK initialization failed");
			break;
		}

	}while(0);

	return;
}
#endif

int32_t of_mcast_appl_ucmcbk_init (void)
{
	CM_CBK_DEBUG_PRINT ("Entry");
	cm_register_app_callbacks (CM_ON_DIRECTOR_NAMESPACE_APPS_MCAST_APPL_ID,&mcast_ucm_callbacks);
	return OF_SUCCESS;
}

/******************************************************************************
 * * * * * * * * * * * * Function Definitions * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

int32_t mcast_addrec (void * config_trans_p,
		struct cm_array_of_iv_pairs * mand_params,
		struct cm_array_of_iv_pairs * opt_params,
		struct cm_app_result ** result_p)
{
	struct cm_app_result *mcast_result = NULL;
	int32_t return_value = OF_FAILURE;
	struct of_ip4_mcfwd_mfe_info mcast_record_info = { };
	uint32_t ns_id;

	CM_CBK_DEBUG_PRINT ("Entered");

	of_memset (&mcast_record_info, 0, sizeof (struct of_ip4_mcfwd_mfe_info));

	if ((mcast_ucm_set_mand_params (mand_params, &ns_id, &mcast_record_info, &mcast_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		//		fill_app_result_struct (&mcast_result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
		*result_p=mcast_result;
		return OF_FAILURE;
	}

	return_value = mcast_ucm_set_opt_params (opt_params, &mcast_record_info, &mcast_result);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Optional Parameters Failed");
		//		fill_app_result_struct (&mcast_result, NULL, CM_GLU_SET_OPT_PARAM_FAILED);
		*result_p = mcast_result;
		return OF_FAILURE;
	}
	/* later extend it to the UCM -- Hard coding here*/
	// strcpy(protocol_info.path_name,"/path/to/name" );
	// protocol_info.transport_type = 6;

	//	return_value=cntlr_add_mcast_record(&mcast_record_info);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("add mcast record info Failed");
		//		fill_app_result_struct (&mcast_result, NULL, CM_GLU_DOMAIN_ADD_FAILED);
		*result_p = mcast_result;
		return OF_FAILURE;
	}

	CM_CBK_DEBUG_PRINT ("mcast record added successfully");
	return OF_SUCCESS;
}

int32_t mcast_modrec (void * config_trans_p,
		struct cm_array_of_iv_pairs * mand_params,
		struct cm_array_of_iv_pairs * opt_params,
		struct cm_app_result ** result_p)
{
	struct cm_app_result *mcast_result = NULL;
	int32_t return_value = OF_FAILURE;
	struct of_ip4_mcfwd_mfe_info mcast_record_info = { };
	uint32_t ns_id;

	CM_CBK_DEBUG_PRINT ("Entered");

	of_memset (&mcast_record_info, 0, sizeof (struct of_ip4_mcfwd_mfe_info));

	if ((mcast_ucm_set_mand_params (mand_params, &ns_id, &mcast_record_info, &mcast_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		//		fill_app_result_struct (&mcast_result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
		*result_p=mcast_result;
		return OF_FAILURE;
	}

	return_value = mcast_ucm_set_opt_params (opt_params, &mcast_record_info, &mcast_result);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Optional Parameters Failed");
		//		fill_app_result_struct (&mcast_result, NULL, CM_GLU_SET_OPT_PARAM_FAILED);
		*result_p = mcast_result;
		return OF_FAILURE;
	}


	//return_value=cntlr_modify_mcast_record(&mcast_record_info);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("mcast record Modification Failed");
		//		fill_app_result_struct (&mcast_result, NULL, CM_GLU_DOMAIN_ADD_FAILED);
		*result_p = mcast_result;
		return OF_FAILURE;
	}


	CM_CBK_DEBUG_PRINT ("mcast record Modifed succesfully");
	return OF_SUCCESS;
}

int32_t mcast_delrec (void * config_transaction_p,
		struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_app_result ** result_p)
{
	struct cm_app_result *mcast_result = NULL;
	int32_t return_value = OF_FAILURE;
	struct of_ip4_mcfwd_mfe_info mcast_record_info = { };
	uint32_t ns_id;

	CM_CBK_DEBUG_PRINT ("Entered");
	of_memset (&mcast_record_info, 0, sizeof (struct of_ip4_mcfwd_mfe_info));

	if ((mcast_ucm_set_mand_params (keys_arr_p, &ns_id, &mcast_record_info, &mcast_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		//		fill_app_result_struct (&mcast_result, NULL, CM_GLU_SET_MAND_PARAM_FAILED);
		*result_p = mcast_result;
		return OF_FAILURE;
	}

//	return_value=cntlr_delete_mcast_record(&mcast_record_info);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Delete Table record failed");
		//		fill_app_result_struct (&mcast_result, NULL, CM_GLU_DATAPATH_DELETE_FAILED);
		*result_p = mcast_result;
		return OF_FAILURE;
	}

	CM_CBK_DEBUG_PRINT ("mcast record deleted succesfully");
	return OF_SUCCESS;
}

int32_t mcast_getfirstnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
		uint32_t * count_p,
		struct cm_array_of_iv_pairs ** array_of_iv_pair_arr_p)
{
	struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
	int32_t return_value = OF_FAILURE;
	uint32_t uiRecCount = 0;
	struct cm_app_result *mcast_result = NULL;
	struct of_ip4_mcfwd_mfe_info mcast_record_info = { };
	uint32_t ns_id;
	CM_CBK_DEBUG_PRINT ("Entered");
	of_memset (&mcast_record_info, 0, sizeof (struct of_ip4_mcfwd_mfe_info));
	if ((mcast_ucm_set_mand_params (keys_arr_p, &ns_id, &mcast_record_info, &mcast_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		return OF_FAILURE;
	}

	return_value=of_ip4_mcfwd_mfe_get_first_record(ns_id,&mcast_record_info);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Get firstnrecords failed for mcast Table ns_id %d",ns_id);
		return OF_FAILURE;
	}

	result_iv_pairs_p =
		(struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
	if (result_iv_pairs_p == NULL)
	{
		CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
		return OF_FAILURE;
	}
	mcast_ucm_getparams (&mcast_record_info, &result_iv_pairs_p[uiRecCount],ns_id);
	uiRecCount++;
	*array_of_iv_pair_arr_p = result_iv_pairs_p;
	*count_p = uiRecCount;
	CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
	return OF_SUCCESS;
}


int32_t mcast_getnextnrecs (struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_array_of_iv_pairs *prev_record_key_p, uint32_t * count_p,
		struct cm_array_of_iv_pairs ** next_n_record_data_p)
{

	struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
	struct cm_app_result *mcast_result = NULL;
	int32_t return_value = OF_FAILURE;
	uint32_t uiRecCount = 0;
	struct of_ip4_mcfwd_mfe_info mcast_record_info={};
	uint32_t ns_id;
	CM_CBK_DEBUG_PRINT ("Entered");
	of_memset (&mcast_record_info, 0, sizeof (struct of_ip4_mcfwd_mfe_info));
	result_iv_pairs_p =
		(struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
	if (result_iv_pairs_p == NULL)
	{
		CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
		return OF_FAILURE;
	}

	if ((mcast_ucm_set_mand_params (keys_arr_p, &ns_id, &mcast_record_info,&mcast_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		return OF_FAILURE;
	}
	if ((mcast_ucm_set_mand_params (prev_record_key_p, &ns_id, &mcast_record_info, &mcast_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		return OF_FAILURE;
	}

	return_value=of_ip4_mcfwd_mfe_get_next_record(ns_id,&mcast_record_info);
	if (return_value == OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Next mcast  record is ");
		mcast_ucm_getparams (&mcast_record_info, &result_iv_pairs_p[uiRecCount],ns_id);
		uiRecCount++;
	}

	CM_CBK_DEBUG_PRINT ("Number of records requested were %u ", *count_p);
	CM_CBK_DEBUG_PRINT ("Number of records found are %u", uiRecCount+1);
	*next_n_record_data_p = result_iv_pairs_p;
	*count_p = uiRecCount;
	CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
	return OF_SUCCESS;
}

int32_t mcast_getexactrec (struct cm_array_of_iv_pairs * keys_arr_p,
		struct cm_array_of_iv_pairs ** pIvPairArr)
{
	struct cm_array_of_iv_pairs *result_iv_pairs_p = NULL;
	struct cm_app_result *mcast_result = NULL;
	int32_t return_value = OF_FAILURE;
	struct of_ip4_mcfwd_mfe_info mcast_record_info={};
	uint32_t ns_id;
	CM_CBK_DEBUG_PRINT ("Entered mcast_getexact fn successfully");
	result_iv_pairs_p =
		(struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
	if (result_iv_pairs_p == NULL)
	{
		CM_CBK_DEBUG_PRINT ("Failed to allocate memory for result_iv_pairs_p");
		return OF_FAILURE;
	}

	if ((mcast_ucm_set_mand_params (keys_arr_p, &ns_id,&mcast_record_info, &mcast_result)) !=
			OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
		return OF_FAILURE;
	}


	of_memset (&mcast_record_info, 0, sizeof (struct of_ip4_mcfwd_mfe_info));
	return_value=of_ip4_mcfwd_mfe_get_exact_record(ns_id,&mcast_record_info);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Error: get dprm table handle failed");
		return OF_FAILURE;
	}

	CM_CBK_DEBUG_PRINT ("Exact matching record found");
	mcast_ucm_getparams (&mcast_record_info, result_iv_pairs_p,ns_id);
	*pIvPairArr = result_iv_pairs_p;
	CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
	return return_value;
}

int32_t mcast_verifycfg (struct cm_array_of_iv_pairs * keys_arr_p,
		uint32_t command_id, struct cm_app_result ** result_p)
{
	struct cm_app_result *mcast_result = NULL;
	int32_t return_value = OF_FAILURE;
	struct of_ip4_mcfwd_mfe_info mcast_record_info = { };

	CM_CBK_DEBUG_PRINT ("Entered");
	of_memset (&mcast_record_info, 0, sizeof (struct of_ip4_mcfwd_mfe_info));

//	return_value = mcast_ucm_validatemandparams (keys_arr_p, &mcast_result);
	if (return_value != OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Validate Mandatory Parameters Failed");
		*result_p = mcast_result;
		return OF_FAILURE;
	}

	if (mcast_ucm_validateoptparams (keys_arr_p, &mcast_result)
			!= OF_SUCCESS)
	{
		CM_CBK_DEBUG_PRINT ("Validate Optional Parameters Failed");
		*result_p = mcast_result;
		return OF_FAILURE;
	}
	*result_p = mcast_result;
	return OF_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * P R O T O C O L  U T I L I T Y  F U N C T I O N S * * * * * * * * * * *
 * * * * * * * * * * * * * **  * * * * * * * **  * * * * * * * * * * * * * */
int32_t mcast_ucm_validatemandparams (struct cm_array_of_iv_pairs *
		params,   struct cm_app_result **presult_p)
{
	uint32_t count;
	struct cm_app_result *mcast_result = NULL;
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
					//	fill_app_result_struct (&mcast_result, NULL, CM_GLU_DOMAIN_NAME_NULL);
					*presult_p = mcast_result;
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
					//	fill_app_result_struct (&mcast_result, NULL, CM_GLU_DATAPATH_ID_NULL);
					*presult_p = mcast_result;
					return OF_FAILURE;
				}
				//	id=of_atoi(params->iv_pairs[count].value_p);
				//			fill_app_result_struct (&mcast_result, NULL, CM_GLU_CNTLR_XPRT_INVALID_PROTOCOL_ID);
				*presult_p = mcast_result;
				return OF_FAILURE;

		}
		break;
	}
CM_CBK_PRINT_IVPAIR_ARRAY (params);
//*presult_p = mcast_result;
return OF_SUCCESS;
}


int32_t mcast_ucm_validateoptparams (struct cm_array_of_iv_pairs *
		params,
		struct cm_app_result ** presult_p)
{
	uint32_t count;
//	struct cm_app_result * mcast_result;

	CM_CBK_DEBUG_PRINT ("Entered");
	for (count = 0; count < params->count_ui; count++)
	{
		switch (params->iv_pairs[count].id_ui)
		{
			default:
				break;
		}
	}
	CM_CBK_PRINT_IVPAIR_ARRAY (params);
	return OF_SUCCESS;
}

int32_t mcast_ucm_set_mand_params (struct cm_array_of_iv_pairs *
		params,uint32_t *ns_id_p,struct of_ip4_mcfwd_mfe_info *mcast_record_info,struct cm_app_result ** presult_p)
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
	CM_CBK_DEBUG_PRINT("Conversion from nsid to string success ns_name %s nsid %u",ns_name,nsid);
//	CM_CBK_DEBUG_PRINT("nnparams->iv_pairs[count].value_p ns_name %s nsid %u",ns_name,nsid);
						*ns_id_p = nsid;   
					}
				}
				break;
			case CM_DM_MCAST_MCASTGROUP_ID:
				{
					CM_CBK_DEBUG_PRINT("mcast group address %s",(char *) params->iv_pairs[count].value_p);
					cm_val_and_conv_aton((char *)params->iv_pairs[count].value_p,&mcast_record_info->mcastgrp);
					CM_CBK_DEBUG_PRINT("mcast group address 0x%x",mcast_record_info->mcastgrp);
				}
				break;
		}
	}
	CM_CBK_PRINT_IVPAIR_ARRAY (params);
	return OF_SUCCESS;
}

int32_t mcast_ucm_set_opt_params (struct cm_array_of_iv_pairs *
		params,
		struct of_ip4_mcfwd_mfe_info * mcast_record_info,
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

int32_t mcast_ucm_getparams (struct of_ip4_mcfwd_mfe_info *mcast_record_info,
		struct cm_array_of_iv_pairs * result_iv_pairs_p,uint32_t ns_id)
{
	uint32_t uindex_i = 0;
	char buf[MAX_BUF_SIZE] = "";
	uint32_t link_id,i;
	char iface_name[32];
	int32_t ret_val = OF_FAILURE;
#define CM_MCAST_CHILD_COUNT 4

	result_iv_pairs_p->iv_pairs =
		(struct cm_iv_pair *) of_calloc (CM_MCAST_CHILD_COUNT, sizeof (struct cm_iv_pair));
	if (result_iv_pairs_p->iv_pairs == NULL)
	{
		CM_CBK_DEBUG_PRINT
			("Memory allocation failed for result_iv_pairs_p->iv_pairs");
		return OF_FAILURE;
	}

	/*Muticast Group*/
	memset(buf, 0, MAX_BUF_SIZE);
	net_ntoa(mcast_record_info->mcastgrp,buf);
	FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i],CM_DM_MCAST_MCASTGROUP_ID,
			CM_DATA_TYPE_STRING, buf);
	uindex_i++;

	/* Src IP Addrees */
	memset(buf, 0, MAX_BUF_SIZE);
	net_ntoa(mcast_record_info->src_ip,buf);
	FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i],CM_DM_MCAST_SRCIPADDRESS_ID,
			CM_DATA_TYPE_STRING, buf);
	uindex_i++;

	/* In Iface */
	memset(buf, 0, MAX_BUF_SIZE);
	ret_val = ip4_mcfwd_get_iface_id_by_vif_id(ns_id, mcast_record_info->vif_id, &link_id);
	if(ret_val == OF_SUCCESS)
	{   ret_val = iface_db_get_ifname_from_ifid(ns_id, link_id, iface_name);
		strcpy(buf, iface_name);
		CM_CBK_DEBUG_PRINT("Invif %s",iface_name);
	}
		if ( buf[0] == '\0')
					strcpy(buf,"-");
	FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i],CM_DM_MCAST_VIFACE_ID,
			CM_DATA_TYPE_STRING, buf);
	uindex_i++;

	memset(buf, 0, MAX_BUF_SIZE);
	for(i=mcast_record_info->minvif; i<=mcast_record_info->maxvif; i++)
	{
		/*Out Ifaces*/
		ret_val = ip4_mcfwd_get_iface_id_by_vif_id(ns_id, i, &link_id);
		if(ret_val == OF_SUCCESS)
		{   
			ret_val = iface_db_get_ifname_from_ifid(ns_id, link_id, iface_name);
			if(i>mcast_record_info->minvif)
					strcat(buf,",");
			strcat(buf, iface_name);
			CM_CBK_DEBUG_PRINT("Outvif %s",iface_name);
		}
	}
		if ( buf[0] == '\0')
					strcpy(buf,"-");
		
		FILL_CM_IV_PAIR (result_iv_pairs_p->iv_pairs[uindex_i],CM_DM_MCAST_OUT_IFACES_ID,
				CM_DATA_TYPE_STRING,buf);
		uindex_i++;

	result_iv_pairs_p->count_ui = uindex_i;
	CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
	return OF_SUCCESS;
}

#endif /* OF_CM_SUPPORT */
