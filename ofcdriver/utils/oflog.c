#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "oflog.h"
#include "controller.h"
#include <sys/time.h>
#include <time.h>
#include <bits/time.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <syslog.h>

/******************************************************************************
* * * * * * * * * * * * * * * MACRO DEFINITIONS* * * * * * * * * * * * * * * * 
*****************************************************************************/
#define OF_LOG_FILE "/var/log/oflog"
#define OF_MAX_DEBUG_FILES 20
#define OF_LOG_FILE_MAX_SIZE  5000000
#define OF_MAX_TRACE_STRING_LEN 2048
#define OF_CHECK_FILE_SIZE_CNT 100

/******************************************************************************
* * * * * * * * * * * * * * * GLOBAL VARIABLES* * * * * * * * * * * * * * * * 
*****************************************************************************/
void of_log_init();
extern __thread uint32_t cntlr_thread_index;
char aLogFileName_g[100] = OF_LOG_FILE;
unsigned int ulFileNo_g = 1;
void oflogPrintMsg (char * pTraceLoc, unsigned int ucLevel, char * pFormatString, va_list ap);
int iFSChkCnt_g=0;
FILE  *fp;
FILE  *fp_old;
cntlr_rwlock_t log_rw_lock_g;
//extern int of_trace_level_g;
void of_log_init()
{

	  fp = fopen (aLogFileName_g, "a");
	  if (!fp)
	  {
		  perror ("fopen");
		  return;
	  }
		CNTLR_RWLOCK_INIT(log_rw_lock_g);
}
void OFLogMessage (int iModuleId, unsigned int ucLevel, const char * pFunc,
    int ulLine, char * pFormatString, ...)
{
  va_list ap;
  char aTraceLoc[100];
  
  if(CNTLR_UNLIKELY(trace_params_values[iModuleId].trace_level == OF_LOG_NONE))
    return;
  if ((trace_params_values[iModuleId].is_trace_module_set == 1) 
		  && (trace_params_values[iModuleId].trace_level >= ucLevel))
  {
    snprintf (aTraceLoc, 100, " %30s(%4d)::  ", pFunc, ulLine);
    va_start (ap, pFormatString);
    oflogPrintMsg (aTraceLoc,  ucLevel, pFormatString, ap);
    va_end (ap);
  }
}

void oflogPrintMsg (char * pTraceLoc, unsigned int ucLevel, char * pFormatString, va_list ap)
{
  char aLrgBuf[OF_MAX_TRACE_STRING_LEN];
  char aCmd[100] = {};
  char aNewFileName[100] = {};
  char aRemoveFileName[100] = {};
  unsigned int ulTraceLength;
  char aTimeStamp[100];
  char athread_info[100];
  struct stat file_status;
  time_t ulTime;
  struct tm                *pLocaltime;
  int iBytesPrinted;
  int iRMFileNo;

  if (!fp)
  {
	  perror ("fopen");
	  return;
  }

//		printf("\nthread %d:Taking read lock\n",cntlr_thread_index);
		CNTLR_RWLOCK_RDLOCK_TAKE(log_rw_lock_g);
  ulTraceLength = OF_MAX_TRACE_STRING_LEN;
  iBytesPrinted=vsnprintf (aLrgBuf, ulTraceLength, pFormatString, ap);
  if( iBytesPrinted >= ulTraceLength)
    aLrgBuf[ulTraceLength - 1] = '\0';
  else
    aLrgBuf[iBytesPrinted] = '\0';

  //printf("of_trace_level_g=%d ucLevel=%d \r\n",of_trace_level_g,ucLevel);
  
#ifdef SYSLOG_SERVER_SUPPORT 
	  syslog(ucLevel,"%s" ,aLrgBuf);
#else
	  ulTime=time(NULL);
	  pLocaltime=localtime(&ulTime);
	  strftime (aTimeStamp, 100, "\n%d %b %y %H:%M:%S ", pLocaltime);

	  snprintf (athread_info, 10, "thread %d",cntlr_thread_index);
	  fprintf (fp, "%s", aTimeStamp);
	  fprintf (fp, "%s", athread_info);
	  fprintf (fp, "%s", pTraceLoc);
	  fprintf (fp, "%s", aLrgBuf);

	  if( iFSChkCnt_g++ < OF_CHECK_FILE_SIZE_CNT )
	  {
		  /* avoid check file size every time
		     Check file size for every predefined number of writes into file 
		   */
///Read-lock release
//		printf("\nthread %d:Releasing rw lock\n",cntlr_thread_index);
			CNTLR_RWLOCK_RELEASE(log_rw_lock_g);
		  return;
	  }
	  iFSChkCnt_g=0;

	  if ((stat (aLogFileName_g, &file_status)) != 0 )
	  {
		  //perror ("stat failed");
//		printf("\nthread %d:Releasing rw lock\n",cntlr_thread_index);
			CNTLR_RWLOCK_RELEASE(log_rw_lock_g);
///Read-lock release
		  return;
	  }
	  if (file_status.st_size > OF_LOG_FILE_MAX_SIZE)
	  {

//		printf("\nthread %d:Releasing rw lock\n",cntlr_thread_index);
			CNTLR_RWLOCK_RELEASE(log_rw_lock_g);
///Read-lock release
///write-lock take
//		printf("\nthread %d:Taking wr lock\n",cntlr_thread_index);
			CNTLR_RWLOCK_WRLOCK_TAKE(log_rw_lock_g);
		  sprintf (aNewFileName, "%s.%d", OF_LOG_FILE, ulFileNo_g);
		  sprintf (aCmd, "mv %s %s", aLogFileName_g, aNewFileName);
		  system(aCmd);
		  memset(&aCmd,0, 100);
		  sprintf (aCmd, "gzip -f %s",aNewFileName);
		  system(aCmd);

		  iRMFileNo=ulFileNo_g - OF_MAX_DEBUG_FILES;
		  if(iRMFileNo > 0) 
		  {
			  sprintf (aRemoveFileName, "%s.%d", OF_LOG_FILE,iRMFileNo);
			  sprintf (aCmd, "rm -f %s", aRemoveFileName);
			  system(aCmd);
			  memset(aRemoveFileName,0,100);
			  sprintf (aRemoveFileName, "%s.%d.gz", OF_LOG_FILE, iRMFileNo);
			  sprintf (aCmd, "rm -f %s", aRemoveFileName);
			  system(aCmd);
		  }
		  ulFileNo_g++;
			fp_old = fp;
		  fp = fopen (aLogFileName_g, "a");
		  if (!fp)
		  {
			  perror ("fopen");
///write-lock release
//		printf("\nthread %d:Releasing rw lock\n",cntlr_thread_index);
			CNTLR_RWLOCK_RELEASE(log_rw_lock_g);
			  return;
		  }
			//printf("\n\nold fp %p new fp %p \n\n",fp_old,fp);
			fclose(fp_old);
///write-lock release
//		printf("\nthread %d:Releasing rw lock\n",cntlr_thread_index);
			CNTLR_RWLOCK_RELEASE(log_rw_lock_g);
			return;
	  }
#endif
//		printf("\nthread %d:Releasing rw lock\n",cntlr_thread_index);
			CNTLR_RWLOCK_RELEASE(log_rw_lock_g);
///Read-lock release
  return ;
}

