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

/*File: fslx_error_msg.h
 *Author: Narayana Rao K V V <narayana.k@freescale.com>
 * Description:
 * This file contains Bind Object Management. 
 */

#ifndef FSLX_ERROR_MSG_H
#define FSLX_ERROR_MSG_H

of_error_strings_t of_expr_error_printable_string_data[] = {

 {OFPET_EXPERIMENTER,FSLXET_TABLE_MOD_FAILED_CODE,
    "Table Mod failed : Table mod request failed."},
 {OFPET_EXPERIMENTER,FSLXBMFC_BIND_OBJ_EXISTS,
   "Bind Mod failed : Already present bind object."},
 {OFPET_EXPERIMENTER,FSLXBMFC_INVALID_BIND_OBJ,
   "Bind Mod failed : Specified bind object is invalid."},
 {OFPET_EXPERIMENTER,FSLXBMFC_UNKNOWN_BIND_OBJ,
   "Bind Mod failed : Attempted to modify/delete a non-existant bind object."},
 {OFPET_EXPERIMENTER,FSLXBMFC_BAD_COMMAND,
   "Bind Mod failed : Bad Command."},
 {OFPET_EXPERIMENTER,FSLXBMFC_UNKNOWN_ERROR,
   "Bind Mod failed : Unknown error."},
 {OFPET_EXPERIMENTER,FSLXBMFC_FLOWS_ATTACHED_TO_BIND_OBJ,
   "Bind Mod failed : flows are attached to bind object."},
 {OFPET_EXPERIMENTER,FSLXBMFC_TIMEOUT_OBJ_EXISTS,
   "Hello protocol failed : Timeout object exists."},
 {OFPET_EXPERIMENTER,FSLXBMFC_INVALID_TIMEOUT_OBJ,
   "Hello protocol failed : Invaliid Timeout."},
 {OFPET_EXPERIMENTER,FSLXBMFC_UNKNOWN_TIMEOUT_OBJ,
   "Hello protocol failed : Unknown Timeout."}
};

#define FSLX_EXPER_ERROR_MSGS_COUNT 10

#define FSLX_GET_ERROR_STRING(error_type,error_code,error_string)\
{\
  uint16_t i;\
  for(i=0;i < FSLX_EXPER_ERROR_MSGS_COUNT;i++){\
     if((error_type == of_expr_error_printable_string_data[i].type) && (error_code == of_expr_error_printable_string_data[i].code)){\
       strcpy(error_string,of_expr_error_printable_string_data[i].error_str);\
       break;\
     }\
  }\
  if(i >= FSLX_EXPER_ERROR_MSGS_COUNT)\
    strcpy(error_string,"UNKNOWN ERROR MESSAGE");\
}

#endif /* FSLX_ERROR_MSG_H */
