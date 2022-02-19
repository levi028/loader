/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2675 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
#include "sony_common.h"

#ifndef _WINDOWS

sony_result_t sony_stopwatch_start (sony_stopwatch_t * pStopwatch)
{
    pStopwatch->startTime = osal_get_tick();
    return SONY_RESULT_OK;
}

sony_result_t sony_stopwatch_sleep (sony_stopwatch_t * pStopwatch, uint32_t ms)
{
    SONY_SLEEP (ms);
    return SONY_RESULT_OK;
}

sony_result_t sony_stopwatch_elapsed (sony_stopwatch_t * pStopwatch, uint32_t* pElapsed)
{
    *pElapsed = osal_get_tick() - pStopwatch->startTime;
    return SONY_RESULT_OK;
}
#else

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
