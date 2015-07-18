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

/**SAMPLE_L2_FWD_TTP: Following macros for SAMPLE_L2_FWD Table type patterns.
    SAMPLE_L2_FWD Table type patterns supports unicast,multicast table types
 **/
/* SAMPLE_L2_FWD unicast ,multicast tables count*/ 
#define CNTLR_SAMPLE_L2_FWD_TTP_TABLES_CNT 4

/*Table id0: EthaddressTypeTable */
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID0_NAME "EthaddressTypeTable"
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID0_COLUMNS 1
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID0_ROWS    64

/*Table id1: MacLearnTable */
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID1_NAME "MacLearnTable"
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID1_COLUMNS 1
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID1_ROWS    64


/*Table id2: FIBTable */
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID2_NAME "FIBTable"
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID2_COLUMNS 1
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID2_ROWS    64


/*Table id3: BroadcastTable */
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID3_NAME "BroadcastTable"
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID3_COLUMNS 0
#define CNTLR_SAMPLE_L2_FWD_TTP_UNICAST_TABLE_ID3_ROWS    64

