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

/**SAMPLE_L3_FWD_TTP: Following macros for SAMPLE_L3_FWD Table type patterns.
    SAMPLE_L3_FWD Table type patterns supports unicast,multicast and ipsec
    table types
 **/
/* SAMPLE_L3_FWD unicast ,multicast &ipsec tables count*/ 
#define CNTLR_SAMPLE_L3_FWD_TTP_TABLES_CNT 6

/**Following  8 tables for unicast table type patters **/
/*Table id0:In Port Classification Table */
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID0_NAME "InPortClassificationTable"
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID0_COLUMNS 1
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID0_ROWS    64

/*Table id1:Etherenet Filtering Table */
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID1_NAME "MultiCastClassifierTable"
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID1_COLUMNS 3
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID1_ROWS    100


/*Table id2:Ethernet Classification Table */
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID2_NAME "MultiCastRoutingTable"
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID2_COLUMNS 2
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID2_ROWS    1024


/*Table id3:ARP Inbound Table */
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID3_NAME "SessionTable"
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID3_COLUMNS 10
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID3_ROWS    4000

/*Table id4:Ipv4 classification Table */
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID4_NAME "L3RoutingTable"
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID4_COLUMNS 3
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID4_ROWS    1024


/*Table id5:IPV4 Policy Based Routing Table */
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID5_NAME "ARPResolveTable"
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID5_COLUMNS 1
#define CNTLR_SAMPLE_L3_FWD_TTP_UNICAST_TABLE_ID5_ROWS    1024
