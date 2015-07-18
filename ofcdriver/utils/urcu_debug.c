//#ifndef _DLL_C
//#define _DLL_C

#include <stdio.h>
#include <controller.h>
#include <dll.h>
#include <pthread.h>


#define DEP_OF_CALLER 3
#define DEBUG_NODE_COUNT 128
#define CALLER_NAME_LEN 256

static uint32_t __thread rcu_take_count_g=0;
static uint32_t __thread rcu_rel_count_g=0;

typedef struct urcu_debug_info {
           struct dll_queue_node node;
           ulong callers[DEP_OF_CALLER];
           char caller_name[CALLER_NAME_LEN];
           uint32_t caller_line;
           uint32_t take_count;
           uint32_t rel_count;
}urcu_debug_node_t;

static struct dll_queue __thread rcu_dbg_queue;
static struct dll_queue __thread dbg_nodes_freelist;
static pthread_t __thread my_thread_id=0;

void urcu_debug_node_release(urcu_debug_node_t  *pNode); 
static urcu_debug_node_t __thread urcu_dbg_node[DEBUG_NODE_COUNT];

void urcu_debug_infra_init()
{
  uint32_t ii;
  dll_queue_init_list (&dbg_nodes_freelist);
  dll_queue_init_list (&rcu_dbg_queue);

  printf("\n%s:%d urcu_debug_infra_init called \n", __FUNCTION__, __LINE__);
  
  for (ii = 0; ii < DEBUG_NODE_COUNT; ii++)
    urcu_debug_node_release(&urcu_dbg_node[ii]);

  printf("Count of Debug Queue::%u \n", PSP_DLLQ_COUNT(&rcu_dbg_queue));
  printf("Count of Freelist::%u \n", PSP_DLLQ_COUNT(&dbg_nodes_freelist));  
  my_thread_id = pthread_self();
}
              
void dump_debug_queue(void)
{
  short ii = 0;
  urcu_debug_node_t *pNode;
  printf("%s:%d debug queue details:: \n", __FUNCTION__, __LINE__);
  printf("Count::%u \n", PSP_DLLQ_COUNT(&rcu_dbg_queue));
  DLL_QUEUE_SCAN( &rcu_dbg_queue, pNode, urcu_debug_node_t*)
  {
     printf(" Node %u Caller: %lx\n", ii++, pNode->callers[0]);
  }


}

/* Freelist management functions */
urcu_debug_node_t *urcu_debug_node_alloc(void)
{
   urcu_debug_node_t *pNode;
   pNode = (urcu_debug_node_t *)DLL_QUEUE_FIRST(&dbg_nodes_freelist); 
   if (!pNode)
   {
     printf("%s %d \n", __FUNCTION__, __LINE__);
     if (self_thread_info)
     {
       printf("\nthread_info = [%u:%s] %s: Alloc debug node failed\n",
         self_thread_info->thread_id, self_thread_info->name, __FUNCTION__);
     }
     else
       printf("%s %d self_thread_info null at this moment for thread %lu\n", 
         __FUNCTION__, __LINE__,
         pthread_self());
     dump_debug_queue();
     return NULL;
   }
   DLL_QUEUE_DELETE_NODE(&dbg_nodes_freelist, &(pNode->node));
   return pNode;
}

void urcu_debug_node_release(urcu_debug_node_t  *pNode)
{
   DLL_QUEUE_APPEND_NODE(&dbg_nodes_freelist, &(pNode->node));
}

/*rcu_dbg_queue management functions */
void urcu_debug_take(char *urcu_caller, uint32_t line)
{
   uint32_t ii;
   urcu_debug_node_t *pDbgNode;
   if (!my_thread_id)
   {
     printf("\n%s:Thread %lx  is not initialized for RCU usage \n", __FUNCTION__, pthread_self());
     //CNTLR_RCU_REGISTER_THREAD();
     urcu_debug_infra_init();
     //return;
   }
   
   pDbgNode = urcu_debug_node_alloc();
   if (!pDbgNode)
   {
     printf("\n%s:Unable to allocate debug info node, aborting\n", __FUNCTION__);
     return;
   }
   /* Store the necessary information into the debug node */
   for (ii = 0; ii < DEP_OF_CALLER; ii++)
	 pDbgNode->callers[ii] = (ulong) __builtin_return_address(ii+1);

   strcpy(pDbgNode->caller_name, urcu_caller);
   pDbgNode->caller_line = line;
   pDbgNode->take_count = rcu_take_count_g++;
   pDbgNode->rel_count = rcu_rel_count_g;
   DLL_QUEUE_PREPEND_NODE(&rcu_dbg_queue, &(pDbgNode->node));
   return;
}


void urcu_debug_release(char *urcu_rel_called, uint32_t line)
{
   urcu_debug_node_t *pNode;
   if (!my_thread_id)
   {
     printf("\n%s:Thread %lx  is not initialized for RCU usage \n", __FUNCTION__, pthread_self());
     printf("\n##### Release from invalid thread = %p %p %p #####\n",
        __builtin_extract_return_addr((void *)__builtin_return_address(0)),
        __builtin_extract_return_addr((void *)__builtin_return_address(1)),
        __builtin_extract_return_addr((void *)__builtin_return_address(2)));
    return;
   }
   pNode = (urcu_debug_node_t *)DLL_QUEUE_FIRST(&rcu_dbg_queue);
   if (!pNode)
   {
     
     printf("\n Released called from fun [%s:%d]\n", urcu_rel_called, line);
     printf("\n%s:%d Thread id %lx\n", __FUNCTION__, __LINE__, my_thread_id);
     printf("\n##### Invalid Release = %p %p %p #####\n",
        __builtin_extract_return_addr((void *)__builtin_return_address(0)),
        __builtin_extract_return_addr((void *)__builtin_return_address(1)),
        __builtin_extract_return_addr((void *)__builtin_return_address(2)));
     return;
   }
   if (pNode->callers[0] != (ulong)__builtin_return_address(1))
   {
     printf("\n <<< Released called from fun %s:%d >>>> \n", urcu_rel_called, line);
     printf("\n <<< Lock taken from %s:%d >>> \n", pNode->caller_name, pNode->caller_line);

     printf("\nself_thread_info = [%u:%s] %s:Probable case of misuse alloc 0x%lx release %p %p\n",
        self_thread_info->thread_id, self_thread_info->name, __FUNCTION__, pNode->callers[0] , __builtin_return_address(0),__builtin_return_address(1));
     printf("\n%s:Stacked callers caller2 0x%lx caller3 0x%lx\n",
         __FUNCTION__, pNode->callers[1] ,pNode->callers[2]);
     //printf("%s%d\n", __FUNCTION__, __LINE__);
#if 1
     printf("\n##### Final alloc  %p %p  %p Release = %p %p %p #####\n",
        __builtin_extract_return_addr((void *)pNode->callers[0]) ,
        __builtin_extract_return_addr((void *)pNode->callers[1]) ,
        __builtin_extract_return_addr((void *)pNode->callers[2]) ,
        __builtin_extract_return_addr((void *)__builtin_return_address(1)),
        __builtin_extract_return_addr((void *)__builtin_return_address(2)),
        __builtin_extract_return_addr((void *)__builtin_return_address(3)));
     printf("%s%dCount of the Debug Queue for this thread %u\n", __FUNCTION__, __LINE__, 
        PSP_DLLQ_COUNT(&rcu_dbg_queue));
#endif
     return;
   }
   DLL_QUEUE_DELETE_NODE(&rcu_dbg_queue, &(pNode->node));
   rcu_rel_count_g++;
   urcu_debug_node_release(pNode);
   //printf("\n%s alloc-count %u rel-count %u \n", __FUNCTION__, rcu_take_count_g, rcu_rel_count_g);
   return;
}
//#endif
