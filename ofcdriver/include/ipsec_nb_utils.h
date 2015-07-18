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

/*File: ipsec_nb_util.c
 *Author: Narayana  Rao KVV <narayana.k@freescale.com>
 * Description:
 * This file contails IPSec Backend North bound API utilities 
 */

#ifdef OPENFLOW_IPSEC_SUPPORT

/***************************************************************************
 * Function Name : of_ipsec_ipv6_addr_prefix_to_mask 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
inline void of_ipsec_ipv6_addr_prefix_to_mask(uint8_t plen, ipv6_addr_t *addr)
{
  uint8_t bytes = plen >> 3;
  uint8_t bits = plen & 0x7;

  memset(&addr->ipv6Addr8, 0, 16);
  memset(&addr->ipv6Addr8, 0xff, bytes);
  if(bits != 0)
    addr->ipv6Addr8[bytes] = (0xff << (8 - bits));
  return;
}
/***************************************************************************
 * Function Name : of_ipsec_fill_selector_info_in_of_msg 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
***************************************************************************/
inline int32_t of_ipsec_fill_selector_info_in_of_msg(struct ipsec_selector *selectors,
                                       struct of_msg  *msg)
{
  int32_t         result = OF_SUCCESS;
  uint32_t        mask = 0;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  /* Source and Destination IP Address */
  if(selectors->ip_version == IP_V4)
  {
    /* Source Ip Address */
    if(selectors->src_ip.addr_type == IPSEC_ADDR_TYPE_SUBNET)
    {
      if(selectors->src_ip.prefixaddr.v4.ipv4plen)
      {
        IPV4_BITS_TO_MASK(selectors->src_ip.prefixaddr.v4.ipv4plen, mask);
        result = of_set_ipv4_source(msg, 
                     &selectors->src_ip.prefixaddr.v4.ipv4addr,
                     TRUE, &mask);
        if(result != OFU_SET_FIELD_SUCCESS)
        {
          OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                     "OF Set IPv4 src field failed, error = %d",result);
          return OF_FAILURE;
        }
      }  
    }
    else /* RANGE */ 
    {
      if((selectors->src_ip.rangeaddr.v4.start) ||
         (selectors->src_ip.rangeaddr.v4.end))
      {
        result = of_set_ipv4_source(msg, 
                     &selectors->src_ip.rangeaddr.v4.start,
                     TRUE, &selectors->src_ip.rangeaddr.v4.end);
        if(result != OFU_SET_FIELD_SUCCESS)
        {
          OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                   "OF Set IPv4 src field failed, error = %d",result);
          return OF_FAILURE;
        }
      } 
    }

    /* Destination Ip Address */
    if(selectors->dest_ip.addr_type == IPSEC_ADDR_TYPE_SUBNET)
    {
      if(selectors->dest_ip.prefixaddr.v4.ipv4plen)
      {
        IPV4_BITS_TO_MASK(selectors->dest_ip.prefixaddr.v4.ipv4plen, mask);
        result = of_set_ipv4_destination(msg, 
                       &selectors->dest_ip.prefixaddr.v4.ipv4addr,
                       TRUE, &mask);
        if(result != OFU_SET_FIELD_SUCCESS)
        {
          OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                     "OF Set IPv4 dest field failed, error = %d",result);
          return OF_FAILURE;
        }
      } 
    }
    else /* RANGE */ 
    {
      if((selectors->dest_ip.rangeaddr.v4.start) ||
         (selectors->dest_ip.rangeaddr.v4.end))
      {
        result = of_set_ipv4_destination(msg, 
                       &selectors->dest_ip.rangeaddr.v4.start,
                       TRUE, &selectors->dest_ip.rangeaddr.v4.end);
        if(result != OFU_SET_FIELD_SUCCESS)
        {
          OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                     "OF Set IPv4 dest field failed, error = %d",result);
          return OF_FAILURE;
        }
      } 
    }
  }
  else
  {
    ipv6_addr_t   ipv6_addr_mask;
    /* Source Ip Address */
    if(selectors->src_ip.addr_type == IPSEC_ADDR_TYPE_SUBNET)
    {
      if(selectors->src_ip.prefixaddr.v6.ipv6plen)
      {
        of_ipsec_ipv6_addr_prefix_to_mask(selectors->src_ip.prefixaddr.v6.ipv6plen,
                                          &ipv6_addr_mask);
        result = of_set_ipv6_source(msg, 
                 (ipv6_addr_t *) &selectors->src_ip.prefixaddr.v6.ipv6addr.b_addr,
                 TRUE, &ipv6_addr_mask);
        if(result != OFU_SET_FIELD_SUCCESS)
        {
          OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                     "OF Set IPv6 src field failed, error = %d",result);
          return OF_FAILURE;
        }
      } 
    }
    else /* RANGE */ 
    {
      memset(&ipv6_addr_mask, 0, sizeof(ipv6_addr_t));
      if((memcmp(&selectors->src_ip.rangeaddr.v6.start.b_addr,
                    &ipv6_addr_mask,
                    sizeof(struct ipv6_address))) ||
         (memcmp(&selectors->src_ip.rangeaddr.v6.end.b_addr,
                    &ipv6_addr_mask,
                    sizeof(struct ipv6_address))))
      {
        result = of_set_ipv6_source(msg, 
                  (ipv6_addr_t *)&selectors->src_ip.rangeaddr.v6.start.b_addr,
                  TRUE, (ipv6_addr_t *)&selectors->src_ip.rangeaddr.v6.end.b_addr);
        if(result != OFU_SET_FIELD_SUCCESS)
        {
          OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                     "OF Set IPv6 src field failed, error = %d",result);
          return OF_FAILURE;
        }
      } 
    }

    /* Destination Ip Address */
    if(selectors->dest_ip.addr_type == IPSEC_ADDR_TYPE_SUBNET)
    {
      if(selectors->dest_ip.prefixaddr.v6.ipv6plen)
      {
        of_ipsec_ipv6_addr_prefix_to_mask(selectors->dest_ip.prefixaddr.v6.ipv6plen,
                                          &ipv6_addr_mask);
        result = of_set_ipv6_destination(msg, 
                  (ipv6_addr_t *) &selectors->dest_ip.prefixaddr.v6.ipv6addr.b_addr,
                  TRUE, &ipv6_addr_mask);
        if(result != OFU_SET_FIELD_SUCCESS)
        {
          OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                     "OF Set IPv6 dest field failed, error = %d",result);
          return OF_FAILURE;
        }
      }  
    }
    else /* RANGE */ 
    {
      memset(&ipv6_addr_mask, 0, sizeof(ipv6_addr_t));
      if((memcmp(&selectors->dest_ip.rangeaddr.v6.start.b_addr,
                    &ipv6_addr_mask,
                    sizeof(struct ipv6_address))) ||
         (memcmp(&selectors->dest_ip.rangeaddr.v6.end.b_addr,
                    &ipv6_addr_mask,
                    sizeof(struct ipv6_address))))
      {
        result = of_set_ipv6_destination(msg, 
                   (ipv6_addr_t *)&selectors->dest_ip.rangeaddr.v6.start.b_addr,
                   TRUE, (ipv6_addr_t *)&selectors->dest_ip.rangeaddr.v6.end.b_addr);

        if(result != OFU_SET_FIELD_SUCCESS)
        {
          OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                     "OF Set IPv6 dest field failed, error = %d",result);
          return OF_FAILURE;
        }
      } 
    }
  }

  /* Protocol */
  if(selectors->protocol_choice.protocol)
  {
    result = of_set_protocol(msg, &selectors->protocol_choice.protocol);
    if(result != OFU_SET_FIELD_SUCCESS)
    {
      OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                     "OF Set Protocol field failed, error = %d",result);
      return OF_FAILURE;
    } 
  }
  /* Source and Destination Ports */
  if(selectors->protocol_choice.protocol)
  {
    switch (selectors->protocol_choice.protocol)
    { 
      case OF_CNTLR_IP_PROTO_TCP:
      {
        if(selectors->protocol_choice.src_port.start)
        {
          result = of_set_tcp_source_port(msg,
                    &selectors->protocol_choice.src_port.start);
          if (result != OFU_SET_FIELD_SUCCESS)
          {
            OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                       "OF Set TCP Src Port field failed,error = %d",result);
            return OF_FAILURE;
          }
        }
        if(selectors->protocol_choice.dest_port.start)
        {
          result = of_set_tcp_destination_port(msg, 
                      &selectors->protocol_choice.dest_port.start);
          if (result != OFU_SET_FIELD_SUCCESS)
          {
            OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                       "OF Set TCP Dest Port field failed, error = %d",result);
            return OF_FAILURE;
          }
        }
        break;
      }
  
      case OF_CNTLR_IP_PROTO_UDP:
      {
        if(selectors->protocol_choice.src_port.start)
        {
          result = of_set_udp_source_port(msg, 
                     &selectors->protocol_choice.src_port.start);
          if (result != OFU_SET_FIELD_SUCCESS)
          {
            OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                       "OF Set udp Src Port field failed,err = %d",result);
            return OF_FAILURE;
          }
        }
        if(selectors->protocol_choice.dest_port.start)
        {
          result = of_set_udp_destination_port(msg,
                     &selectors->protocol_choice.dest_port.start);
          if (result != OFU_SET_FIELD_SUCCESS)
          {
            OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                       "OF Set udp Dst Port field failed,err = %d",result);
            return OF_FAILURE;
          }
        }
        break;
      }
  
      case OF_CNTLR_IP_PROTO_ICMP:
      {
        uint8_t type, code;
        if(selectors->protocol_choice.src_port.start)
        {
          type = (uint8_t)selectors->protocol_choice.src_port.start;
          result = of_set_icmpv4_type(msg, &type);
          if (result != OFU_SET_FIELD_SUCCESS)
          {
            OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                       "OF Set ICMP Type field failed,err = %d",result);
            return OF_FAILURE;
          }
        }
        if(selectors->protocol_choice.dest_port.start)
        {
          code = (uint8_t)selectors->protocol_choice.dest_port.start;
          result = of_set_icmpv4_code(msg, &code);
          if (result != OFU_SET_FIELD_SUCCESS)
          {
            OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                       "OF Set ICMP Code field failed,err = %d",result);
            return OF_FAILURE;
          }
        }
        break;
      }
  
      default:
      {
        OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
                 "Un supported selector protocol (%d)",
                 selectors->protocol_choice.protocol);
        return OF_FAILURE;
      }
    }
  }
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OF_SUCCESS;
}

/***************************************************************************
 * Function Name : of_ipsec_calculate_selector_msg_len 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
 * NOTE : Openflow Range is not defind, currently it treated as subnet
 ***************************************************************************/
inline int32_t of_ipsec_calculate_selector_msg_len(struct ipsec_selector *selectors,
                                     uint16_t *out_msg_len)
{
  uint16_t        msg_len = 0;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  cntlr_assert(selectors);

  /* Source and Destination IP Address */
  if(selectors->ip_version == IP_V4)
  {
    /* Source Ip Address */
    if(selectors->src_ip.addr_type == IPSEC_ADDR_TYPE_SUBNET)
    { 
      msg_len +=  OFU_IPV4_SRC_MASK_LEN;
    }
    else /* RANGE */ 
    {
      msg_len +=  OFU_IPV4_SRC_MASK_LEN;
    }

    /* Destination Ip Address */
    if(selectors->dest_ip.addr_type == IPSEC_ADDR_TYPE_SUBNET)
    { 
      msg_len +=  OFU_IPV4_DST_MASK_LEN;
    }
    else /* RANGE */ 
    {
      msg_len +=  OFU_IPV4_DST_MASK_LEN;
    }
  }
  else if(selectors->ip_version == IP_V6)
  {
    /* Source Ip Address */
    if(selectors->src_ip.addr_type == IPSEC_ADDR_TYPE_SUBNET)
    { 
      msg_len +=  OFU_IPV6_DST_MASK_LEN;
    }
    else /* RANGE */ 
    {
      msg_len +=  OFU_IPV4_DST_MASK_LEN;
    }

    /* Destination Ip Address */
    if(selectors->dest_ip.addr_type == IPSEC_ADDR_TYPE_SUBNET)
    { 
      msg_len +=  OFU_IPV6_DST_MASK_LEN;
    }
    else /* RANGE */ 
    {
      msg_len +=  OFU_IPV6_DST_MASK_LEN;
    }
  }
  else
  {
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
               "Invalid selector address type, type(%d)",
               selectors->ip_version);
    return OF_FAILURE;
  }
  /* Protocol */
  if(selectors->protocol_choice.protocol)
  {
    msg_len += OFU_IP_PROTO_FIELD_LEN;
  }
  else
  {
    *out_msg_len = msg_len;
    OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_DEBUG, "out msg len = %d", msg_len);
    return OF_SUCCESS;
  }
  /* Source and Destination Ports */
  switch (selectors->protocol_choice.protocol)
  { 
    case OF_CNTLR_IP_PROTO_TCP:
    {
      msg_len += OFU_TCP_SRC_FIELD_LEN + OFU_TCP_DST_FIELD_LEN;
      break;
    }
    case OF_CNTLR_IP_PROTO_UDP:
    {
      msg_len += OFU_UDP_SRC_FIELD_LEN + OFU_UDP_DST_FIELD_LEN;
      break;
    }
    case OF_CNTLR_IP_PROTO_ICMP:
    {
      msg_len += OFU_ICMPV4_TYPE_FIELD_LEN + OFU_ICMPV4_CODE_FIELD_LEN;
      break;
    }
    default:
    {
      OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_ERROR,
               "Un supported selector protocol (%d)",
               selectors->protocol_choice.protocol);
       return OF_FAILURE;
    }
  }
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_DEBUG, "out msg len = %d", msg_len);

  *out_msg_len = msg_len;
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "EXIT");
  return OF_SUCCESS;
}

/***************************************************************************
 * Function Name : of_ipsec_calculate_sa_ctx_msg_len 
 * Description   : 
 * Parameter     : None 
 * Return value  : None
 * NOTE : Openflow Range is not defind, currently it treated as subnet
 ***************************************************************************/
inline int32_t of_ipsec_calculate_sa_ctx_msg_len(struct ipsec_sa *sa,  uint16_t *out_msg_len) 
{
  uint16_t        msg_len = 0, ii;

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");

  msg_len += FSLX_ACTION_IPSEC_SA_CREATE_LEN;


  if((sa->crypto_params.auth_algo != 0) ||
     (sa->crypto_params.auth_algo != IPSEC_AUTH_ALG_NONE))
  {
    msg_len += IPSEC_SA_AUTH_ALGO_INFO_LEN;
    msg_len += (sa->crypto_params.authkey_len_bits >> 3);
  }

  if(sa->crypto_params.enc_algo != 0)
  {
    msg_len += IPSEC_SA_ENC_ALGO_INFO_LEN;
    if(sa->crypto_params.enc_algo != IPSEC_ENC_ALG_NULL)
      msg_len += (sa->crypto_params.enckey_len_bits >> 3);
  }

  if(sa->crypto_params.comb_algo != 0)
  {
    msg_len += IPSEC_SA_ENC_AUTH_ALGO_INFO_LEN;
    msg_len += (sa->crypto_params.combkey_len_bits >> 3);
  }

//struct fslx_ipsec_sa_enc_algo_iv_info

  if(sa->crypto_params.iv)
  {
    msg_len += IPSEC_SA_BLOCK_OR_IV_INFO_LEN;
    msg_len += (sa->crypto_params.iv_len_bits >> 3);
  }  

  if(sa->nat_info.nat_oa_peer_addr.ip_version == IP_V4)
  {
    msg_len += IPSEC_SA_IPV4_UDP_ENCAP_INFO_LEN;
  }
  else if(sa->nat_info.nat_oa_peer_addr.ip_version == IP_V6)
  {
    msg_len += IPSEC_SA_IPV6_UDP_ENCAP_INFO_LEN;
  }

  for(ii = 0; ii < sa->n_selector; ii++)
  {
    if(sa->selectors[ii].selector.ip_version == IP_V4)
    {
      msg_len += IPSEC_SA_IPV4_SELECTOR_INFO_LEN;
    }
    else
    {
      msg_len += IPSEC_SA_IPV6_SELECTOR_INFO_LEN;
    }
  }

  if(sa->te_addr.ip_version == IP_V4)
  {
    msg_len += IPSEC_SA_IPV4_TUNNEL_ENDS_INFO_LEN;
  }
  else if(sa->te_addr.ip_version == IP_V6)
  {
    msg_len += IPSEC_SA_IPV6_TUNNEL_ENDS_INFO_LEN;
  }

  if(!((sa->dscp_start == 0) && (sa->dscp_end == 0)))
  {
    msg_len += IPSEC_SA_DSCP_RANGE_INFO_LEN;
  }
  *out_msg_len = msg_len;
  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_DEBUG, "out message length = %d",msg_len);

  OF_LOG_MSG(OF_LOG_IPSEC, OF_LOG_INFO, "ENTRY");
  return OF_SUCCESS;
}

#endif /* OPENFLOW_IPSEC_SUPPORT */

