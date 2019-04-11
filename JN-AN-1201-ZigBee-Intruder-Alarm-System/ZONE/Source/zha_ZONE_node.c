/*****************************************************************************
 *
 * MODULE:             JN-AN-1201
 *
 * COMPONENT:          zha_ZONE_node.c
 *
 * DESCRIPTION:        ZHA Demo : Stack <-> Zone App Interaction
 *                     (Implementation)
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5164,
 * JN5161, JN5148, JN5142, JN5139].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
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
 * Copyright NXP B.V. 2013. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include <appapi.h>
#include "os.h"
#include "os_gen.h"
#include "pdum_apl.h"
#include "pdum_gen.h"
#include "pdm.h"
#include "dbg.h"
#include "dbg_uart.h"
#include "pwrm.h"
#include "zps_gen.h"
#include "zps_apl_af.h"
#include "zps_apl_zdo.h"
#include "zps_apl_aib.h"
#include "zps_apl_zdp.h"
#include "rnd_pub.h"

#include "app_common.h"
#include "groups.h"

#include "PDM_IDs.h"

#include "app_timer_driver.h"
#include "zha_ZONE_node.h"

#include "app_zcl_ZONE_task.h"
#include "app_zbp_utilities.h"

#include "app_events.h"
#include "zcl_customcommand.h"
#include "app_buttons.h"
#include "app_ias_enroll_req.h"
#include "ha.h"
#include "haEzJoin.h"
#include "zcl_common.h"
#include "app_ias_indicator.h"
#include "app_ias_save.h"
#include "app_sleep_functions.h"
#include "PingParent.h"
#ifdef CSW
#include "GenericBoard.h"
#endif

#ifdef VMS
#include "TSL2550.h"
#endif
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifdef DEBUG_ZONE_NODE
    #define TRACE_ZONE_NODE   TRUE
#else
    #define TRACE_ZONE_NODE   FALSE
#endif

static uint8 s_au8LnkKeyArray[16] = {0x5a, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6c,
                                     0x6c, 0x69, 0x61, 0x6e, 0x63, 0x65, 0x30, 0x39};

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PUBLIC void vStopAllTimers(void);
PRIVATE void vHandleAppEvent( APP_tsEvent sAppEvent );
PRIVATE void vDeletePDMOnButtonPress(uint8 u8ButtonDIO);
PRIVATE void app_vRestartNode (void);
PRIVATE void app_vStartNodeFactoryNew(void);
PRIVATE void vHandleJoinAndRejoinNWK( ZPS_tsAfEvent *pZPSevent,teEZ_JoinAction eJoinAction  );
PRIVATE void app_vUpdateZoneStatusAttribute(  uint8     u8SourceEndPoint,
                                              uint16    u16StatusBitMask,
                                              bool_t    bStatusState);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

PUBLIC  tsDeviceDesc           sDeviceDesc;
uint16 u16GroupId;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
/*Primary channel Set */
PRIVATE uint16 u16FastPoll;


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: bMgmtBindServerCallback
 *
 * DESCRIPTION:
 * Called back when mgmt server req is received by the stack and served
 *
 * RETURNS:
 * bool
 *
 ****************************************************************************/
PUBLIC bool bMgmtBindServerCallback(uint16 u16ClusterId)
{
    if (u16ClusterId == 0x0033 )
    {
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
 *
 * NAME: APP_vInitialiseNode
 *
 * DESCRIPTION:
 * Initialises the application related functions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_vInitialiseNode(void)
{
    PDM_teStatus eStatusReload;
    uint16 u16ByteRead;
    DBG_vPrintf(TRACE_ZONE_NODE, "\nAPP_vInitialiseNode*");


    /*Initialise the application buttons*/
    /* Initialise buttons; if a button is held down as the device is reset, delete the device
     * context from flash
     */
    APP_bButtonInitialise();

    /*In case of a deep sleep device any button wake up would cause a PDM delete , only check for DIO8
     * pressed for deleting the context */
    vDeletePDMOnButtonPress(APP_BUTTONS_BUTTON_1);

    /* Restore any application data previously saved to flash */
    PDM_eReadDataFromRecord(PDM_ID_APP_IASZONE_NODE, &sDeviceDesc,
            sizeof(tsDeviceDesc), &u16ByteRead);

    /*Load the IAS Zone Server attributes from EEPROM */
    eStatusReload = eLoadIASZoneServerAttributesFromEEPROM();

    /* Initialise ZBPro stack */
    ZPS_vAplSecSetInitialSecurityState(ZPS_ZDO_PRECONFIGURED_LINK_KEY, (uint8 *)&s_au8LnkKeyArray, 0x00, ZPS_APS_GLOBAL_LINK_KEY);
    DBG_vPrintf(TRACE_ZONE_NODE, "Set Sec state\n");

    /* Store channel mask */
    vEZ_RestoreDefaultAIBChMask();

    /* Initialize ZBPro stack */
    ZPS_eAplAfInit();

    DBG_vPrintf(TRACE_ZONE_NODE, "ZPS_eAplAfInit\n");

    /*Set Save default channel mask as it is going to be manipulated */
    vEZ_SetDefaultAIBChMask();

    /*Fix the channel for testing purpose*/
    #if (defined FIX_CHANNEL)
        DBG_vPrintf(TRACE_ZONE_NODE,"\nCurrent Channel = 0x%08x\n",ZPS_psAplAibGetAib()->apsChannelMask);
        ZPS_eAplAibSetApsChannelMask(1<<FIX_CHANNEL);
        DBG_vPrintf(TRACE_ZONE_NODE,"\nCurrent Channel = 0x%08x\n",ZPS_psAplAibGetAib()->apsChannelMask);
    #endif

    APP_ZCL_vInitialise();

    /*Copy the EEPROM stuff on the shared structure */
    if (eStatusReload != PDM_E_STATUS_OK)
    {
        vSaveIASZoneAttributes(ZONE_ZONE_ENDPOINT);
    }
    else
    {
        vLoadIASZoneAttributes(ZONE_ZONE_ENDPOINT);
    }

    /* If the device state has been restored from flash, re-start the stack
     * and set the application running again.
     */
    if (sDeviceDesc.eNodeState == E_RUNNING)
    {
        app_vRestartNode();
    }
    else
    {
        app_vStartNodeFactoryNew();
    }
    vInitIndicationLEDs();

#ifdef VMS
    bool_t bStatus= bTSL2550_Init();
    DBG_vPrintf(TRACE_ZONE_NODE,"bTSL2550_Init = %d\n",bStatus);
#endif

    OS_eActivateTask(APP_ZHA_Switch_Task);
    OS_eStartSWTimer(APP_IndicatorTimer, APP_TIME_MS(1000), NULL);
}
/****************************************************************************
 *
 * NAME: APP_ZHA_Switch_Task
 *
 * DESCRIPTION:
 * Task that handles the application related functionality
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
OS_TASK(APP_ZHA_Switch_Task)
{
    APP_tsEvent sAppEvent;
    ZPS_tsAfEvent sStackEvent;
    sStackEvent.eType = ZPS_EVENT_NONE;
    sAppEvent.eType = APP_E_EVENT_NONE;
     static uint8 uint8NoData  = 0;

    /*Collect the application events*/
    if (OS_eCollectMessage(APP_msgEvents, &sAppEvent) == OS_E_OK)
    {

    }
    /*Collect stack Events */
    else if ( OS_eCollectMessage(APP_msgZpsEvents, &sStackEvent) == OS_E_OK)
    {

        /* Mgmt Leave Received */
        if( ZPS_EVENT_NWK_LEAVE_INDICATION == sStackEvent.eType )
        {
            if( sStackEvent.uEvent.sNwkLeaveIndicationEvent.u64ExtAddr == 0 )
            {
                DBG_vPrintf(TRACE_ZONE_NODE, "ZDO Leave\n" );
            }
        }
        /*******************************************************************************/
    }


    /* Handle events depending on node state */
    switch (sDeviceDesc.eNodeState)
    {
        case E_STARTUP:
            vHandleJoinAndRejoinNWK(&sStackEvent,E_EZ_JOIN);
            break;

        case E_REJOINING:
            vHandleJoinAndRejoinNWK(&sStackEvent,E_EZ_REJOIN);
            DBG_vPrintf(TRACE_ZONE_NODE, "In E_REJOIN - Kick off Tick Timer \n");
            OS_eStartSWTimer(APP_TickTimer, ZCL_TICK_TIME, NULL);
            vHandleAppEvent( sAppEvent );

            break;

        case E_RUNNING:
            DBG_vPrintf(TRACE_ZONE_NODE, "E_RUNNING\r\n");
            if (sStackEvent.eType == ZPS_EVENT_NWK_FAILED_TO_JOIN)
            {
                DBG_vPrintf(TRACE_ZONE_NODE, "Start join failed tmr 1000\n");
                vStopAllTimers();
                OS_eStartSWTimer(APP_TickTimer, ZCL_TICK_TIME, NULL);
                /* Start BackOff Timer */
                u32BackOffTime = 0;
                OS_eStartSWTimer(APP_BackOffTimer, APP_TIME_MS(BACKOFF_TIMEOUT_IN_MS),NULL);
                vStartStopTimer( APP_JoinTimer, APP_TIME_MS(1000),(uint8*)&(sDeviceDesc.eNodeState),E_REJOINING );
                DBG_vPrintf(TRACE_ZONE_NODE, "failed join running %02x\n",sStackEvent.uEvent.sNwkJoinFailedEvent.u8Status );
            }
            /* Handle joined as end device event */
            if(ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE  == sStackEvent.eType)
            {
                /* As just rejoined so start ping time from here again */
                bPingSent = FALSE;
                vResetPingTime();
            }

            if((ZPS_EVENT_APS_DATA_INDICATION == sStackEvent.eType) &&
                (0 == sStackEvent.uEvent.sApsDataIndEvent.u8DstEndpoint))
            {

            }

            else if (ZPS_EVENT_NWK_POLL_CONFIRM == sStackEvent.eType)
            {

                if ((MAC_ENUM_SUCCESS == sStackEvent.uEvent.sNwkPollConfirmEvent.u8Status) ||
                    (MAC_ENUM_NO_ACK == sStackEvent.uEvent.sNwkPollConfirmEvent.u8Status))
                {
                    uint8NoData = 0;
                     /*Start the APP_PollTimer to continue the Poll tasks to detect parent loss or
                      * if frame pending bit is set to poll the Pending data */
                     if (OS_eGetSWTimerStatus(APP_PollTimer) != OS_E_SWTIMER_RUNNING)
                     {
                        OS_eStartSWTimer(APP_PollTimer, APP_TIME_MS(100), NULL);
                     }
                }
                else if (MAC_ENUM_NO_DATA == sStackEvent.uEvent.sNwkPollConfirmEvent.u8Status)
                {
                    DBG_vPrintf(SLEEP_INFO, "\r\nPoll Confirm, No Data");
                    vCheckForSleep( &uint8NoData );
                }
            }
            vHandleAppEvent( sAppEvent );
            break;
        default:
            break;
    }

    /*
     * Global clean up to make sure any PDUs have been freed
     */
    if (sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        PDUM_eAPduFreeAPduInstance(sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    }
}
/****************************************************************************
 *
 * NAME: vHandleJoinAndRejoinNWK
 *
 * DESCRIPTION:
 * Handles the Start UP events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vHandleJoinAndRejoinNWK( ZPS_tsAfEvent *pZPSevent,teEZ_JoinAction eJoinAction  )
{
    teEZ_State ezState;
    teCLD_IASZone_State   eZoneState;

    vSetIASDeviceState(E_IAS_DEV_STATE_NOT_JOINED);

    /* Start Indication Timer */
    if (OS_eGetSWTimerStatus(APP_IndicatorTimer) != OS_E_SWTIMER_RUNNING)
    {
        OS_eStartSWTimer(APP_IndicatorTimer, APP_TIME_MS(250), NULL);
    }

    /*Call The EZ mode Handler passing the events*/
    vEZ_EZModeNWKJoinHandler(pZPSevent,eJoinAction);
    ezState = eEZ_GetJoinState();
    DBG_vPrintf(TRACE_ZONE_NODE, "EZ_STATE\%x r\n", ezState);

    if(ezState == E_EZ_DEVICE_IN_NETWORK)
    {
        if(eJoinAction == E_EZ_REJOIN)
        {
            /* Go to the state before rejoining was triggered */
            eZCL_ReadLocalAttributeValue(
                                     ZONE_ZONE_ENDPOINT,                           /*uint8                      u8SrcEndpoint,*/
                                     SECURITY_AND_SAFETY_CLUSTER_ID_IASZONE,       /*uint16                     u16ClusterId,*/
                                     TRUE,                                         /*bool                       bIsServerClusterInstance,*/
                                     FALSE,                                        /*bool                       bManufacturerSpecific,*/
                                     FALSE,                                        /*bool_t                     bIsClientAttribute,*/
                                     E_CLD_IASZONE_ATTR_ID_ZONE_STATE,             /*uint16                     u16AttributeId,*/
                                     &eZoneState);

            if(eZoneState == E_CLD_IASZONE_STATE_ENROLLED)
            {
                vSetIASDeviceState(E_IAS_DEV_STATE_ENROLLED);
            }
            else
            {
                vSetIASDeviceState(E_IAS_DEV_STATE_JOINED);
            }
        }
        else
        {
            vSetIASDeviceState(E_IAS_DEV_STATE_JOINED);
        }
        DBG_vPrintf(TRACE_ZONE_NODE, "HA EZMode EVT: E_EZ_SETUP_DEVICE_IN_NETWORK \n");
        vStartStopTimer( APP_JoinTimer, APP_TIME_MS(500),(uint8*)&(sDeviceDesc.eNodeState),E_RUNNING );
        u16GroupId=ZPS_u16AplZdoGetNwkAddr();
        PDM_eSaveRecordData(PDM_ID_APP_IASZONE_NODE, &sDeviceDesc,sizeof(tsDeviceDesc));
        ZPS_vSaveAllZpsRecords();
        /* Start 1 seconds polling */
        OS_eStartSWTimer(APP_PollTimer, POLL_TIME, NULL);
    }

}


/****************************************************************************
 *
 * NAME: vDeletePDMOnButtonPress
 *
 * DESCRIPTION:
 * PDM context clearing on button press
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vDeletePDMOnButtonPress(uint8 u8ButtonDIO)
{
    bool_t bDeleteRecords = FALSE;
    uint32 u32Buttons = u32AHI_DioReadInput() & (1 << u8ButtonDIO);
    if (u32Buttons == 0)
    {
        bDeleteRecords = TRUE;
    }
    else
    {
        bDeleteRecords = FALSE;
    }
    /* If required, at this point delete the network context from flash, perhaps upon some condition
     * For example, check if a button is being held down at reset, and if so request the Persistent
     * Data Manager to delete all its records:
     * e.g. bDeleteRecords = vCheckButtons();
     * Alternatively, always call PDM_vDeleteAllDataRecords() if context saving is not required.
     */
    if(bDeleteRecords)
    {
        DBG_vPrintf(TRACE_ZONE_NODE,"Deleting the PDM\n");
        PDM_vDeleteAllDataRecords();
    }
}

/****************************************************************************
 *
 * NAME: vHandleAppEvent
 *
 * DESCRIPTION:
 * Function to handle the app event - buttons
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vHandleAppEvent( APP_tsEvent sAppEvent )
{
    switch(sAppEvent.eType)
    {
        case APP_E_EVENT_BUTTON_DOWN:
        {
                switch(sAppEvent.uEvent.sButton.u8Button)
                {
                    case APP_E_BUTTONS_BUTTON_1:
                        vSendEnrollReq(ZONE_ZONE_ENDPOINT);
                    break;
                    #ifdef CSW
                    case APP_E_BUTTONS_BUTTON_SW3:
                    {
                        vGenericLEDSetOutput(GEN_BOARD_LED_D1_VAL,TRUE);
                        DBG_vPrintf(TRACE_ZONE_NODE,"CLD_IASZONE_STATUS_MASK_ALARM1,CLD_IASZONE_STATUS_MASK_SET\n ");
                        app_vUpdateZoneStatusAttribute (
                                                        ZONE_ZONE_ENDPOINT,            /*uint8                             u8SourceEndPoint,*/
                                                        CLD_IASZONE_STATUS_MASK_ALARM1,/*uint16                            u16StatusBitMask,*/
                                                        CLD_IASZONE_STATUS_MASK_SET    /*bool_t                            bStatusState);*/
                                                        );

                    }
                    break;
                    case APP_E_BUTTONS_BUTTON_SW2:
                    {
                        vGenericLEDSetOutput(GEN_BOARD_LED_D1_VAL,FALSE);
                        DBG_vPrintf(TRACE_ZONE_NODE,"CLD_IASZONE_STATUS_MASK_ALARM1,CLD_IASZONE_STATUS_MASK_RESET\n ");
                        app_vUpdateZoneStatusAttribute (
                                                        ZONE_ZONE_ENDPOINT,            /*uint8                             u8SourceEndPoint,*/
                                                        CLD_IASZONE_STATUS_MASK_ALARM1,/*uint16                            u16StatusBitMask,*/
                                                        CLD_IASZONE_STATUS_MASK_RESET  /*bool_t                            bStatusState);*/
                                                        );
                    }
                    break;
                    case APP_E_BUTTONS_BUTTON_SW1:
                    {
                        vGenericLEDSetOutput(GEN_BOARD_LED_D2_VAL,TRUE);
                        DBG_vPrintf(TRACE_ZONE_NODE,"CLD_IASZONE_STATUS_MASK_ALARM2,CLD_IASZONE_STATUS_MASK_SET\n ");
                        app_vUpdateZoneStatusAttribute (
                                                        ZONE_ZONE_ENDPOINT,            /*uint8                             u8SourceEndPoint,*/
                                                        CLD_IASZONE_STATUS_MASK_ALARM2,/*uint16                            u16StatusBitMask,*/
                                                        CLD_IASZONE_STATUS_MASK_SET    /*bool_t                            bStatusState);*/
                                                        );
                    }
                    break;
                    case APP_E_BUTTONS_BUTTON_SW4:
                    {
                        vGenericLEDSetOutput(GEN_BOARD_LED_D2_VAL,FALSE);
                        DBG_vPrintf(TRACE_ZONE_NODE,"CLD_IASZONE_STATUS_MASK_ALARM2,CLD_IASZONE_STATUS_MASK_RESET\n ");
                        app_vUpdateZoneStatusAttribute (
                                                        ZONE_ZONE_ENDPOINT,            /*uint8                             u8SourceEndPoint,*/
                                                        CLD_IASZONE_STATUS_MASK_ALARM2,/*uint16                            u16StatusBitMask,*/
                                                        CLD_IASZONE_STATUS_MASK_RESET  /*bool_t                            bStatusState);*/
                                                        );
                    }
                    break;

                    #endif
                }
        break;
        }
        default:
            break;
    }
}


/****************************************************************************
 *
 * NAME: vStopAllTimers
 *
 * DESCRIPTION:
 * Stops all the timers
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vStopAllTimers(void)
{
    OS_eStopSWTimer(APP_PollTimer);
    OS_eStopSWTimer(APP_ButtonsScanTimer);
    OS_eStopSWTimer(APP_TickTimer);
    OS_eStopSWTimer(APP_JoinTimer);
    OS_eStopSWTimer(APP_BackOffTimer);
}
/****************************************************************************
 *
 * NAME: vManageWakeUponSysControlISR
 *
 * DESCRIPTION:
 * Called from SysControl ISR to process the wake up conditions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vManageWakeUponSysControlISR(void)
{
    DBG_vPrintf(SLEEP_INFO, "vManageWakeUponSysControlISR\n");

        /*In any case this could be a wake up from timer interrupt or from buttons
         * press
         * */
            /*Only called if the module is comming out of sleep */
    vAppWakeCallBack();

}

/****************************************************************************
 *
 * NAME: APP_PollTask
 *
 * DESCRIPTION:
 * Poll Task for the polling as well it triggers the rejoin in case of pool failure
 * It also manages sleep timing.
 * RETURNS:
 * void
 *
 ****************************************************************************/
OS_TASK(APP_PollTask)
{
    uint32 u32PollPeriod = POLL_TIME;

    if( u16FastPoll )
    {
        u16FastPoll--;
        u32PollPeriod = POLL_TIME_FAST;
    }
    ZPS_teStatus u8PStatus;

    u8PStatus = ZPS_eAplZdoPoll();
    if( u8PStatus )
    {
        DBG_vPrintf(TRACE_ZONE_NODE, "\nPoll Failed \n", u8PStatus );
    }

    OS_eStopSWTimer(APP_PollTimer);
    OS_eStartSWTimer(APP_PollTimer, u32PollPeriod, NULL);
}

/****************************************************************************
 *
 * NAME: app_vRestartNode
 *
 * DESCRIPTION:
 * Start the Restart the ZigBee Stack after a context restore from
 * the EEPROM/Flash
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void app_vRestartNode (void)
{
    ZPS_tsNwkNib * thisNib;

    /* Get the NWK Handle to have the NWK address from local node and use it as GroupId*/
    thisNib = ZPS_psNwkNibGetHandle(ZPS_pvAplZdoGetNwkHandle());

    /* The node is in running state indicates that
     * the EZ Mode state is as E_EZ_SETUP_COMPLETED*/
    eEZ_UpdateEZState(E_EZ_DEVICE_IN_NETWORK);

    DBG_vPrintf(TRACE_ZONE_NODE, "\nNon Factory New Start");

    ZPS_vSaveAllZpsRecords();
    u16GroupId = thisNib->sPersist.u16NwkAddr;
    /* Start 1 seconds polling */
    OS_eActivateTask(APP_PollTask);

    /*Rejoin NWK when coming from reset.*/
    ZPS_eAplZdoRejoinNetwork(FALSE);
}


/****************************************************************************
 *
 * NAME: app_vStartNodeFactoryNew
 *
 * DESCRIPTION:
 * Start the ZigBee Stack for the first ever Time.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void app_vStartNodeFactoryNew(void)
{
    eEZ_UpdateEZState(E_EZ_START);

    /* Stay awake for joining */
    DBG_vPrintf(TRACE_ZONE_NODE, "\nFactory New Start");
    vStartStopTimer( APP_JoinTimer, APP_TIME_MS(1000),(uint8*)&(sDeviceDesc.eNodeState),E_STARTUP );
}

#ifdef VMS
/****************************************************************************
 *
 * NAME: APP_PIRTask
 *
 * DESCRIPTION:
 * PIR task for the light sensor
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
OS_TASK(APP_PIRTask)
{
    bool_t bStatus;
    uint16 u16ALSResult;
    uint16 u16ZoneStatus = 0;

    bStatus = bTSL2550_StartRead(TSL2550_CHANNEL_0);
    DBG_vPrintf(TRACE_ZONE_NODE,"\nStart Read Status =%d",bStatus);

    u16ALSResult = u16TSL2550_ReadResult();
    DBG_vPrintf(TRACE_ZONE_NODE,"\nResult = %d",u16ALSResult);

    eZCL_ReadLocalAttributeValue(
                        ZONE_ZONE_ENDPOINT,                     /*uint8                      u8SrcEndpoint,*/
                         SECURITY_AND_SAFETY_CLUSTER_ID_IASZONE,/*uint16                     u16ClusterId,*/
                         TRUE,                                  /*bool                       bIsServerClusterInstance,*/
                         FALSE,                                 /*bool                       bManufacturerSpecific,*/
                         FALSE,                                 /*bool_t                     bIsClientAttribute,*/
                         E_CLD_IASZONE_ATTR_ID_ZONE_STATUS,     /*uint16                     u16AttributeId,*/
                         &u16ZoneStatus
                        );

    if( (u16ALSResult > LIGHT_SENSOR_THRESHOLD_FOR_RESETTING_MASK) && (u16ZoneStatus != 0))
    {
        app_vUpdateZoneStatusAttribute (
                            ZONE_ZONE_ENDPOINT,            /*uint8                             u8SourceEndPoint,*/
                            CLD_IASZONE_STATUS_MASK_ALARM1,/*uint16                            u16StatusBitMask,*/
                            CLD_IASZONE_STATUS_MASK_RESET  /*bool_t                            bStatusState);*/
                            );
    }
    if( (u16ALSResult < LIGHT_SENSOR_THRESHOLD_FOR_SETTING_MASK) && (u16ZoneStatus == 0))
    {
        app_vUpdateZoneStatusAttribute (
                            ZONE_ZONE_ENDPOINT,            /*uint8                             u8SourceEndPoint,*/
                            CLD_IASZONE_STATUS_MASK_ALARM1,/*uint16                            u16StatusBitMask,*/
                            CLD_IASZONE_STATUS_MASK_SET    /*bool_t                            bStatusState);*/
                            );
    }
}
#else
/****************************************************************************
 *
 * NAME: APP_PIRTask
 *
 * DESCRIPTION:
 * PIR task for the light sensor so keep it NULL for other device
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
OS_TASK(APP_PIRTask)
{

}
#endif

/****************************************************************************
 *
 * NAME: app_vUpdateZoneStatusAttribute
 *
 * DESCRIPTION:
 * Update zone status attribute and saves it on PDM as well.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void app_vUpdateZoneStatusAttribute(  uint8     u8SourceEndPoint,
                                              uint16    u16StatusBitMask,
                                              bool_t    bStatusState)
{
    eCLD_IASZoneUpdateZoneStatus (
                        u8SourceEndPoint, /*uint8                             u8SourceEndPoint,*/
                        u16StatusBitMask, /*uint16                            u16StatusBitMask,*/
                        bStatusState      /*bool_t                            bStatusState);*/
                        );

    vSaveIASZoneAttributes(u8SourceEndPoint);
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
