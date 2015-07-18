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

#ifndef __QOS_COMMON_H
#define __QOS_COMMON_H

#define TC_CEETM_NUM_MAX_Q 8
#define MAX_NUM_OF_CHANNELS 16
#define NUM_OF_QUEUES_PER_CHANNEL 16

enum nf_qos_cfg_oper{
       QOS_CFG_ADD = 1,
       QOS_CFG_DEL
};


enum qos_obj_types{
        CEETM_Q_ROOT = 1,/*!< Root Qdisc which has Priority Scheduler
                           and Shaper at Logical Network Interface */
        CEETM_Q_PRIO,   /*!< Inner Qdisc which has Priority Scheduler
                           functionality */
        CEETM_Q_WBFS,    /*!< Inner Qdisc which has Weighted Fair
                           Queueing functionality */
        CEETM_CLASS ,
        CEETM_FILTER
};

#endif
