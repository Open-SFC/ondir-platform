/**
 *  Copyright (c) 2012, 2013  Freescale.
 *   
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 *  unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 **/
#ifndef _OF_LOCK_H_
#define _OF_LOCK_H_

#ifndef __KERNEL__
/**Lock definitions*/
typedef pthread_mutex_t cntlr_lock_t;
typedef pthread_mutex_t PSPLock_t;
typedef pthread_rwlock_t cntlr_rwlock_t;
#define CNTLR_LOCK_INIT(lock) pthread_mutex_init(&(lock), NULL)

#define CNTLR_LOCK_TAKE(lock) pthread_mutex_lock(&(lock))


#define CNTLR_LOCK_RELEASE(lock) pthread_mutex_unlock(&(lock))

#define CNTLR_RWLOCK_INIT(lock) pthread_rwlock_init(&(lock), NULL)

#define CNTLR_RWLOCK_RDLOCK_TAKE(lock) \
{\
pthread_rwlock_rdlock(&(lock));    \
}
   
#define CNTLR_RWLOCK_WRLOCK_TAKE(lock) \
{\
pthread_rwlock_wrlock(&(lock));    \
}
   
//  printf(" %s: Lock Release",__FUNCTION__);    
#define CNTLR_RWLOCK_RELEASE(lock)    \
{\
pthread_rwlock_unlock(&(lock));  \
} 



#define CNTLR_RCU_DE_REFERENCE(data,copy) \
do {\
   copy = rcu_dereference(data);\
} while(0)

//#define URCU_DEBUG
#undef URCU_DEBUG

#ifdef URCU_DEBUG
void  urcu_debug_infra_init(void);
//void  urcu_debug_take(const char *, uint32_t);
//void  urcu_debug_release(const char *, uint32_t);
#define CNTLR_RCU_ASSIGN_POINTER(p,v) \
  do {\
	  rcu_assign_pointer(p,v) ;\
  }while(0)
    

void  urcu_debug_take( char *, uint32_t);
void  urcu_debug_release(char *, uint32_t);
#define CNTLR_RCU_REGISTER_THREAD()  \
  do { \
       rcu_register_thread();\
       urcu_debug_infra_init();\
     }while(0)

#define CNTLR_RCU_READ_LOCK_TAKE() \
  do {\
    char name[256]; \
    uint32_t line = __LINE__; \
    strcpy(name,__FUNCTION__); \
   urcu_debug_take(name,line);\
   rcu_read_lock();  \
  }while(0)

#define CNTLR_RCU_READ_LOCK_RELEASE() \
  do {\
    char name[256]; \
    uint32_t line = __LINE__; \
    strcpy(name,__FUNCTION__); \
    urcu_debug_release(name, line);\
    rcu_read_unlock(); \
  }while(0)

#else
#define CNTLR_RCU_REGISTER_THREAD() rcu_register_thread()
#define CNTLR_RCU_READ_LOCK_TAKE() rcu_read_lock()
#define CNTLR_RCU_READ_LOCK_RELEASE() rcu_read_unlock()
#define CNTLR_RCU_ASSIGN_POINTER(p,v) rcu_assign_pointer(p,v)
#endif


#define CNTLR_CALL_RCU(pArg,pFreeHandler)  \
do {\
    call_rcu((struct rcu_head *) pArg, pFreeHandler);  \
} while(0)

#else

/**Lock definitions*/
typedef spinlock_t cntlr_lock_t;
typedef spinlock_t PSPLock_t;
#define CNTLR_LOCK_INIT(lock) spin_lock_init(&(lock))

#define CNTLR_LOCK_TAKE(lock) spin_lock_bh(&(lock))

#define CNTLR_LOCK_RELEASE(lock) spin_unlock_bh(&(lock))

#define CNTLR_RCU_ASSIGN_POINTER(p,v) rcu_assign_pointer(p,v)

#define CNTLR_RCU_DE_REFERENCE(data,copy) \
do {\
   copy = rcu_dereference(data);\
} while(0)

// #define CNTLR_RCU_REGISTER_THREAD() rcu_register_thread()
#define CNTLR_RCU_READ_LOCK_TAKE() rcu_read_lock()
#define CNTLR_RCU_READ_LOCK_RELEASE() rcu_read_unlock()

#define CNTLR_CALL_RCU(pArg,pFreeHandler)  \
do {\
    call_rcu((struct rcu_head *) pArg, pFreeHandler);  \
} while(0)

#endif

#endif/*_OF_LOCK_H_*/
