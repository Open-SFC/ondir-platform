/** Network Service resource manager  source file
 * * Copyright (c) 2012, 2013  Freescale.
 * *
 * * Licensed under the Apache License, Version 2.0 (the "License");
 * * you may not use this file except in compliance with the License.
 * * You may obtain a copy of the License at:
 * *
 * * http://www.apache.org/licenses/LICENSE-2.0
 * *
 * * Unless required by applicable law or agreed to in writing, software
 * * distributed under the License is distributed on an "AS IS" BASIS,
 * * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * * See the License for the specific language governing permissions and
 * * limitations under the License.
 * **/

/*
 *
 * File name:nsrm_zone.c
 * Author: P.Vinod Sarma <B46178@freescale.com>
 * Date:   25/01/2014
 * Description:
 */

#include  "nsrm_system_dependencies.h"
#include  "crm_tenant_api.h"
extern void     *nsrm_zone_mempool_g;
extern void     *nsrm_notifications_mempool_g;

extern uint32_t nsrm_no_of_zone_buckets_g;
extern  of_list_t nsrm_zone_notifications[];

#define NSRM_MAX_ZONE_RULE_ENTRIES 10

void nsrm_free_zone_rule_entry_rcu(struct rcu_head *zone_rule_entry_p)
{


}    
int32_t nsrm_add_zone_to_nschainset(struct  nsrm_zone_direction_rule_key*  key_info_p,
                                    int32_t number_of_config_params,
                                    struct  nsrm_zone_direction_rule_config_info*   config_info_p)

{
  struct   nsrm_nschainset_object* nschainset_object_node_scan_p = NULL;
  uint64_t tenant_handle,nschainset_object_handle;
  uchar8_t* hashobj_p;
  uint64_t  hashkey,offset,zone_record_handle;
  int32_t  ret_value = NSRM_SUCCESS;
  uint32_t index,magic,apphookoffset;
  uint8_t  heap_b=TRUE,status_b = FALSE;

  struct   nsrm_zone_record *zone_record_p,*zone_record_node_p;
  union  nsrm_zone_notification_data  notification_data = {};
  struct nsrm_notification_app_hook_info *app_entry_p = NULL;


  if(key_info_p->name_p == NULL)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR, "zone rule name is null");
    return NSRM_ERROR_NWSERVICE_OBJECT_NAME_INVALID;
  }
  
  if(key_info_p->nschainset_object_name_p == NULL)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR, "nschainset object name is null");
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  if(key_info_p->tenant_name_p == NULL)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR, "tenant name is null");
    return NSRM_FAILURE;
  }

  ret_value = crm_get_tenant_handle(key_info_p->tenant_name_p,&tenant_handle);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"tenant doesnt exist");
    return NSRM_FAILURE;
  }

  ret_value = nsrm_get_nschainset_object_handle(key_info_p->nschainset_object_name_p , &nschainset_object_handle);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object doesnt exist");
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  CNTLR_RCU_READ_LOCK_TAKE();
  
  ret_value = nsrm_get_nschainset_byhandle(nschainset_object_handle , &nschainset_object_node_scan_p);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"failed to get nschainset object");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  if(nschainset_object_node_scan_p->nszone_direction_record_set == NULL)
  {
    nschainset_object_node_scan_p->nszone_direction_record_set = (struct mchash_table*)calloc(1,(NSRM_MAX_ZONE_RULE_ENTRIES)*sizeof(struct nsrm_zone_record));
    ret_value = mchash_table_create(1 ,  
                                    NSRM_MAX_ZONE_RULE_ENTRIES,
                                    nsrm_free_zone_rule_entry_rcu,
                                    &(nschainset_object_node_scan_p->nszone_direction_record_set));

    if(ret_value != NSRM_SUCCESS)
    {
      OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG,"zone table not created");
      CNTLR_RCU_READ_LOCK_RELEASE();
      return NSRM_FAILURE;
    }
  }

  if(nschainset_object_node_scan_p->nszone_direction_record_set == NULL)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG,"Mem not allocated for zone table");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_FAILURE;
  }
 
  hashkey = 0;
  offset = NSRM_ZONE_RECORD_TBL_OFFSET;
  MCHASH_TABLE_SCAN(nschainset_object_node_scan_p->nszone_direction_record_set,hashkey)
  {
    MCHASH_BUCKET_SCAN(nschainset_object_node_scan_p->nszone_direction_record_set,hashkey,
                       zone_record_node_p,
              struct nsrm_zone_record*, offset)
      {
          if((strcmp(zone_record_node_p->name_p,key_info_p->name_p)) == 0)
          {
              OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG,"This zone record is already present in this nschainset");
              CNTLR_RCU_READ_LOCK_RELEASE();
              return NSRM_FAILURE;
          }
          continue;
      }
      break;
  }

  ret_value = mempool_get_mem_block(nsrm_zone_mempool_g,(uchar8_t **)&zone_record_p,&heap_b);
  if(ret_value != MEMPOOL_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR, "Failed to get mempool for zone record");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_FAILURE;
  }

  zone_record_p->name_p = (char *)calloc(1,128);
  strcpy(zone_record_p->name_p,key_info_p->name_p);
  zone_record_p->nschainset_object_saferef = nschainset_object_handle;

  //strncpy(zone_record_p->zone,config_info_p->value.zone,CRM_MAX_ZONE_SIZE);
  strncpy(zone_record_p->zone,key_info_p->name_p,CRM_MAX_ZONE_SIZE);
  zone_record_p->zone_direction_e = config_info_p[0].value.zone_direction_e;
  OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR, "zone direction : %d", config_info_p[0].value.zone_direction_e);

  zone_record_p->heap_b = heap_b;  
 
  hashobj_p = (uchar8_t *)zone_record_p + NSRM_ZONE_RECORD_TBL_OFFSET;
  hashkey   = 0;
  offset    = NSRM_ZONE_RECORD_TBL_OFFSET;

  MCHASH_APPEND_NODE(nschainset_object_node_scan_p->nszone_direction_record_set,hashkey, zone_record_p,index, magic, hashobj_p, status_b);
  if(status_b == FALSE)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR," Failed to add zone record");
    free(zone_record_p->name_p);
    CNTLR_RCU_READ_LOCK_RELEASE();
    if(mempool_release_mem_block(nsrm_zone_mempool_g,
                                (uchar8_t*)&zone_record_p,
                                 zone_record_p->heap_b) != MEMPOOL_SUCCESS)
    {
      OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR, "Failed to release  allocated memory block for zone record.");
    }  
    return NSRM_SELECTION_RULE_ADD_FAIL;
  }

  zone_record_handle = magic;
  zone_record_handle = (((zone_record_handle)<<32) | (index));
  zone_record_p->magic = magic;
  zone_record_p->index = index;
  zone_record_p->zone_record_handle = zone_record_handle;
  nschainset_object_node_scan_p->no_of_zone_records++;

  CNTLR_RCU_READ_LOCK_RELEASE();
  apphookoffset = NSRM_NOTIFICATION_APP_HOOK_OFFSET;
  notification_data.add_del.zone_name_p = (char*)calloc(1,(strlen(zone_record_p->zone)+1));
  strcpy(notification_data.add_del.zone_name_p ,zone_record_p->zone);

  OF_LIST_SCAN(nsrm_zone_notifications[NSRM_ZONE_ADDED_TO_NSCHAINSET_OBJECT],
          app_entry_p,
          struct nsrm_notification_app_hook_info *,
          apphookoffset)
  {
      ((nsrm_zone_notifier)(app_entry_p->call_back))(NSRM_ZONE_ADDED_TO_NSCHAINSET_OBJECT,
      zone_record_handle,
      notification_data,
      app_entry_p->cbk_arg1,
      app_entry_p->cbk_arg2
      );
  }

  OF_LIST_SCAN(nsrm_zone_notifications[NSRM_ZONE_ALL] ,
          app_entry_p,
          struct nsrm_notification_app_hook_info *,
          apphookoffset)
  {
      ((nsrm_zone_notifier)(app_entry_p->call_back))(NSRM_ZONE_ADDED_TO_NSCHAINSET_OBJECT,
      zone_record_handle,
      notification_data,
      app_entry_p->cbk_arg1,
      app_entry_p->cbk_arg2
      );
  }


  return NSRM_SUCCESS;
}

int32_t nsrm_del_zone_from_nschainset(struct  nsrm_zone_direction_rule_key*    key_info_p)
{
  struct   nsrm_nschainset_object* nschainset_object_scan_node_p = NULL;
  uint64_t nschainset_object_handle;
  uint64_t hashkey,offset;
  int32_t  ret_value = NSRM_SUCCESS,apphookoffset;
  uint8_t  zone_node_found_b = FALSE,status_b = FALSE;
  struct   nsrm_zone_record* zone_record_node_p;

  union  nsrm_zone_notification_data  notification_data = {};
  struct nsrm_notification_app_hook_info *app_entry_p = NULL;

  if(key_info_p->name_p == NULL)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"zone record name is null");
    return NSRM_ERROR_INVALID_ZONE_RECORD_NAME;
  }

  ret_value = nsrm_get_nschainset_object_handle(key_info_p->nschainset_object_name_p , &nschainset_object_handle);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object not valid");
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  CNTLR_RCU_READ_LOCK_TAKE();

  ret_value = nsrm_get_nschainset_byhandle(nschainset_object_handle , &nschainset_object_scan_node_p);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object not valid");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }
  hashkey = 0;
  offset = NSRM_ZONE_RECORD_TBL_OFFSET;
  MCHASH_TABLE_SCAN(nschainset_object_scan_node_p->nszone_direction_record_set,hashkey)
  {
    MCHASH_BUCKET_SCAN(nschainset_object_scan_node_p->nszone_direction_record_set,hashkey,
                       zone_record_node_p,struct nsrm_zone_record*, offset)
    {
      if(strcmp(zone_record_node_p->name_p , key_info_p->name_p) == 0)
      {
        OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG,"zone node found to delete");
        zone_node_found_b = TRUE;
        break;
      }
    }
    break;
  }
  if(zone_node_found_b == FALSE)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"zone node not found");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_ERROR_INVALID_ZONE_RECORD_NAME;
  }
  apphookoffset = NSRM_NOTIFICATION_APP_HOOK_OFFSET;
  notification_data.add_del.zone_name_p = (char*)calloc(1,(strlen(zone_record_node_p->zone)+1));
  strcpy(notification_data.add_del.zone_name_p ,zone_record_node_p->zone);

  OF_LIST_SCAN(nsrm_zone_notifications[NSRM_ZONE_DELETED_FROM_NSCHAINSET_OBJECT],
          app_entry_p,
          struct nsrm_notification_app_hook_info *,
          apphookoffset)
  {
      ((nsrm_zone_notifier)(app_entry_p->call_back))(NSRM_ZONE_DELETED_FROM_NSCHAINSET_OBJECT,
      zone_record_node_p->zone_record_handle,
      notification_data,
      app_entry_p->cbk_arg1,
      app_entry_p->cbk_arg2
      );
  }

  OF_LIST_SCAN(nsrm_zone_notifications[NSRM_ZONE_ALL] ,
          app_entry_p,
          struct nsrm_notification_app_hook_info *,
          apphookoffset)
  {
      ((nsrm_zone_notifier)(app_entry_p->call_back))(NSRM_ZONE_DELETED_FROM_NSCHAINSET_OBJECT,
      zone_record_node_p->zone_record_handle,
      notification_data,
      app_entry_p->cbk_arg1,
      app_entry_p->cbk_arg2
      );
  }
  hashkey =0;
  offset = NSRM_ZONE_RECORD_TBL_OFFSET;
  status_b = FALSE;
  MCHASH_DELETE_NODE_BY_REF(nschainset_object_scan_node_p->nszone_direction_record_set,
                            zone_record_node_p->index, zone_record_node_p->magic,
                            struct nsrm_zone_record *,offset, status_b);
 
  CNTLR_RCU_READ_LOCK_RELEASE();
  if(status_b == TRUE)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG, "zone record deleted successfully");
    nschainset_object_scan_node_p->no_of_zone_records--; 
    return NSRM_SUCCESS;
  }
  else
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR, "failed to delete zone record");
    return NSRM_ZONE_RECORD_DEL_FAIL;
  }

  return NSRM_SUCCESS;
}

int32_t nsrm_modify_zone(
                        struct  nsrm_zone_direction_rule_key*           key_info_p,
                        int32_t number_of_config_params,
                        struct  nsrm_zone_direction_rule_config_info*   config_info_p)

{
  struct   nsrm_nschainset_object* nschainset_object_scan_node_p = NULL;
  uint64_t nschainset_object_handle;
  uint64_t hashkey,offset;
  int32_t  ret_value = NSRM_SUCCESS,apphookoffset;
  uint8_t  zone_node_found_b = FALSE;
  struct   nsrm_zone_record* zone_record_node_p;

  union  nsrm_zone_notification_data  notification_data = {};
  struct nsrm_notification_app_hook_info *app_entry_p = NULL;

  if(key_info_p->name_p == NULL)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"zone record name is null");
    return NSRM_ERROR_INVALID_ZONE_RECORD_NAME;
  }

  ret_value = nsrm_get_nschainset_object_handle(key_info_p->nschainset_object_name_p , &nschainset_object_handle);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object not valid");
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  CNTLR_RCU_READ_LOCK_TAKE();

  ret_value = nsrm_get_nschainset_byhandle(nschainset_object_handle , &nschainset_object_scan_node_p);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object not valid");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  hashkey = 0;
  offset = NSRM_ZONE_RECORD_TBL_OFFSET;
  MCHASH_TABLE_SCAN(nschainset_object_scan_node_p->nszone_direction_record_set,hashkey)
  {
    MCHASH_BUCKET_SCAN(nschainset_object_scan_node_p->nszone_direction_record_set,hashkey,
                       zone_record_node_p,struct nsrm_zone_record*, offset)
    {
      if(strcmp(zone_record_node_p->name_p , key_info_p->name_p) == 0)
      {
        OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG,"zone node found to delete");
        zone_node_found_b = TRUE;
        break;
      }
    }
    break;
  }
  if(zone_node_found_b == FALSE)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"zone node not found");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_ERROR_INVALID_ZONE_RECORD_NAME;
  }

  /* Check zone with received zone. If matching,update zone_direction with received zone_direction. */
  if(!strncmp(zone_record_node_p->zone,key_info_p->name_p,CRM_MAX_ZONE_SIZE))
  {
    zone_record_node_p->zone_direction_e = config_info_p->value.zone_direction_e;
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG, "zone record modified successfully");
    CNTLR_RCU_READ_LOCK_RELEASE();
  }
  else
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG, "Failed to modify zone record");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_FAILURE;
  }
  /* Notify to reset the zone direction for the zone so that a relookup is done */
  apphookoffset = NSRM_NOTIFICATION_APP_HOOK_OFFSET;
  notification_data.add_del.zone_name_p = (char*)calloc(1,(strlen(zone_record_node_p->zone)+1));
  strcpy(notification_data.add_del.zone_name_p ,zone_record_node_p->zone);

  OF_LIST_SCAN(nsrm_zone_notifications[NSRM_ZONE_MODIFIED_IN_NSCHAINSET_OBJECT],
          app_entry_p,
          struct nsrm_notification_app_hook_info *,
          apphookoffset)
  {
      ((nsrm_zone_notifier)(app_entry_p->call_back))(NSRM_ZONE_MODIFIED_IN_NSCHAINSET_OBJECT,
      zone_record_node_p->zone_record_handle,
      notification_data,
      app_entry_p->cbk_arg1,
      app_entry_p->cbk_arg2
      );
  }

  OF_LIST_SCAN(nsrm_zone_notifications[NSRM_ZONE_ALL] ,
          app_entry_p,
          struct nsrm_notification_app_hook_info *,
          apphookoffset)
  {
      ((nsrm_zone_notifier)(app_entry_p->call_back))(NSRM_ZONE_MODIFIED_IN_NSCHAINSET_OBJECT,
      zone_record_node_p->zone_record_handle,
      notification_data,
      app_entry_p->cbk_arg1,
      app_entry_p->cbk_arg2
      );
  }

  return NSRM_SUCCESS;
}

int32_t nsrm_get_first_zone_from_nschainset_object (
                                                   struct   nsrm_nschainset_object_key*     nschainset_object_key_p,
                                                   int32_t  number_of_zones_requested,
                                                   int32_t* number_of_zones_returned_p,
                                                   struct   nsrm_zone_rule*  recs_p)
{
  struct   nsrm_zone_record* zone_record_node_p;  
  struct   nsrm_nschainset_object*  nschainset_object_scan_node_p = NULL;
  uint64_t hashkey,nschainset_object_handle;
  int32_t  offset, zone_record_returned = 0;
  int32_t  zone_record_requested = number_of_zones_requested;
  int32_t  status = NSRM_FAILURE,ret_value = NSRM_FAILURE;

  if(nschainset_object_key_p->name_p == NULL)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"NSchainset name is NULL");
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  ret_value = nsrm_get_nschainset_object_handle(nschainset_object_key_p->name_p , &nschainset_object_handle);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object not valid");
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  CNTLR_RCU_READ_LOCK_TAKE();

  ret_value = nsrm_get_nschainset_byhandle(nschainset_object_handle , &nschainset_object_scan_node_p);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object not valid");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  if(nschainset_object_scan_node_p->no_of_zone_records == 0)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG,"No zone records attached to this chainset");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_FAILURE;
  }

  hashkey = 0;
  offset = NSRM_ZONE_RECORD_TBL_OFFSET;
  MCHASH_TABLE_SCAN(nschainset_object_scan_node_p->nszone_direction_record_set,hashkey)
  {
    MCHASH_BUCKET_SCAN(nschainset_object_scan_node_p->nszone_direction_record_set,hashkey,
                       zone_record_node_p,struct nsrm_zone_record*, offset)
    {
      strcpy(recs_p[zone_record_returned].key.name_p , zone_record_node_p->name_p);
      //strncpy(recs_p[zone_record_returned].info[0].value.zone,zone_record_node_p->zone,CRM_MAX_ZONE_SIZE);
      OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"zone direction : %d",zone_record_node_p->zone_direction_e);
      recs_p[zone_record_returned].info[0].value.zone_direction_e = zone_record_node_p->zone_direction_e;

      status = NSRM_SUCCESS; 
      number_of_zones_requested--;
      zone_record_returned++;
      if(number_of_zones_requested)
        continue;
      else
        break;
    }
    if(number_of_zones_requested)
      continue;
    else
      break;
  } 
  if(zone_record_requested != zone_record_returned)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"Invalid number of zone returned / requested ");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_FAILURE;
  }
  if(status == NSRM_SUCCESS)
  {
    *number_of_zones_returned_p = zone_record_returned;
    CNTLR_RCU_READ_LOCK_RELEASE();
    return status;
  }
  CNTLR_RCU_READ_LOCK_RELEASE();
  OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG, "No More Entries in the zone hash table!.");
  return NSRM_ERROR_NO_MORE_ENTRIES;
}

int32_t nsrm_get_next_zone_from_nschainset_object (
                                                   struct   nsrm_nschainset_object_key*    nschainset_object_key_p,
                                                   struct   nsrm_zone_direction_rule_key*  relative_record_p,
                                                   int32_t  number_of_zones_requested,
                                                   int32_t* number_of_zones_returned_p,
                                                   struct   nsrm_zone_rule* recs_p)
{
  struct   nsrm_zone_record* zone_record_node_p;
  struct   nsrm_nschainset_object*  nschainset_object_scan_node_p = NULL;
  uint64_t hashkey,nschainset_object_handle;
  int32_t  offset, zone_record_returned = 0;
  int32_t  zone_record_requested = number_of_zones_requested;
  int32_t  status = NSRM_FAILURE,ret_value = NSRM_FAILURE;
  uint8_t   relative_node_found_b = FALSE;

  if(nschainset_object_key_p->name_p == NULL)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"NSchainset name is NULL");
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  ret_value = nsrm_get_nschainset_object_handle(nschainset_object_key_p->name_p , &nschainset_object_handle);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object not valid");
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  CNTLR_RCU_READ_LOCK_TAKE();

  ret_value = nsrm_get_nschainset_byhandle(nschainset_object_handle , &nschainset_object_scan_node_p);
  if(ret_value != NSRM_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object not valid");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  if(nschainset_object_scan_node_p->no_of_zone_records == 0)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG,"No zone records attached to this chainset");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_FAILURE;
  }

  hashkey = 0;
  offset = NSRM_ZONE_RECORD_TBL_OFFSET;
  MCHASH_TABLE_SCAN(nschainset_object_scan_node_p->nszone_direction_record_set,hashkey)
  {
    MCHASH_BUCKET_SCAN(nschainset_object_scan_node_p->nszone_direction_record_set,hashkey,
                       zone_record_node_p,struct nsrm_zone_record*, offset)
    {
      if(relative_node_found_b == FALSE)
      {
        if(strcmp(relative_record_p->name_p , zone_record_node_p->name_p) != 0)
        {
          continue;
        }
        else
        {
          relative_node_found_b = TRUE;
          continue;
        }
      }
      if(relative_node_found_b != TRUE)
      {
        CNTLR_RCU_READ_LOCK_RELEASE();
        return NSRM_FAILURE;
      }
    
      strcpy(recs_p[zone_record_returned].key.name_p , zone_record_node_p->name_p);
      //strncpy(recs_p[zone_record_returned].info[0].value.zone,zone_record_node_p->zone,CRM_MAX_ZONE_SIZE);
      recs_p[zone_record_returned].info[0].value.zone_direction_e = zone_record_node_p->zone_direction_e;

      status = NSRM_SUCCESS;
      number_of_zones_requested--;
      zone_record_returned++;
      if(number_of_zones_requested)
        continue;
      else
        break;
    }                                             
    if(number_of_zones_requested)
      continue;
    else
      break;
  }
  if(zone_record_requested != zone_record_returned)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"Invalid number of zone returned / requested ");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_FAILURE;
  }
  if(status == NSRM_SUCCESS)
  {
    *number_of_zones_returned_p = zone_record_returned;
    CNTLR_RCU_READ_LOCK_RELEASE();
    return status;
  }
  CNTLR_RCU_READ_LOCK_RELEASE();
  OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG, "No More Entries in the zone hash table!.");
  return NSRM_ERROR_NO_MORE_ENTRIES;
}

int32_t nsrm_get_exact_zone_from_nschainset_object(
                                                   struct   nsrm_nschainset_object_key*     nschainset_object_key_p,
                                                   struct   nsrm_zone_direction_rule_key    *key_info_p,
                                                   struct   nsrm_zone_rule  *recs_p)

{
  struct   nsrm_nschainset_object*  nschainset_object_scan_node_p = NULL;
  struct   nsrm_zone_record* zone_record_node_p;
  uint64_t hashkey , nschainset_object_handle;
  int32_t  offset;
  int32_t  ret_value = NSRM_FAILURE;
  uint8_t  current_node_found_b = FALSE;

  if(nschainset_object_key_p->name_p == NULL)
  {
      OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"NSchainset name is NULL");
      return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  ret_value = nsrm_get_nschainset_object_handle(nschainset_object_key_p->name_p , &nschainset_object_handle);
  if(ret_value != NSRM_SUCCESS)
  {
      OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object not valid");
      return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  CNTLR_RCU_READ_LOCK_TAKE();

  ret_value = nsrm_get_nschainset_byhandle(nschainset_object_handle , &nschainset_object_scan_node_p);
  if(ret_value != NSRM_SUCCESS)
  {
      OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR,"nschainset object not valid");
      CNTLR_RCU_READ_LOCK_RELEASE();
      return NSRM_ERROR_NSCHAINSET_OBJECT_NAME_INVALID;
  }

  if(nschainset_object_scan_node_p->no_of_zone_records == 0)
  {
      OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG,"No zone records attached to this chainset");
      CNTLR_RCU_READ_LOCK_RELEASE();
      return NSRM_FAILURE;
  }

  hashkey = 0;
  offset = NSRM_ZONE_RECORD_TBL_OFFSET;
  MCHASH_TABLE_SCAN(nschainset_object_scan_node_p->nszone_direction_record_set,hashkey)
  {
    MCHASH_BUCKET_SCAN(nschainset_object_scan_node_p->nszone_direction_record_set,hashkey,
                       zone_record_node_p,struct nsrm_zone_record*, offset)
    {
      if(strcmp(key_info_p->name_p , zone_record_node_p->name_p) != 0)
        continue;
      else
      {
        current_node_found_b = TRUE;
        break;
      }
    }
    break;
  }

  if(current_node_found_b == FALSE)
  {
    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG,"no zone record with such name exists");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return NSRM_ERROR_INVALID_ZONE_RECORD_NAME;
  }

  strcpy(recs_p[0].key.name_p , zone_record_node_p->name_p);
  //strncpy(recs_p[0].info[0].value.zone,zone_record_node_p->zone,CRM_MAX_ZONE_SIZE);
  recs_p[0].info[0].value.zone_direction_e = zone_record_node_p->zone_direction_e;

  CNTLR_RCU_READ_LOCK_RELEASE();
  OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG, "Exact entry found and returned!.");
  return NSRM_SUCCESS;
}

int32_t nsrm_register_zone_notifications(
        uint8_t notification_type,
        nsrm_zone_notifier  zone_notifier,
        void* callback_arg1,
        void* callback_arg2)
{
    struct  nsrm_notification_app_hook_info *app_entry_p=NULL;
    uint8_t heap_b;
    int32_t retval = NSRM_SUCCESS;
    uchar8_t lstoffset;
    lstoffset = NSRM_NOTIFICATION_APP_HOOK_OFFSET;

    if((notification_type < NSRM_ZONE_FIRST_NOTIFICATION_TYPE) ||
            (notification_type > NSRM_ZONE_LAST_NOTIFICATION_TYPE))
    {
        OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR, "Invalid Notification type:%d", notification_type);
        return NSRM_FAILURE;
    }
    if(zone_notifier == NULL)
    {
        OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR, "callback function is null");
        return NSRM_FAILURE;
    }

    retval = mempool_get_mem_block(nsrm_notifications_mempool_g, (uchar8_t**)&app_entry_p, &heap_b);
    if(retval != MEMPOOL_SUCCESS)
    {
        OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_ERROR, "Failed to get mempool");
        return NSRM_FAILURE;
    }

    app_entry_p->call_back = (void*)zone_notifier;
    app_entry_p->cbk_arg1  = callback_arg1;
    app_entry_p->cbk_arg2  = callback_arg2;
    app_entry_p->heap_b    = heap_b;

    /* Add Application to the list of nschainset object notifications */
    OF_APPEND_NODE_TO_LIST(nsrm_zone_notifications[notification_type],app_entry_p, lstoffset);
    if(retval != NSRM_SUCCESS)
        mempool_release_mem_block(nsrm_notifications_mempool_g,(uchar8_t*)app_entry_p, app_entry_p->heap_b);

    OF_LOG_MSG(OF_LOG_NSRM, OF_LOG_DEBUG, "callback function reg with ZONE success");
    return retval;
}








