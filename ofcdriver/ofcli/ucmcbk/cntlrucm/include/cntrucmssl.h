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
#ifndef __CNTRUCMSSL_H
#define __CNTRUCMSSL_H

#define CNTLR_CM_CERT_FILE_PATH_SIZE 256

struct cntlr_ucm_certs_info{
   char ca_cert_file[CNTLR_CM_CERT_FILE_PATH_SIZE];
   char self_cert_file[CNTLR_CM_CERT_FILE_PATH_SIZE];
   char priv_key_file[CNTLR_CM_CERT_FILE_PATH_SIZE];
   char priv_key_pwd[32];
};

struct cntlr_ucm_cacert_info{
   char ca_cert_file[CNTLR_CM_CERT_FILE_PATH_SIZE];
};

int32_t
of_ssl_load_ca_certificate(struct cntlr_ucm_cacert_info *pca_cert_info);

int32_t
of_ssl_load_certificates(struct cntlr_ucm_certs_info *pcert_info);


#endif
