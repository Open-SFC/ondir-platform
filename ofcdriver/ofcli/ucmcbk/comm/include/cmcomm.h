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

#ifndef CM_COMM_H
#define CM_COMM_H

#define CM_MAX_CPDP_CMCCNTXT_BUF 2048
//#define  CM_COMM_DEBUG

#ifdef CM_COMM_DEBUG
#define CM_COMM_DEBUG_PRINT(format,args...) printf("\r\n%s(%d)::"format,__FUNCTION__,__LINE__,##args)
#else
#define CM_COMM_DEBUG_PRINT(format,args...)
#endif

int32_t UCMCreateCommChannelwithController();


#endif /* CM_CNTRLR_H */
