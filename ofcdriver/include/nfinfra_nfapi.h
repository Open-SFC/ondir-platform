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

/*!
 * @file  nfinfra_nfapi.h
 *
 * @brief  This file contains the NF-Infra Network Function API
 *
 * @addtogroup NF-INFRA
*//***************************************************************************/
#ifndef __FSL_NFAPI_NFINFRA_H
#define __FSL_NFAPI_NFINFRA_H

#define BIT(x)  (1<<((x)))

/*! Name space ID. */
typedef uint16_t nf_ns_id;
/*! Zone ID. */
typedef uint16_t nf_zone;
/*! Interface ID */
typedef uint32_t nf_if_id;

/*! Network Interface ID */
typedef uint32_t nf_ni_id;

/*! API behavioural flags. */
enum nf_api_control_flags {
	NF_API_CTRL_FLAG_ASYNC = BIT(0), /*!< If set, API call should be
			asynchronous. Otherwise API call will be synchronous.*/
	NF_API_CTRL_FLAG_NO_RESP_EXPECTED = BIT(1), /*!< If set, no response is
			expected for this API call */
};
/*! API behavioural flags. */
typedef uint32_t nf_api_control_flags;

/*! IPv4 address */
typedef uint32_t nf_ipv4_addr;

/*!
 * NFInfra fetch operations
 */
enum nfinfra_get_op {
        NFINFRA_GET_FIRST = 0,  /**< Fetch first entry in the database */
        NFINFRA_GET_NEXT,       /**< Fetch next entry for given entry */
        NFINFRA_GET_EXACT       /**< Fetch a specific entry */
};

/*!
 * IPv6 address
 */
struct nf_ipv6_addr{
        /*!  U8 Addr Len. */
#define NF_IPV6_ADDRU8_LEN 16
        /*!  U16 Addr len */
#define NF_IPV6_ADDRU16_LEN 8
        /*!  U32 Addr len */
#define NF_IPV6_ADDRU32_LEN 4

       union {
               uint8_t b_addr[NF_IPV6_ADDRU8_LEN];
                       /**< Byte addressable v6 address */
               uint32_t w_addr[NF_IPV6_ADDRU32_LEN];
                       /**< Word addressable v6 address */
       };
};

/*!
 * IP Version
 */
enum nf_ip_version {
        NF_IPV4 = 4, /**< IPv4 Version */
        NF_IPV6 = 6 /**< IPv6 Version */
};


/*! IPv4/IPv6 address type. */
enum nf_ip_addr_type {
        NF_IPA_ANY = 0,/**< None */
        NF_IPA_SINGLE ,/**< Single IP address */
        NF_IPA_SUBNET,  /**< Subnet */
        NF_IPA_RANGE,   /**< Range of IP addresses */
        NF_IPA_NET_OBJ, /**< Single network object */
        NF_IPA_NET_GRP_OBJ      /**< Group of netork objects */
};


/*! IPv4 address type for single/rangle/subnet/object/object group. */
struct nf_ipv4_addr_info {

        uint8_t type;           /**< Type of ipv4 address data.
                                 * See "enum nf_ip_addr_type". */
        union {
                struct {
                        nf_ipv4_addr ip_addr; /**< Single IPv4 address */
                } single;

                struct {
                        nf_ipv4_addr addr; /**<  Network address */
                        uint32_t prefix_len; /**< Prefix length */
                } subnet;

                struct {
                        nf_ipv4_addr begin; /**< Start of range of IPv4
                                        * addresses. */
                        nf_ipv4_addr end; /**< End of range of
                                        IPv4 addresses. */
                } ip_range;

                uint32_t ip4_net_obj_id; /**< Network object  ID */

                uint32_t ip4_netgrp_obj_id; /**< Network group object ID  */
        };
};

/*! IPv6 address type for single/rangle/subnet/object/object group. */
struct nf_ipv6_addr_info {
        uint8_t type;           /**< Type of ipv4 address data.
                                 * See "enum nf_ip_addr_type". */
        union {
                struct {
                        struct nf_ipv6_addr ip; /**< Single IPv6 address */
                } single;
                struct {
                        struct nf_ipv6_addr addr; /**<  Network address */
                        uint32_t prefix_len; /**< Prefix length */
                } subnet;
                struct {
                        struct nf_ipv6_addr begin; /**< Start of range of IPv6
                                                * addresses. */
                        struct nf_ipv6_addr end; /**< End of range of
                                                IPv6 addresses. */
                } range;
                uint32_t ip6_net_obj_id; /**< Network object  ID */

                uint32_t ip6_netgrp_obj_id; /**< Network group object ID  */
        };
};

/*! Port type. */
enum nf_l4_port_type {
        NF_L4_PORT_ANY,         /**< Wild card */
        NF_L4_PORT_SINGLE,              /**< Single port */
        NF_L4_PORT_RANGE,               /**< Range of ports */
        NF_L4_PORT_SERVICE,             /**< Standard or custom service port */
        NF_L4_PORT_SERVICE_GRP          /**< Service group */
};


/*! Data type for service single/range/object. */
struct nf_l4_port {
        uint8_t type;           /**< Type of port data.
                                 * See "enum nf_l4_port_type". */
        union {
                struct {
                        uint16_t port;  /**< Single port */
                } single;
                struct {
                        uint16_t begin; /**< Start of range of ports */
                        uint16_t end;   /**< End of range of ports */
                } range;
                uint32_t srv_obj_id; /**< ID of standard or custom service
                                        record */
                uint32_t srv_grp_obj_id; /**< ID of service group record */
        };
};



/*!
 * Structure that defines the arguments for the call back API that registered by 
 * the application to receive packets from DP/AIOP to CP/GPP 
 */
struct nf_pkt_buf
{
     nf_ns_id nsid; /*!< Name space id*/
     void *pkt; /*!< Buffer from DP  - TBD */
     int32_t ifid;
};

/*! Definition of response callback function for asynchronous API
 * call.
 */
typedef void (*nf_api_resp_cbfn)(void *cbarg, int32_t cbarg_len, void *outargs);

/*! API response argumentes structure. */
struct nf_api_resp_args {
	nf_api_resp_cbfn cbfn; /*!< Response callback function pointer */
	void *cbarg;	/*!< Pointer to response callback argument data */
	int32_t cbarg_len;/*!< Length of callback argument data */
};


/*! NFInfra notification hooks structure.
 * This contains hooks for receiving unsolicited notifications.
 */
#if 0
struct nfinfra_notification_hooks {
	void (*receive_packet)(
		/* TODO: describe packet structure based on NADK definitions */
			); /*!<
			Handle a packet in control plane. This hook typically
			gets called for unhandled packets in data-path due to
			unknown l2 protocol or due to a request from other
			data paths to push this packet to control path. */
	void (*transmit_packet)(
		/* TODO: describe packet structure based on NADK definitions */
			); /*!<
			Handle packet transmission on an interface for which
			driver is not present in data path. Typically this
			happens when data path frind packet for transmission
			on a logical interface (e.g: gre, ipip) */
};
#endif
/*! Network namespace statistics structure. */
struct nfinfra_netns_stats {
	uint64_t in_pkts;	/*!< Ingress packets */
	uint64_t in_bytes;	/*!< Egress packets */
	uint64_t out_pkts;	/*!< Ingress bytes */
	uint64_t out_bytes;	/*!< Egress bytes */
};

/*! Network infra structure DP capabilities structure. */
struct nfinfra_capabilities { 
	uint64_t capabilities; /*!< List of capabilities */
};

/*! Generic output parameters structure for NFInfra NF API. */
struct  nfinfra_outargs{
	int32_t result;	/*!< Result code of the requested operation.
			Success or error code indicating failure */
};


/*! Output arguments structure for namespace statistics retrieval
 *  API 
 */
struct nfinfra_netns_get_stats_outargs {
	int32_t result;	/*!< Result code of the requested operation.
			Success or error code indicating failure */
	struct nfinfra_netns_stats stats; /*!< namespace statistics */
};


/*!
 * @brief  Gets the version string of NFinfra NF API
 *
 * @param[out]	version - Pointer to string in which API version string is
 *		copied.
 * @returns  0 on Success or negative value on failure
 *
 * @ingroup NF-INFRA 
 */ 
int32_t nfinfra_api_get_version(char *version);



/*!
 * @brief Gets the capabilities of NFInfra DP.
 *
 * @param[out]	capabilities - Pointer to nfinfra_capabilities structure.
 *
 * @returns  0 on Success or negative value on failure
 *
 * @ingroup NF-INFRA 
 */ 
int32_t nfinfra_api_get_capabilities(struct nfinfra_capabilities *capabilities);


/*!
 * @brief   Adds a network namespace
 *
 * @details    This function first validates the incoming parameters and 
 *             if all validations
 *             succeed, adds the entry in the database.
 *
 * @param[in]  nsid - Network namespace ID.
 *
 * @param[in]  flags - API behavioural flags.
 *
 * @param[out] out - output arguments structure.
 *
 * @param[in]  resp - Response arguments for asynchronous call.
 *
 * @returns    0 on Success or negative value on failure
 *
 * @ingroup NF-INFRA 
 */ 
int32_t nfinfra_netns_add(
			nf_ns_id nsid,
			nf_api_control_flags flags,
			struct nfinfra_outargs *out,
			struct nf_api_resp_args *resp);


/*!
 * @brief   Deletes a network namespace in NFInfra DP.
 *
 * @param[in]  nsid - Network namespace ID.
 *
 * @param[in]  flags - API behavioural flags.
 *
 * @param[out] out - output arguments structure.
 *
 * @param[in] resp - Response arguments for asynchronous call.
 *
 * @returns   0 on Success or negative value on failure
 *
 * @ingroup NF-INFRA 
 */ 
int32_t nfinfra_netns_delete(
			nf_ns_id nsid,
			nf_api_control_flags flags,
			struct nfinfra_outargs *out,
			struct nf_api_resp_args *resp);


/*!
 * @brief  Binds an interface to a network namespace.
 *
 * @param[in]  nsid - Network namespace ID.
 *
 * @param[in]  ifid - interface identifier.
 *
 * @param[in]  flags - API behavioural flags.
 *
 * @param[out]  out - output arguments structure.
 *
 * @param[in]  resp - Response arguments for asynchronous call.
 *
 * @returns  0 on Success or negative value on failure
 *
 * @ingroup NF-INFRA 
 */ 
int32_t nfinfra_bind_iface(
			nf_ns_id nsid,
			nf_if_id ifid,
			nf_api_control_flags flags,
			struct nfinfra_outargs *out,
			struct nf_api_resp_args *resp);



/*!
 * @brief  Unbinds an interface from a network namespace.
 *
 * @param[in]  nsid - Network namespace ID.
 *
 * @param[in]  ifid - interface identifier.
 *
 * @param[in]  flags - API behavioural flags.
 *
 * @param[out]  out - output arguments structure.
 *
 * @param[in]  resp - Response arguments for asynchronous call.
 *
 * @returns  0 on Success or negative value on failure
 *
 * @ingroup NF-INFRA 
 */ 
int32_t nfinfra_unbind_iface(
			nf_ns_id nsid,
			nf_if_id ifid,
			nf_api_control_flags flags,
			struct nfinfra_outargs *out,
			struct nf_api_resp_args *resp);


/*!
 * @brief   Sets a zone value to an interface.
 *
 * @details   By default each interface contains zero as zone value. 
 *            This API can be used to set a different value. 
 *            As such NFInfra DP treats zone value as opaque.
 *
 * @param[in]  nsid - Network namespace ID.
 *
 * @param[in]  ifid - interface identifier.
 *
 * @param[in]  zone - zone value.
 *
 * @param[in]  flags - API behavioural flags.
 *
 * @param[out]  out - output arguments structure.
 *
 * @param[in]  resp - Response arguments for asynchronous call.
 *
 * @returns   0 on Success or negative value on failure
 *
 * @ingroup NF-INFRA 
 */ 
int32_t nfinfra_set_zone(
			nf_ns_id nsid,
			nf_if_id ifid,
			nf_zone zone,
			nf_api_control_flags flags,
			struct nfinfra_outargs *out,
			struct nf_api_resp_args *resp);

/*!
 * @brief    This API retrieves statistics of a given network namespace.
 * 
 * @param[in]  nsid - Network namespace ID.
 * 
 * @param[in]  flags - API behavioural flags.
 * 
 * @param[out]  out - pointer to nfinfra_netns_get_stats_outargs structure.
 * 
 * @param[in]  resp - Response arguments for asynchronous call.
 *
 * @returns   0 on Success or negative value on failure
 *
 * @ingroup NF-INFRA 
 */ 
int32_t nfinfra_netns_stats_get(
			nf_ns_id nsid,
			nf_api_control_flags flags,
			struct nfinfra_netns_get_stats_outargs *out,
			struct nf_api_resp_args *resp);




/*!
 * @brief    Register callback hooks for receiving unsolicited notifications
 *               sent by NFInfra NF-DP.
 *
 * @param[out]   hooks - Pointer to nfinfra_notification_hooks structure
 *		 containing callback function pointers.
 *
 * @returns      0 on Success or negative value on failure
 *
 * @ingroup NF-INFRA 
 */ 
#if 0
int32_t nfinfra_notification_hooks_register(
			struct nfinfra_notification_hooks *hooks);
#endif
/*!
 * @brief  Deregister NFInfra DP notification callback hooks.
 *
 * @returns  0 on Success or negative value on failure
 *
 * @ingroup NF-INFRA 
 */ 
int32_t nfinfra_notification_hooks_deregister(void);






/*!
 * Enumeration of network device types.
 */
enum nfinfra_netdev_type {
        NFINFRA_NETDEV_TYPE_UNKNOWN = 0, /**< Unknown/Invalid */
        NFINFRA_NETDEV_TYPE_ETHER = 1, /**< Ethernet */
        NFINFRA_NETDEV_TYPE_VLAN = 2, /**< VLAN (8021q) */
        NFINFRA_NETDEV_TYPE_BRIDGE = 3, /**< Bridge */
        NFINFRA_NETDEV_TYPE_IPIP = 4, /**< IPv4 in IPv4 */
        NFINFRA_NETDEV_TYPE_IP6TUN = 5, /**< IPv6 Tunnel Device */
        NFINFRA_NETDEV_TYPE_GRE = 6, /**< GRE */
        NFINFRA_NETDEV_TYPE_SIT = 7, /**< IPv6 in IPv4 Tunnel Device */
        NFINFRA_NETDEV_TYPE_PPP = 8, /**< PPP */
        NFINFRA_NETDEV_TYPE_PIMREG = 9, /**< PIMREG */
        NFINFRA_NETDEV_TYPE_LOOPBACK = 10, /**< Loopback */
        NFINFRA_NETDEV_TYPE_UNUSED = 14, /**< Start of unused IDs. */
        NFINFRA_NETDEV_TYPE_MAX = 24 /**< */
};


/*!
 * This enumeration defines network device related flags. These
 * flags are not allowed to change after creation of network device.
 */
enum nfinfra_netdev_flags {
        NFINFRA_NETDEV_FLAGS_ETHER = BIT(0), /**< Ethernet type of network
                        device. e.g: Ethernet, VLAN, Bridge etc. */
        NFINFRA_NETDEV_FLAGS_P2P = BIT(1), /**< Point to point type of network
                        device. e.g: PPP, GRE, IPIP, SIT etc.*/
};



/*!
 * This enumeration defines operation flags of network device.
 * These flags are allowed to change at run-time.
 */
enum nfinfra_netdev_opflags {
        NFINFRA_NETDEV_OPFLAGS_UP = BIT(0), /**< Administratively up or
                        down */
        NFINFRA_NETDEV_OPFLAGS_LINK_UP = BIT(1), /**< Underlying link is up or
                        down. */
        NFINFRA_NETDEV_OPFLAGS_BRIDGE_PORT = BIT(2), /**< Attached to a
                        software based bridge or not. */
        NFINFRA_NETDEV_OPFLAGS_ARP = BIT(3), /**< ARP enabled on this
                        network device or not. This bit is valid only
                        if NFINFRA_NETDEV_FLAGS_ETHER bit is set in flags. */
        NFINFRA_NETDEV_OPFLAGS_BCAST = BIT(4), /**< Broadcast enabled on this
                        network device or not. */
        NFINFRA_NETDEV_OPFLAGS_MCAST = BIT(5), /**< Multicast enabled on this
                        network device or not */
        NFINFRA_NETDEV_OPFLAGS_UNUSED = BIT(6), /**< Start of unused flags and
                        these can be used by other DPs. */
};



/*!
 * Maximum size of driver specific custom data
 */
#define NFINFRA_NETDRV_CDATA_MAX_SIZE (128)


/*! Network device structure */
struct nfinfra_netdev {
        nf_if_id ifid;    /**< Unique ID of this network device */
        enum nfinfra_netdev_type type;   /**< Type of the network device */
        uint8_t flags; /**< enum nfinfra_netdev_flags */
        uint16_t opflags; /**< enum nfinfra_netdev_opflags */
        uint16_t mtu;   /**< Maximum transmit unit */
        nf_ni_id pexi_niid; /**< related ni-id of NI object for packet exchange
                        with CP. */
        uint8_t netdrv_cdata[NFINFRA_NETDRV_CDATA_MAX_SIZE]; /**< Driver
                        specific command data */
        uint8_t netdrv_cdata_len; /**< Valid length of driver specific
                        command data */
};

/*! Network device add structure for NFInfra NF API. */
enum nfinfra_netdev_mod_flags {
        NFINFRA_NETDEV_MOD_FLAG_OPFLAGS = BIT(0), /**< If set, opflags field
                        is to be modified */
        NFINFRA_NETDEV_MOD_FLAG_MTU = BIT(1), /**< If set, mtu field
                        is to be modified */
        NFINFRA_NETDEV_MOD_FLAG_DRV_CDATA = BIT(2), /**< If set, driver
                        custom data field is to be modified */
};


/*! Network device modify structure for NFInfra NF API. */
struct nfinfra_netdev_mod_inargs {
        nf_if_id ifid;  /**< Unique ID of this network device */
        enum nfinfra_netdev_type type;   /**< Type of the network device */
        uint32_t mod_flags; /**< Boolean flags signifying which fields are
                        modified. */

        uint16_t opflags; /**< enum nfinfra_netdev_opflags */
        uint16_t mtu;   /**< Maximum transmit unit */
        uint8_t netdrv_cdata[NFINFRA_NETDRV_CDATA_MAX_SIZE]; /**< Driver
                        specific command data */
        uint8_t netdrv_cdata_len; /**< Valid length of driver specific
                        command data */
};

/*! Network device delete structure for NFInfra NF API. */
struct nfinfra_netdev_del_inargs {
        nf_if_id ifid;    /**< Unique ID of this network device */
        enum nfinfra_netdev_type type;   /**< Type of the network device */
};

/*!
 * Network device related statistics
 */
struct nfinfra_netdev_stats {
        uint64_t in_pkts; /**< Received packets */
        uint64_t in_bytes; /**< Amount of received data, in bytes */
        uint64_t out_pkts; /**< Sent packets */
        uint64_t out_bytes; /**< Amount of sent data, in bytes */
        uint64_t dropped_in_pkts; /**< Dropped incoming packets */
        uint64_t dropped_in_bytes; /**< Amount of received data dropped */
        uint64_t dropped_out_pkts; /**< Dropped outgoing packets */
        uint64_t dropped_out_bytes; /**< Amount of outgoing data dropped */
};



/*!
 * Input parameters to fetch statistics of a network device
 */
struct nfinfra_netdev_stats_get_inargs {
        nf_if_id ifid;    /**< Unique ID of this network device */

};

/*! Output arguments structure for network device statistics retrieval
 *  API 
 */
struct nfinfra_netdev_stats_get_outargs {
        int32_t result; /**< Result code of the requested operation.
                        Success or error code indicating failure */
        struct nfinfra_netdev_stats stats; /**< Network device statistics */
};

/*!
 * Input parameters to fetch network device information
 */
struct nfinfra_netdev_get_inargs{
        nf_if_id ifid;  /**< Unique ID of this network device. This does not
                        need to contain valid information if the operation
                        code is NFINFRA_GET_FIRST */
        enum nfinfra_get_op operation; /**< Requested operation code */
};

/*! Output arguments structure for network device information retrieval
 *  API 
 */
struct nfinfra_netdev_get_outargs {
        int32_t result; /**< Result code of the requested operation.
                        Success or error code indicating failure */
        struct nfinfra_netdev netdev; /**< Network device statistics */
};



int32_t nfinfra_netdev_add(
                        nf_ns_id nsid,
                        const struct nfinfra_netdev *in,
                        nf_api_control_flags flags,
                        struct nfinfra_outargs *out,
                        struct nf_api_resp_args *resp);



int32_t nfinfra_netdev_mod(
                        nf_ns_id nsid,
                        const struct nfinfra_netdev_mod_inargs *in,
                        nf_api_control_flags flags,
                        struct nfinfra_outargs *out,
                        struct nf_api_resp_args *resp);

int32_t nfinfra_netdev_del(
                        nf_ns_id nsid,
                        const struct nfinfra_netdev_del_inargs *in,
                        nf_api_control_flags flags,
                        struct nfinfra_outargs *out,
                        struct nf_api_resp_args *resp);



int32_t nfinfra_netdev_stats_get(
                        nf_ns_id nsid,
                        const struct nfinfra_netdev_stats_get_inargs *in,
                        nf_api_control_flags flags,
                        struct nfinfra_netdev_stats_get_outargs *out,
                        struct nf_api_resp_args *resp);


int32_t nfinfra_netdev_get(
                        nf_ns_id nsid,
                        const struct nfinfra_netdev_get_inargs *in,
                        nf_api_control_flags flags,
                        struct nfinfra_netdev_get_outargs *out,
                        struct nf_api_resp_args *resp);



#define NF_IFACE_NAME_LEN (16)
#define NF_HWADDR_LEN     (6)


struct iface_info
{
  uint16_t        ns_id;
  uint32_t        flags;
  int32_t         mtu;
  uint16_t        type;

  struct {
    void  *ip4;
    void  *ip6;
    void  *pOpaque1;
    void  *pOpaque2;
  } l3;

  //unsigned char     hw_addr[NF_HWADDR_LEN];
  char     hw_addr[NF_HWADDR_LEN];
  uint16_t     hw_addr_len;

  int32_t      os_ifindex;
  char      iface_name[NF_IFACE_NAME_LEN];
  uint32_t  ip_addr;
  uint32_t dst_ipaddr;
};

struct vlan_iface_info
{
  uint16_t        vlan_id;
  uint32_t      parent_os_ifindex;
  struct iface_info iface;
};
#endif /* __FSL_NFAPI_NFINFRA_H */

