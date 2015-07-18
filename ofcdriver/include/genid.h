/* 
 *
 * Copyright  2012, 2013  Freescale Semiconductor
 *
 *
 * Licensed under the Apache License, version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
*/

/*
 *
 * File name: genid.h
 * Author: Narayana Rao KVV <B38869@freescale.com>
 * Date:   09/30/2013
 * Description: This file contains the code for generating Instance ID
*/

#ifndef _GENID_H
#define _GENID_H

#define GENID_IPSEC_INBOUND_SUBTTP_ID  1
#define GENID_IPSEC_OUTBOUND_SUBTTP_ID 2
#define GENID_IPV4_FIREWALL_SUBTTP_ID  3
#define GENID_FW4_CONN_TEMPLATES_ID    4

#define  MAX_BIT_POS 8


#define GENID_SET_BYTE_FOR_SUBTTP( x,  y) \
{ \
  x = ((y << ((sizeof(unsigned int)*8) - MAX_BIT_POS)) | x); \
}

#define GENID_RESET_BYTE_FOR_SUBTTP(x) \
{ \
  x = (0x00FFFFFF & x);\
}

#define GENID_GET_SUBTTP_ID(id, subttpid) \
{ \
    subttpid = (id >> ((sizeof(unsigned int)*8) - MAX_BIT_POS)); \
}

int16_t of_genid_get_new_instance_id (uint32_t *instance_id);

int16_t of_genid_set_bit_for_instance_id (uint32_t instance_id,  uint8_t  setflag);

void of_genid_set_instance_id_value (uint32_t instance_val);

int16_t of_genid_check_instance_id (uint32_t instance_id);

#endif /* _GENID_H */
