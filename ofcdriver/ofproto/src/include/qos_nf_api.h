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
 * @file    qos_nf_api.h
 * @brief   This header file contains the QOS NF API prototypes, 
 *      related macros and data structures
 * @addtogroup  QOS
 * @ingroup EGRESS_QOS
 */
#ifndef __NF_QOS_H
#define __NF_QOS_H
#include "qos_common.h"
/*!
 * Input parameters used to configure qos record
 */
struct nf_qos_cfg_inargs{
		uint32_t   object_type; /*!< Q_ROOT/Q_PRIO/Q_WBFS/CLASS*/
		uint32_t   object_id;   /*!< depending on object type, this could be qdisc handle or classid*/
		uint32_t   parent_id;   /*!< 0xffffffff means ROOT*/
        uint32_t if_id;         /*!< interface on which qos is being configured*/
		uint32_t   rate;         
		uint32_t   ceil;
        uint32_t  class_weight;
		uint16_t  mpu;    
		uint16_t   overhead;
		uint16_t   queues;
		uint8_t    weight[TC_CEETM_NUM_MAX_Q];
		uint8_t    cr_map[TC_CEETM_NUM_MAX_Q];/*!< Each byte 0-7 indicates the Comitted Rate */
		uint8_t   er_map[TC_CEETM_NUM_MAX_Q]; /*!< Excess Rate Eligibilty of respective queue */
        uint8_t    oper;/*!< ADD-1,DEL-2*/

}; 
   

/*!
 * Input parameters used to configure filter record
 */
struct nf_qos_filter_inargs{
		uint32_t if_id;
        uint32_t priority;
		uint32_t src_ip;
		uint32_t dst_ip;
        uint32_t src_mask;
        uint32_t dst_mask;
		uint32_t src_port;
		uint32_t dst_port;
		uint32_t vlan_id;
        uint32_t classQ_id;
		uint8_t protocol;
		uint8_t dscp;
};

/*!
 *  Output parameters used for addition and deletion of
 *  qos records
 */
struct nf_qos_outargs{
    int32_t result; /*!< contains the result of the operation*/
};

/*!
 * @brief   This API is used to create/update/delete schedulers and shapers for egress traffic. 
 *
 * @details This function first validates incoming parameters and then invokes main-ttp api.In CEETM case,this api will be invoked for  adding/deleting tc qdiscs and tc classes.
 *      
 *
 * @param[in]   nsid - Name space Identifier.
 * @param[in]   in - Pointer to input param structure 
 *      which contains  ARP record information.
 * @param[in]   resp - Response arguments for asynchronous call.
 *
 * @param[out]  out - Pointer to output param structure
 *      that will be filled with output values of this API.
 *
 * @returns SUCCESS on success; FAILURE otherwise.
 * @ingroup QOS 
 */
int32_t nf_qos_add_config_egress_traffic_mgmt (
    uint32_t      nsid,
    struct nf_qos_cfg_inargs *in,
    struct nf_qos_outargs *out,
    void *resp);

/*!
 * @brief   This API is used to delete schedulers and shapers for egress traffic. 
 *
 * @details This function first validates incoming parameters and then invokes main-ttp api.In CEETM case,this api will be invoked for  deleting/resetting current configuration.
 *      
 *
 * @param[in]   nsid - Name space Identifier.
 * @param[in]   in - Pointer to input param structure 
 *      which contains  ARP record information.
 * @param[in]   resp - Response arguments for asynchronous call.
 *
 * @param[out]  out - Pointer to output param structure
 *      that will be filled with output values of this API.
 *
 * @returns SUCCESS on success; FAILURE otherwise.
 * @ingroup QOS 
 */
int32_t nf_qos_del_config_egress_traffic_mgmt (
    uint32_t      nsid,
    struct nf_qos_cfg_inargs *in,
    struct nf_qos_outargs *out,
    void *resp);


   
/*!
 * @brief   This API is used to add classification rule to classify egress traffic into some already added traffic class. 
 *
 * @details This function first validates incoming parameters and then invokes registered main-ttp api.In CEETM case,this api will be invoked to add tc filter rules.
 *      
 *
 * @param[in]   nsid - Name space Identifier.
 * @param[in]   in - Pointer to input param structure 
 *      which contains  ARP record information.
 * @param[in]   resp - Response arguments for asynchronous call.
 *
 * @param[out]  out - Pointer to output param structure
 *      that will be filled with output values of this API.
 *
 * @returns SUCCESS on success; FAILURE otherwise.
 * @ingroup QOS 
 */
int32_t nf_qos_add_egress_cls_rule(
    uint32_t      nsid,
    struct nf_qos_filter_inargs *in,
    struct nf_qos_outargs *out,
    void *resp);


/*!
 * @brief   This API is used to delete classification rules added to classify  egress traffic. 
 *
 * @details This function first validates incoming parameters and then invokes registered main-ttp api.In CEETM case,this api will be invoked to delete tc filter rules.
 *      
 *
 * @param[in]   nsid - Name space Identifier.
 * @param[in]   in - Pointer to input param structure 
 *      which contains  ARP record information.
 * @param[in]   resp - Response arguments for asynchronous call.
 *
 * @param[out]  out - Pointer to output param structure
 *      that will be filled with output values of this API.
 *
 * @returns SUCCESS on success; FAILURE otherwise.
 * @ingroup QOS 
 */
int32_t nf_qos_del_egress_cls_rule(
    uint32_t      nsid,
    struct nf_qos_filter_inargs *in,
    struct nf_qos_outargs *out,
    void *resp);

#endif 
