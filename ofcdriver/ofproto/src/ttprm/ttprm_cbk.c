/**
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

#include "controller.h"
#include "cntlr_transprt.h"
#include "cntlr_event.h"
#include "cntlr_tls.h"
#include "dprmldef.h"
#include "dprm.h"
#include "ttprmgdef.h"
#include "ttprmldef.h"

extern struct   mchash_table *ttprm_table_g;
extern int32_t get_ttprm_node_byname(char *main_ttp_name_p, 
                      struct ttprm_entry **ttprm_entry_p_p);

int32_t  get_first_ttp(struct ttprm_config_info  *ttp_config_info_p)
{
  uint32_t hashkey;
  struct ttprm_entry *ttprm_info_p=NULL;
  uint16_t offset=TTPRM_NODE_OFFSET;

  CNTLR_RCU_READ_LOCK_TAKE();

  if(ttprm_table_g == NULL)
  {
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  } 
  
  MCHASH_TABLE_SCAN(ttprm_table_g, hashkey)
  {
    MCHASH_BUCKET_SCAN(ttprm_table_g, hashkey, ttprm_info_p, \
                       struct ttprm_entry*,offset)
    {
      strcpy(ttp_config_info_p->name, ttprm_info_p->main_ttp_name);
      ttp_config_info_p->no_of_tbls = ttprm_info_p->number_of_tables;
      ttp_config_info_p->no_of_domains =  ttprm_info_p->number_of_domains;
      CNTLR_RCU_READ_LOCK_RELEASE();
      return CNTLR_TTP_SUCCESS;
    }
  } 
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_FAILURE;
}

int32_t  get_next_ttp(char *ttp_name_p, 
                      struct ttprm_config_info *ttp_config_info_p)
{
  uint32_t hashkey;
  struct ttprm_entry *ttprm_info_p=NULL;
  uint16_t offset=TTPRM_NODE_OFFSET;
  uint8_t  current_entry_found_b=FALSE;

  CNTLR_RCU_READ_LOCK_TAKE();

  if(ttprm_table_g == NULL)
  {
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  } 
  
  MCHASH_TABLE_SCAN(ttprm_table_g, hashkey)
  {
    MCHASH_BUCKET_SCAN(ttprm_table_g, hashkey, ttprm_info_p, \
                       struct ttprm_entry*,offset)
    {
      if(current_entry_found_b == FALSE)
      {
        if(strcmp(ttp_name_p, ttprm_info_p->main_ttp_name))
          continue;
        else
        {
          current_entry_found_b = TRUE;
          continue;
        }
      }
      strcpy(ttp_config_info_p->name, ttprm_info_p->main_ttp_name);
      ttp_config_info_p->no_of_tbls = ttprm_info_p->number_of_tables;
      ttp_config_info_p->no_of_domains =  ttprm_info_p->number_of_domains;
      CNTLR_RCU_READ_LOCK_RELEASE();
      return CNTLR_TTP_SUCCESS;
    }
  }   
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_FAILURE;
}

int32_t  get_exact_ttp(char *ttp_name_p, 
                       struct  ttprm_config_info  *ttp_config_info_p)
{
  uint32_t hashkey;
  struct ttprm_entry *ttprm_info_p=NULL;
  uint16_t offset=TTPRM_NODE_OFFSET;

  CNTLR_RCU_READ_LOCK_TAKE();

  if(ttprm_table_g == NULL)
  {
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  } 
  
  MCHASH_TABLE_SCAN(ttprm_table_g, hashkey)
  {
    MCHASH_BUCKET_SCAN(ttprm_table_g, hashkey, ttprm_info_p, \
                       struct ttprm_entry*,offset)
    {
      if(!strcmp(ttp_name_p, ttprm_info_p->main_ttp_name))
      {    
        strcpy(ttp_config_info_p->name, ttprm_info_p->main_ttp_name);
        ttp_config_info_p->no_of_tbls = ttprm_info_p->number_of_tables;
        ttp_config_info_p->no_of_domains =  ttprm_info_p->number_of_domains;
        CNTLR_RCU_READ_LOCK_RELEASE();
        return CNTLR_TTP_SUCCESS;
      }
    }
  }   
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_FAILURE;
}


int32_t get_first_ttp_tbl(char *ttp_name_p, 
     struct ttprm_tbl_info  *ttp_tbl_info_p)
{
  int32_t ret_val;
  struct ttprm_entry *ttprm_entry_p = NULL;
  struct  ttprm_table_entry *ttp_table_node_p=NULL;
  uint8_t  entry_found_b = 0;

  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "inavild main-ttp name:%s.",ttp_name_p);
    CNTLR_RCU_READ_LOCK_RELEASE();
   return CNTLR_TTP_FAILURE;
  }


  OF_LIST_SCAN(ttprm_entry_p->table_list, 
               ttp_table_node_p, 
               struct  ttprm_table_entry *, 
               TTPRM_TBL_LST_NODE_OFFSET)
  {
    entry_found_b = 1;
    break;
  }
  if(entry_found_b == 0 )
  {
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }

  if ((strlen(ttp_table_node_p->table_name) == 0))
  {

    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }

  ttp_tbl_info_p->tbl_id = ttp_table_node_p->table_id;
  strcpy(ttp_tbl_info_p->tbl_name, ttp_table_node_p->table_name);
  ttp_tbl_info_p->max_rows    = ttp_table_node_p->max_rows;
  ttp_tbl_info_p->max_columns = ttp_table_node_p->max_columns;
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "tbl_id:%d,tbl_name:%s,rows:%d,columns:%d",
                                        ttp_tbl_info_p->tbl_id ,
                                        ttp_tbl_info_p->tbl_name, 
                                        ttp_tbl_info_p->max_rows,
                                        ttp_tbl_info_p->max_columns);
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_SUCCESS;
}

int32_t get_next_ttp_tbl(char *ttp_name_p, 
                         char *tbl_name_p,  
                         struct ttprm_tbl_info  *ttp_tbl_info_p)
{
  int32_t ret_val;
  uint8_t  current_entry_found_b=FALSE;

  struct ttprm_entry *ttprm_entry_p = NULL;
  struct  ttprm_table_entry *ttp_table_node_p=NULL;

  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, \
               "inavild main-ttp name:%s.",ttp_name_p);
   CNTLR_RCU_READ_LOCK_RELEASE();
   return CNTLR_TTP_FAILURE;
  }


  OF_LIST_SCAN(ttprm_entry_p->table_list, 
               ttp_table_node_p, 
               struct  ttprm_table_entry *, 
               TTPRM_TBL_LST_NODE_OFFSET)
  {
    if(current_entry_found_b == FALSE)
    {
      if(strcmp(tbl_name_p, ttp_table_node_p->table_name))
        continue;
      else
      {
        current_entry_found_b = TRUE;
        continue;
      }
    }
    else
    {
      ttp_tbl_info_p->tbl_id = ttp_table_node_p->table_id;
      strcpy(ttp_tbl_info_p->tbl_name, ttp_table_node_p->table_name);
      ttp_tbl_info_p->max_rows    = ttp_table_node_p->max_rows;
      ttp_tbl_info_p->max_columns = ttp_table_node_p->max_columns;
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, \
                 "tbl_id:%d,tbl_name:%s,rows:%d,columns:%d",\
                  ttp_tbl_info_p->tbl_id ,ttp_tbl_info_p->tbl_name, \
                  ttp_tbl_info_p->max_rows,ttp_tbl_info_p->max_columns);

      CNTLR_RCU_READ_LOCK_RELEASE();
      return CNTLR_TTP_SUCCESS;


    }
  }
  if(current_entry_found_b == FALSE)
  {

    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_FAILURE;
}


int32_t get_exact_ttp_tbl(char *ttp_name_p, 
                          char *tbl_name_p,  
                          struct ttprm_tbl_info  *ttp_tbl_info_p)
{
  int32_t ret_val;
  struct ttprm_entry *ttprm_entry_p = NULL;
  struct  ttprm_table_entry *ttp_table_node_p=NULL;

  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "inavild main-ttp name:%s.",\
                                          ttp_name_p);
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }


  OF_LIST_SCAN(ttprm_entry_p->table_list, 
               ttp_table_node_p, 
               struct  ttprm_table_entry *, 
               TTPRM_TBL_LST_NODE_OFFSET)
  {
    if(!strcmp(ttp_table_node_p->table_name,tbl_name_p))
    {
      ttp_tbl_info_p->tbl_id = ttp_table_node_p->table_id;
      strcpy(ttp_tbl_info_p->tbl_name, ttp_table_node_p->table_name);
      ttp_tbl_info_p->max_rows    = ttp_table_node_p->max_rows;
      ttp_tbl_info_p->max_columns = ttp_table_node_p->max_columns;
      CNTLR_RCU_READ_LOCK_RELEASE();
      return CNTLR_TTP_SUCCESS;
    }
  }
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_FAILURE;
}

int32_t get_first_ttp_tbl_match_field(char *ttp_name_p,  
                                      char *tbl_name_p, 
           struct match_field_info  *match_field_info_p)
{
  int32_t ret_val;
  struct ttprm_entry *ttprm_entry_p = NULL;
  struct  ttprm_table_entry *ttp_table_node_p=NULL;
  struct  table_match_field_entry *table_match_field_node_p=NULL;
  uint8_t  entry_found_b = 0;

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered.ttp_name:%s,table_name_p=%s",\
              ttp_name_p,tbl_name_p);
  ret_val = get_ttprm_node_byname(ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "inavild main-ttp name:%s.",\
                                          ttp_name_p);
    return CNTLR_TTP_FAILURE;
  }
  OF_LIST_SCAN(ttprm_entry_p->table_list, 
               ttp_table_node_p, 
               struct  ttprm_table_entry *, 
               TTPRM_TBL_LST_NODE_OFFSET)
 {
   if(!strcmp(ttp_table_node_p->table_name,tbl_name_p))
   {
     entry_found_b = 1;
     break;
   }
 }

  if(entry_found_b == 0)
    return CNTLR_TTP_FAILURE;

  entry_found_b=0;
  OF_LIST_SCAN(ttp_table_node_p->match_fields_list, 
               table_match_field_node_p, 
               struct  table_match_field_entry *, 
               TTPRM_TBL_MATCH_FIELD_LST_NODE_OFFSET)
  {
    entry_found_b = 1;
    break;
  }
  if(entry_found_b == 0)
    return CNTLR_TTP_FAILURE;

  match_field_info_p->match_field_id = \
                             OXM_FIELD(table_match_field_node_p->match_field);
  match_field_info_p->is_wild_card = \
                            table_match_field_node_p->is_wild_card;
  match_field_info_p->is_lpm = \
                            table_match_field_node_p->is_lpm;
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "first match_field_id=%d",\
                            match_field_info_p->match_field_id);
  return CNTLR_TTP_SUCCESS;
}

int32_t get_next_ttp_tbl_match_field(char *ttp_name_p, 
                                     char *tbl_name_p,  
                                     uint32_t match_field_id, 
                  struct match_field_info *match_field_info_p)
{
  int32_t ret_val;
  struct  ttprm_entry *ttprm_entry_p = NULL;
  struct  ttprm_table_entry *ttp_table_node_p=NULL;
  struct  table_match_field_entry *table_match_field_node_p=NULL;
  uint8_t entry_found_b = 0;

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, \
             "ttp_name:%s,table_name_p=%s,match_field_id",\
              ttp_name_p,tbl_name_p, match_field_id);

  ret_val = get_ttprm_node_byname(ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "inavild main-ttp name:%s.",ttp_name_p);
    return CNTLR_TTP_FAILURE;
  }
  OF_LIST_SCAN(ttprm_entry_p->table_list, 
               ttp_table_node_p, 
               struct  ttprm_table_entry *, 
               TTPRM_TBL_LST_NODE_OFFSET)
 {
   if(!strcmp(ttp_table_node_p->table_name,tbl_name_p))
   {
     entry_found_b = 1;
     break;
   }
 }

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "table entry found");

  if(entry_found_b == 0)
    return CNTLR_TTP_FAILURE;

  entry_found_b=0;
  OF_LIST_SCAN(ttp_table_node_p->match_fields_list, 
               table_match_field_node_p, 
               struct  table_match_field_entry *, 
               TTPRM_TBL_MATCH_FIELD_LST_NODE_OFFSET)
  {
    if(entry_found_b ==0)
    {
      if(OXM_FIELD(table_match_field_node_p->match_field) == match_field_id)
      {
        OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, " match field found");
	entry_found_b = 1;
	continue;
      }
      else
        continue;
    }
    else
    {
      match_field_info_p->match_field_id = \
                      OXM_FIELD(table_match_field_node_p->match_field);
      match_field_info_p->is_wild_card = table_match_field_node_p->is_wild_card;
      match_field_info_p->is_lpm = table_match_field_node_p->is_lpm;
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                 "Matchfield found match_field_id=%d",
                  match_field_info_p->match_field_id);
      return CNTLR_TTP_SUCCESS;
    }
  }
  return CNTLR_TTP_FAILURE;
}

int32_t get_first_ttp_domain(char *ttp_name_p,  char *domain_name_p)
{
  int32_t ret_val;
  struct ttprm_entry *ttprm_entry_p = NULL;
  struct ttprm_domain_entry *ttprm_domain_node_p=NULL;
  uint8_t  entry_found_b = 0;

  CNTLR_RCU_READ_LOCK_TAKE();

  ret_val = get_ttprm_node_byname(ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "inavild main-ttp name:%s.",ttp_name_p);
  CNTLR_RCU_READ_LOCK_RELEASE();
   return CNTLR_TTP_FAILURE;
  }


  OF_LIST_SCAN(ttprm_entry_p->domain_list, 
               ttprm_domain_node_p, 
               struct  ttprm_domain_entry *, 
               TTPRM_DOMAIN_LST_NODE_OFFSET)
  {
    entry_found_b = 1;
    break;
  }

  if(entry_found_b == 0 )
  {
  
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }

  if ((strlen(ttprm_domain_node_p->domain_name) == 0))
  {
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }
  strcpy(domain_name_p, ttprm_domain_node_p->domain_name);
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_SUCCESS;

}
int32_t get_next_ttp_domain(char *ttp_name_p, 
                            char *domain_name_p,  
                            char *next_domain_name_p)
{
  int32_t ret_val;
  uint8_t  current_entry_found_b=FALSE;

  struct ttprm_entry *ttprm_entry_p = NULL;
  struct  ttprm_domain_entry *ttprm_domain_node_p=NULL;

  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, 
               "inavild main-ttp name:%s.",ttp_name_p);
   CNTLR_RCU_READ_LOCK_RELEASE();
   return CNTLR_TTP_FAILURE;
  }


  OF_LIST_SCAN(ttprm_entry_p->domain_list, 
               ttprm_domain_node_p, 
               struct  ttprm_domain_entry *, 
               TTPRM_DOMAIN_LST_NODE_OFFSET)
  {
    if(current_entry_found_b == FALSE)
    {
      if(strcmp(domain_name_p, ttprm_domain_node_p->domain_name))
        continue;
      else
      {
        current_entry_found_b = TRUE;
        continue;
      }
    }
    else
    {
      strcpy(domain_name_p, ttprm_domain_node_p->domain_name);
      CNTLR_RCU_READ_LOCK_RELEASE();
      return CNTLR_TTP_SUCCESS;
    }
  }

  if(current_entry_found_b == FALSE)
  {
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }

  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_FAILURE;
}
int32_t get_exact_ttp_domain(char *ttp_name_p, char *domain_name_p)
{
  int32_t ret_val;
  struct ttprm_entry *ttprm_entry_p = NULL;
  struct  ttprm_domain_entry *ttprm_domain_node_p=NULL;

  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR,"inavild main-ttp name:%s.",ttp_name_p);
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }


  OF_LIST_SCAN(ttprm_entry_p->domain_list, 
               ttprm_domain_node_p, 
               struct  ttprm_domain_entry *, 
               TTPRM_TBL_LST_NODE_OFFSET)
  {
    if(!strcmp(ttprm_domain_node_p->domain_name,domain_name_p))
    {
      CNTLR_RCU_READ_LOCK_RELEASE();
      return CNTLR_TTP_SUCCESS;
    }
  }
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_FAILURE;
}
