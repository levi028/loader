/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/

#include "sony_common.h"

#ifndef _WINDOWS
sony_result_t sony_stopwatch_start (sony_stopwatch_t * pStopwatch)
{
//#error sony_stopwatch_start is not implemented
	pStopwatch->startTime = osal_get_tick();
	return SONY_RESULT_OK;
}

sony_result_t sony_stopwatch_sleep (sony_stopwatch_t * pStopwatch, uint32_t ms)
{
//#error sony_stopwatch_sleep is not implemented
    SONY_SLEEP (ms);
    return SONY_RESULT_OK;
}

sony_result_t sony_stopwatch_elapsed (sony_stopwatch_t * pStopwatch, uint32_t* pElapsed)
{
//#error sony_stopwatch_elapsed is not implemented
//    return 0;
    *pElapsed = osal_get_tick() - pStopwatch->startTime;
    return SONY_RESULT_OK;
}
#else

#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

sony_result_t sony_stopwatch_start (sony_stopwatch_t * pStopwatch)
{
    SONY_TRACE_ENTER("sony_stopwatch_start");

    if (!pStopwatch) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    pStopwatch->startTime = timeGetTime ();

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_stopwatch_sleep (sony_stopwatch_t * pStopwatch, uint32_t ms)
{
    SONY_TRACE_ENTER("sony_stopwatch_sleep");
    if (!pStopwatch) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    SONY_ARG_UNUSED(*pStopwatch);
    SONY_SLEEP (ms);

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}

sony_result_t sony_stopwatch_elapsed (sony_stopwatch_t * pStopwatch, uint32_t* pElapsed)
{
    SONY_TRACE_ENTER("sony_stopwatch_elapsed");

    if (!pStopwatch || !pElapsed) {
        SONY_TRACE_RETURN(SONY_RESULT_ERROR_ARG);
    }
    *pElapsed = timeGetTime () - pStopwatch->startTime;

    SONY_TRACE_RETURN(SONY_RESULT_OK);
}
#endif
