/*****************************************************************************
 *
 * MODULE:             JN-AN-1201
 *
 * COMPONENT:          zha_CIE_node.c
 *
 * DESCRIPTION:        CIE node (Implementation)
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
 * Copyright NXP B.V. 2014. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include "stdlib.h"
#include "stdio.h"
#include "dbg.h"
#include "os.h"
#include "os_gen.h"
#include "pdum_gen.h"
#include "pdm.h"
#include "pdum_gen.h"
#include "zps_gen.h"
#include "zps_apl.h"
#include "zps_apl_aib.h"
#include "zps_nwk_sap.h"
#include "app_common.h"
#include "app_events.h"
#include "app_zcl_CIE_task.h"
#include "zha_CIE_node.h"
#include "ha.h"
#include "haEzJoin.h"
#include "PDM_IDs.h"
#include "zcl_options.h"
#include "app_zbp_utilities.h"
#include "zcl_common.h"
#include "app_buttons.h"
#include "app_pdm.h"
#include "AppHardwareApi.h"
#include "LcdDriver.h"
#include "LcdFont.h"
#include "IASWD.h"
#include "IASACE.h"
#include "app_zone_client.h"
#include "app_CIE_display.h"
#include "GenericBoard.h"
#include "app_CIE_save.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef DEBUG_CIE_NODE
    #define TRACE_CIE_NODE   FALSE
#else
    #define TRACE_CIE_NODE   TRUE
#endif

/*DIO2 LED indication for Permit join True */
#define LED_PERMIT_JOIN 0x00000008UL   /* using DIO3 */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void app_vStartNodeFactoryNew(void);
PRIVATE void vHandleStartUp( ZPS_tsAfEvent *pZPSevent );
PRIVATE void vHandleRunningEvent(ZPS_tsAfEvent *sStackEvent);
PRIVATE void vDeletePDMOnButtonPress(uint8 u8ButtonDIO);
PRIVATE void vInitDR1174LED(void);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
extern uint8 u8Flash;

PUBLIC  uint8 s_au8LnkKeyArray[16] = {0x5a, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6c,
        0x6c, 0x69, 0x61, 0x6e, 0x63, 0x65, 0x30, 0x39};
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
tsDeviceDesc sDeviceDesc;
PDM_tsRecordDescriptor sDevicePDDesc;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vPermitJoinIndication
 *
 * DESCRIPTION:
 * LED Indication
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vPermitJoinIndication(void)
{
    if(ZPS_bGetPermitJoiningStatus())
    {
        vAHI_DioSetOutput(0,LED_PERMIT_JOIN);
    }
    else
    {
        vAHI_DioSetOutput(LED_PERMIT_JOIN,0);

    }
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
    /* If required, at this point delete the network context from flash, perhaps upon some condition
     * For example, check if a button is being held down at reset, and if so request the Persistent
     * Data Manager to delete all its records:
     * e.g. bDeleteRecords = vCheckButtons();
     * Alternatively, always call PDM_vDelete() if context saving is not required.
     */

     APP_bButtonInitialise();
     vDeletePDMOnButtonPress(APP_BUTTONS_BUTTON_1);

    /* Restore any application data previously saved to flash */
    /* Restore device state previously saved to flash */
    eRestoreDeviceState();

    /*Load the IAS ACE Cluster from EEPROM */
    vLoadIASCIEFromEEPROM(CIE_EP);

    /* Initialise ZBPro stack */
    ZPS_vAplSecSetInitialSecurityState(ZPS_ZDO_PRECONFIGURED_LINK_KEY, (uint8 *)&s_au8LnkKeyArray, 0x00, ZPS_APS_GLOBAL_LINK_KEY);

    /* Store channel mask */
    vEZ_RestoreDefaultAIBChMask();

    /* Initialise ZBPro stack */
    ZPS_eAplAfInit();

    /*Set Save default channel mask as it is going to be manipulated */
    vEZ_SetDefaultAIBChMask();

    /*Fix the channel for testing purpose*/
    #if (defined FIX_CHANNEL)
       DBG_vPrintf(TRACE_CIE_NODE,"\nCurrent Channel = 0x%08x\n",ZPS_psAplAibGetAib()->apsChannelMask);
       ZPS_eAplAibSetApsChannelMask(1<<FIX_CHANNEL);
       DBG_vPrintf(TRACE_CIE_NODE,"\nCurrent Channel = 0x%08x\n",ZPS_psAplAibGetAib()->apsChannelMask);
    #endif

    /* Initialise ZCL */
    APP_ZCL_vInitialise();

    /*Verify Load for the EEPROM */
    vVerifyIASCIELoad(CIE_EP);

    if (E_RUNNING == eGetNodeState())
    {
        app_vRestartNode();
        eLCDState = E_LCD_NETWORK_FORMED;
    }
    else
    {
        app_vStartNodeFactoryNew();
    }
    vLCDDisplay();
    vInitDR1174LED();
    
}

/****************************************************************************
 *
 * NAME: APP_taskCIE
 *
 * DESCRIPTION:
 * Main state machine
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
OS_TASK(APP_taskCIE)
{
    ZPS_tsAfEvent sStackEvent = {0};
    APP_tsEvent sAppButtonEvent = {0};

    /* The button event for the SSL bulbs */
    if (OS_eCollectMessage(APP_msgEvents, &sAppButtonEvent) == OS_E_OK)
    {

        DBG_vPrintf(TRACE_CIE_NODE,"\n\nButton Q\n EventType = %d\n", sAppButtonEvent.eType);
        DBG_vPrintf(TRACE_CIE_NODE," Button = %d\n", sAppButtonEvent.uEvent.sButton.u8Button);
        vHandleButtonPress(sAppButtonEvent.eType,sAppButtonEvent.uEvent.sButton.u8Button);
    }

    /*Collect stack Events */
    if ( OS_eCollectMessage(APP_msgZpsEvents, &sStackEvent) == OS_E_OK)
    {

        #ifdef DEBUG_CIE_NODE
            vDisplayStackEvent( sStackEvent );
        #endif
    }


    /* Handle events depending on node state */
    switch (sDeviceDesc.eNodeState)
    {
        case E_STARTUP:
            DBG_vPrintf(TRACE_CIE_NODE, "\nE_STARTUP" );
            eLCDState = E_LCD_NETWORK_FORMATION;
            vLCDDisplay();
            vHandleStartUp(&sStackEvent);
            break;

        case E_RUNNING:
            DBG_vPrintf(TRACE_CIE_NODE, "E_RUNNING\r\n");
            vHandleRunningEvent(&sStackEvent);
            break;

        default:
            DBG_vPrintf(TRACE_CIE_NODE, "ERR: Unknown State %d\n", sDeviceDesc.eNodeState );
            break;
    }

    /* Global clean up to make sure any APDUs have been freed   */
    if (sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        PDUM_eAPduFreeAPduInstance(sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    }
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
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
PUBLIC void app_vRestartNode (void)
{
    /* The node is in running state indicates that
     * the EZ Mode state is as E_EZ_SETUP_DEVICE_IN_NETWORK*/
    eEZ_UpdateEZState(E_EZ_DEVICE_IN_NETWORK);

    sDeviceDesc.eNodeState = E_RUNNING;

    /* Store the NWK frame counter increment */
    ZPS_vSaveAllZpsRecords();

    DBG_vPrintf(TRACE_CIE_NODE, "Restart Running\n");
    OS_eActivateTask(APP_taskCIE);

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
    /* The node is in running state indicates that
     * the EZ Mode state is as E_EZ_SETUP_START*/
    vEZ_FormNWK();
    vEZ_SetUpPolicy(E_EZ_JOIN_ELSE_FORM_IF_NO_NETWORK);

    eEZ_UpdateEZState(E_EZ_START);

    DBG_vPrintf(TRACE_CIE_NODE, "\nRun and activate\n");
    vStartStopTimer( APP_StartUPTimer, APP_TIME_MS(500),(uint8*)&(sDeviceDesc.eNodeState),E_STARTUP );
    DBG_vPrintf(TRACE_CIE_NODE, "Start Factory New\n");
}



/****************************************************************************
 *
 * NAME: vHandleStartUp
 *
 * DESCRIPTION:
 * Handles the Start UP events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vHandleStartUp( ZPS_tsAfEvent *pZPSevent )
{
    teEZ_State ezState;
    /*Call The EZ mode Handler passing the events*/
    vEZ_EZModeNWKJoinHandler(pZPSevent,E_EZ_JOIN);
    ezState = eEZ_GetJoinState();
    DBG_vPrintf(TRACE_CIE_NODE, "EZ_STATE\%x r\n", ezState);
    if(ezState == E_EZ_DEVICE_IN_NETWORK)
    {
        DBG_vPrintf(TRACE_CIE_NODE, "HA EZMode E_EZ_SETUP_DEVICE_IN_NETWORK \n");
        vStartStopTimer( APP_StartUPTimer, APP_TIME_MS(500),(uint8*)&(sDeviceDesc.eNodeState),E_RUNNING );
        eLCDState = E_LCD_NETWORK_FORMED;
        vLCDDisplay();
        vEnablePermitJoin(EZ_MODE_TIME * 60);

        PDM_eSaveRecordData( PDM_ID_APP_APP_ROUTER,
                         &sDeviceDesc,
                         sizeof(tsDeviceDesc));

        ZPS_vSaveAllZpsRecords();
    }
}


/****************************************************************************
 *
 * NAME: vHandleRunningEvent
 *
 * DESCRIPTION:
 * Handles the running events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vHandleRunningEvent(ZPS_tsAfEvent *psStackEvent)
{
    /*Request response event, call the child age logic */
    if((ZPS_EVENT_APS_DATA_INDICATION == psStackEvent->eType) &&
        (0 == psStackEvent->uEvent.sApsDataIndEvent.u8DstEndpoint))
    {
        ZPS_tsAfZdpEvent sAfZdpEvent;
        zps_bAplZdpUnpackResponse(psStackEvent,
                                  &sAfZdpEvent);

        DBG_vPrintf(TRACE_CIE_NODE, "APP: vCheckStackEvent: ZPS_EVENT_APS_ZDP_REQUEST_RESPONSE, Cluster = %x\n",    sAfZdpEvent.u16ClusterId );

        vHandleZDPReqResForZone(&sAfZdpEvent);
        if(ZPS_u16AplZdoGetNwkAddr() != psStackEvent->uEvent.sApsDataIndEvent.uSrcAddress.u16Addr)
        {
            vHandleDeviceAnnce(&sAfZdpEvent);
        }
    }

    /* Mgmt Leave Received */
    if( ZPS_EVENT_NWK_LEAVE_INDICATION == psStackEvent->eType )
    {
        DBG_vPrintf(TRACE_CIE_NODE, "MgmtLeave\n" );
        /* add leave handling here */
    }

    if(ZPS_EVENT_NWK_NEW_NODE_HAS_JOINED == psStackEvent->eType )
    {
        DBG_vPrintf(TRACE_CIE_NODE, "New Node Joined\n" );
    }

    if (ZPS_EVENT_ERROR == psStackEvent->eType)
    {
        DBG_vPrintf(TRACE_CIE_NODE, "Error Type %d\n" , psStackEvent->uEvent.sAfErrorEvent.eError);
    }
}

/****************************************************************************
 *
 * NAME: eGetNodeState
 *
 * DESCRIPTION:
 * returns the device state
 *
 * RETURNS:
 * teNODE_STATES
 *
 ****************************************************************************/
PUBLIC teNODE_STATES eGetNodeState(void)
{
    return sDeviceDesc.eNodeState;
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
     * Alternatively, always call PDM_vDelete() if context saving is not required.
     */
    if(bDeleteRecords)
    {
        DBG_vPrintf(TRACE_CIE_NODE,"Deleting the PDM\n");
        PDM_vDeleteAllDataRecords();
    }
}


/****************************************************************************
 *
 * NAME: vInitDR1174LED
 *
 * DESCRIPTION:
 * LED indication
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vInitDR1174LED(void)
{

    /*Enable Pull Ups : Not really required for the outputs */
    vAHI_DioSetPullup(0,LED_PERMIT_JOIN);

    /*Make the DIo as output*/
    vAHI_DioSetDirection(0,LED_PERMIT_JOIN);
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
