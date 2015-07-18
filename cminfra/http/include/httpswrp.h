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
/*  File        : httpswrp.h                                              */
/*                                                                        */
/*  Description : This file contains function prototypes required by      */
/*                Https server.                                           */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.14.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef __HTTPSWRP_H
#define __HTTPSWRP_H

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 *
 */
void IGWSSL_load_error_strings( void );

/**
 *
 */
void IGWSSLeay_add_ssl_algorithms( void );

/**
 *
 */
SSL_METHOD *IGWSSLv23_server_method( void );

/**
 *
 */
SSL_CTX *IGWSSL_CTX_new( SSL_METHOD  *meth );

/**
 *
 */
int32_t IGWSSL_CTX_use_certificate( SSL_CTX  *ctx,
                                    X509     *pCert );

/**
 *
 */
int32_t IGWSSL_CTX_use_PrivateKey( SSL_CTX   *ctx,
                                   EVP_PKEY  *pKey );

/**
 *
 */
int32_t IGWSSL_CTX_check_private_key( SSL_CTX  *ctx );

/**
 *
 */
int32_t IGWSSL_use_certificate( SSL   *ctx,
                                X509  *pCert );

/**
 *
 */
int32_t IGWSSL_use_PrivateKey( SSL       *ctx,
                               EVP_PKEY  *pKey );

/**
 *
 */
int32_t IGWSSL_check_private_key( SSL  *ctx );

/**
 *
 */
int32_t IGWSSL_write( SSL      *ssl,
                      void   *buf,
                      int32_t  len );

/**
 *
 */
int32_t IGWSSL_read( SSL      *llsd,
                     void   *lbuf,
                     int32_t  len,
                     uint32_t *pRdStatus );

/**
 *
 */
SSL *IGWSSL_new( SSL_CTX  *ctx );

/**
 *
 */
int32_t IGWSSL_accept( SSL  *ssl );

/**
 *
 */
int32_t IGWSSL_set_rfd( SSL      *ssl,
                        int32_t  handle );

/**
 *
 */
int32_t IGWSSL_set_fd( SSL      *ssl,
                       int32_t  handle );

/**
 *
 */
int32_t IGWSSL_is_init_finished( SSL  *ssl );

/**
 *
 */
int32_t IGWSSL_get_error( SSL      *ssl,
                          int32_t  iRetVal );

/**
 *
 */
void IGWSSL_free( SSL  *ssl );

#ifdef HTTPS_ENABLE
int32_t SaveHttpsCerts( void );
#endif /* HTTPS_ENABLE */

#endif /* __HTTPSWRP_H */
