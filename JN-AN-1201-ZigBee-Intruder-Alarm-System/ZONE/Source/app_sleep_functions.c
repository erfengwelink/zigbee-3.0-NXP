/*****************************************************************************
 *
 * MODULE:             JN-AN-
 *
 * COMPONENT:          app_sleep_functions.c
 *
 * AUTHOR:             jpenn
 *
 * DESCRIPTION:        Application Sleep Handler Functions
 *
 * $HeadURL $
 *
 * $Revision: 9281 $
 *
 * $LastChangedBy: nxp33194 $
 *
 * $LastChangedDate: 2012-06-08 15:13:02 +0100 (Fri, 08 Jun 2012) $
 *
 * $Id: app_sleep_functions.c 9281 2012-06-08 14:13:02Z nxp33194 $
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142,
 * JN5139]. You, and any third parties must reproduce the copyright and
 * warranty notice and any other legend of ownership on each copy or partial
 * copy of the software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include "os.h"
#include "os_gen.h"
#include "pdum_apl.h"
#include "pdum_gen.h"
#include "pdm.h"
#include "pwrm.h"
#include "dbg.h"
#include "app_sleep_functions.h"
#include "zha_ZONE_node.h"
#include "app_common.h"
#include "app_zcl_ZONE_task.h"
#include "PingParent.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void vStopSWTimers (void);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
static uint8 u8PingCount = 0;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

PRIVATE pwrm_tsWakeTimerEvent   sWake;

/****************************************************************************
 *
 * NAME: vStopSWTimers
 *
 * DESCRIPTION:
 * Stops any timers ready for sleep
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vStopSWTimers (void)
{
    if (OS_eGetSWTimerStatus(APP_ButtonsScanTimer) != OS_E_SWTIMER_STOPPED)
    {
        OS_eStopSWTimer(APP_ButtonsScanTimer);
    }

    if (OS_eGetSWTimerStatus(APP_PollTimer) != OS_E_SWTIMER_STOPPED)
    {
        OS_eStopSWTimer(APP_PollTimer);
    }

    if (OS_eGetSWTimerStatus(APP_JoinTimer) != OS_E_SWTIMER_STOPPED)
    {
        OS_eStopSWTimer(APP_JoinTimer);
    }

    if (OS_eGetSWTimerStatus(APP_TickTimer) != OS_E_SWTIMER_STOPPED)
    {
        OS_eStopSWTimer(APP_TickTimer);
    }

    if (OS_eGetSWTimerStatus(APP_BackOffTimer) != OS_E_SWTIMER_STOPPED)
    {
        OS_eStopSWTimer(APP_BackOffTimer);
    }

    if (OS_eGetSWTimerStatus(APP_IndicatorTimer) != OS_E_SWTIMER_STOPPED)
    {
        OS_eStopSWTimer(APP_IndicatorTimer);
    }

}

/****************************************************************************
 *
 * NAME: vScheduleSleep
 *
 * DESCRIPTION:
 * Schedule a wake event
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vScheduleSleep(void)
{

    vStopSWTimers();

#ifdef PERIODIC_WAKE
    PWRM_teStatus teStatus = PWRM_eScheduleActivity(&sWake, SLEEP_PERIOD , vAppWakeCallBack  );     /* Schedule the next sleep point */

    if(!teStatus)
    {
        DBG_vPrintf(SLEEP_INFO, "\r\n Scheduling sleep point in %d Seconds, status = %d\n",(SLEEP_PERIOD / 32768),teStatus);
    }
    else
    {
        DBG_vPrintf(SLEEP_INFO, "\r\n Timer already scheduled");
    }

#endif



}

/****************************************************************************
 *
 * NAME: vWakeCallBack
 *
 * DESCRIPTION:
 * Wake up call back called upon wake up by the schedule activity event.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppWakeCallBack(void)
{
    DBG_vPrintf(SLEEP_INFO, "vAppWakeCallBack & Poll\n");

    /*Start the APP_TickTimer to continue the ZCL tasks */
    if (OS_eGetSWTimerStatus(APP_TickTimer) != OS_E_SWTIMER_RUNNING)
    {
        OS_eStartSWTimer(APP_TickTimer, ZCL_TICK_TIME, NULL);
    }

    /* Increment sleep time based on seconds one slept for */
    vIncrementPingTime(SLEEP_TIME_IN_SECS);
    /* Ping Parent for rejoin in case of Child ageing */
    bPingParent();
    u8PingCount = 0;

#if SLEEP_INFO
    DBG_vPrintf(SLEEP_INFO, "Poll req %02x\n",
    ZPS_eAplZdoPoll());
#else
    ZPS_eAplZdoPoll();
#endif

#ifdef VMS
    OS_eActivateTask(APP_PIRTask);
#endif
}

/****************************************************************************
 *
 * NAME: vCheckForSleep
 *
 * DESCRIPTION:
 * Checks the number of polls without data, then schedules sleep if
 * conditions met
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vCheckForSleep( uint8 * u8NoDataCount )
{
    teCLD_IASZone_State   eZoneState;
    /* If we are enrolled, we can start checking for sleep */
    eZCL_ReadLocalAttributeValue(
                             1,                                     /*uint8                      u8SrcEndpoint,*/
                             SECURITY_AND_SAFETY_CLUSTER_ID_IASZONE,/*uint16                     u16ClusterId,*/
                             TRUE,                                  /*bool                       bIsServerClusterInstance,*/
                             FALSE,                                 /*bool                       bManufacturerSpecific,*/
                             FALSE,                                 /*bool_t                     bIsClientAttribute,*/
                             E_CLD_IASZONE_ATTR_ID_ZONE_STATE,      /*uint16                     u16AttributeId,*/
                             &eZoneState);
    DBG_vPrintf(SLEEP_INFO, "Check for sleep Enrrolled %d ", eZoneState);
    if(eZoneState == E_CLD_IASZONE_STATE_ENROLLED)
    {
        *u8NoDataCount += 1;
        DBG_vPrintf(SLEEP_INFO, "poll count %d\n", *u8NoDataCount);

        /* Do not sleep if Pinging Parent wait for atleast 3 polls */
        if( (TRUE == bPingSent) && (FALSE == bPingRespRcvd))
        {
            DBG_vPrintf(SLEEP_INFO, "\r\n%d Ping sent",u8PingCount);
            if(u8PingCount >= MAX_PINGS_NO_RSP)
            {
                DBG_vPrintf(SLEEP_INFO, "\r\nRejoin");
                ZPS_eAplZdoRejoinNetwork(FALSE);
                bPingSent = FALSE;
            }else
            {
                if(u8PingCount < MAX_PINGS_NO_RSP)
                {
                    if(u8PingCount != 0)
                    {
                        /* Increment ping time so that we definitely ping the parent */
                        vIncrementPingTime(PING_PARENT_TIME);
                        /* Ping Parent for rejoin in case of Child ageing */
                        bPingParent();
                    }
                    else
                    {
                        OS_eStartSWTimer(APP_PollTimer, APP_TIME_MS(1000), NULL);
                    }
                }
                u8PingCount += 1;
            }
        }else
        {
            if( *u8NoDataCount >= MAX_POLLS_NO_DATA )
            {
                DBG_vPrintf(SLEEP_INFO, "\r\nNo Data Polls Exceeded");
                *u8NoDataCount = 0;
                vScheduleSleep();
            }
        }
    }
    DBG_vPrintf(SLEEP_INFO, "\n");
}

/****************************************************************************
 *
 * NAME: vCheckForSleepPostTx
 *
 * DESCRIPTION:
 * Checks for sleep after transmission
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vCheckForSleepPostTx( void )
{
    teCLD_IASZone_State   eZoneState;
    /* If we are enrolled, we can start checking for sleep */
    eZCL_ReadLocalAttributeValue(
                             1,                                     /*uint8                      u8SrcEndpoint,*/
                             SECURITY_AND_SAFETY_CLUSTER_ID_IASZONE,/*uint16                     u16ClusterId,*/
                             TRUE,                                  /*bool                       bIsServerClusterInstance,*/
                             FALSE,                                 /*bool                       bManufacturerSpecific,*/
                             FALSE,                                 /*bool_t                     bIsClientAttribute,*/
                             E_CLD_IASZONE_ATTR_ID_ZONE_STATE,      /*uint16                     u16AttributeId,*/
                             &eZoneState);
    DBG_vPrintf(SLEEP_INFO, "Check for sleep Enrrolled Post TX %d ", eZoneState);

    if(eZoneState == E_CLD_IASZONE_STATE_ENROLLED)
    {
        ZPS_eAplZdoPoll();
    }
}

/****************************************************************************
 *
 */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
