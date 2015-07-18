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

#ifndef CNTRLR_UCMCBK_H
#define CNTRLR_UCMCBK_H
/* 
 * It describes the ucm component at Controller side
 */
int32_t of_switch_appl_ucmcbk_init (void);
int32_t of_domain_appl_ucmcbk_init (void);

int32_t of_switch_tablestats_ucmcbk_init (void);
int32_t of_switch_portstats_ucmcbk_init (void);
int32_t of_switch_flowstats_ucmcbk_init (void);

int32_t dprm_table_appl_ucmcbk_init (void);
int32_t datapath_appl_ucmcbk_init (void);
int32_t of_port_appl_ucmcbk_init (void);

int32_t of_portstats_appl_ucmcbk_init (void);
int32_t of_tablestats_appl_ucmcbk_init (void);
int32_t of_flowstats_appl_ucmcbk_init (void);
int32_t of_aggrstats_appl_ucmcbk_init (void);
int32_t of_tblfeatures_appl_ucmcbk_init(void);


#endif /*CNTRLR_UCMCBK_H*/
