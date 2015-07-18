/**
 *  Copyright (c) 2012, 2013  Freescale.
 *   
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 **/
/*
 * File:        pethqueue.c
 * 
 * Description: This file contains the queue utilities for peth net devices.
 *              These utilities are used in both user space and in kernel space.
 *
 * Authors:     mallik.mlr@freescale.com
 *
 */

/* INCLUDE_START */
#ifndef __KERNEL__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <dirent.h>
#include <dlfcn.h>

#include <urcu/arch.h>
#include <urcu.h>
#include <urcu-call-rcu.h>

#else

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/netdevice.h>
#include <linux/ioctl.h>
#include <linux/skbuff.h>
#include <linux/kthread.h>

#endif

#include "../ofproto/src/include/cntlr_types.h"
#include "../ofproto/src/include/cntlr_lock.h"
#include "../include/dll.h"
#include "../include/pktmbuf.h"              
#include "../include/pethapi.h" 


#ifdef __KERNEL__
#define printf pr_debug
#else
#define printf(format,args...)	
#endif



inline uint16_t peth_drv_queue_count (struct peth_net_queue *queue_p)
{
        if (queue_p->write >= (queue_p->read + 1))
                return (queue_p->write - (queue_p->read + 1));
        else
                return (queue_p->write + queue_p->queue_size - (queue_p->read + 1));
}

inline uint16_t peth_drv_queue_free_count (struct peth_net_queue *queue_p)
{
        if (queue_p->read >= queue_p->write)
                return (queue_p->read - queue_p->write);
        else
                return (queue_p->read + queue_p->queue_size - queue_p->write);
}

/*
 * De-queue the requested number of elements from the queue into the
 * array of void pointers.
 *
 * Returns the number of elements de-queued.
 */
uint16_t peth_drv_queue_get (struct peth_net_queue *queue_p,
			struct peth_queue_elem *data, uint16_t num_elem)
{
        uint16_t last_read = queue_p->read;
        uint16_t cur_read;
        uint16_t write_pos = queue_p->write;
        unsigned int count = 0;

        for (count = 0; count < num_elem; ++count) {
                cur_read = last_read + 1;
                if (cur_read >= queue_p->queue_size)
                        cur_read -= queue_p->queue_size; /* wrap around */

                if (cur_read == write_pos) /* queue empty */
		{
//			printf("queue empty. write_pos = %d cur_read = %d queue_size = %d\n",write_pos, cur_read, queue_p->queue_size);
                        break;
		}

		/* structure assignment. */
                data[count] = queue_p->queue_elems[cur_read];

                last_read = cur_read;
        }
        queue_p->read = last_read;
        return (count);
}

/*
 * Enqueue the given buffer pointers to the queue.
 *
 * Returns the number of elements enqueued.
 */
uint16_t peth_drv_queue_put (struct peth_net_queue *queue_p,
			struct peth_queue_elem *data, uint16_t num_elem)
{
        uint16_t write_pos = queue_p->write;
        uint16_t read_pos = queue_p->read;
        unsigned int count = 0;

        for (count = 0; count < num_elem; ++count) {
                if (write_pos == read_pos) /* queue full */
		{
//			printf(" queue full. write_pos = %d read_pos = %d queue_size = %d\n",write_pos, read_pos, queue_p->queue_size);
                        break;
		}

		/* structure assignment. */
                queue_p->queue_elems[write_pos] = data[count];

                if (++write_pos >= queue_p->queue_size)
                        write_pos -= queue_p->queue_size;
        }
        queue_p->write = write_pos;
        return (count);
}

/**
 * Initializes the peth_net_queue structure
 */
void peth_net_queue_init (struct peth_net_queue *queue_p, uint16_t size)
{
        if (size < 2) {
//		printf ("%s: queue size too small %d\n", __FUNCTION__, size);
                return;
        }
        queue_p->write = 0;
        queue_p->read = size -1;
        queue_p->queue_size = size;
        queue_p->elem_size = sizeof(struct peth_queue_elem);
	// Initialize the array pointer to 0s.
	memset ((void *)(queue_p->queue_elems), 0,
			size * sizeof(struct peth_queue_elem));
	return;
}
