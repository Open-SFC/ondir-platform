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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include "trincld.h"
//#include "cmtransgdf.h"
#include "cmincld.h"
#include <signal.h>
#include <setjmp.h>
#include "cmdefs.h"
#include "cmutil.h"
#include "dmgdef.h"
#include "cmtransgdf.h"
#include "jegdef.h"
#include "jegif.h"
#include "histedit.h"
#include "transgif.h"
#include "jewrap.h"


#define SUCCESS 0
#define FAILURE -1


#define UCMJE_CONFIGREQUEST_PORT 63008


static PyObject* add_record(PyObject* self, PyObject * args, PyObject *kwargs);
static PyObject* update_record(PyObject* self,PyObject * args, PyObject *kwargs);
static PyObject* delete_record(PyObject* self,PyObject * args, PyObject *kwargs);
static PyObject* get_exact_record(PyObject* self, PyObject * args, PyObject *kwargs);


static inline PyObject* send_cmd(uint32_t command_id,PyObject * args, PyObject *kwargs);

void *create_config_session(void);
void create_transport_channel(void);
void close_transport_channel(void);
