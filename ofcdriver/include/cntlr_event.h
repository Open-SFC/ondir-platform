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

#define OPEN_CONN_FROM_DP_EVENT   1
#define CLOSE_CONN_FROM_DP_EVENT  2
#define DP_CONN_TIMEOUT_EVENT     3
#define DP_READY_EVENT            4
#define DP_PORT_STATUS_CHANGE     5
#define DP_ERROR_MESSAGE          6
#define DP_EXPERIMENTER_MSG_EVENT 7 
#define DP_BIND_MOD_ERROR_MSG_EVENT 8 
#define DP_BIND_MOD_MULTIPART_RESP_EVENT 9 
#define DP_DETACH_EVENT            10

#define APP_EVENT_MIN_PRIORITY   1
#define APP_EVENT_MAX_PRIORITY   16

#define PASS_EVENT_TO_NEXT_APP    1
#define EVENT_CONSUMED_BY_APP     2
#define EVENT_DROP_BY_APP     3

typedef int32_t (*async_event_cbk_fn)(uint64_t  controller_handle,
                                    uint64_t   domain_handle,
                                    uint64_t   datapath_handle,
                                    void      *event_info1,
                                    void      *event_info2);
extern struct dprm_datapath_entry *data_path;
void
inform_event_to_app(struct dprm_datapath_entry *data_path,
                    uint8_t                     event_type,
                    void                       *event_info1,
                    void                       *event_info2);
