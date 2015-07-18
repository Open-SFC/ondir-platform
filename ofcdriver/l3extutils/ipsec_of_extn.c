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

/*File: ipsec_of_extn.c
 *Author: Narayana  Rao KVV <narayana.k@freescale.com>
 * Description:
 * This file contails NB APIs of experimenter L3 extension instructions 
 */

#ifdef OPENFLOW_IPSEC_SUPPORT
#include "controller.h"
#include "of_utilities.h"
#include "cntlr_tls.h"
#include "cntrl_queue.h"
#include "cntlr_transprt.h"
#include "cntlr_event.h"
#include "dprmldef.h"
#include "of_utilities.h"
#include "fsl_ext.h"
#include "oflog.h"


#include "ipsec_ldef.h"
#include "cntrlappcmn.h"
#include "nfapi_common.h"
#include "fsl_nfapi_ipsec.h"
#include "ipsec_nb_utils.h"

//#include "ipsec_be_def.h"


//extern inline void of_ipsec_ipv6_addr_prefix_to_mask(uint8_t plen,
   //                                           ipv6_addr_t *addr);

#define IPSEC_SA_MAP_AUTH_ALGO_NF_API_TO_OF(nf_algo, of_algo)\
{\
  switch(nf_algo)\
  {\
    case IPSEC_AUTH_ALG_MD5HMAC:\
      of_algo = FSLX_AUTH_ALG_HMAC_MD5;\
      break;\
    case IPSEC_AUTH_ALG_SHA1HMAC:\
      of_algo = FSLX_AUTH_ALG_HMAC_SHA1;\
      break;\
    case IPSEC_AUTH_ALG_SHA2_256_HMAC:\
      of_algo = FSLX_AUTH_ALG_HMAC_SHA2_256;\
      break;\
    case IPSEC_AUTH_ALG_SHA2_384_HMAC:\
      of_algo = FSLX_AUTH_ALG_HMAC_SHA2_384;\
      break;\
    case IPSEC_AUTH_ALG_SHA2_512_HMAC:\
      of_algo = FSLX_AUTH_ALG_HMAC_SHA2_512;\
      break;\
    case IPSEC_AUTH_ALG_AESXCBC:\
      of_algo = FSLX_AUTH_ALG_AES_XCBC_MAC;\
      break;\
    case IPSEC_AUTH_ALG_NONE:\
    default:\
    {\
      OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,\
                 "Authentication Algorithem Not Supported, type = %d",nf_algo);\
      return OF_FAILURE;\
    }\
  }\
}

#define IPSEC_SA_MAP_ENC_ALGO_NF_API_TO_OF(nf_algo, of_algo)\
{\
  switch(nf_algo)\
  {\
    case IPSEC_ENC_ALG_DES_CBC:\
      of_algo = FSLX_ENCR_ALG_CBC_DES;\
      break;\
    case IPSEC_ENC_ALG_3DES_CBC:\
      of_algo = FSLX_ENCR_ALG_CBC_3DES;\
      break;\
    case IPSEC_ENC_ALG_NULL:\
      of_algo = FSLX_ENCR_ALG_NULL;\
      break;\
    case IPSEC_ENC_ALG_AES_CBC:\
      of_algo = FSLX_ENCR_ALG_AES_CBC;\
      break;\
    case IPSEC_ENC_ALG_AES_CTR:\
      of_algo = FSLX_ENCR_ALG_AES_CTR;\
      break;\
    default:\
    {\
      OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,\
                 "Encryption Algorithem Not Supported, type = %d",nf_algo);\
      return OF_FAILURE;\
    }\
  }\
}

#define IPSEC_SA_MAP_COMB_MODE_ALGO_NF_API_TO_OF(nf_algo, of_algo)\
{\
  switch(nf_algo)\
  {\
   case IPSEC_COMB_AES_CCM:\
     of_algo = FSLX_CM_ALG_AES_CCM;\
     break;\
   case IPSEC_COMB_AES_GCM:\
     of_algo = FSLX_CM_ALG_AES_GCM;\
     break;\
   case IPSEC_COMB_AES_GMAC:\
     of_algo = FSLX_CM_ALG_AES_GMAC;\
     break;\
    default:\
    {\
      OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,\
                 "Combine Mode Algorithem Not Supported, type = %d",nf_algo);\
      return OF_FAILURE;\
    }\
  }\
}

#define OF_OXM_HDR_LEN   (4)

/***************************************************************************
 * Function Name : of_set_spi 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t of_set_spi(struct of_msg *msg, uint32_t  *spi)
{
  struct of_msg_descriptor *msg_desc = &msg->desc;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  cntlr_assert(msg != NULL);
  msg_desc = &msg->desc;

  if( (msg_desc->ptr1+msg_desc->length1 + OFU_IPSEC_SPI_FIELD_LEN) >
          (msg->desc.start_of_data + msg_desc->buffer_len)   ) 
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_WARN,
               "Length of buffer is not sufficient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }
//  *(uint32_t*)(msg_desc->ptr1+msg_desc->length1)  = htonl(OXM_OF_IPSEC_SPI);
  *(uint32_t*)(msg_desc->ptr1+msg_desc->length1 + OF_OXM_HDR_LEN) = htonl(*spi); 
  msg_desc->length1 += OFU_IPSEC_SPI_FIELD_LEN;
      //msg_desc->curr_ptr +=  OFU_IPV4_SRC_FIELD_LEN;
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_SET_FIELD_SUCCESS;
}

/* Experimenter Actions */
/***************************************************************************
 * Function Name : fslx_action_attach_bind_object 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_action_attach_bind_object(struct of_msg *msg, uint32_t bind_obj_id, uint8_t obj_type)
{
  struct of_msg_descriptor *msg_desc;
  struct fslx_action_attach_bind_obj *bind_obj;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLX_ACTION_ATTACH_BIND_OBJ_ID_LEN)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_WARN,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }

  bind_obj = 
     (struct fslx_action_attach_bind_obj *)(msg_desc->ptr3 + msg_desc->length3);

  bind_obj->type = htons(OFPAT_EXPERIMENTER);
  bind_obj->len = htons(FSLX_ACTION_ATTACH_BIND_OBJ_ID_LEN);
  bind_obj->vendor = htonl(FSLX_VENDOR_ID);
  bind_obj->subtype = htons(FSLXAT_ATTACH_BIND_OBJ);
  bind_obj->bind_obj_id = htonl(bind_obj_id);
  bind_obj->obj_type    = obj_type;
  bind_obj->hit_count   = 0;
  bind_obj->flag = 0;
  memset(bind_obj->pad, 0, 4);

  msg_desc->length3 += (FSLX_ACTION_ATTACH_BIND_OBJ_ID_LEN);
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_ACTION_PUSH_SUCCESS;
}

/***************************************************************************
 * Function Name : fslx_action_detach_bind_object 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_action_detach_bind_object(struct of_msg *msg, uint32_t bind_obj_id)
{
  struct of_msg_descriptor *msg_desc;
  struct fslx_action_detach_bind_obj *bind_obj;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLX_ACTION_DETACH_BIND_OBJ_ID_LEN)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_WARN,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }

  bind_obj = 
     (struct fslx_action_detach_bind_obj *)(msg_desc->ptr3 + msg_desc->length3);

  bind_obj->type = htons(OFPAT_EXPERIMENTER);
  bind_obj->len = htons(FSLX_ACTION_DETACH_BIND_OBJ_ID_LEN);
  bind_obj->vendor = htonl(FSLX_VENDOR_ID);
  bind_obj->subtype = htons(FSLXAT_DETACH_BIND_OBJ);
  bind_obj->bind_obj_id = htonl(bind_obj_id);
  memset(&bind_obj->pad, 0 , sizeof(bind_obj->pad));

  msg_desc->length3 += (FSLX_ACTION_DETACH_BIND_OBJ_ID_LEN);
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_ACTION_PUSH_SUCCESS;
}

/***************************************************************************
 * Function Name : fslx_action_ipsec_set_app_state_info 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_action_ipsec_set_app_state_info(struct of_msg *msg,
                                      struct ipsec_sa_state_info_req *req)
{
  struct of_msg_descriptor *msg_desc;
  struct fslx_action_set_app_state_info *state;
  struct ipsec_sa_state_info_req *temp;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLX_ACTION_SET_APP_STATE_INFO_LEN)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_WARN,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }

   state = 
     (struct fslx_action_set_app_state_info *)(msg_desc->ptr3 + msg_desc->length3);

   state->type = htons(OFPAT_EXPERIMENTER);
   state->len = htons(FSLX_ACTION_SET_APP_STATE_INFO_LEN);
   state->vendor = htonl(FSLX_VENDOR_ID);
   state->subtype = htons(FSLXAT_SET_APP_STATE_INFO);
   state->length = htonl(FSLX_ACTION_SET_APP_STATE_INFO_LEN);
   memset(&state->pad, 0 , sizeof(state->pad));

   msg_desc->length3 += FSLX_ACTION_SET_APP_STATE_INFO_LEN;

  temp = 
     (struct ipsec_sa_state_info_req *)(msg_desc->ptr3 + msg_desc->length3);
  temp->seq_num = htonl(req->seq_num);
  temp->hi_seq_num = htonl(req->hi_seq_num);
  temp->elapsed_time_sec = htonll(req->elapsed_time_sec);
  temp->bytes_processed = htonll(req->bytes_processed);
  temp->pkts_processed = htonll(req->pkts_processed);

  msg_desc->length3 += IPSEC_SA_STATE_INFO_REQ_LEN;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_ACTION_PUSH_SUCCESS;
}

/***************************************************************************
 * Function Name : fslx_action_ipsec_send_appl_state_info
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_action_ipsec_send_appl_state_info(struct of_msg *msg, 
                               enum fslx_app_state_event_type e_type,
                               enum fslx_app_state_event_method e_method,
                               uint64_t value)
{
  struct of_msg_descriptor *msg_desc;
  struct fslx_action_send_app_state_info *state;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLX_ACTION_SEND_APP_STATE_INFO_LEN)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_WARN,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }

  state = 
     (struct fslx_action_send_app_state_info *)(msg_desc->ptr3 + msg_desc->length3);

  state->type = htons(OFPAT_EXPERIMENTER);
  state->len = htons(FSLX_ACTION_SEND_APP_STATE_INFO_LEN);
  state->vendor = htonl(FSLX_VENDOR_ID);
  state->subtype = htons(FSLXAT_SEND_APP_STATE_INFO);

  state->event_type = e_type; 
  state->event_method = e_method; 
  state->event_count = htonll(value);

  msg_desc->length3 += FSLX_ACTION_SEND_APP_STATE_INFO_LEN;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_ACTION_PUSH_SUCCESS;
}

/***************************************************************************
 * Function Name : fslx_action_ipsec_sa_delete 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_action_ipsec_sa_delete(struct of_msg *msg)
{
  struct of_msg_descriptor *msg_desc;
  struct fslx_delete_ipsec_sa *sa_del;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLX_ACTION_IPSEC_SA_DELETE_LEN)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_WARN,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }

  sa_del = 
     (struct fslx_delete_ipsec_sa *)(msg_desc->ptr3 + msg_desc->length3);

  sa_del->type = htons(OFPAT_EXPERIMENTER);
  sa_del->len = htons(FSLX_ACTION_IPSEC_SA_DELETE_LEN);
  sa_del->vendor = htonl(FSLX_VENDOR_ID);
  sa_del->subtype = htons(FSLXAT_DELETE_SA_CNTXT);
  memset(&sa_del->pad, 0 , sizeof(sa_del->pad));

  msg_desc->length3 += FSLX_ACTION_IPSEC_SA_DELETE_LEN;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_ACTION_PUSH_SUCCESS;
}

/***************************************************************************
 * Function Name : fslx_action_ipsec_encap 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_action_ipsec_encap(struct of_msg *msg)
{
  struct of_msg_descriptor *msg_desc;
  struct fslx_ipsec_encap *encap;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");
  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLX_ACTION_IPSEC_ENACAP_LEN)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_WARN,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }

  encap = 
     (struct fslx_ipsec_encap *)(msg_desc->ptr3 + msg_desc->length3);

  encap->type = htons(OFPAT_EXPERIMENTER);
  encap->len = htons(FSLX_ACTION_IPSEC_ENACAP_LEN);
  encap->vendor = htonl(FSLX_VENDOR_ID);
  encap->subtype = htons(FSLXAT_IPSEC_ENCAP);
  memset(&encap->pad, 0 , sizeof(encap->pad));

  msg_desc->length3 += FSLX_ACTION_IPSEC_ENACAP_LEN;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_ACTION_PUSH_SUCCESS;
}

/***************************************************************************
 * Function Name : fslx_action_ipsec_decap 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_action_ipsec_decap(struct of_msg *msg)
{
  struct of_msg_descriptor *msg_desc;
  struct fslx_ipsec_decap *decap;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLX_ACTION_IPSEC_DECAP_LEN)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_WARN,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }

  decap = 
     (struct fslx_ipsec_decap *)(msg_desc->ptr3 + msg_desc->length3);

  decap->type = htons(OFPAT_EXPERIMENTER);
  decap->len = htons(FSLX_ACTION_IPSEC_DECAP_LEN);
  decap->vendor = htonl(FSLX_VENDOR_ID);
  decap->subtype = htons(FSLXAT_IPSEC_DECAP);
  memset(&decap->pad, 0 , sizeof(decap->pad));

  msg_desc->length3 += FSLX_ACTION_IPSEC_DECAP_LEN;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_ACTION_PUSH_SUCCESS;
}

/***************************************************************************
 * Function Name : fslx_action_create_sa_context 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_action_create_sa_context(struct of_msg *msg,
                                      struct ipsec_sa_cfg_inargs *in) 
{
  struct of_msg_descriptor *msg_desc;
  struct fslx_create_ipsec_sa *sa_ctx;
  struct ipsec_sa          *sa;
  struct ipsec_selector    *sa_selectors;
  uint8_t                  ii;
  uint16_t                 algo = 0, len = 0, total_len = 0;
  uint32_t                 ipv4_mask = 0;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLX_ACTION_IPSEC_SA_CREATE_LEN)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }
  sa = in->sa_params;
  sa_ctx = 
     (struct fslx_create_ipsec_sa *)(msg_desc->ptr3 + msg_desc->length3);

  sa_ctx->type = htons(OFPAT_EXPERIMENTER);
  sa_ctx->vendor = htonl(FSLX_VENDOR_ID);
  sa_ctx->subtype = htons(FSLXAT_CREATE_SA_CNTXT);

  sa_ctx->flags = FSLX_IPSEC_SA_FLAGS_NONE;

  if(in->dir == IPSEC_INBOUND)
    sa_ctx->flags |= FSLX_IPSEC_SA_DIRECTION_INBOUND;

  if(sa->flags & IPSEC_SA_REDSIDE_FRAGMENTATION) 
    sa_ctx->flags |= FSLX_IPSEC_SA_ENABLE_REDSIDE_FRAG;

/*
   if(do_decap_dscp FSLX_IPSEC_SA_COPY_DSCP_BITS)
*/
  sa_ctx->spi = htonl(sa->spi);
  sa_ctx->protocol = sa->protocol;
/*
   if(sa->handle_dfbit)
     sa_ctx->handle_df_bit; 
*/
  sa_ctx->copy_dscp_value = sa->dscp;
  sa_ctx->replay_window_size = htonl(sa->replaywindow_size);

  if(sa->flags & IPSEC_SA_ENCAP_MODE_TRANSPORT) 
    sa_ctx->encapsulation_mode = FSLX_SA_TRANSPORT_MODE;
  else
    sa_ctx->encapsulation_mode = FSLX_SA_TUNNEL_MODE;

  msg_desc->length3 += FSLX_ACTION_IPSEC_SA_CREATE_LEN;

  total_len += FSLX_ACTION_IPSEC_SA_CREATE_LEN;

  if((sa->crypto_params.auth_algo != 0) &&
     (sa->crypto_params.auth_algo != IPSEC_AUTH_ALG_NONE))
  {
    struct fslx_ipsec_sa_auth_algo_info *auth;

    if(!sa->crypto_params.authkey_len_bits)
    {
      OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR, "Invalid Auth Key Length");
      return OF_FAILURE;
    }
    auth = 
     (struct fslx_ipsec_sa_auth_algo_info *)(msg_desc->ptr3 + msg_desc->length3);
   
    auth->type = htons(FSLX_IPSEC_SA_AUTH_ALGO);
    IPSEC_SA_MAP_AUTH_ALGO_NF_API_TO_OF(sa->crypto_params.auth_algo, algo);
    auth->auth_algo = htons(algo);
    auth->icv_size_in_bits = htons(sa->crypto_params.icv_len_bits);
    auth->authkey_len_in_bits = htons(sa->crypto_params.authkey_len_bits);
    memset(&auth->pad, 0 , sizeof(auth->pad));
    memcpy(&auth->authkey_val, sa->crypto_params.authkey, (sa->crypto_params.authkey_len_bits/8));

    len = IPSEC_SA_AUTH_ALGO_INFO_LEN + (sa->crypto_params.authkey_len_bits >> 3);
    msg_desc->length3 += len;
    auth->length = htons(len);
    total_len += len;
  }

  if(sa->crypto_params.enc_algo != 0)
  {
    struct fslx_ipsec_sa_enc_algo_info *encr;

    if((sa->crypto_params.enc_algo != IPSEC_ENC_ALG_NULL) &&
       (!sa->crypto_params.enckey_len_bits))
    {
      OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR, "Invalid Encryption Key Length");
      return OF_FAILURE;
    }
    encr = 
     (struct fslx_ipsec_sa_enc_algo_info *)(msg_desc->ptr3 + msg_desc->length3);
   
    encr->type = htons(FSLX_IPSEC_SA_ENCRYPT_ALGO);
    IPSEC_SA_MAP_ENC_ALGO_NF_API_TO_OF(sa->crypto_params.enc_algo, algo);
    encr->enc_algo = htons(algo);
    if(sa->crypto_params.enc_algo != IPSEC_ENC_ALG_NULL)
    {
      encr->enc_key_len_in_bits = htons(sa->crypto_params.enckey_len_bits);
//    memset(&encr->pad, 0 , sizeof(encr->pad));
      memcpy(&encr->enc_key, sa->crypto_params.enckey, 
                          (sa->crypto_params.enckey_len_bits >> 3));
      len = IPSEC_SA_ENC_ALGO_INFO_LEN + (sa->crypto_params.enckey_len_bits >> 3);
    }
    else
    {
      encr->enc_key_len_in_bits = 0;
      len = IPSEC_SA_ENC_ALGO_INFO_LEN;
    } 
    msg_desc->length3 += len;
    encr->length = htons(len);
    total_len += len;
  }

  if(sa->crypto_params.comb_algo != 0)
  {
    struct fslx_ipsec_sa_enc_auth_algo_info *comb;

    if(!sa->crypto_params.combkey_len_bits)
    {
      OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR, "Invalid Combine Mode Key Length");
      return OF_FAILURE;
    }
    comb = 
     (struct fslx_ipsec_sa_enc_auth_algo_info *)(msg_desc->ptr3 + msg_desc->length3);
   
    comb->type = htons(FSLX_IPSEC_SA_CMB_MODE_ALGO);
    IPSEC_SA_MAP_COMB_MODE_ALGO_NF_API_TO_OF(sa->crypto_params.comb_algo, algo);
    comb->enc_auth_algo = htons(algo);
    comb->enc_auth_key_len_in_bits = htons(sa->crypto_params.combkey_len_bits);
    comb->icv_size_in_bits = htons(sa->crypto_params.icv_len_bits);
    memset(&comb->pad, 0 , sizeof(comb->pad));
    memcpy(&comb->enc_auth_key , sa->crypto_params.combkey,
                              (sa->crypto_params.combkey_len_bits >> 3));

    len = IPSEC_SA_ENC_AUTH_ALGO_INFO_LEN + (sa->crypto_params.combkey_len_bits >> 3);
    msg_desc->length3 += len;
    comb->length = htons(len);
    total_len += len;
  }

//struct fslx_ipsec_sa_enc_algo_iv_info

  if(sa->crypto_params.iv)
  {
    struct fslx_ipsec_sa_aesctr_block_or_iv_info *salt;  

    if(!sa->crypto_params.iv_len_bits)
    {
      OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR, "Invalid IV Length");
      return OF_FAILURE;
    }
    salt = 
     (struct fslx_ipsec_sa_aesctr_block_or_iv_info *)(msg_desc->ptr3 + msg_desc->length3);
   
    salt->type = htons(FSLX_IPSEC_SA_AESCTR_OR_CMBN_MODE_SALT);
    salt->aesctr_block_or_iv_len_bits = htons(sa->crypto_params.iv_len_bits);
    memset(&salt->pad, 0 , sizeof(salt->pad));
    memcpy(&salt->aesctr_block_or_iv, sa->crypto_params.iv, 
                                     (sa->crypto_params.iv_len_bits >> 3));

    len = IPSEC_SA_BLOCK_OR_IV_INFO_LEN + (sa->crypto_params.iv_len_bits >> 3);
    msg_desc->length3 += len;
    salt->length = htons(len);
    total_len += len;
  }  

  if(sa->nat_info.nat_oa_peer_addr.ip_version == IP_V4)
  {
    struct fslx_ipsec_sa_ipv4_udp_encap_info *encap;

    encap = 
     (struct fslx_ipsec_sa_ipv4_udp_encap_info *)(msg_desc->ptr3 + msg_desc->length3);
   
    encap->type = htons(FSLX_IPSEC_SA_IPV4_UDP_ENCAP);
    encap->length = htons(IPSEC_SA_IPV4_UDP_ENCAP_INFO_LEN);
    if(sa->nat_info.version == IPSEC_IKE_NATV1)
      encap->nat_type = FSLX_UDP_ENCAP_TYPE_ESP_IN_UDP;
    else if(sa->nat_info.version == IPSEC_IKE_NATV2)
      encap->nat_type = FSLX_UDP_ENCAP_TYPE_ESP_IN_UDP_NON_IKE;   
    encap->src_port = htons(sa->nat_info.src_port);   
    encap->dst_port = htons(sa->nat_info.dest_port);
    memset(&encap->pad, 0 , sizeof(encap->pad));
    encap->ip4_oa_addr = htonl(sa->nat_info.nat_oa_peer_addr.ip4);
    msg_desc->length3 += IPSEC_SA_IPV4_UDP_ENCAP_INFO_LEN;
    total_len += IPSEC_SA_IPV4_UDP_ENCAP_INFO_LEN;
  }
  else if(sa->nat_info.nat_oa_peer_addr.ip_version == IP_V6)
  {
    struct fslx_ipsec_sa_ipv6_udp_encap_info *encap;

    encap = 
     (struct fslx_ipsec_sa_ipv6_udp_encap_info *)(msg_desc->ptr3 + msg_desc->length3);
   
    encap->type = htons(FSLX_IPSEC_SA_IPV6_UDP_ENCAP);
    encap->length = htons(IPSEC_SA_IPV6_UDP_ENCAP_INFO_LEN);
    if(sa->nat_info.version == IPSEC_IKE_NATV1)
      encap->nat_type = FSLX_UDP_ENCAP_TYPE_ESP_IN_UDP;
    else if(sa->nat_info.version == IPSEC_IKE_NATV2)
      encap->nat_type = FSLX_UDP_ENCAP_TYPE_ESP_IN_UDP_NON_IKE;   
    encap->src_port = htons(sa->nat_info.src_port);   
    encap->dst_port = htons(sa->nat_info.dest_port);
    memset(&encap->pad, 0 , sizeof(encap->pad));
    memcpy(&encap->ip6_oa_addr, &sa->nat_info.nat_oa_peer_addr.ip6.b_addr,
             sizeof(encap->ip6_oa_addr));
    msg_desc->length3 += IPSEC_SA_IPV6_UDP_ENCAP_INFO_LEN;
    total_len += IPSEC_SA_IPV6_UDP_ENCAP_INFO_LEN;
  }
  for(ii = 0; ii < sa->n_selector; ii++)
  {
    sa_selectors = &in->sa_params->selectors[ii].selector;
    if(sa_selectors->ip_version == IP_V4)
    {
      struct fslx_ipsec_sa_ipv4_selectors_info *v4_sel;

      v4_sel = 
         (struct fslx_ipsec_sa_ipv4_selectors_info *)(msg_desc->ptr3 + msg_desc->length3);
     
      v4_sel->type = htons(FSLX_IPSEC_SA_IPV4_SELECTOR);
      v4_sel->length = htons(IPSEC_SA_IPV4_SELECTOR_INFO_LEN);
      v4_sel->protocol = sa_selectors->protocol_choice.protocol;
      v4_sel->src_start_port = sa_selectors->protocol_choice.src_port.start;
      v4_sel->src_end_port = sa_selectors->protocol_choice.src_port.end;
      v4_sel->dst_start_port= sa_selectors->protocol_choice.dest_port.start;
      v4_sel->dst_end_port= sa_selectors->protocol_choice.dest_port.end;
      memset(&v4_sel->pad, 0 , sizeof(v4_sel->pad)); 

      if(sa_selectors->src_ip.addr_type == IPSEC_ADDR_TYPE_SUBNET)
      {
        v4_sel->src_start_addr = htonl(sa_selectors->src_ip.prefixaddr.v4.ipv4addr);
        IPV4_BITS_TO_MASK(sa_selectors->src_ip.prefixaddr.v4.ipv4plen, ipv4_mask);
        v4_sel->src_mask_or_end_addr = htonl(ipv4_mask);

        v4_sel->dst_start_addr= htonl(sa_selectors->dest_ip.prefixaddr.v4.ipv4addr);
        ipv4_mask = 0;
        IPV4_BITS_TO_MASK(sa_selectors->dest_ip.prefixaddr.v4.ipv4plen, ipv4_mask);
        v4_sel->dst_mask_or_end_addr= htonl(ipv4_mask);
      }
      else
      {
        v4_sel->src_start_addr = htonl(sa_selectors->src_ip.rangeaddr.v4.start);
        v4_sel->src_mask_or_end_addr = htonl(sa_selectors->src_ip.rangeaddr.v4.end);
        v4_sel->dst_start_addr= htonl(sa_selectors->dest_ip.rangeaddr.v4.start);
        v4_sel->dst_mask_or_end_addr= htonl(sa_selectors->dest_ip.rangeaddr.v4.end);
      }
      msg_desc->length3 += IPSEC_SA_IPV4_SELECTOR_INFO_LEN;
      total_len += IPSEC_SA_IPV4_SELECTOR_INFO_LEN;
    }
    else
    {
      struct fslx_ipsec_sa_ipv6_selectors_info *v6_sel;
  
      v6_sel = 
         (struct fslx_ipsec_sa_ipv6_selectors_info *)(msg_desc->ptr3 + msg_desc->length3);
     
      v6_sel->type = htons(FSLX_IPSEC_SA_IPV6_SELECTOR);
      v6_sel->length = htons(IPSEC_SA_IPV6_SELECTOR_INFO_LEN);
      v6_sel->protocol = sa_selectors->protocol_choice.protocol;
      v6_sel->src_start_port = sa_selectors->protocol_choice.src_port.start;
      v6_sel->src_end_port = sa_selectors->protocol_choice.src_port.end;
      v6_sel->dst_start_port= sa_selectors->protocol_choice.dest_port.start;
      v6_sel->dst_end_port= sa_selectors->protocol_choice.dest_port.end;
//      memset(&v6_sel->pad, 0 , sizeof(v6_sel->pad)); 

      if(sa_selectors->src_ip.addr_type == IPSEC_ADDR_TYPE_SUBNET)
      {
        memcpy(&v6_sel->src_start_addr,
                  &sa_selectors->src_ip.prefixaddr.v6.ipv6addr.b_addr,
                  sizeof(v6_sel->src_start_addr));
        of_ipsec_ipv6_addr_prefix_to_mask(sa_selectors->src_ip.prefixaddr.v6.ipv6plen,
                                      (ipv6_addr_t *) &v6_sel->src_mask_or_end_addr);
        memcpy(&v6_sel->dst_start_addr,
                  &sa_selectors->dest_ip.prefixaddr.v6.ipv6addr.b_addr,
                  sizeof(v6_sel->dst_start_addr));
        of_ipsec_ipv6_addr_prefix_to_mask(sa_selectors->dest_ip.prefixaddr.v6.ipv6plen,
                                          (ipv6_addr_t *)&v6_sel->dst_mask_or_end_addr);
      }
      else
      {
        memcpy(&v6_sel->src_start_addr,
                  &sa_selectors->src_ip.prefixaddr.v6.ipv6addr.b_addr,
                  sizeof(v6_sel->src_start_addr));
        memcpy(&v6_sel->src_mask_or_end_addr,
                  &sa_selectors->src_ip.prefixaddr.v6.ipv6addr.b_addr,
                  sizeof(v6_sel->src_mask_or_end_addr));
        memcpy(&v6_sel->dst_start_addr,
                  &sa_selectors->dest_ip.prefixaddr.v6.ipv6addr.b_addr,
                  sizeof(v6_sel->dst_start_addr));
        memcpy(&v6_sel->dst_mask_or_end_addr,
                  &sa_selectors->dest_ip.prefixaddr.v6.ipv6addr.b_addr,
                  sizeof(v6_sel->dst_mask_or_end_addr));
      }
      msg_desc->length3 += IPSEC_SA_IPV6_SELECTOR_INFO_LEN;
      total_len += IPSEC_SA_IPV6_SELECTOR_INFO_LEN;
    }
  }

  if(sa->te_addr.ip_version == IP_V4)
  {
    struct fslx_ipsec_sa_ipv4_tunnel_end_addrs_info *tun_addrs;

    tun_addrs = 
     (struct fslx_ipsec_sa_ipv4_tunnel_end_addrs_info *)(msg_desc->ptr3 + msg_desc->length3);
   
    tun_addrs->type = htons(FSLX_IPSEC_SA_IPV4_TUNNEL_END_ADDRS);
    tun_addrs->length = htons(IPSEC_SA_IPV4_TUNNEL_ENDS_INFO_LEN);
    tun_addrs->src_addr = htonl(sa->te_addr.src_ip.ip4);
    tun_addrs->dest_addr = htonl(sa->te_addr.dest_ip.ip4);
    memset(&tun_addrs->pad, 0 , sizeof(tun_addrs->pad));
    msg_desc->length3 += IPSEC_SA_IPV4_TUNNEL_ENDS_INFO_LEN;
    total_len += IPSEC_SA_IPV4_TUNNEL_ENDS_INFO_LEN;
  }
  else if(sa->te_addr.ip_version == IP_V6)
  {
    struct fslx_ipsec_sa_ipv6_tunnel_end_addrs_info *tun_addrs;

    tun_addrs = 
     (struct fslx_ipsec_sa_ipv6_tunnel_end_addrs_info *)(msg_desc->ptr3 + msg_desc->length3);
   
    tun_addrs->type = htons(FSLX_IPSEC_SA_IPV6_TUNNEL_END_ADDRS);
    tun_addrs->length = htons(IPSEC_SA_IPV6_TUNNEL_ENDS_INFO_LEN);
    memcpy(&tun_addrs->src_addr, &sa->te_addr.src_ip.ip6.b_addr,
              sizeof(tun_addrs->src_addr));
    memcpy(&tun_addrs->dest_addr, &sa->te_addr.dest_ip.ip6.b_addr,
              sizeof(tun_addrs->dest_addr));
    memset(&tun_addrs->pad, 0 , sizeof(tun_addrs->pad));
    msg_desc->length3 += IPSEC_SA_IPV6_TUNNEL_ENDS_INFO_LEN;
    total_len += IPSEC_SA_IPV6_TUNNEL_ENDS_INFO_LEN;
  }

  if(!((sa->dscp_start == 0) && (sa->dscp_end == 0)))
  {
    struct fslx_ipsec_sa_dscp_range *dscp;

    dscp = 
     (struct fslx_ipsec_sa_dscp_range *)(msg_desc->ptr3 + msg_desc->length3);
   
    dscp->type = htons(FSLX_IPSEC_SA_DSCP_RANGE);
    dscp->length = htons(IPSEC_SA_DSCP_RANGE_INFO_LEN);
    dscp->start = sa->dscp_start;   
    dscp->end = sa->dscp_end;
    memset(&dscp->pad, 0 , sizeof(dscp->pad));
    msg_desc->length3 += IPSEC_SA_DSCP_RANGE_INFO_LEN;
    total_len += IPSEC_SA_DSCP_RANGE_INFO_LEN;
  }

  sa_ctx->len = htons(total_len);
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT, total len = %u",total_len);
  return OFU_ACTION_PUSH_SUCCESS;
}

/***************************************************************************
 * Function Name : fslx_action_execute_bind_obj_actions 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_action_execute_bind_obj_actions(struct of_msg *msg,
                                             uint32_t bind_obj_id)
{
  struct of_msg_descriptor *msg_desc;
  struct fslx_action_execute_bind_obj_apply_actns *bind_action;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");
  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLXAT_BIND_OBJECT_APPLY_ACTIONS)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_WARN,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }

  bind_action = 
     (struct fslx_action_execute_bind_obj_apply_actns *)(msg_desc->ptr3 + msg_desc->length3);

  bind_action->type = htons(OFPAT_EXPERIMENTER);
  bind_action->len = htons(FSLX_ACTION_EXECUTE_BIND_OBJ_APPLY_ACTNS_LEN);
  bind_action->vendor = htonl(FSLX_VENDOR_ID);
  bind_action->subtype = htons(FSLXAT_BIND_OBJECT_APPLY_ACTIONS);
  bind_action->bind_obj_id = htonl(bind_obj_id);
//  memset(&bind_action->pad, 0 , sizeof(bind_action->pad));
  bind_action->obj_type =  0;

  msg_desc->length3 += (FSLX_ACTION_EXECUTE_BIND_OBJ_APPLY_ACTNS_LEN);
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_ACTION_PUSH_SUCCESS;
}

/***************************************************************************
 * Function Name : fslx_pop_natt_udp_header_action 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
int32_t fslx_pop_natt_udp_header_action(struct of_msg *msg)
{
  struct of_msg_descriptor *msg_desc;
  struct fslx_action_header *pop_udp_hdr;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  msg_desc = &msg->desc;
  if((msg_desc->ptr3+msg_desc->length3+(FSLX_ACTION_POP_NATT_UDP_HEADER_LEN)) >
      (msg->desc.start_of_data + msg_desc->buffer_len))
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_WARN,
               "Length of buffer is not suffeceient to add data");
    return OFU_NO_ROOM_IN_BUFFER;
  }

  pop_udp_hdr = 
     (struct fslx_action_header *)(msg_desc->ptr3 + msg_desc->length3);

  pop_udp_hdr->type = htons(OFPAT_EXPERIMENTER);
  pop_udp_hdr->len = htons(FSLX_ACTION_POP_NATT_UDP_HEADER_LEN);
  pop_udp_hdr->vendor = htonl(FSLX_VENDOR_ID);
  pop_udp_hdr->subtype = htons(FSLXAT_POP_NATT_UDP_HEADER);
  memset(&pop_udp_hdr->pad, 0 , sizeof(pop_udp_hdr->pad));

  msg_desc->length3 += (FSLX_ACTION_POP_NATT_UDP_HEADER_LEN);
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OFU_ACTION_PUSH_SUCCESS;
}

#endif /* OPENFLOW_IPSEC_SUPPORT */

