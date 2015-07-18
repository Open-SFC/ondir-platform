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
/**************************************************************************/
/*  File        : hcmgrldf.h                                              */
/*                                                                        */
/*  Description : This file contains local definitions required by the    */
/*                https certificate functions.                            */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.11.04    DRAM      Modified during the code review.     */
/**************************************************************************/


#ifndef HTTPSLDEF_H
#define HTTPSLDEF_H

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  HTTPS_NO_ERROR                 (0)
#define  HTTPS_ENCODED_BIN              (1)
#define  HTTPS_ENCODED_B64              (2)
#define  HTTPS_ENCODED_PEM              (3)

#define  HTTPS_CERTS_INVALID            (0)
#define  HTTPS_CERTS_SELF               (1)
#define  HTTPS_CERTS_TRUSTED            (2)

#define  HTTPS_MEM_ALLOC_FAILED         (0x0001)
#define  HTTPS_PEM_MALFORMED            (0x0101)
#define  HTTPS_I2D_EXTN_DECODE_FAILED   (0x0202)

#define  HTTPS_ERROR_MAX_SELF_CERTS     (0x0203)

#define  HTTPS_MAX_SELF_CERTS           (10)
#define  HTTPS_FAILED                   (11)
#define  HTTPS_DEVICE_NONE              (12)
#define  HTTPS_MAX_CERT_DATA            (2000)
#define  HTTPS_CHAIN_FORM_FAILED        (2003)

/**
 * Specifies a single four octet IPV4 address.
 */
#define  HTTPS_ID_IPV4_ADDR             (1)

/**
 * Specifies a fully qualified domain name.
 */
#define  HTTPS_ID_FQDN                  (2)

/**
 * Specifies a fully qualified username.
 */
#define  HTTPS_ID_USER_FQDN             (3)

/**
 * Specifies a range of IPV4 addresses, represented by two four octet
 * values. The first value is an IPV4 address and second is an IPV4 network
 * mask.
 */
#define  HTTPS_IPV4_ADDR_SUBNET         (4)

/**
 * Specifies a 16 octet IPV6 address.
 */
#define  HTTPS_IPV6_ADDR                (5)

/**
 * Specifies a range of IPV6 addresses, represented by two 16 octet values.
 * The first value is an IPV6 address and second is an IPV6 network mask.
 */
#define  HTTPS_IPV6_ADDR_SUBNET         (6)

/**
 * Specifies a range of IPV4 addresses, represented by two four octet
 * values. The first value is the beginning IPV4 address and second value
 * is the ending IPV4 address both inclusive.
 */
#define  HTTPS_IPV4_ADDR_RANGE          (7)

/**
 * Specifies a range of IPV6 addresses, represented by two four octet
 * values. The first value is the beginning IPV6 address and second value
 * is the ending IPV6 address both inclusive.
 */
#define  HTTPS_IPV6_ADDR_RANGE          (8)

/**
 * Specifies the binary DER encoding of an ASN.1 X.500 Distinguished Name
 * of the principal whose certificates are being exchanged to establish the
 * SA.
 */
#define  HTTPS_ID_DER_ASN1_DN           (9)

/**
 * Specifies the binary DER encoding of an ASN.1 X.500 GeneralName of the
 * principal whose certificates are being exchanged to establish the SA.
 */
#define  HTTPS_ID_DER_ASN1_GN           (10)

/**
 * Specifies an opaque byte stream which may be used to pass vendor-
 * specific information necessary to identify which pre shared key should be
 * used to authenticate aggressive mode negotiations.
 */
#define  HTTPS_ID_KEY_ID                (11)

#endif
