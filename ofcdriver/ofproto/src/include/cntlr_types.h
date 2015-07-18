/**
 *  Copyright (c) 2012, 2013  Freescale.
 *   
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 **/

/* File: cntlr_types.h
 * 
 * Description: Common definitions  of Controller 
 * 
 * Authors:     Rajesh Madabushi <rajesh.madabushi@freescale.com>
 */

/*IPV4 address typedef*/
typedef uint32_t ipv4_addr_t;

/*IPV6 address typedef*/
typedef union {

     /** 8 bit format */
     uint8_t ipv6Addr8[16];

     /** 16 bit format */
     uint16_t ipv6Addr16[8];

     /** 32 bit format */
     uint32_t ipv6Addr32[4];

} ipv6_addr_t;

#define CNTLR_IPV4_ADDR 1
#define CNTLR_IPV6_ADDR 2
/*IP Address*/
typedef struct ipaddr_s
{
   uint8_t type;/*IPV4 or IPV6*/
   union {
    ipv4_addr_t v4_val;
    ipv6_addr_t v6_val;
   };
} ippaddr_t;


#if 0
/*Store 64 bit value*/
typedef struct of_uint64_s 
{
    union
    {
       uint32_t ul[2];
       uint64_t val;
    };
} of_uint64_t;

/*Network to host conversion of 64 bit value*/
#define ntohll(value,val_64)\
{\
   value.ul[0] = ntohl((uint32_t)(val_64 >>32));\
   value.ul[1] = ntohl((uint32_t)val_64);\
}

#endif

typedef uint64_t t_phyaddr;

/*Following data-structure used to reply multipart reply message header */
typedef struct of_multipart_reply_hdr_s
{
   uint16_t type;              /* One of the OFPMP_* constants. */
   uint16_t flags;             /* OFPMPF_REPLY_* flags. */
   uint8_t pad[4];
} of_multipart_reply_hdr_t;

typedef unsigned char bool8_t;
typedef unsigned char uchar8_t;

struct safe_ref {
  uint32_t uiIndex;
  uint32_t uiMagic;
};


#define PSP_IPv6_ADDRU8_LEN 16
#define PSP_IPv6_ADDRU16_LEN 8
#define PSP_IPv6_ADDRU32_LEN 4

typedef struct PSPIPv6Addr_s {
    union {
      uint8_t  PSPIPv6AddrU8[PSP_IPv6_ADDRU8_LEN];
      uint16_t PSPIPv6AddrU16 [PSP_IPv6_ADDRU16_LEN];
      uint32_t PSPIPv6AddrU32 [PSP_IPv6_ADDRU32_LEN];
    }Addr;
    #define PSPIPv6AddrU8 Addr.PSPIPv6AddrU8
    #define PSPIPv6AddrU16 Addr.PSPIPv6AddrU16
    #define PSPIPv6AddrU32 Addr.PSPIPv6AddrU32
} PSPIPv6Addr_t;

#define PSP_PRIVATE static

//should be derivable at run-time!!
#ifdef CONFIG_BOARD_P4080
#define CNTLR_MAX_NUMBER_OF_CORES 4
#elif CONFIG_BOARD_P2041
#define CNTLR_MAX_NUMBER_OF_CORES 2
#elif CONFIG_BOARD_T4240 
#define CNTLR_MAX_NUMBER_OF_CORES 24
#else
#define CNTLR_MAX_NUMBER_OF_CORES 8
#endif
#define CNTLR_DFLT_NUMBER_OF_CORES  1 

#define FWNAT_CB_MAIN_APP_TYPE 9 
#define SKB_CB_FWNAT_APP_DATA 33 

#define SKB_CB_MAIN_APP_TYPE 45 
#define SKB_CB_APP_TYPE   46 
#define SKB_CB_APP_SUBTYPE 47

#define KERNEL_IFACE_FLAG_ETHER 0x0001
#define KERNEL_IFACE_FLAG_BRIDGE 0x0002
#define KERNEL_IFACE_FLAG_VLAN 0x0004

