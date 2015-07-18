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
@File          fsl_nfapi_ipsec.h

@Description   This file contains the IPSec NF APIs,related macros and 
	       data structures.
*//***************************************************************************/

#ifndef __IPSEC_API_H
#define __IPSEC_API_H

#define BIT(x)	(1<<((x)))

enum ip_version {
  IP_V4 = 1,
  IP_V6 = 2
};

/**************************************************************************//**
@Description	 IPv4 address
*//***************************************************************************/
typedef uint32_t ipv4_address;

/**************************************************************************//**
@Description	IPv6 address
*//***************************************************************************/
 struct ipv6_address{
	union {
		uint8_t b_addr[16];
			/* Byte addressable v6 address*/
		uint32_t w_addr[4];
			/*Word addressable v6 address */
	};
};

/**************************************************************************//**
@Description	 IP address
*//***************************************************************************/
struct ip_addr {
	uint8_t ip_version; /* IP Version v4/V6 */
	union {
		ipv4_address ip4; /* IPv4 Address */
		struct ipv6_address ip6; /* IPv6 Address */
	};
};


/**************************************************************************//**
@Description	SPD Policy/SA direction information
*//***************************************************************************/
enum ipsec_direction {
	IPSEC_INBOUND =1, /* Inbound Direction */
	IPSEC_OUTBOUND /* Outbound Direction */
};


/**************************************************************************//**
@Description	SPD Policy Action information
*//***************************************************************************/
enum ipsec_policy_rule_action {
	IPSEC_POLICY_ACTION_IPSEC = 1, /* Apply IPSec processing on Packet*/
	IPSEC_POLICY_ACTION_DISCARD, /* Discard or Drop the packet */
	IPSEC_POLICY_ACTION_BYPASS /* Bypass/Allow to pass the packet */
};

/**************************************************************************//**
@Description	SPD Policy Position information
*//***************************************************************************/
enum ipsec_policy_rule_position{
	IPSEC_POLICY_POSITION_BEGIN = 1, /* Add beginning of the list */
	IPSEC_POLICY_POSITION_BEFORE, /* Add before the mentioned Policy */
	IPSEC_POLICY_POSITION_AFTER, /* Add after the mentioned Policy */
	IPSEC_POLICY_POSITION_END /* Add end of the list */
};


/**************************************************************************//**
@Description	DSCP Range information
*//***************************************************************************/
struct ipsec_policy_rule_dscprange {
	uint8_t start; /* Start value in Range */
	uint8_t end; /* End value  in Range*/
};

/**************************************************************************//**
@Description	Fragmentation Options information
*//***************************************************************************/
enum ipsec_policy_handle_fragments_opts {
	IPSEC_POLICY_FRAGOPTS_REASSEMBLE_BEFORE_IPSEC=0,
		/* IPSec Policy for Frag Option to Reassemble
		before IPsec.*/

	IPSEC_POLICY_FRAGOPTS_SAMESA_FOR_NONINITIAL_FRAG,
		/* IPSec Policy for Frag option for same SA for
	non-initial fragments.*/

	IPSEC_POLICY_FRAGOPTS_STATEFULL_FRAG_CHECK,
		/*IPSec Policy for Frag option statefull
		fragmentation check.*/

	IPSEC_POLICY_FRAGOPTS_SEPARATESA_FOR_NONINITIAL_FRAG
		/* IPSec Policy for Frag option for separate
	SA for non-initial fragments.*/
};

/**************************************************************************//**
@Description	Fragmentation Before Encapsulation (Redside Fragmentation)
*//***************************************************************************/
enum ipsec_policy_redside_fragmentation {
	IPSEC_POLICY_REDSIDE_FRAGMENTATION_DISABLE = 0,
		/* Redside fragmentation disable in IPSec Policy */
	IPSEC_POLICY_REDISIDE_FRAGMENTATION_ENABLE
		/* Redside fragmentation enable in IPSec Policy */
};


/**************************************************************************//**
@Description	Prefix and IPv4 address
*//***************************************************************************/
struct ipv4_prefix{
	ipv4_address ipv4addr; /* IPv4 address */
	uint8_t ipv4plen; /* Prefix length in bits (1-32) */
};

/**************************************************************************//**
@Description	IPv4 Range Address
*//***************************************************************************/
struct ipsec_ipv4_rangeaddr{
	ipv4_address start; /* Start address in Range */
	ipv4_address end;  /* End address in Range*/
};

/**************************************************************************//**
@Description	IPv6 Range Address
*//***************************************************************************/
struct ipsec_ipv6_rangeaddr{
	struct ipv6_address  start; /* Start address in Range*/
	struct ipv6_address  end; /* End Address in Range */
};


/**************************************************************************//**
@Description	Range Address
*//***************************************************************************/
typedef union {
	struct ipsec_ipv4_rangeaddr v4; /* IPv4 Range */
	struct ipsec_ipv6_rangeaddr v6; /* IPv6 Range */
} ipsec_rangeaddr;

/**************************************************************************//**
@Description	Port Range 
*//***************************************************************************/
struct ipsec_portrange{
	uint16_t start; /* First port in range */
	uint16_t end; /* Last port in range */
};

/**************************************************************************//**
@Description	Prefix and IPv6 address
*//***************************************************************************/
struct ipv6_prefix{
	struct ipv6_address ipv6addr;/* IPv6 address */
	uint8_t       ipv6plen;/* Prefix length in bits (1-128) */
};

/**************************************************************************//**
@Description	Prefix and IP address
*//***************************************************************************/
typedef union {
	struct ipv4_prefix v4; /* IPv4 prefix address */
	struct ipv6_prefix v6; /* IPv6 prefix address */
} ipsec_prefix;

/**************************************************************************//**
@Description	Selector Address type
*//***************************************************************************/
enum ipsec_selector_addr_type {
	IPSEC_ADDR_TYPE_SUBNET=0,
		/* Address type is Subnet */
	IPSEC_ADDR_TYPE_RANGE
		/* Address type is Range */ 
} ;
/**************************************************************************//**
@Description	SPD Policy Selector Address
*//***************************************************************************/
struct ipsec_selector_addr {
	enum ipsec_selector_addr_type addr_type; 
		/*  Address type: Subnet or Range */
	union {
		ipsec_prefix prefixaddr; 
			/* Prefix Address, valid when address type is Subnet */
		ipsec_rangeaddr  rangeaddr; 
			/* Range address, valid when address type Range  */
	};
};

/**************************************************************************//**
@Description	Protocol Choice 
*//***************************************************************************/
struct ipsec_selector_protocol_choice {
	uint8_t protocol; /* IP Transport Protocol or ANY */
	struct ipsec_portrange src_port; 
		/* IP Source Port or ICMP Type(If protocol is ICMP) */
	struct ipsec_portrange dest_port;  
		/* IP Destination Port or ICMP code(If protocol is ICMP) */	
};

/**************************************************************************//**
@Description	IPSec Selector  information
*//***************************************************************************/
struct ipsec_selector {
	uint8_t ip_version; 
		/* 4 or 6 : IPv4 or IPv6 */
	struct ipsec_selector_protocol_choice protocol_choice; 
		/* Protocol Choice: Contains Protocol and Port Information */
	struct ipsec_selector_addr src_ip; 
		/*  Source IP */
	struct ipsec_selector_addr dest_ip; 
		/* Destination IP */
};


/**************************************************************************//**
@Description	SPD Policy Policy Status information
*//***************************************************************************/
enum ipsec_policy_rule_status{
	IPSEC_POLICY_STATUS_ENABLE = 0, /* IPSec Policy enable */
	IPSEC_POLICY_STATUS_DISABLE /* IPSec Policy disable */
};

/**************************************************************************//**
@Description	SPD Policy Parameters information
*//***************************************************************************/
struct ipsec_policy {
	uint32_t policy_id; 
	/* SPD Policy ID which is uniquely identifies the 
	policy in given Tunnel Id and Name Space Id*/

	enum ipsec_policy_rule_action policy_action; /* SPD Policy Action */
	enum ipsec_policy_rule_status status; /* SPD Policy Status */
	enum ipsec_policy_rule_position policy_position; /* Policy Position */

	uint8_t n_dscp_range; /* Number of DSCP Ranges */
	struct ipsec_policy_rule_dscprange *dscp_ranges; 
		/* Array of DSCP Ranges */

	enum ipsec_policy_redside_fragmentation redside;
		/* Fragmentation before Encapsulation option: TRUE/FALSE */
	enum ipsec_policy_handle_fragments_opts handle_fragments_opts; 
		/* Fragment handling options */
	uint32_t n_selector; /* Number of selectors */
	struct ipsec_selector *selectors; /* Array of Selectors */
};

/**************************************************************************//**
@Description	IPSec API error codes
*//***************************************************************************/
enum ipsec_api_error_code {
	IPSEC_INVALID_NAME_SPACE_ID =1,
	IPSEC_RESOURCE_NOT_AVAILABLE,

	IPSEC_OUT_SPD_NOT_FOUND ,
	IPSEC_OUT_SPD_ALREADY_EXISTS,
	
	IPSEC_IN_SPD_NOT_FOUND ,
	IPSEC_IN_SPD_ALREADY_EXISTS,


	IPSEC_OUT_SA_NOT_FOUND,
	IPSEC_OUT_SA_ALREADY_EXISTS,
	
	IPSEC_IN_SA_NOT_FOUND,
	IPSEC_IN_SA_ALREADY_EXISTS,


	IPSEC_ICMP_POL_NOT_FOUND ,
	IPSEC_ICMP_POL_ALREADY_EXISTS,

	/* TBD */



};

/**************************************************************************//**
@Description	SPD configuration operations
*//***************************************************************************/
enum ipsec_spd_cfg_op {
	IPSEC_SPD_ADD = 0, /* New SPD Policy addition */
	IPSEC_SPD_MOD, /* Modifying existing SPD Policy */
	IPSEC_SPD_DEL /* Deleting existing SPD policy */
};


/**************************************************************************//**
@Description	Input parameters to SPD Policy configuration
*//***************************************************************************/
struct ipsec_spd_cfg_inargs{
	uint32_t tunnel_id;  /* Tunnel Id */ 
	enum ipsec_direction dir; /* Direction: Inbound our Outbound */
	enum ipsec_spd_cfg_op operation; /* Operation mentions add/mod/del */
	uint32_t policy_id;   
	/* Policy Index which is uniquely identifies 
	the policy in given Name Space and Tunnel Instance */
	struct ipsec_policy *spd_params; /* Policy details */
};

/**************************************************************************//**
@Description	Output parameters to SPD Policy configuration
*//***************************************************************************/
struct ipsec_spd_cfg_outargs{
	int32_t result; 
		/* 0:Sucess; Non Zero value: Error code indicating failure */
};

/**************************************************************************//**
@Function	ipsec_spd_config

@Description	This API is used for Inbound/Outbound SPD configuration for 
		add or modify or delete operations. This database 
		is maintained per Name Space and Tunnel instance. 
		This function first validates the incoming parameters 
		and if all validations succeed, the following is performed 
		depending on the type of operation:
		if operation is add, new SPD policy is added to the database.
		if operation is modify, finds the entry in the database 
		with given information and modifies it.
		if operation is delete, finds the entry in the database
		with given information and deletes it. 
		
@Param[in]	nsid - NamesSpace ID.
@Param[in]	flags - API behavioural flags.
@Param[in]	in - Pointer to input param structure 
		which contains  spd policy information.
@Param[in]	resp - Response arguments for asynchronous call.

@Param[out]	out - Pointer to output param structure
		that will be filled with output values of this API.

@Return		Success or Failure (more errors TBD)

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_spd_config(
	ns_id nsid,
	api_control_flags flags,
	struct ipsec_spd_cfg_inargs *in,
	struct ipsec_spd_cfg_outargs *out,
	struct api_resp_args *resp);


/**************************************************************************//**
@Description  SPD Policy Statistics information structure 
*//***************************************************************************/
struct ipsec_spd_stats {
	uint64_t pkts_rcvd; 
		/* Received Outbound/Inbound Pkts */
	uint64_t pkts_processed; 
		/* Processed Outbound/Inbound Pkts */
	uint64_t sec_apply_pkts_rcvd; 
		/* Received Outbound/Inbound Pkts to apply security */
	uint64_t sec_applied_pkts; 
		/* Outbound/Inbound Pkts applied security */
	uint64_t bytes_processed_on_sa; 
		/* Number of bytes processed on Inbound/Outbound SA */
	
	uint32_t no_tail_room; /* Buffer has no tail room */
	uint32_t invalid_ipsec_pkt; /* Invalid IPSec(ESP/AH) packet */
	uint32_t inner_pkt_is_not_ipv4_or_ipv6_pkt; 
		/* Decrypted packet is not IPv4 or IPv6 packet Packet */
	uint32_t invalid_pad_length; /* Invalid pad length */
		
	uint32_t submit_to_sec_failed; 
		/* Submission to SEC failed for crypto operations */
	uint32_t invalid_seq_number;
		/* Invalid Sequence number received */
	uint32_t anti_replay_late_pkt;
		/* Anti Replay Check: Late packet */
	uint32_t anti_replay_replay_pkt;
		/* Anti Replay Check : Replay packet */
	uint32_t invalid_icv;
		/* ICV comparison failed */
	uint32_t crypto_op_failed; /* Crypto operations failed */
	uint32_t seq_num_over_flow; /* Sequence number over flow */
	uint32_t sa_sel_verify_failed; /* SA selector verificationfailed */
	uint32_t icmp_sent_for_pmtu;
		/* ICMP error message sent for PMTU */
	uint32_t sa_hard_life_time_expired;
		/* SA hard life time expired */
	uint32_t no_matching_dscp_range;
		/* Matching dscp range not found in the SPD policy */
		
	/* TBD */
};

/**************************************************************************//**
@Description	SPD fetch operations
*//***************************************************************************/
enum ipsec_spd_get_op {
	IPSEC_SPD_GET_FIRST = 0, /* Fetch first entry in the database */
	IPSEC_SPD_GET_NEXT, /* Fetch next entry for given SPD policy */
	IPSEC_SPD_GET_EXACT /* Fetch entry for given SPD policy*/
};

/**************************************************************************//**
@Description	Input parameters to fetch SPD Policy information
*//***************************************************************************/
struct ipsec_spd_get_inargs{
	uint32_t tunnel_id;  /* Tunnel Id */ 
	enum ipsec_direction dir; /* Direction: Inbound our Outbound */
	enum ipsec_spd_get_op operation; /* Operation mentions get_first/
		get_next/get_exact */
	uint32_t policy_id;   
	/* Policy Index which is uniquely identifies 
	the policy in given Name Space and Tunnel Instance.  
	Not valid/filled for get_first */

};

/**************************************************************************//**
@Description	Output parameters to fetch SPD Policy information
*//***************************************************************************/
struct ipsec_spd_get_outargs{
	int32_t result; /* 0:Sucess; Non Zero value: Error code indicating failure */
	struct ipsec_policy *spd_params; /* Policy details */
	struct ipsec_spd_stats stats; /* SPD policy related stats */
};

/**************************************************************************//**
@Function	ipsec_spd_get

@Description	This API is used to fetch Inbound/Outbound SPD information. 
		This database is maintained per Name Space and Tunnel instance. 
		This function first validates the incoming parameters 
		and if all validations succeed, the following is performed 
		depending on the type of operation:
		if operation is get_first, fetches first entry information 
		from SPD database.
		if operation is get_next, finds the entry in the SPD database 
		with given information and returns the next entry.
		if operation is get_exact, finds the entry and returns it.

@Param[in]	nsid - NamesSpace ID.		
@Param[in]	flags - API behavioural flags.
@Param[in]	in - Pointer to input param structure 
		which contains  spd policy information.
@Param[in]	resp - Response arguments for asynchronous call.

@Param[out]	out - Pointer to output param structure
		that will be filled with output values of this API.

@Return		Success or Failure (more errors TBD)

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_spd_get(
	ns_id nsid,
	api_control_flags flags,
	struct ipsec_spd_get_inargs *in,
	struct ipsec_spd_get_outargs *out,
	struct api_resp_args *resp);


/**************************************************************************//**
@Function	ipsec_spd_flush

@Description	This API is used to flush/delete all Inbound and Outbound SPD 
		policies in a given name space.

@Param[in]	nsid - NamesSpace ID.		
@Param[in]	flags - API behavioural flags.
@Param[in]	resp - Response arguments for asynchronous call.

@Return		Success or Failure (more errors TBD)

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_spd_flush(
	ns_id nsid,
	api_control_flags flags,
	struct api_resp_args *resp);

/**************************************************************************//**
@Description	ICMP Type & Code Policy configuration operations
*//***************************************************************************/
enum ipsec_icmp_cfg_op {
	IPSEC_ICMP_ADD = 0, /* New ICMP Type & Code Policy addition */
	IPSEC_ICMP_MOD, /* Modifying existing ICMP Type & Code Policy */
	IPSEC_ICMP_DEL /* Deleting existing ICMP Type & Code policy */
};

/**************************************************************************//**
@Description	Input parameters to  ICMP Message Type and Code Policy 
		Configuration
*//***************************************************************************/
struct ipsec_icmp_msg_typecode_cfg_inargs{
	enum ipsec_icmp_cfg_op operation; /* Operation mentions add/mod/del */
	uint8_t entry_id;  
	   /* Index, which is used to identify the policy uniquely 
 		in the database  */
	uint8_t ip_version; /* IP version 4 or 6 : IPv4 or IPV6 */
	uint8_t type; /* ICMP Type */
	uint8_t start_code; /* ICMP start code */
	uint8_t end_code; /* ICMP start code */
};

/**************************************************************************//**
@Description	Output parameters to add ICMP Message Type and Code Policy
*//***************************************************************************/
struct ipsec_icmp_msg_typecode_cfg_outargs {
	int32_t result; 
		/* 0:Sucess; Non Zero value: Error code indicating failure */
};


/**************************************************************************//**
@Function	ipsec_icmp_msg_typecode_config

@Description	This API is used for ICMP Message Type and Code configuration 
		for add or modify or delete operations. This database 
		is maintained per Name Space instance. 
		This function first validates the incoming parameters 
		and if all validations succeed, the following is performed 
		depending on the type of operation:
		if operation is add, new ICMP policy is added to the database.
		if operation is modify, finds the entry in the database 
		with given information and modifies it.
		if operation is delete, finds the entry in the database
		with given information and deletes it. 
		
@Param[in]	nsid - NamesSpace ID.
@Param[in]	flags - API behavioural flags.
@Param[in]	in - Pointer to input param structure 
		which contains  ICMP message type and code information.
@Param[in]	resp - Response arguments for asynchronous call.

@Param[out]	out - Pointer to output param structure
		that will be filled with output values of this API.

@Return		Success or Failure (more errors TBD)

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_icmp_msg_typecode_config(
	ns_id nsid,
	api_control_flags flags,
	struct ipsec_icmp_msg_typecode_cfg_inargs *in,
	struct ipsec_icmp_msg_typecode_cfg_outargs *out,
	struct api_resp_args *resp);

/**************************************************************************//**
@Description	SPD fetch operations
*//***************************************************************************/
enum ipsec_icmp_get_op {
	IPSEC_ICMP_GET_FIRST = 0, 
		/* Fetch first entry in the database */
	IPSEC_ICMP_GET_NEXT, 
		/* Fetch next entry for given ICMP Type & Code policy */
	IPSEC_ICMP_GET_EXACT 
		/* Fetch entry for given ICMP Type & Code policy*/
};

/**************************************************************************//**
@Description	Input parameters to fetch 
		ICMP Message Type and Code Policy Configuration
*//***************************************************************************/
struct ipsec_icmp_msg_typecode_get_inargs{
	enum ipsec_icmp_get_op operation; /* Operation mentions get_first/
		get_next/get_exact */
	uint8_t entry_id;  
	   /* Index, which is used to identify the policy uniquely in the 
 		database. Not valid/Filled for get_first operation */
};

/**************************************************************************//**
@Description	Output parameters to fetch 
		ICMP Message Type and Code Policy Configuration
*//***************************************************************************/
struct ipsec_icmp_msg_typecode_get_outargs {
	int32_t result; 
		/* 0:Sucess; Non Zero value: Error code indicating failure */
	uint8_t ip_version; /* IP version 4 or 6 : IPv4 or IPV6 */
	uint8_t type; /* ICMP Type */
	uint8_t start_code; /* ICMP start code */
	uint8_t end_code; /* ICMP start code */
};

/**************************************************************************//**
@Function	ipsec_icmp_msg_typecode_get

@Description	This API is used to fetch ICMP Message Type and Code 
		information. 
		This database is maintained per Name Space and Tunnel instance. 
		This function first validates the incoming parameters 
		and if all validations succeed, the following is performed 
		depending on the type of operation:
		if operation is get_first, fetches first entry information 
		from ICMP database.
		if operation is get_next, finds the entry in the ICMP database 
		with given information and returns the next entry.
		if operation is get_exact, finds the entry and returns it.

@Param[in]	nsid - NamesSpace ID.		
@Param[in]	flags - API behavioural flags.
@Param[in]	in - Pointer to input param structure 
		which contains  ICMP record information.
@Param[in]	resp - Response arguments for asynchronous call.

@Param[out]	out - Pointer to output param structure
		that will be filled with output values of this API.

@Return		Success or Failure (more errors TBD)

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_icmp_msg_typecode_get(
	ns_id nsid,
	api_control_flags flags,
	struct ipsec_icmp_msg_typecode_get_inargs *in,
	struct ipsec_icmp_msg_typecode_get_outargs *out,
	struct api_resp_args *resp);


/**************************************************************************//**
@Description	SA Selector information  
*//***************************************************************************/
struct ipsec_sa_selector {
	uint32_t spd_policy_id;   /*  Corresponding SPD Policy Index */
	struct ipsec_selector  selector; 
	/* Selector information for which SA processing required */
};

/**************************************************************************//**
@Description	Nat Version information  
*//***************************************************************************/
enum ipsec_nat_version {
	IPSEC_IKE_NATV1 =1, /* UDP encapsulation using  port 500 */
	IPSEC_IKE_NATV2 /* UDP encapsulation using port 4500  */
};

/**************************************************************************//**
@Description	NAT Version and Port information
*//***************************************************************************/
struct ipsec_nat_info{
	enum ipsec_nat_version version; /* Nat Version */
	uint16_t dest_port; /* Destination Port */
	uint16_t src_port; /* Source Port */
	struct ip_addr nat_oa_peer_addr; 
	/* Original Peer address, valid if encapsulation mode is Transport */
};


/**************************************************************************//**
@Description	Tunnel End Point information
*//***************************************************************************/
struct ipsec_tunnel_end_addr{
	uint8_t  ip_version; /* IPVersion v4/v6 */
	struct ip_addr src_ip; /* Tunnel Source Address */
	struct ip_addr dest_ip; /* Tunnel End Address */
};

/**************************************************************************//**
@Description	IPSec Security Protocol Macros
*//***************************************************************************/

#define IPSEC_PROTOCOL_ESP 50 /* IPSec Protocol is ESP */
#define IPSEC_PROTOCOL_AH  51 /* IPSec Protocol is AH */

/**************************************************************************//**
@Description	IPSec SA DF bit handle values
*//***************************************************************************/
enum ipsec_df_bit_handle {
	IPSEC_DF_COPY = 1,
		/* Handle DF bit.  Copy DF bit from inner to outer header */
	IPSEC_DF_CLEAR,
		/* Handle DF bit.  Clear DF bit in outer header */
	IPSEC_DF_SET
		/* Handle DF bit.  Set DF bit in outer header */
};

/**************************************************************************//**
@Description	IPSec SA DSCP handle values
*//***************************************************************************/
enum ipsec_dscp_handle {
	IPSEC_DSCP_COPY = 1,
		/* Copy DSCP value from inner to outer header */
	IPSEC_DSCP_CLEAR,
		/* Clear(no) DSCP value in outer header */
	IPSEC_DSCP_SET
		/* Set with mentioned DSCP value in outer header */
};

/**************************************************************************//**
@Description	SA flags information
*//***************************************************************************/
enum ipsec_sa_flags {
	IPSEC_SA_REDSIDE_FRAGMENTATION = BIT(1), 
		/* When enabled, packets processed against this
	SA should undergo Red-side fragmentation if the post-encryption
	packet size exceeded the Path MTU.*/

	IPSEC_SA_VERIFY_PKTSEL_WITH_SA_SEL = BIT(2),
		/*When enabled, the decrypted packet's selectors will
	be checked against the SA's selectors, to ensure packet came on
	the right SA.*/

	IPSEC_SA_DO_PEERGW_CHANGE_ADAPTATION = BIT(3),
		/* When enabled, IPSec-DP when it detects changes to peer
	gateway as part of inbound processing, should apply the changes
	for Outbound SA so that traffic through the tunnel will not be
	disrupted. */

	IPSEC_SA_DO_UDP_ENCAP_FOR_NAT_TRAVERSAL = BIT(4),
	/* When enabled, this indicates that IPSec-DP has to do
	UDP encapsulation/decapsulation for IPSec packet so that they
	can traverse through NAT boxes. UDP Encapsulation/ decapsulation
	is to be applied for packets that get processed off this SA.*/

	IPSEC_SA_USE_ESN = BIT(5),
		/* While encrypting and decrypting packets, extended
	sequence number will be used for Anti-replay window check.*/

	IPSEC_SA_PROPOGATE_ECN = BIT(6),
		/* When enabled,  ECN value from the outer tunnel packet
	will get populated to the decrypted packet's IP header, for those
	processed off this SA.*/

	IPSEC_SA_LIFETIME_IN_SEC = BIT(7),
		/* This indicates that the SA life time is in seconds */

	IPSEC_SA_LIFETIME_IN_BYTES = BIT(8),
		/* This indicates that the SA life time is in kilo bytes */

	IPSEC_SA_LIFETIME_IN_PKT_CNT = BIT(9),
		/* This indicates that the SA life time is in packet count */

	IPSEC_SA_DO_ANTI_REPLAY_CHECK = BIT(10),
		/*When enabled, it indicates packets need to be encrypted
	and sent out with valid sequence numbers such that it passes
	Anti-replay window check at the peer gateway. Similarly on the
	inbound side, sequence number in the packet will undergo anti-replay
	checks.*/

	IPSEC_SA_ENCAP_MODE_TRANSPORT = BIT(11)
		/* This indicates whether the encapsulation mode to be
	used on the SA is either transport or tunnel.*/
	
};

/**************************************************************************//**
@Description	Authentication Algorithms information
*//***************************************************************************/
enum ipsec_auth_alg {
	IPSEC_AUTH_ALG_NONE=1, /* No Authentication */
	IPSEC_AUTH_ALG_MD5HMAC, /* MD5 HMAC Authentication Algorithm */
	IPSEC_AUTH_ALG_SHA1HMAC, /* SHA1 HMAC Authentication Algorithm */
	IPSEC_AUTH_ALG_AESXCBC, /* AES-XCBC Authentication Algorithm */
	IPSEC_AUTH_ALG_SHA2_256_HMAC, 
		/* SHA2 HMAC Authentication Algorithm with Key length 256 bits*/
	IPSEC_AUTH_ALG_SHA2_384_HMAC,
		/* SHA2 HMAC Authentication Algorithm with Key length 384 bits*/
	IPSEC_AUTH_ALG_SHA2_512_HMAC,
		/* SHA2 HMAC Authentication Algorithm with Key length 512 bits*/
};

/**************************************************************************//**
@Description	Encryption Algorithms information
*//***************************************************************************/
enum ipsec_enc_alg{
	IPSEC_ENC_ALG_NULL=1, /* NULL Encryption Algorithm */
	IPSEC_ENC_ALG_DES_CBC, /* DES-CBC Encryption Algorithm */
	IPSEC_ENC_ALG_3DES_CBC, /* 3DES-CBC Encryption Algorithm */
	IPSEC_ENC_ALG_AES_CBC, /* AES-CBC Encryption Algorithm */
	IPSEC_ENC_ALG_AES_CTR /* AES-CTR Encryption Algorithm */
};

/**************************************************************************//**
@Description	Combinedmode Algorithms information
*//***************************************************************************/
enum ipsec_combined_alg{
	IPSEC_COMB_AES_CCM=1, /* AES-CCM Combined mode Algorithm */
	IPSEC_COMB_AES_GCM, /* AES-GCM Combined mode Algorithm */
	IPSEC_COMB_AES_GMAC /* AES-GMAC Combined mode Algorithm */
};

/**************************************************************************//**
@Description	SA crypto suite information
*//***************************************************************************/
struct ipsec_sa_crypto_params {
	enum ipsec_auth_alg auth_algo; /* Authentication algorithm */
	uint8_t *authkey; /* Authentication key */
	uint32_t authkey_len_bits; /* Authentication key length in bits */

	enum ipsec_enc_alg enc_algo; /* Encryption algorithm */
	uint8_t *enckey; /* Encryption/decryption Key */
	uint32_t enckey_len_bits; /* Encryption/decryption Key Length in bits. 
		It holds the nonce length (32 bits) followed by 
		the key if encryption algorithm is AES-CTR. */
	
	enum ipsec_combined_alg comb_algo; /* Combined mode/aead algorithm.*/
	uint8_t *combkey; /* Combined mode key.  */
	uint32_t combkey_len_bits; /* Combined mode key length in bits.  
		It holds the salt length followed by the key. */

	uint8_t icv_len_bits; /* ICV-Integrity Checksum Value size in bits */
	uint8_t *iv; /* Intialization Vector */
	uint8_t iv_len_bits; /* IV	length in bits */
};
/**************************************************************************//**
@Description	IPSec SA inbound/outbound information
*//***************************************************************************/
struct ipsec_sa{
	uint32_t spi; /* Security Parameter Index of the SA */
	enum ipsec_sa_flags flags; 
		/* Flags indicates SA related information */
	uint8_t protocol; 
		/* Security Protocol ESP/AH */
	struct ipsec_sa_crypto_params crypto_params;
		/* SA crypto suite information */
	uint32_t periodc_time_interval; 
		/* Periodic update interval value in seconds */
	uint64_t soft_bytes_limit; /* Soft bytes Expiry */
	uint64_t hard_bytes_limit; /* Hand bytes Expiy */
	uint64_t soft_packet_limit; /* Soft Packet count Expiry */
	uint64_t hard_packet_limit; /* Hard Packet count Expiry */
	uint64_t soft_seconds_limit; /* Soft Seconds Expiry */
	uint64_t hard_seconds_limit; /* Hard Second Expiry */
	uint8_t replaywindow_size; /* AntiReplay Windo Size in bytes*/
	uint8_t dscp; /* DSCP value, valid when DSCP handle is SET */
	uint16_t mtu; /* Maximum transmission unit.  */
	uint8_t dscp_start;
		/* DSCP start value.  Valid for only Outbound SA and 
		SA per DSCP option is enabled in the SPD Policy*/
	uint8_t dscp_end; 
		/* DSCP end value.  Valid for only Outbound SA and 
		SA per DSCP option is enabled in the SPD Policy*/
	uint32_t out_spi; 
		/* Outbound SA SPI value. Valid for only Inbound SA. */
	uint32_t n_selector; /* Number of selectors */
	struct ipsec_sa_selector *selectors; /* Array of Selectors */
	struct ipsec_nat_info nat_info; /* NAT port information */
	struct ipsec_tunnel_end_addr te_addr; /* Tunnel End Point address */
};

/**************************************************************************//**
@Description	SA configuration operations
*//***************************************************************************/
enum ipsec_sa_cfg_op {
	IPSEC_SA_ADD = 0, /* New SA addition */
	IPSEC_SA_DEL /* Deleting existing SA */
};

/**************************************************************************//**
@Description	Input parameters for SA configuration
*//***************************************************************************/
struct ipsec_sa_cfg_inargs{
	uint32_t tunnel_id;  /* Tunnel Id */ 
	enum ipsec_direction dir; /* Direction: Inbound our Outbound */
	enum ipsec_sa_cfg_op operation; /* Operation mentions add/mod/del */	
	/* Following three fields are used to identify SA.  
 		Valid/Filled for del operation */
	uint32_t spi; /* SPI Value */
	struct ip_addr dest_ip; /* Destination Gateway Address */
	uint8_t protocol; /* Security Protocol (ESP/AH) */
	struct ipsec_sa  *sa_params; 
		/* Pointer to SA Parameters.  Valid/Filled for add operation */

};

/**************************************************************************//**
@Description	Output parameters for  SA configuration
*//***************************************************************************/
struct ipsec_sa_cfg_outargs{
	int32_t result; 
		/* 0:Sucess; Non Zero value: Error code indicating failure */
};

/**************************************************************************//**
@Function	ipsec_sa_config

@Description	This API is used for Inbound/Outbound SA configuration for 
		add  or delete operations. This database 
		is maintained per Name Space instance. 
		This function first validates the incoming parameters 
		and if all validations succeed, the following is performed 
		depending on the type of operation:
		if operation is add, new SA is added to the database.
		if operation is delete, finds the entry in the database
		with given information and deletes it. 
		
@Param[in]	nsid - NamesSpace ID.
@Param[in]	flags - API behavioural flags.
@Param[in]	in - Pointer to input param structure 
		which contains  SA information.
@Param[in]	resp - Response arguments for asynchronous call.

@Param[out]	out - Pointer to output param structure
		that will be filled with output values of this API.

@Return		Success or Failure (more errors TBD)

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_sa_config(
	ns_id nsid,
	api_control_flags flags,
	struct ipsec_sa_cfg_inargs *in,
	struct ipsec_sa_cfg_outargs *out,
	struct api_resp_args *resp);


/**************************************************************************//**
@Description	IPSec SPD update flags information
*//***************************************************************************/
enum ipsec_spd_update_flags {
	IPSEC_SA_UPDATE_LOCAL_GW_IFNO = 1,
		/* By setting this flag, local gateway information will updated
		in the given SA */
	IPSEC_SA_UPDATE_PEER_GW_IFNO,
		/* By setting this flag, Peer gateway information will updated
		in the given SA */
	IPSEC_SA_UPDATE_MTU ,
		/* By setting this flag, MTU will be updated in the given SA.
		Valid for Outbound direction SA */ 
	IPSEC_SA_ADD_SEL ,
		/* By setting this flag, Selector will be added 
 			to the given SA. */
	IPSEC_SA_DEL_SEL 
		/* By setting this flag, Selector will be deleted 
 			from the given SA. */

};
/**************************************************************************//**
@Description	Input parameters for SA updation
*//***************************************************************************/
struct ipsec_sa_update_inargs{
	enum ipsec_direction dir; /* Direction: Inbound our Outbound */
	uint32_t spi; /* SPI Value */
	struct ip_addr dest_ip; /* Destination Gateway Address */
	uint8_t protocol; /* Security Protocol (ESP/AH) */
	enum ipsec_spd_update_flags flags;
		/* flags indicating update type */
	union {
		/* Following structure will be filled when update type
		is IPSEC_SA_UPDATE_LOCAL_GW_IFNO or 
		IPSEC_SA_UPDATE_PEER_GW_IFNO */
		struct {
			uint16_t port; /* New port */
			struct ip_addr addr; /* New IP Address */
		}addr_info;
		uint32_t mtu; /* New SA MTU value.  Valid/Filled when update
		type is IPSEC_SA_UPDATE_MTU */
		struct ipsec_sa_selector selector; 
			/* Selector information. Valid/Filled when update 
 			type is IPSEC_SA_ADD_SEL/IPSEC_SA_DEL_SEL */
	};
};

/**************************************************************************//**
@Description	Output parameters for  SA updation
*//***************************************************************************/
struct ipsec_sa_update_outargs{
	int32_t result; 
		/* 0:Sucess; Non Zero value: Error code indicating failure */
};


/**************************************************************************//**
@Function	ipsec_sa_update

@Description	This API updates an Inbound/Outbound SA in the 
		Inbound/Outbound SA database.  
		This database is maintained per Name Space instance. 
		This function first validates the incoming parameters and 
		if all validations succeed, updates the given SA.  

@Param[in]	flags - API behavioural flags.

@Param[in]	nsid - NamesSpace ID.		
@Param[in]	in - Pointer to input param structure which contains required
		infomation to update SA.

@Param[in]	resp - Response arguments for asynchronous call.

@Param[out]	out - Pointer to output param structure
		that will be filled with output values of this API.

@Return		Success or Failure (more errors TBD).

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_sa_update(
	ns_id nsid,
	api_control_flags flags,
	struct ipsec_sa_update_inargs *in,
	struct ipsec_sa_update_outargs *out,
	struct api_resp_args *resp);


/**************************************************************************//**
@Description	SA fetch operations
*//***************************************************************************/
enum ipsec_sa_get_op {
	IPSEC_SA_GET_FIRST = 0, /* Fetch first entry in the database */
	IPSEC_SA_GET_NEXT, /* Fetch next entry for given SA */
	IPSEC_SA_GET_EXACT /* Fetch entry for given SA policy*/
};

/**************************************************************************//**
@Description	Input parameters to fetch SA  information
*//***************************************************************************/
struct ipsec_sa_get_inargs{
	enum ipsec_direction dir; /* Direction: Inbound our Outbound */
	enum ipsec_sa_get_op operation; /* Operation mentions get_first/
		get_next/get_exact */
	/* Following  fields are not valid/filled for get_first */
	uint32_t spi; /* SPI Value */
	struct ip_addr dest_ip; /* Destination Gateway Address */
	uint8_t protocol; /* Security Protocol (ESP/AH) */

};

/**************************************************************************//**
@Description	Output parameters to fetch SA information
*//***************************************************************************/
struct ipsec_sa_get_outargs{
	int32_t result; 
		/* 0:Sucess; Non Zero value: Error code indicating failure */
	struct ipsec_sa *sa_params; /* SA details */
	uint64_t pkts_processed;
		/* Number of Packets processed by SA */
	uint64_t bytes_processed; 
		/* Number of bytes processed by SA */

};

/**************************************************************************//**
@Function	ipsec_sa_get

@Description	This API is used to fetch Inbound/Outbound SA information. This database 
		is maintained per Name Space. 
		This function first validates the incoming parameters 
		and if all validations succeed, the following is performed 
		depending on the type of operation:
		if operation is get_first, fetches first entry information 
		from SA database.
		if operation is get_next, finds the entry in the SA database 
		with given information and returns the next entry.
		if operation is get_exact, finds the entry and returns it.

@Param[in]	nsid - NamesSpace ID.		
@Param[in]	flags - API behavioural flags.
@Param[in]	in - Pointer to input param structure 
		which contains  SA information.
@Param[in]	resp - Response arguments for asynchronous call.

@Param[out]	out - Pointer to output param structure
		that will be filled with output values of this API.

@Return		Success or Failure (more errors TBD)

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_sa_get(
	ns_id nsid,
	api_control_flags flags,
	struct ipsec_sa_get_inargs *in,
	struct ipsec_sa_get_outargs *out,
	struct api_resp_args *resp);

/**************************************************************************//**
@Function	ipsec_sa_flush

@Description	This API is used to flush/delete all Inbound and Outbound SAs 
		in a given name space.

@Param[in]	nsid - NamesSpace ID.		
@Param[in]	flags - API behavioural flags.
@Param[in]	resp - Response arguments for asynchronous call.

@Return		Success or Failure (more errors TBD)

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_sa_flush(
	ns_id nsid,
	api_control_flags flags,
	struct api_resp_args *resp);

/**************************************************************************//**
@Description	Input parameters to fetch Global Statistics 
*//***************************************************************************/
struct ipsec_global_stats_get_inargs {
	/* At present this structure is not containing any fields */
};
/**************************************************************************//**
@Description	Output parameters to fetch Global Statistics 
*//***************************************************************************/
struct ipsec_global_stats_get_outargs {
	int32_t result; 
		/* 0:Sucess; Non Zero value: Error code indicating failure */
	
	uint64_t outb_pkts_rcvd;
		/* Received Outbound Pkts */
	uint64_t outb_pkts_processed;
		/* Processed Outbound Pkts */
	uint64_t outb_sec_apply_pkts_rcvd; 
		/* Received Outound Pkts to apply security */
	uint64_t outb_sec_applied_pkts; 
		/* Outbound Pkts applied security */
	uint64_t inb_pkts_rcvd;
		/* Received Inbound Pkts */
	uint64_t inb_pkts_processed;
		/* Processed Inbound Pkts */
	uint64_t inb_sec_apply_pkts_rcvd; 
		/* Received Inbound Pkts to apply security */
	uint64_t inb_sec_applied_pkts; 
		/* Inbound Pkts applied security */

	uint32_t no_tail_room; 
		/* Buffer has no tail room */
	uint32_t invalid_ipsec_pkt;
		/* Invalid IPSec(ESP/AH) packet */
	uint32_t inner_pkt_is_not_ipv4_or_ipv6_pkt; 
		/* Decrypted packet is not IPv4 or IPv6 packet Packet */
	uint32_t invalid_pad_length; 
		/* Invalid pad length */
		
	uint32_t submit_to_sec_failed; 
		/* Submission to SEC failed for crypto operations */
	uint32_t invalid_seq_number;
		/* Invalid Sequence number received */
	uint32_t anti_replay_late_pkt;
		/* Anti Replay Check: Late packet */
	uint32_t anti_replay_replay_pkt;
		/* Anti Replay Check : Replay packet */
	uint32_t invalid_icv;
		/* ICV comparison failed */
	uint32_t crypto_op_failed; 
		/* Crypto operations failed */
	uint32_t seq_num_over_flow; 
		/* Sequence number over flow */
	uint32_t sa_sel_verify_failed; 
		/* SA selector verificationfailed */
	uint32_t icmp_sent_for_pmtu;
		/* ICMP error message sent for PMTU */
	uint32_t sa_hard_life_time_expired; 
		/* SA hard life time expired */
	uint32_t no_outb_sa; 
		/* Outbound SA not found */
	uint32_t no_inb_sa; 
		/* Inbound bound SA not found */
	uint32_t frag_failed; 
		/* Fragmentation failed */
	uint32_t no_l2blob; 
		/* L2blob is not present in the Outbound SA */

	uint32_t mem_alloc_failed;
		/* Memory allocation failed 
		for SA/SPD/Shared descripted,,etc.. */

	/* TBD */
};

/**************************************************************************//**
@Function	ipsec_global_stats_get

@Description	This API fetches Global Statistics for given Name Space. 

@Param[in]	nsid - NamesSpace ID.		
@Param[in]	flags - API behavioural flags.
@Param[in]	in - Pointer to input param structure which 
			contains Name Space Id
@Param[in]	resp - Response arguments for asynchronous call.

@Param[out]	out - Pointer to output param Structure that will be filled 
		with output values of this API.

@Return		Success or Failure (more errors TBD).

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_global_stats_get(
	ns_id nsid,
	api_control_flags flags,
	struct ipsec_global_stats_get_inargs *in,
	struct ipsec_global_stats_get_outargs *out,
	struct api_resp_args *resp);


/**************************************************************************//**
@Description	In parameters to inject packet for IPSec encryption
*//***************************************************************************/
struct ipsec_encrypt_inject {
	uint32_t tunnel_id;  /* Tunnel Id */ 
	uint32_t policy_id; /* SPD Policy Id */   
	uint32_t spi; /* SPI Value */
	struct ip_addr dest_ip; /* Destination Gateway Address */ 
	uint8_t protocol; /* Security Protocol (ESP/AH) */
	void    *pkt;
	/* TODO : Packet details*/
};

/**************************************************************************//**
@Function	ipsec_encrypt_and_send

@Description	Controlplance application uses this function to request 
		IPSec-DP to encrypt and send the packet out.


@Param[in]	nsid - NamesSpace ID.		
@Param[in]	in - Pointer to input param structure which contains packet 
			details and matching SPD and SA details


@Param[out]	None

@Return		Success or Failure (more errors TBD).

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_encrypt_and_send(ns_id nsid, struct ipsec_encrypt_inject *in);

/**************************************************************************//**
@Description	In parameters to inject packet for IPSec decryption
*//***************************************************************************/
struct ipsec_decrypt_inject {
	void    *pkt;
	/* TODO : Packet details*/

};

/**************************************************************************//**
@Function	ipsec_decrypt_and_send

@Description	Control plane application uses this function to request 
		IPSec-DP to decrypt and send the packet out.
		Details: TBD

@Param[in]	in - Pointer to input param structure which contains 
			packet details.


@Param[out]	None

@Return		Success or Failure (more errors TBD).

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_decrypt_and_send(ns_id nsid, struct ipsec_decrypt_inject *in);

/**************************************************************************//**
@Description	IPSec Module Authentication Algorithm Capabilities 
*//***************************************************************************/
struct ipsec_auth_algo_cap {
	uint32_t  md5:1,
	sha1:1,
	sha2:1,
	aes_xcbc:1,
	none:1;

};

/**************************************************************************//**
@Description	IPSec Module Encryption Algorithm Capabilities 
*//***************************************************************************/
struct ipsec_enc_algo_cap {
	uint32_t des:1,
	des_3:1,
	aes:1,
	aes_ctr:1,
	null:1;
};

/**************************************************************************//**
@Description	IPSec Module Combined mode Algorithm Capabilities 
*//***************************************************************************/
struct ipsec_comb_algo_cap {
	uint32_t aes_ccm:1,
	aes_gcm:1,
	aes_gmac:1;
};


/**************************************************************************//**
@Description	IPSec Module Capabilities 
*//***************************************************************************/
struct ipsec_capabilities { 
	/* This parameter indicates if IPSec-DP is capable of doing SPD
	rule search for incoming or outgoing datagrams.\n*/
	uint32_t
	sel_store_in_spd : 1,
	/*Authentication Header Processing*/
	ah_protocol:1,
	/* ESP Header Processing */
	esp_protocol:1,
	/* IPComp related processing */
	ipcomp_protocol:1,
	/* IPSec Tunnel Mode Processing */
	tunnel_mode:1,
	/* IPSec Tunnel Mode Processing */
	transport_mode:1,
	/* This indicates if IPSec-DP has capability to generate
	(for Outbound) and verify (for Inbound) extended sequence numbers.*/
	esn:1,
	/* This option indicates if IPSec-DP has capability to do ESP
	and AH processing together on a same packet for a given SA.*/
	multi_sec_protocol:1,
	/* This option indicates if IPSec-DP can handle
	packets that need to be processed on SAs for which life
	time in seconds option has been	selected.*/
	lifetime_in_sec:1,
	/* This option indicates if IPSec-DP can handle
	packets that need to be processed on SAs for which
	life time in KB option has been selected. */
	lifetime_in_kbytes:1,
	/* This option indicates if IPSec-DP can handle
	packets that need to be processed on SAs for which
	life time in number of packets option has been selected. */

	lifetime_in_pkt_cnt:1,
	/* This option indicates whether IPSec-DP can
	handle the necessary UDP Encapsulation required at
	IPSec level for traversing NAT boxes.*/
	udp_encap:1,
	/* This option indicates whether IPSec-DP can fragment packets
	before IPSec encryption, so that the resulting IPSec encrypted
	fragments do not exceed MTU*/
	redside_frag:1,
	/* Due to intermediate NAT boxes,
	destination gateway tunnel endpoint address may change.
	Based on SPI value, an IPSec implementation	can typically
	find out if there has been a change in the destination
	tunnel endpoint address. Once a change is detected, the IPSec
	implementation has to update the destination gateway address
	for the	Outbound SA of the tunnel so that outbound traffic
	will continue. This option indicates if IPSec_DP is capable of
	detecting changes in destination gateway address and adapting
	to the same.*/
	peer_gw_adaptation:1,
	/* This option indicates if IPSec-DP is capable of
	detecting changes to the local gateway endpoint address
	and updates its tunnel data	structures.*/
	local_gw_adaptation:1,
	/* This option indicates whether IPSec-DP is capable of providing
	limited Traffic Flow confidentiality */
	tfc:1,
	/* This option indicates whether IPSec_DP is capable of handing
	fragments(Seperate SA for non initial, Statefull fragmentation..) */
	/* */
	frag_options:1,
	/* This option indicates whether IPSec-DP is capable of
	accepting or rejecting the icmp error messages, by searching the
	ICMP record using type and code */
	icmp_error_msg_process:1;
	
	/* Authentication Algorithms supported such as MD5,
	SHA1, AES-XCBC, etc..*/
	struct ipsec_auth_algo_cap    auth_algo_cap;
	/* Encryption Algorithms supported such as DES,
	3DES, AES-CBC,AES-CTR, etc...\n
	These algorithms have to be considered with capabilities
	such as esp_protocol and ah_protocol to understand
	he SA based functionality supported by IPSec-DP.*/
	struct ipsec_enc_algo_cap    enc_algo_cap;
	/* Combined mode Algorithms supported such as aes-ccm,
	aes-gcm, etc... */
	struct ipsec_comb_algo_cap    comb_algo_cap;

	/* Maximum number of Name Spaces supported by IPSec-DP.*/
	uint32_t	     max_name_spaces;
	/* Maximum number of Tunnel interfaces supported
	in each name space.*/
	uint32_t	     max_tunnels;
	/* Indicates the maximum number of IN and OUT
	PDContainers for each name space. For example if the supplied
	value is 64, it indicates IPSec-DP supports 64 IN SPD policies
	and 64 OUT SPD policies.*/
	uint32_t	     max_spd_policies;
	/* Indicates the maximum number of IN and OUT
	IPSec SAs in each name space.  For example if the supplied
	value is 1k, it indicates IPSec-DP supports 1k IN IPSEC SAS
	and 1k OUT IPSec SAs in each name space.*/
	uint32_t	     max_sas;
	/* Maximum number of ICMP policies supported by IPSec-DP.*/
	uint32_t	     max_icmp_policies;


};
/**************************************************************************//**
@Description	Output parameters to fetch IPSec module capabilities 
*//***************************************************************************/
struct ipsec_get_cap_outargs {
	struct ipsec_capabilities cap; /* Module Capabilities */
};
/**************************************************************************//**
@Function	ipsec_get_capabilities

@Description	This API fetches IPSec module Capabilities 

@Param[in]	flags - API behavioural flags.

@Param[out]	out - Pointer to output param structure.
		Capabilities of underlying IPSec module which offers to 
			control plane application.
@Param[in]	resp - Response arguments for asynchronous call.

@Return		Success or Failure (more errors TBD).

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_get_capabilities(
	api_control_flags flags,
	struct ipsec_get_cap_outargs *out,
	struct api_resp_args *resp);

/**************************************************************************//**
@Function	ipsec_api_get_version

@Description	This API fetches   IPSec module API version

@Param[in]	-

@Param[out]	Version - API Version

@Return		Success or Failure (more errors TBD).

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_api_get_version(char *version);

/**************************************************************************//**
@Description	IPSec DP status flag information
*//***************************************************************************/
enum ipsec_status_flag {
	IPSEC_STATUS_ENABLE = 0, /*Set  IPSec-DP status as enable */ 
	IPSEC_STATUS_DISABLE /*Set  IPSec-DP status as disable */
};
	

/**************************************************************************//**
@Function	ipsec_dp_set_status

@Description	This API is used to set IPSec-DP 
		status as enable/disable for given name space. 

@Param[in]	nsid - NamesSpace ID.		
@Param[in]	flags - API behavioural flags.
@Param[in]	status - Status indicating enable/disable.
@Param[in]	resp - Response arguments for asynchronous call.

@Return		Success or Failure (more errors TBD).

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_dp_set_status(
	ns_id nsid,
	enum ipsec_status_flag status,
	api_control_flags flags,
	struct api_resp_args *resp);


/**************************************************************************//**
@Function	ipsec_dp_revalidate

@Description	This API is used to inform IPSec-DP to revalidate its Policies 
		or SAs or any runtime information in a given name space. 
@Param[in]	nsid - NamesSpace ID.		
@Param[in]	flags - API behavioural flags.
@Param[in]	status - Status indicating enable/disable.
@Param[in]	resp - Response arguments for asynchronous call.

@Return		Success or Failure (more errors TBD).

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_dp_revalidate(
	ns_id nsid,
	api_control_flags flags,
	struct api_resp_args *resp);

/**************************************************************************//**
@Description	No outbound SA notification information structure.
*//***************************************************************************/
struct no_out_sa_notification {
	uint32_t tunnel_id;  /* Tunnel Id */ 
	uint32_t policy_id;   /* Outbound SPD Policy ID */
	void      *pkt;
	/* TODO Pkt details*/
};

/**************************************************************************//**
@Description	No Inbound SA notification information structure.
*//***************************************************************************/
struct no_in_sa_notification {
	void      *pkt;
	uint32_t spi; /* SPI Value */
	struct ip_addr dest_ip; /* Destination Gateway Address */
	uint8_t protocol; /* Security Protocol (ESP/AH) */
	/* TODO Pkt details*/
};

/**************************************************************************//**
@Description	Decrypted Self Packet notification information structure.
*//***************************************************************************/
struct self_decrypted_pkt_notification {
	void      *pkt;
	uint32_t spi; /* SPI Value */
	struct ip_addr dest_ip; /* Destination Gateway Address */
	uint8_t protocol; /* Security Protocol (ESP/AH) */
};

/**************************************************************************//**
@Description	SA expiry notification types
*//***************************************************************************/
enum ipsec_sa_expiry_notification_type{
	IPSEC_SA_SOFT_LIFETIME_OUT_BY_SEC = 1, 
		/* Indication for Soft Life time out in Seconds */
	IPSEC_SA_HARD_LIFETIME_OUT_BY_SEC,	
		/* Indication for Hard Life time out in Seconds */
	IPSEC_SA_SOFT_LIFETIME_OUT_BY_BYTES,
		/* Indication for Soft Life time out in Bytes */
	IPSEC_SA_HARD_LIFETIME_OUT_BY_BYTES,
		/* Indication for Hard Life time out in Bytes */
	IPSEC_SA_SOFT_LIFETIME_OUT_BY_PACKETS,
		/* Indication for Soft Life time out in packets */
	IPSEC_SA_HARD_LIFETIME_OUT_BY_PACKETS
		/* Indication for Hard Life time out in packets */
};

/**************************************************************************//**
@Description	SA expiry notification information structure.
*//***************************************************************************/
struct sa_expiry_notification {
	enum ipsec_direction dir; /* Direction: Inbound our Outbound */
	enum ipsec_sa_expiry_notification_type expiry_type;
		/* SA Expirty notification type */
	uint32_t spi; /* SPI Value */
	struct ip_addr dest_ip; /* Destination Gateway Address */
	uint8_t protocol; /* Security Protocol (ESP/AH) */
};

/**************************************************************************//**
@Description	Peer Gateway change adapt notification information structure.
*//***************************************************************************/
struct peer_gw_change_adapt_notification {
	uint32_t spi; /* SPI Value */
	uint8_t protocol; /* Security Protocol (ESP/AH) */
	struct ip_addr new_dst_adr; /* New Destination Gateway Address */
	struct ip_addr old_dst_adr; /* Old Destination Gateway Address */
	uint16_t new_port; /* New Port */
	uint16_t old_port; /* old Port */
};

/**************************************************************************//**
@Description	Sequence Number Overflow notification information structure.
*//***************************************************************************/
struct seq_num_overflow_notification {
	uint32_t spi; /* SPI Value */
	struct ip_addr dest_ip; /* Destination Gateway Address */
	uint8_t protocol; /* Security Protocol (ESP/AH) */
};

/**************************************************************************//**
@Description	SA Periodic update notification information structure.
*//***************************************************************************/
struct sa_periodic_update_notification {
	enum ipsec_direction dir; /* Direction: Inbound our Outbound */
	uint32_t spi; /* SPI Value */
	struct ip_addr dest_ip; /* Destination Gateway Address */
	uint8_t protocol; /* Security Protocol (ESP/AH) */

	uint32_t seq_num; /* SA sequence number */
	uint32_t hi_seq_num; /* SA Higher order sequence number.
	 	Valid when ESN is enabled on SA */
	uint32_t elapsed_time_sec;
	       /* Elapsed Seconds of SA life time */
	uint64_t pkts_processed;
		/* Number of Packets processed by SA */
	uint64_t bytes_processed; 
		/* Number of bytes processed by SA */

	/* TBD */
};

/**************************************************************************//**
@Description	IPSec Log message IDs
*//***************************************************************************/
enum ipsec_log_msg_id {
	IPSEC_LOG_MSG_ID1 = 1
		/* IPSec-DP uses this Id when it receives invalid length 
 			ESP packet */
	/* TBD */
	
};

/**************************************************************************//**
@Description	IPSec log notification information structure.
*//***************************************************************************/
struct ipsec_log_notification {
	enum ipsec_direction dir; /* Direction: Inbound our Outbound */
	enum ipsec_log_msg_id msg_id; /* Log message Id */
	uint8_t msg[200]; /* Message to be logged */

	uint32_t tunnel_id;  /* Tunnel Id */ 
	uint32_t policy_id;   /* Outbound SPD Policy ID */

	uint32_t spi; /* SPI Value */
	struct ip_addr dest_ip; /* Destination Gateway Address */
	uint8_t protocol; /* Security Protocol (ESP/AH) */

	/* TBD */

};

/* manish changed return type int32_t 
typedef void (*ipsec_cbk_no_out_sa_f)(
	ns_id nsid,
	struct no_out_sa_notification *in);
typedef void (*ipsec_cbk_no_in_sa_f)(
	ns_id nsid,
	struct no_in_sa_notification *in);
typedef void (*ipsec_cbk_sa_expiry_f)(
	ns_id nsid,
	struct sa_expiry_notification *in);
typedef void (*ipsec_cbk_peer_gw_change_adapt_f)(
	ns_id nsid,
	struct peer_gw_change_adapt_notification *in);
typedef void (*ipsec_cbk_seq_num_overflow_f)(
	ns_id nsid,
	struct seq_num_overflow_notification *in);

typedef void (*ipsec_cbk_sa_periodic_update_f)(
	ns_id nsid,
	struct sa_periodic_update_notification *in);

typedef void (*ipsec_cbk_log_notification_f)(
	ns_id nsid,
	struct ipsec_log_notification *in);
*/

typedef int32_t (*ipsec_cbk_no_out_sa_f)(
	ns_id nsid,
	struct no_out_sa_notification *in);
typedef int32_t (*ipsec_cbk_no_in_sa_f)(
	ns_id nsid,
	struct no_in_sa_notification *in);
typedef int32_t (*ipsec_cbk_sa_expiry_f)(
	ns_id nsid,
	struct sa_expiry_notification *in);
typedef int32_t (*ipsec_cbk_peer_gw_change_adapt_f)(
	ns_id nsid,
	struct peer_gw_change_adapt_notification *in);
typedef int32_t (*ipsec_cbk_seq_num_overflow_f)(
	ns_id nsid,
	struct seq_num_overflow_notification *in);

typedef int32_t (*ipsec_cbk_sa_periodic_update_f)(
	ns_id nsid,
	struct sa_periodic_update_notification *in);

typedef int32_t (*ipsec_cbk_log_notification_f)(
	ns_id nsid,
	struct ipsec_log_notification *in);
typedef int32_t (*ipsec_cbk_self_decrypted_pkt_f)(
	ns_id nsid,
	struct self_decrypted_pkt_notification *in);


/**************************************************************************//**
@Description	IPSec notification hooks structure.
**************************************************************************//**
@Description	IPSec notification hooks structure.
		This structure contains hooks for receiving 
		unsolicited notifications.
*//***************************************************************************/
struct ipsec_notification_hooks {

	ipsec_cbk_no_out_sa_f no_out_sa_f;
	       /* This callback function is invoked when no
		outbound SA is found in IPSec-DP*/

	ipsec_cbk_no_in_sa_f no_in_sa_f;
	       	       /* This callback function is invoked when no
		inbound SA is found in IPSec-DP*/

	ipsec_cbk_sa_expiry_f sa_expiry_f;
	       	/* This callback function is invoked when the
		IPSec-DP detects  SA expiry.*/
	ipsec_cbk_peer_gw_change_adapt_f peer_gw_change_adapt_f;

		/* This callback function is invoked when the IPSec-DP
		identifies Peer gateway change */
	ipsec_cbk_seq_num_overflow_f seq_num_overflow_f;
		/* This callback function is invoked when the IPSec-DP
		encounters sequence number overflow */

	ipsec_cbk_sa_periodic_update_f sa_periodic_update_f;
		/* This callback function is invoked for periodic
		updates for SA */

	ipsec_cbk_log_notification_f log_notification_f;
		/* This callback function is invoked when IPSec-DP
		sends a log message */

        ipsec_cbk_self_decrypted_pkt_f self_decrypted_pkt_f;
};

/**************************************************************************//**
@Function	ipsec_notification_hooks_register

@Description	This API registers callback hooks for receiving notifications
		sent by IPSec NF-DP 

@Param[in]	hooks - Pointer to ipsec_notification_hooks structure containing
		callback function pointers.

@Param[in]	resp - Response arguments for asynchronous call.

@Return		Success or Failure

@Cautions	None.
*//***************************************************************************/
int32_t ipsec_notification_hooks_register(
		struct ipsec_notification_hooks *hooks);

/**************************************************************************//**
@Function	ipsec_notification_hooks_register

@Description	This API deregisters IPSec NF-DP notification callback hooks.


@Return		Success or Failure

@Cautions	None.
*//***************************************************************************/

int32_t ipsec_notification_hooks_deregister(void);


#endif /* __IPSEC_API_H */
