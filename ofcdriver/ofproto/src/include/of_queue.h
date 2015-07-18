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

#ifndef __OF_QUEUE_H
#define __OF_QUEUE_H
#if 0
//#include "cntlr_list.h"
#include "controller.h"
#include "of_utilities.h"
#include "of_msgapi.h"
#include "of_multipart.h"
#include "cntrl_queue.h"
#include "of_msgpool.h"
#include "cntlr_tls.h"
#include "cntlr_event.h"
#include "oflog.h"
#include "of_group.h"
#include "of_flowmod.h"
#include "of_flowinfo.h"
#include "cntlr_transprt.h"
#include "fsl_ext.h"
#include "nicira_ext.h"
#endif



#include "controller.h"
#include "of_utilities.h"
#include "of_msgapi.h"
#include "of_multipart.h"
#include "cntrl_queue.h"
#include "of_msgpool.h"
#include "cntlr_tls.h"
#include "cntlr_transprt.h"
#include "cntlr_event.h"
//#include "dprmldef.h"
#include "multi_part.h"
#include "oflog.h"
//#include "of_group.h"
#include "of_flowmod.h"
#include "of_flowinfo.h"
#include "nicira_ext.h"


int32_t
of_queue_send_get_queue_config_request(uint64_t datapath_handle,uint32_t port);
int32_t of_queue_frame_response(char *msg,struct ofl_queue_config *queue_config_info, uint32_t msg_length);
void of_queue_free_queue_entry(struct rcu_head *node);

void of_queue_config_reply_handler(uint64_t  controller_handle,
                                                uint64_t  domain_handle,
                                                uint64_t  datapath_handle,
                                                 struct of_msg *msg,
                                                void     *cbk_arg1,
                                                void     *cbk_arg2,
                                                uint8_t  status,
                                                struct ofl_queue_config *queue_config);

#endif

