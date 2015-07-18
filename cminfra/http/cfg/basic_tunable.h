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

/************************************************************************ */
/*   File            : tunable.h                                          */
/*   Purpose         : Options file for modsets		                  */
/*   Date            : 18-10-2003                                         */   
/*------------------------------------------------------------------------*/

#define PROD_PKG   ENTR-LX


/*
 * The following Buffer related Macroes are moved from igwcfg.h.tpl
 */
#define IGW_MAX_DSMEM_POOL_LENGTH 400
#define IGW_MAX_KERNEL_RAM_LEFT_FOR_OS 0
/* The IGW_BUF_MAX_TOTAL_COUNT determines maximum allowed IGW buffers 
 * in the system at any time.
 * The IGW_BUF_MAX_STATIC_COUNT determines how many out of the above
 * mentioned total buffers need to be statically allocated in the 
 * system 
 * NOTE:Presently there are no heap allocated buffers. However there 
 * is a provision for the same.
 */

#define IGW_BUF_MAX_TOTAL_COUNT 50000
#define IGW_BUF_MAX_STATIC_COUNT 5000

/* Privileged IGW Buffers Counts */
#define IGW_BUF_PRIV_MAX_TOTAL_COUNT 50
#define IGW_BUF_PRIV_MAX_STATIC_COUNT 50

/* 
 * This is deprecated now, we use MemPools to handle all such cases
 * However Proxy Module seems to be using it.. 
 */
/*The following macro should be defined as per system requirements
 used in IGWSTARTINITALLOC and IGWENDINITALLOC*/
#define IGWMAXBUFALLOC 2048
 /* NTP Daemon Module Basic Tunable Parameters */

 /* Max no of Poll fds allowed */
  #define NTP_MAX_POLL_FDS  4  

 /* Default no of samples used to send 
  * time request packets for each time server */
  #define NTP_DEFAULT_SAMPLE_COUNT  4  



/* NETACCESS Modules Tunable Parameters */

/* NTD Modules Basic Tunable Parameters */
#define NTD_MAX_STATIC_SIZE   5
#define NTD_MAX_LIST_SIZE    15

 /* Defines maximum number of dynamic DNS records. */
#define	DDNS_MAX_RECORDS  10

	/*  Protocol Specific Tunable Parameters */

/* This parameter defines user agent string for DDNS client. Customers have requested in the past to send customized string in this place. */
#define	DDNS_USER_AGENT_STR "intoto/iGateway_NetAccess/1.0"

/*Even if there is no change in ipaddress , after twenty eight days (since last update)DDNS has to send the update.*/
#define	TWENTY_EIGHT_DAYS        (28*24*60)

/* This parameter defines the check ip url */
#define	DDNS_CHECKIP_URL "checkip.dyndns.org"

/* This parameter defines the time interval to run check ip script next time.*/
#define	DDNS_CHECKIP_INTERVAL  10    /* Mins */

/* This parameter defines the check ip port */
#define	DDNS_CHECKIP_PORT      8245           

/* IGMP Proxy module Basic Tunable Parameters */
/*
 * Maximum size of groups static free list
 */
#define IGMP_STATIC_GROUPS_FREE_POOL_SIZE  32

/*
 * Maximum number of groups supported 
 */
#define IGMP_MAX_GROUPS	     1024	

/*
 * Maximum size of sources static free list
 */
#define IGMP_STATIC_SOURCES_FREE_POOL_SIZE 32

/*
 * Minimum number of sources supported 
 */
#define IGMP_MAX_SOURCES     366

/*
 * Maximum number of pending queries supported 
 */
#define IGMP_MAX_PENDING_QUERIES      32	

/*
 * Maximum number of interface supported for multicasting
 */
#define IGMP_MAX_ALLOWED_IFACES MAX_NUM_INTERFACES



/* IGMP Proxy module Protocol Specific Tunable Parameters */
/*
 * Default number of retries 
 */
#define IGMP_MAX_QRY_COUNT   4 	

/*
 * Default query interval code based on which as per RFC query interval 
 * time is calculated
 */
#define IGMP_QRY_INTERVAL    100

/*
 * IGMP Proxy attached interface default version
 */
#define IGMP_DEFAULT_IFACE_VER  3



/*Linux Mcast module Basic Tunable Parameters */
/*
 *Multicast routes static free pool size
 */
#define IGMP_STATIC_MCAST_ROUTES_FREE_POOL_SIZE 64

/*
 * Maximum number of multicast routes supported
 */
#define IGMP_MAX_MCAST_ROUTES 256


/* Diag module Basic Tunable Parameters */
/*
 * Maximum number of simultaneous requests processed 
 */
#define DIAG_MAX_SESSIONS 10

/*
 * Maximum number of results stored in Diag Results Store module
 */
#define DIAG_MAX_RESULTS 10


/* LDAP Auth module Basic Tunable Parameters */
/*
 *Maximum number of static LDAP Server configuration records
 */ 
#define LDAPAUTHD_MAX_RECORDS 5


/* LDAP Libray Basic Tunable Parameters */
/*
 *Maximum number of simultaneous active session can be present 
 */ 
#define LDAPLIB_MAX_SESSIONS 10


/* LDAP Libray Protocol Specific Tunable Parameters */
/*
 *Maximum wait time for the response from the Server (in seconds)
 */ 
#define LDAPLIB_MAX_RESP_TIMEOUT 40 

/*
 *Maximum time server performs a search operation (in seconds)
 */ 
#define  LDAPLIB_MAX_QUERY_TIMEOUT 30

/* BigPond Client Protocol Specific Tunable Parameters
 */

/* The key used for excrypting the password using the RC4 algorithm
 */
#define   BPCLIENT_RC4_KEY                       "Intoto_Software_(I)_Pvt._Ltd."


/* DHCP Server Protocol Specific Tunable Parameters
 */

/* The factory default lease time (in secs) that is to be offered to the
 * Clients.
 */
#define DHCPD_FAC_DEF_LEASE_TIME                 86400

/* The time interval (in secs) to wait before reclaiming abandoned leases.
 */
#define DHCPD_LEASE_RECLAIM_INTERVAL             60

/* No. of retries to send a DHCP Packet if SendPacket fails for some reason.
 */
#define DHCPD_PKT_SEND_RETRY_CNT                 3

/* This macro is used to indicate whether the leases are to be saved
 * automatically or not. Value 1 is auto and 0 is manual.
 */
#define DHCPD_AUTO_LEASE_SAVE                    0

/* This macro specifies the interval (in secs) in which the leases are to be
 * saved. The default is every one hour.
 */
#define DHCPD_LEASE_SAVE_INTERVAL                3600


/* The Basic Tunable Parameters for L2Bridge
 */

/* The maximum number of bridges allowed in the system
 */
#define L2BRIDGE_MAX_DEVICES                     4
#define L2BRIDGE_MAX_BRIDGES                     L2BRIDGE_MAX_DEVICES

/* The maximum filtering database entries allowed in each bridge
 */
#define L2BRIDGE_FDB_THRESHOLD_CNT               512

/* The Protocol Specific Tunable Parameters for L2Bridge
 */

/* The filtering database entry timeout value specified in secs.
 */
#define L2BRIDGE_FDB_DEFAULT_TIMEOUT             600


/* Wan Mode Switch Basic Tunable Parameters */
#define IGW_WAN_ETH_PREFIX             "eth"
#define IGW_WAN_DIALUP_REC_PREFIX      "ser"
#define IGW_WAN_PPPOE_REC_PREFIX       "dipe"
#define IGW_WAN_PPTP_REC_PREFIX        "dipt"
#define IGW_WAN_L2TP_REC_PREFIX        "dipl"

#define IGW_WAN_SCREENNAME_PREFIX      "WAN"

/* RADIUS Libray Basic Tunable Parameters */
/*
 *Maximum number of ports.
 */ 
#define        RADIUS_MAX_PORTS                   16
/*
 *Maximum number of IP Addresses under a domain.
 */ 
#define        RADIUS_MAX_IPADRS_IN_POOL          10
/*
 *Framed MTU Value .
 */ 
#define        RADIUS_FRAMED_MTU                  1500
/*
 *Maximum number of context blocks for handling sessions.
 */ 
#define        RADIUS_MAX_CONTEXT_BLKS            16
/*
 *Maximum value of row size to handle sessions .
 */ 
#define        RADIUS_CONTEXT_BLK_DB_ROW_LEN      4
/*
 *Maximum value of column size to handle sessions .
 */ 
#define        RADIUS_CONTEXT_BLOCK_DB_COL_LEN    4
/*
 *Maximum value of row length to handle sessions .
 */ 
#define        RADIUS_PORTINFO_DB_ROW_LEN         4
/*
 *Maximum value of column length to handle sessions .
 */ 
#define        RADIUS_PORTINFO_DB_COL_LEN         4
/*
 *Port info free block size .
 */ 
#define        RADCLI_PORTINFO_FREELIST_SIZE      16
/*
 * Maximum number of Context block to be allocated statically.
 */ 
#define        RADCLI_CTXTBLOCK_FREELIST_SIZE     RADIUS_MAX_CONTEXT_BLKS       
/*
 * Maximum number of Authentication servers configured per domain.
 */ 
#define        RADIUS_MAX_AUTH_SERV               5 
/*
 * Maximum number of Accounting servers configured per domain.
 */ 
#define        RADIUS_MAX_ACCT_SERV               5 



/* 
 * Maximum number of HTTP Sessions Allowd 
 */ 
#define WPXY_MAX_SESSIONS_ALLOWED 250

/*
 * Maximum Static HTTP Sessons Allowed
 */
#define WPXY_MAX_STATIC_SESSIONS_ALLOWED 50

/*
 * Maximum HTTP Requests Allowed
 */
#define WPXY_MAX_REQUESTS_ALLOWED 400

/*
 * Maximum Static HTTP Requests Allowed
 */
#define WPXY_MAX_STATIC_REQUESTS_ALLOWED 75

/*
 * Maximum Number of Keyword Allowed to configure.
 */
#define WPXY_MAX_KEYWORDS 100

/*
 * Maximum Number of Time Window Records Allowed to configure.
 */
#define WPXY_MAX_TIME_WINDOW_RECORD 15

/*
 * Maximum Number of WhiteList Url Allowed to configure.
 */
#define WPXY_MAX_WHITE_LIST 100

/*
 * Maximum Number of Blacklist Url Allowed to configure.
 */
#define WPXY_MAX_BLCK_LIST 100

/*
 * Maximum Number of Trusted IPs Allowed to configure.
 */
#define WPXY_MAX_TRST_IPS 20

/*
 * Maximum Number of Trusted Hosts Allowed to configure.
 */
#define WPXY_MAX_TRST_HOSTS 20

/*
 * Maximum Number of IP records for default profile Allowed to configure.
 */
#define WPXY_MAX_IPRECS_FOR_PROFILE 10

#define PCQ_GET_LICENSE_INTERFACE_NAME IFACE0                                 






/*
 * Total Number of Static Selinfo Nodes present.
 */
#define IGW_PT_MAX_STATIC_SELINFO_NODES 200
 

/*
 * Maximum Number of Static TCB's which can allocated.
 */
#define IGW_PT_MAX_STATIC_TCBS  200  /*350*/

/*
 * used for allocating the memory required for 'IGW_PT_MAX_STATIC_UCBS'
   during system initialisation 
 */
#define IGW_PT_MAX_STATIC_UCBS  200 
 
/*
 * Maximum Number of Static PxyDrv Sessions's which can allocated.
 */
#define IGW_PT_MAX_STATIC_DRV_SESS  100  
 
/*
 * Maximum Number of Static PT Output Packets which can allocated.
 */
#define IGW_PT_MAX_STATIC_OUT_PKTS 200  


#define IGW_PT_MAX_PTPPC      5 

#define  IGW_PT_MAX_PRIORITY_LEVELS  4
/**It conveys that applications can do IGW_PT_MAX_PRIORITY_LEVELS multiple
 * registrations  with PT for the same destnation port.For Eg
 * any one application can register IGW_PT_MAX_PRIORITY_LEVELS times for the
 * destination port 80,today as if like Web alg and Web Proxy.
 */ 

#define IGW_PT_MAX_STATIC_BACKLOG_NODES 200

#define IGW_PT_HASH_TBL_SIZE  50 

#define IGW_PT_MAX_STATIC_PTMAP_RECS 50

#define IGW_PT_MAX_STATIC_PNC 30

#define IGW_PT_MAX_PTMAP_PNC_ALLOWED 200

#define IGW_PT_MAX_STATIC_PIB 30

#define IGW_PT_MAX_PTMAP_PIB_ALLOWED 200




#ifdef HTTP_REDIRECT_HTTPS
#define HTTP_MAX_SERVERS_NORMAL  	1
#else
#define HTTP_MAX_SERVERS_NORMAL  	3
#endif

#ifdef HTTP_REDIRECT_HTTPS
#define HTTP_MAX_SERVERS_SSL     	1
#else
#ifdef HTTPS_ENABLE
#define HTTP_MAX_SERVERS_SSL     	3
#else
#define HTTP_MAX_SERVERS_SSL     	0
#endif /* HTTPS_ENABLE */
#endif /*HTTP_MAX_SERVERS_SSL*/


/**
 * Default port number on which http server has to bind.
 * Need to change 8082 for UCM to run on standard 80 and 443 ports.
 */
#define  HTTPD_DEF_PORT               	(80)

#ifdef OF_HTTPS_SUPPORT

/**
 * Default port number on which secured http server has to bind.
 */
#ifdef OF_SSLVPN_PROD_SUPPORT

#define  HTTPS_DEF_PORT               	(8443)
#else
#define  HTTPS_DEF_PORT               	(8443)

#endif /*OF_SSLVPN_PROD_SUPPORT*/

#endif /* OF_HTTPS_SUPPORT */

/**
 * Default http server task priority.
 */
#define  HTTPD_DEF_PRIO               	(230)

/**
 * Default maximum allowed sessions.
 */
#define  HTTPD_DEF_MAX_SESSIONS       	(10)

/**
 * Default maximum pending requests.
 */
#define  HTTPD_DEF_MAX_PENDREQS      	 (10)

/**
 * Http server name.
 */
#define  HTTPD_SERVER_NAME            	"Intoto Http Server"

/**
 * Http server version.
 */
#define  HTTPD_SERVER_VER             	"1.1"

/**
 * Default index page.
 */
#define  HTTPD_INDEX_FILE             	"loadwelcome.htm"

/**
 * Default cookiename.
 */

#define  HTTP_DEF_COOKIE_NAME         	"defaultcookie"


/**
 * Maximum number of application cookies that http server can support. Its
 * static allocation and more cookies are needed, its possible to increase
 * them dynamically.
 */
#define  HTTPD_MAX_COOKIES            	(6)

#define  HTTPD_MAX_REG_MODULES          (5)

/****
 * *
 * * SSL-VPN requirements
 * */

#define HTTP_MAX_FDS             	(32)
#define REQTABLE_SIZE            	(HTTP_MAX_FDS)
#define HTTPCOOKIE_MAX           	(120)
#define MAX_TIMER_LISTS		 	(4)	

#define  HTTP_DEF_LOGIN_FILE          "login.htm"
#define  HTTP_DEF_LOGOUT_FILE         	"logout.htm"
#define  HTTP_DEF_ERR_FILE            	"errlogin.htm"
#define  HTTP_DEF_ULOGOUT_FILE        	"ulogout.htm"
#define  HTTP_DEF_LOGO_FILE           	"logo.jpg"

#define  HTTP_HTML_PREFIX_VAL           "+SVPN"

#ifdef OF_XML_SUPPORT
#define  HTTP_DEF_XML_LOGIN_FILE        "login.xml"
#endif /*OF_XML_SUPPORT*/

#define  HTTP_DEF_XSL_LOGOUT_FILE       "/failure_en.xsl"

#define  RANDOM_ACCESS_ERR_PAGE         "loginout.htm"

#define  HTTP_MIN_SECS               	(300)
#define  HTTP_MAX_SECS                	(3600)
/*#define  HTTP_COOKIE_BUFF_LEN 	(1024)
#define  HTTP_COOKIE_VAL_LEN           	(1024)*/

#define  HTTP_COOKIE_BUFF_LEN   	(8*1024)  /*Temporary fix for xml*/
#define  HTTP_COOKIE_VAL_LEN           	(8*1024)   /*Temporary fix for xml*/

#define  HTTP_URL_LEN                   (1024)  
#define  HTTP_APPDATA_LEN               (512)  

/**
 * Offset of html file system.
 * NOTE: Set the offset to zero if it is normal file system.
 */
#define  HTMLFS_OFFSET                	(0)

/**
 * Size of html file system.
 * NOTE: Set the size to zero if it is normal file system.
 */
#define  HTMLFS_SIZE                  	(0)
/**
 * Html document path
 */
#ifdef OF_CM_SWITCH_CBK_SUPPORT
#define  HTTPD_DOC_PATH               	"/psp/html/"
#endif

#ifdef OF_CM_CNTRL_SUPPORT
#define  HTTPD_DOC_PATH                 "/ondirector/html/"
#endif
/**
 * The maximum size of data that will be 
 * transmitted for a write event on the FD
 */
#define HTTP_MAX_TRANSMIT_BUFFER     	(512)

/**
 * The maximum size of data that can be stored 
 * in each node of the transmit buffer
 */
#define HTTP_MAX_DATA_BUFFER         	(2*1024)




/*******************************************************************/
/*                                                                 */
/*                 Wan Tunable Parameters                          */
/*                                                                 */
/*******************************************************************/
/*******************************************************************/
/*                                                                 */
/*            Modem(DialUp) Basic Tunable Parameters               */
/*                                                                 */
/*******************************************************************/
/*
 * This macro indicates the maximum number of serial port connections 
 * allowed in the system
 */
#define MAX_SERIAL_CONNECTIONS 1
#define WAN_BASE_IP "10.254.254.1"
/*******************************************************************/
/*                                                                 */
/*                PPTP Basic Tunable Parameters                    */
/*                                                                 */
/*******************************************************************/
/*
 * This macro indicates the maximum number of PPTP connections 
 * allowed in the system
 */
#define MAX_PPTP_CONNECTIONS 4
/*******************************************************************/
/*                                                                 */
/*                L2TP Basic Tunable Parameters                    */
/*                                                                 */
/*******************************************************************/
/*
 * This macro indicates the maximum number of L2TP connections 
 * allowed in the system
 */
#define MAX_L2TP_CONNECTIONS 4

/*
 * This MACRO is used for identifying the Maximum Number of FDs used in L2TP.
 * The Max. No. of FDs in L2tp is equal to sum of following 3 parameters.
 * 1. Max. No.of IpsecL2tp Tunnels which need PortChange(NAT Traversal).
 *    This cannot exceed Maximum number of L2TP connections in the system.
 * 2. Some static value(4- L2tpCommandSocket,InterfaceSocket,ApplicationFd,DriverFD)
 * 3. Max.No. of Interfaces.(Because for each interface one FD will be added).
 *    This value can be equal to sum of Max.No.Of PhysicalInterfaces and 
 *    Dynamic Interfaces(Aliases,PPPInterfaces etc.,). This value can be tuned
 *    by using the following MACRO.
 */
#define MAX_L2TP_VIRTUAL_INTERFACES 50

/*******************************************************************/
/*                                                                 */
/*       L2TP Protocol Specific Tunable Parameters                 */
/*                                                                 */
/*******************************************************************/
#define L2TP_HOST_NAME          "IntotoSoft"
#define L2TP_VENDOR_NAME        "Intoto"

/*******************************************************************/
/*                                                                 */
/*            DoDSL(PPPoE) Basic Tunable Parameters                */
/*                                                                 */
/*******************************************************************/

/*
 * This macro indicates the maximum number of PPPoE connections 
 * allowed in the system
 */
#define MAX_PPPOE_CONNECTIONS 2

/*******************************************************************/
/*                                                                 */
/*       PPPoE Protocol Specific Tunable Parameters                */
/*                                                                 */
/*******************************************************************/
#define PPPoE_HOST_UNIQ_TAG     "Intoto" 

/********************************************************************
 *                 HAMGR basic tunable parameters 
 ********************************************************************/
#define HA_MGR_BULK_SYNC_TIMEOUT             180
#define HA_MGR_GRACEFULL_SHUTDOWN_TIMEOUT    180

#if 0
   #define AD_MAX_ASSOC_PER_SUBSCRIBER    4096
   #define AD_MAX_ASSOC                   4096

   #define AD_HASH_MAX                    64
   #define IAP_MAX_POL_ALWD               50    

   #define AD_SNET_ASSOC_DEFACTR          1300
   #define AD_EXT_ASSOC_DEFACTR           1275
   #define AD_SELF_ASSOC_DEFACTR          190
   #define AD_EXT2SELF_TCP_DEFACTR        30


   #define ADBCAST_MAX_SESSIONS 10
   #define ADBCAST_HASH_MAX 8
#endif

   #define AD_MAX_ASSOC                   500000
   #define AD_MAX_ASSOC_PER_SUBSCRIBER    ((AD_MAX_ASSOC)/IGW_MAX_SNETS)

   #ifdef IGW_GBL_STATIC_HASH_ARRAY
      #define AD_HASH_MAX                    (256*1024)
   #else
   /* when hash table changed from single to two levels*/ 
      #define AD_HASH_MAX                  (256*1024) 
   /*   #define AD_HASH_MAX                  2048  */
   #endif
   #define AD_MAX_HASH_BUCKET             2048   
   #define IAP_MAX_POL_ALWD               1000 
    
   #define AD_SNET_ASSOC_DEFACTR          1300
   #define AD_EXT_ASSOC_DEFACTR           1275
   #define AD_SELF_ASSOC_DEFACTR          190
   #define AD_EXT2SELF_TCP_DEFACTR        30

   #define ADBCAST_MAX_SESSIONS 10000
   #define ADBCAST_HASH_MAX 8

 /* Maximum ruleids with which an external 
  * module can register with firewall RID module */
  #define IAP_MAX_DEFAULT_RULE_VALUE      2048

/*
 * Maximum number of Issue Ids per Message Id.
 */
#define IGW_MSGS_MAX_ISSUEID_PER_MSGID 20

/*
 * Maximum Threshold Q length.
 */
#define IGW_MSGS_MAX_Q_THRESHOLD     75

/*
 * Maximum Threshold time in minutes.
 */
#define IGW_MSGS_MAX_TIME_THRESHOLD  60 

#define IGW_LOG_CLTR_MONITOR_TMR_IN_SECS  120   

/* 
 * Default Throttling count
 */
#define IGW_MSGS_THROTT_COUNT 100

/* 
 * Default Throttling time in seconds
 */
#define IGW_MSGS_THROTT_TIME 300

//for the sake of all the macro in licmgr
#define IGWLIMT_RSApk "-----BEGIN PUBLIC KEY-----\n" \
  "MIIBIDANBgkqhkiG9w0BAQEFAAOCAQ0AMIIBCAKCAQEAqw0e9lemLc8OlZL9tVby\n" \
  "VERYc29DUia6iWLkmUXzPSFHPqTUQVQt0jS98VbWNp/TJSyxxl2Ssh/D/TBEK/v6\n" \
  "dbTgKRsSdoEXkVrTxvegDUABAgLb22zcxvOmvm5+0WgPulngA0vj1hFYB0nx+JS0\n" \
  "reuJvJzYJcVwnR8YnMapSTQdcLU5uOTGl5d4svPNr/7XWJZmdBe7weygLWBMY0KD\n" \
  "r8P6XOB2U9nso7v09HjMJKglo6XoYy1WpvLECYLaN3rjTWBVJ3dB9hZrIC6tyAZD\n" \
  "86NmRJd7NoKF8zWbNozHWI9OW0wZMXdGRtzCW26zuS8IX9LN6FeDZ1V9rsv3Kzrt\n" \
  "JwIBAw==\n" \
  "-----END PUBLIC KEY-----\n"

#define IGWLIMT_MAX_LICENSE_FILES  2000
#define IGWLIMT_IGATEWAY_LICENSE_DIR "/igateway/license"

#define IGWLIMT_PRODUCT_ID "iGatewayEnterpriseUTM"
#define IGWLIMT_DISTRIBUTOR_ID "Intoto"
#define IGWLIMT_DISTRIBUTOR_ID_FILE "/igateway/config/distributor.id"
#define IGWLIMT_DISTRIBUTOR_PRODUCT_NAME "PN-X"

/*Partner Service IDs*/

#define IGWCLAVT_SERVICE_ID "CLAMAV"
#define IGWCLAVT_SERVICE_NAME "ClamAV AntiVirus Engine"

#define IGWKAVT_SERVICE_ID "KAV"
#define IGWKAVT_SERVICE_NAME "Kaspersky AntiVirus Engine"

#define IGWSAT_SERVICE_ID "SPAMASIN"
#define IGWSAT_SERVICE_NAME "SA AntiSpam Engine"

#define IGWCOMMT_SERVICE_ID "COMMTCH"
#define IGWCOMMT_SERVICE_NAME "Commtouch AntiSpam Engine"
