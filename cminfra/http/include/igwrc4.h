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
/************************************************************************/
/*  File         : igwrc4.h                                             */
/*                                                                      */
/*  Description  : This file contains the data structure definations    */
/*                 and function prototypes for RC4 encryption algorithm.*/
/*                                                                      */
/*  Version      Date      Author      Change Description               */
/*  -------    --------    ------    ----------------------             */
/*                                                                      */
/************************************************************************/

#ifndef _IGW_RC4_H_			 
#define _IGW_RC4_H_

#define MAX_KEY_STATE 256
typedef struct IGWRC4key_s 
{ 
  uint32_t ulFirstIdx; 
  uint32_t ulSecondIdx; 
  uint32_t state[MAX_KEY_STATE]; 
}IGWRC4key_t; 

void IGWRC4SetKey(
                    IGWRC4key_t  * key,
                    uint32_t  lDatalen, 
                    const uint8_t * pDataPtr
                  );
uint32_t IGWRC4(
              IGWRC4key_t * pRC4Key, 
              uint32_t usKeyLen, 
              const uint8_t * pKey,
              uint8_t * pOutDataPtr
            );
  

#endif /* _IGW_RC4_H_ */

