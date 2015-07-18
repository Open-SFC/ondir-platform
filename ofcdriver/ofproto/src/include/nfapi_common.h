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

/**************************************************************************//**
@File          nfapi_common.h

@Description   This file contains the NF-Infra Network Function API
*//***************************************************************************/

#ifndef NFAPI_COMMON_H
#define NFAPI_COMMON__H

typedef uint16_t nsid_t;
typedef uint16_t ns_id;
typedef uint16_t zone;
typedef uint32_t if_id;

#define IP4_MAX_IFACE_NAME_LEN 16

/**************************************************************************//**
@Description	API behavioural flags.
*//***************************************************************************/
enum api_control_flags {
  API_CTRL_FLAG_ASYNC = 1 << 0, /**< If set, API call should be
			asynchronous. Otherwise API call will be synchronous.*/
  API_CTRL_FLAG_NO_RESP_EXPECTED = 1 << 1, /**< If set, no response is
			expected for this API call */
};
typedef uint32_t api_control_flags;

/**************************************************************************//**
@Description	Structure that defines the arguments for the call back API that registered by 
the application to receive packets from DP/AIOP to CP/GPP 
*//***************************************************************************/
struct nfpkt_buf
{
  uint32_t ns_id; /* Name space id*/
  uint32_t ifid; /* pETH interface id*/
  char peth_name[IP4_MAX_IFACE_NAME_LEN];  /*PETH interface name*/ /*Deepthi */
  struct pkt_mbuf *pkt;  /*Packet Buffer from DP */
};

typedef void (*api_resp_cbfn)(void *cbarg, int cbarg_len, void *outargs);

/**************************************************************************//**
@Description	API response argumentes structure.
*//***************************************************************************/
struct api_resp_args {
	api_resp_cbfn cbfn; /**< Response callback function pointer */
	void *cbarg;	/**< Pointer to response callback argument data */
	int32_t cbarg_len;/**< Length of callback argument data */
};

#endif /* NFAPI_COMMON_H */
