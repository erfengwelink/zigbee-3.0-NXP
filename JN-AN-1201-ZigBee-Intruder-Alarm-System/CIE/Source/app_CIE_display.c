/*****************************************************************************
 *
 * MODULE:             JN-AN-1201
 *
 * COMPONENT:          app_CIE_display.c
 *
 * DESCRIPTION:        CIE display file
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
#include "app_zcl_CIE_task.h"
#include "zha_CIE_node.h"
#include "app_pdm.h"
#include "AppHardwareApi.h"
#include "LcdDriver.h"
#include "LcdFont.h"
#include "IASWD.h"
#include "IASACE.h"
#include "app_zone_client.h"
#include "string.h"
#include "app_CIE_display.h"
#include "haEzJoin.h"
#include "app_zbp_utilities.h"
#include "os_gen.h"
#include "app_CIE_save.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifndef DEBUG_CIE_DISPLAY
    #define TRACE_CIE_DISPLAY   FALSE
#else
    #define TRACE_CIE_DISPLAY   TRUE
#endif

#define MAXIMUM_NUMBER_OF_ROWS_PER_PANEL        7
#define MAXIMUM_NUMBER_OF_COLUMNS               22
#define MAXIMUM_NUMBER_OF_ROWS                  18
#define MAXIMUM_LENGTH_OF_STRING                255
#define NUMBER_OF_CIE_MAIN_MENU_OPTION          6
#define MAXIMUM_INDEX_OF_ARM_GROUP_SCREEN       4
#define MAXIMUM_INDEX_OF_CONFIGURATION_SCREEN   2
#define MAXIMUM_INDEX_OF_ARM_SYSTEM_SCREEN      4
#define MAXIMUM_INDEX_OF_ARM_GROUP_SCREEN       4
#define MINIMUM_INDEX_OF_ARM_GROUP_SCREEN       2
#define MINIMUM_INDEX_OF_ZONE_INFO_SCREEN       2
#define MAXIMUM_INDEX_OF_ZONE_INFO_SCREEN       5
#define MINIMUM_INDEX_OF_SCREEN                 1

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vCheckForMenuIndex(void);
PRIVATE void vSetIntialDisplayPostition(void);
PRIVATE void vMoveScreenUp(uint8 u8MinimumIndex);
PRIVATE void vMoveScreenAndDeviceUp(void);
PRIVATE void vMoveScreenDown(uint8 u8MaximumIndex);
PRIVATE void vMoveScreenAndDeviceDown(void);
PRIVATE void vMoveScreenAndDeviceUp(void);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
teLCDState eLCDState;
uint8 u8DeviceIndex = 0;
uint8 u8MenuIndex = 0;
uint8 gu8CursorPosition = 0;
uint8 u8AceMenuIndex = 1;
char acACELCDString[MAXIMUM_NUMBER_OF_ROWS][MAXIMUM_NUMBER_OF_COLUMNS];
bool bAceMessage = FALSE;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static teLCDState ePreviousLCDState = E_LCD_INTIAL_DISPLAY;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vGetZoneTypeString
 *
 * DESCRIPTION:
 * Converts the Zone Type into the character string based on the zone server
 * for the LCD display. For example zonetype keypad & keyfob will go into keypads.
 * Zones types like CSW,VMS will go into Zone.
 *
 * PARAMETERS:      Name              Usage
 *                  str               Pointer to the string
 *                  u16ZoneType       Zone type
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vGetZoneTypeString(char *str, uint16 u16ZoneType)
{
    memset(str,0,255);
    switch(u16ZoneType)
    {
        case 0x0015:
        case 0x002d:
            strcpy(str,"Zone");
        break;
        case 0x021d:
            strcpy(str,"KeyPad");
        break;
        case 0x0225:
            strcpy(str,"Warning");
        break;
        default:
            strcpy(str,"Unknown");
        break;
    }

}

/****************************************************************************
 *
 * NAME: vGetActualZoneTypeString
 *
 * DESCRIPTION:
 * Converts the Zone Type into the character string for the LCD display
 *
 * PARAMETERS:      Name              Usage
 *                  str               Pointer to the string
 *                  u16ZoneType       Zone type
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vGetActualZoneTypeString(char *str, uint16 u16ZoneType)
{
    memset(str,0,255);
    switch(u16ZoneType)
    {
        case 0x0015:
            strcpy(str,"Contact Switch");
        break;
        case 0x002d:
            strcpy(str,"Motion Sensor");
        break;
        case 0x021d:
            strcpy(str,"KeyPad");
        break;
        case 0x0225:
            strcpy(str,"Warning Device");
        break;
        default:
            strcpy(str,"Unknown");
        break;
    }

}

/****************************************************************************
 *
 * NAME: vGetZoneIDString
 *
 * DESCRIPTION:
 * Converts the Zone ID into the character string for the LCD display.
 *
 * PARAMETERS:      Name              Usage
 *                  str               Pointer to the string
 *                  u8ZoneID          Zone ID
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vGetZoneIDString(char *str, uint8 u8ZoneID)
{
    memset(str,0,255);

    /*As of now only valid zone IDs range form 0 - (CLD_IASACE_ZONE_TABLE_SIZE - 1)*/
    if(u8ZoneID < CLD_IASACE_ZONE_TABLE_SIZE)
    {
        u8ZoneID += 0x30;
        *str = u8ZoneID;
    }else{
        strcpy(str,"]");
    }

}

/****************************************************************************
 *
 * NAME: vGetPanelStatusString
 *
 * DESCRIPTION:
 * Converts the Panel Status into the character string for the LCD display.
 *
 * PARAMETERS:      Name              Usage
 *                  str               Pointer to the string
 *                  u8PanelStatus     Panel Status
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vGetPanelStatusString(char *str,uint8 u8PanelStatus)
{
    memset(str,0,255);
    switch(u8PanelStatus)
    {
        case E_CLD_IASACE_PANEL_STATUS_PANEL_DISARMED:
            strcpy(str,"Ready To Arm");
        break;
        case E_CLD_IASACE_PANEL_STATUS_PANEL_ARMED_DAY:
            strcpy(str,"ArmedDay");
        break;
        case E_CLD_IASACE_PANEL_STATUS_PANEL_ARMED_NIGHT:
            strcpy(str,"ArmedNight");
        break;
        case E_CLD_IASACE_PANEL_STATUS_PANEL_ARMED_AWAY:
            strcpy(str,"ArmedAway");
        break;
        case E_CLD_IASACE_PANEL_STATUS_PANEL_EXIT_DELAY:
            strcpy(str,"ExitDelay");
        break;
        case E_CLD_IASACE_PANEL_STATUS_PANEL_ENTRY_DELAY:
            strcpy(str,"EntryDelay");
        break;
        case E_CLD_IASACE_PANEL_STATUS_PANEL_NOT_READY_TO_ARM:
            strcpy(str,"NotReadyToArm");
        break;
        case E_CLD_IASACE_PANEL_STATUS_PANEL_IN_ALARM:
            strcpy(str,"InAlarm");
        break;
        case E_CLD_IASACE_PANEL_STATUS_PANEL_ARMING_STAY:
            strcpy(str,"ArmingStay");
        break;
        case E_CLD_IASACE_PANEL_STATUS_PANEL_ARMING_NIGHT:
            strcpy(str,"ArmingNight");
        break;
        case E_CLD_IASACE_PANEL_STATUS_PANEL_ARMING_AWAY:
            strcpy(str,"ArmingAway");
        break;
        default:
            strcpy(str,"Unknown");
        break;
    }
}

/****************************************************************************
 *
 * NAME: vGetAlarmStatusString
 *
 * DESCRIPTION:
 * Converts the Alarm Status into the character string for the LCD display.
 *
 * PARAMETERS:      Name              Usage
 *                  str               Pointer to the string
 *                  u8AlarmStatus     Alarm Status
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vGetAlarmStatusString(char *str,uint8 u8AlarmStatus)
{
    memset(str,0,255);
    switch(u8AlarmStatus)
    {
        case E_CLD_IASACE_ALARM_STATUS_NO_ALARM:
            strcpy(str,"No Alarm");
        break;
        case E_CLD_IASACE_ALARM_STATUS_BURGLAR:
            strcpy(str,"Burglar");
        break;
        case E_CLD_IASACE_ALARM_STATUS_FIRE:
            strcpy(str,"Fire");
        break;
        case E_CLD_IASACE_ALARM_STATUS_EMERGENCY:
            strcpy(str,"Emergency");
        break;
        case E_CLD_IASACE_ALARM_STATUS_POLICE_PANIC:
            strcpy(str,"Police Panic");
        break;
        case E_CLD_IASACE_ALARM_STATUS_FIRE_PANIC:
            strcpy(str,"Fire Panic");
        break;
        case E_CLD_IASACE_ALARM_STATUS_EMERGENCY_PANIC:
            strcpy(str,"EmergencyPanic");
        break;
        default:
            strcpy(str,"Unknown");
        break;
    }
}

/****************************************************************************
 *
 * NAME: vGetSoundString
 *
 * DESCRIPTION:
 * Converts the Audible notification field into the character string for the LCD display.
 *
 * PARAMETERS:      Name              Usage
 *                  str               Pointer to the string
 *                  u8Sound           Audible Notification
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vGetSoundString(char *str,uint8 u8Sound)
{
    memset(str,0,255);
    switch(u8Sound)
    {
        case E_CLD_IASACE_AUDIBLE_NOTIF_MUTE:
            strcpy(str,"Mute");
        break;
        case E_CLD_IASACE_AUDIBLE_NOTIF_DEFAULT_SOUND:
            strcpy(str,"Default");
        break;
        default:
            strcpy(str,"Unknown");
        break;
    }
}

/****************************************************************************
 *
 * NAME: vNumberToString
 *
 * DESCRIPTION:
 * Converts an 8-bit,16-bit,64-bit value to a string of the textual decimal representation.
 * Adds a text string after the text.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  pcOutString     R   Location for new string
 *                  u16Num          R   Value to convert
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vNumberToString(char * str, uint64 u64Num, teZCL_ZCLAttributeType eType)
{
    uint8 u8Nybble,u8Shift = 0;
    int i;

    memset(str,0,255);

    switch(eType)
    {
        case E_ZCL_UINT8:
            u8Shift = 4;
        break;
        case E_ZCL_UINT16:
            u8Shift = 12;
        break;
        case E_ZCL_UINT64:
            u8Shift = 60;
        break;
        default:
            break;
    }

    for (i = u8Shift; i >= 0; i -= 4)
    {
        u8Nybble = (uint8)((u64Num >> i) & 0x0f);
        u8Nybble += 0x30;
        if (u8Nybble > 0x39)
            u8Nybble += 7;

        *str = u8Nybble;
        str++;
    }
}

/****************************************************************************
 *
 * NAME: vGetZoneStatusBitString
 *
 * DESCRIPTION:
 * Converts an 16-bit value to a string of the bit map representation.
 * Adds a text string after the text.
 *
 * PARAMETERS:      Name              Usage
 *                  str               Poniter to the string
 *                  u16ZoneStatus     Zone Status
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vGetZoneStatusBitString(char *str,uint16 u16ZoneStatus)
{
    int i;
    memset(str,0,255);
    for(i = 0; i<8 ; i++)
    {
        if((((uint8)u16ZoneStatus >> i) & 0x01))
            strcat(str,"1 ");
        else
            strcat(str,"0 ");
    }
}

/****************************************************************************
 *
 * NAME: vGetZoneConfigString
 *
 * DESCRIPTION:
 * Finds whether a zone belongs to day / night group & append the appropriate string
 *
 * PARAMETERS:      Name              Usage
 *                  str               Pointer to the string
 *                  u8DeviceIndex     Device Index in the discovery table
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vGetZoneConfigString(char *str,uint8 u8DeviceIndex)
{
    memset(str,0,255);
    if(sDiscovedZoneServers[u8DeviceIndex].u8Config & CLD_IASACE_ZONE_CONFIG_FLAG_DAY_HOME)
    {
        strcat(str,"D ");
    }
    if(sDiscovedZoneServers[u8DeviceIndex].u8Config & CLD_IASACE_ZONE_CONFIG_FLAG_NIGHT_SLEEP)
    {
        strcat(str,"N ");
    }
    if(sDiscovedZoneServers[u8DeviceIndex].u8Config & CLD_IASACE_ZONE_CONFIG_FLAG_BYPASS)
    {
        strcat(str,"B ");
    }
    /* No flags set ? */
    if (0 == strlen(str))
    {
		/* Use dash */
		strcat(str, "] ");
	}
}

/****************************************************************************
 *
 * NAME: vGetZoneLabelString
 *
 * DESCRIPTION:
 * Converts an zone type value to a string for LCD display.
 * Adds a text string after the text.
 *
 * PARAMETERS:      Name              Usage
 *                  str               pointer to the string
 *                  u16ZoneType       Zone Type
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vGetZoneLabelString(char *str,uint16 u16ZoneType)
{
    memset(str,0,255);
    switch(u16ZoneType)
    {
        case 0x0015:
            strcpy(str,"Kitchen");
        break;
        case 0x002d:
            strcpy(str,"BedRoom 1");
        break;
        case 0x021d:
            strcpy(str,"KeyPad");
        break;
        case 0x0225:
            strcpy(str,"Sounder");
        break;
        default:
        break;
    }
}

/****************************************************************************
 *
 * NAME: vLCDDisplay
 *
 * DESCRIPTION:
 * Displays the LCD Panel based on the user interaction
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vLCDDisplay(void)
{
        int i;
        char acString1[MAXIMUM_LENGTH_OF_STRING];
        char acLCDString[MAXIMUM_NUMBER_OF_ROWS][MAXIMUM_NUMBER_OF_COLUMNS];
        memset(acString1,0,MAXIMUM_LENGTH_OF_STRING);
        memset(acLCDString,0,(MAXIMUM_NUMBER_OF_ROWS * MAXIMUM_NUMBER_OF_COLUMNS));
        switch (eLCDState)
        {
            case E_LCD_INTIAL_DISPLAY:
                /* Initial display screen after erasing the PDM */
                strcpy(acLCDString[0],"Panel");
                break;

            case E_LCD_NETWORK_FORMATION:
                /* Display screen when the CIE is searching for network  */
                strcpy(acLCDString[0],"Panel");
                strcpy(acLCDString[1],"Scanning");
                break;

            case E_LCD_NETWORK_FORMED:
            {
                /* Display screen when the CIE has formed network &
                 * discovered security devices  */
                strcpy(acLCDString[0],"Panel");
                strcpy(acLCDString[1],"Network Formed");
                for(i=0; i < u8Discovered; i++)
                {
                    vGetZoneTypeString(acString1,sDiscovedZoneServers[i].u16ZoneType);
                    strcat(acString1,"    Joined");
                    if(i+2 > 17) /* As we have just 18 rows */
                        break;
                    strcpy(acLCDString[i+2],acString1);
                }
                break;
            }
            case E_LCD_ACE_MENU:
            {
                /* Display ACE screen */
                bAceMessage = FALSE;
                memcpy(acLCDString,acACELCDString,(MAXIMUM_NUMBER_OF_ROWS * MAXIMUM_NUMBER_OF_COLUMNS));
                strcpy(acLCDString[0],"ACE display");
                break;
            }
            case E_LCD_CIE_MAIN_MENU:
            {
                /* Display CIE Main menu screen */
                strcpy(acLCDString[0],"CIE MAIN MENU");
                strcpy(acLCDString[1],"Security Devices");
                strcpy(acLCDString[2],"Configuration");
                if(ZPS_bGetPermitJoiningStatus())
                    strcpy(acLCDString[3],"Disable Join");
                else
                    strcpy(acLCDString[3],"Enable Join");
                strcpy(acLCDString[4],"Arm Disarm");
                strcpy(acLCDString[5],"Bypass Zones");
                strcpy(acLCDString[6],"Panel Status");
                break;
            }

            case E_LCD_DISPLAY_SECURITY_DEVICES:
            {
                /* Displays all the security devices which has joined */
                strcpy(acLCDString[0],"Security Devices");
                for(i=0; i < u8Discovered; i++)
                {
                    if(sDiscovedZoneServers[i].bValid == TRUE)
                    {
                        vGetZoneIDString(acString1,sDiscovedZoneServers[i].u8ZoneId);
                        strcpy(acLCDString[i+1],acString1);
                        strcat(acLCDString[i+1],"   ");
                        vGetZoneTypeString(acString1,sDiscovedZoneServers[i].u16ZoneType);
                        strcat(acString1,"   ");
                        strcat(acLCDString[i+1],acString1);
                        vNumberToString(acString1,(uint64)sDiscovedZoneServers[i].u16NwkAddrOfServer,E_ZCL_UINT16);
                        strcat(acLCDString[i+1],acString1);
                    }
                }
            break;
            }
            case E_LCD_DISPLAY_ARM_DISARM_THE_SYSTEM:
            {
                /* Display for Arming the system
                 */
                uint8 u8ParameterValue = 0;
                strcpy(acLCDString[0],"Panel: ");
                eCLD_IASACEGetPanelParameter(CIE_EP,E_CLD_IASACE_PANEL_PARAMETER_PANEL_STATUS,&u8ParameterValue);
                vGetPanelStatusString(acString1,u8ParameterValue);
                strcat(acLCDString[0],acString1);
                strcpy(acLCDString[1],"Disarm Or CancelAlarm");
                strcpy(acLCDString[2],"Arm All Zones");
                strcpy(acLCDString[3],"Arm Day Zones");
                strcpy(acLCDString[4],"Arm Night Zones");
             break;
            }
            case E_LCD_DISPLAY_CONFIG_PERMIT_ENROLLMENT_ZONES:
            {
                /* Display for Configuration Option under CIE main menu for allowing
                 * Enroll Permit for a particular zone
                 */
                strcpy(acLCDString[0],"No Enroll Permit Cnfg");
                for(i=0; i < u8Discovered; i++)
                {
                    vGetZoneTypeString(acString1,sDiscovedZoneServers[i].u16ZoneType);
                    if(sDiscovedZoneServers[i].bValid == TRUE)
                    {
                        if(sDiscovedZoneServers[i].u8PermitEnrol){
                            strcat(acString1,"     Allow");
                        }else{
                            strcat(acString1,"     NotAllow");
                        }
                        strcpy(acLCDString[i+1],acString1);
                    }
                }
            break;
            }

            case E_LCD_DISPLAY_CONFIG_BYPASSED_LIST:
            {
                /* Display for Bypass Zones Option under CIE main menu for allowing
                 * adding zones into bypass List
                 */
                strcpy(acLCDString[0],"Bypassed");
                for(i=0; i < u8Discovered; i++)
                {
                    if((sDiscovedZoneServers[i].bValid == TRUE) &&
                            (sDiscovedZoneServers[i].u8ZoneId != 0xFF))
                    {
                        vGetZoneIDString(acString1,sDiscovedZoneServers[i].u8ZoneId);
                        strcpy(acLCDString[i+1],acString1);
                        strcat(acLCDString[i+1],"   ");
                        vGetZoneTypeString(acString1,sDiscovedZoneServers[i].u16ZoneType);
                        if(ZCL_IS_BIT_SET(uint8,sDiscovedZoneServers[i].u8ArmBypass,CLD_IASACE_ZONE_STATUS_FLAG_BYPASS)){
                            strcat(acString1,"     Yes");
                        }else{
                            strcat(acString1,"     No");
                        }
                        strcat(acLCDString[i+1],acString1);
                    }
                }
            break;
            }

            case E_LCD_DISPLAY_PANEL_STATUS:
            {
                /* Display For Panel Status option under CIE Main Menu */
                uint8 u8ParameterValue = 0;
                strcpy(acLCDString[0],"         Panel Status");
                eCLD_IASACEGetPanelParameter(CIE_EP,E_CLD_IASACE_PANEL_PARAMETER_PANEL_STATUS,&u8ParameterValue);
                vGetPanelStatusString(acString1,u8ParameterValue);
                strcpy(acLCDString[1], "Panel: ");
                strcat(acLCDString[1],acString1);
                eCLD_IASACEGetPanelParameter(CIE_EP,E_CLD_IASACE_PANEL_PARAMETER_ALARM_STATUS,&u8ParameterValue);
                vGetAlarmStatusString(acString1,u8ParameterValue);
                strcpy(acLCDString[2],"Alarm: ");
                strcat(acLCDString[2],acString1);
                eCLD_IASACEGetPanelParameter(CIE_EP,E_CLD_IASACE_PANEL_PARAMETER_AUDIBLE_NOTIFICATION,&u8ParameterValue);
                vGetSoundString(acString1,u8ParameterValue);
                strcpy(acLCDString[3],"Sound: ");
                strcat(acLCDString[3],acString1);
                eCLD_IASACEGetPanelParameter(CIE_EP,E_CLD_IASACE_PANEL_PARAMETER_SECONDS_REMAINING,&u8ParameterValue);
                vNumberToString(acString1,(uint64)u8ParameterValue,E_ZCL_UINT8);
                strcpy(acLCDString[4],"Seconds Left: ");
                strcat(acLCDString[4],acString1);
            break;
            }

            case E_LCD_DISPLAY_CONFIG_ARM_GROUP_ZONES:
            {
                /* Display for Arm Group Option under Security devices menu for allowing
                 * adding zones to be added into Day,Night,Bypass
                 */
                vGetZoneTypeString(acString1,sDiscovedZoneServers[u8DeviceIndex].u16ZoneType);
                strcpy(acLCDString[0],acString1);
                strcpy(acLCDString[1],"Arm Groups");
                strcpy(acLCDString[2],"Day");
                if(sDiscovedZoneServers[u8DeviceIndex].u8Config & CLD_IASACE_ZONE_CONFIG_FLAG_DAY_HOME)
                {
                    strcat(acLCDString[2],"              Yes");
                }else{
                    strcat(acLCDString[2],"              No");
                }
                strcpy(acLCDString[3],"Night");
                if(sDiscovedZoneServers[u8DeviceIndex].u8Config & CLD_IASACE_ZONE_CONFIG_FLAG_NIGHT_SLEEP)
                {
                    strcat(acLCDString[3],"            Yes");
                }else{
                    strcat(acLCDString[3],"            No");
                }
                strcpy(acLCDString[4],"Bypass");
                if(sDiscovedZoneServers[u8DeviceIndex].u8Config & CLD_IASACE_ZONE_CONFIG_FLAG_BYPASS)
                {
                    strcat(acLCDString[4],"         Yes");
                }else{
                    strcat(acLCDString[4],"         No");
                }
            break;
            }

            case E_LCD_DISPLAY_ZONE_INFORMATION:
            {
                /* Display for Information of different Security devices
                */
                vGetActualZoneTypeString(acString1,sDiscovedZoneServers[u8DeviceIndex].u16ZoneType);
                strcpy(acLCDString[0],acString1);
                vGetZoneLabelString(acString1,sDiscovedZoneServers[u8DeviceIndex].u16ZoneType);
                strcpy(acLCDString[1],acString1);
                strcpy(acString1,"Status:");
                if(sDiscovedZoneServers[u8DeviceIndex].u8ZoneId != 0xFF)
                    strcat(acString1," Enrolled");
                else
                    strcat(acString1," Not Enrolled");
                strcpy(acLCDString[2],acString1);
                strcpy(acLCDString[3],"1 2 T B S R F M");
                vGetZoneStatusBitString(acString1,sDiscovedZoneServers[u8DeviceIndex].u16ZoneStatus);
                strcpy(acLCDString[4],acString1);
                strcpy(acLCDString[5],"Arm Groups:");
                vGetZoneConfigString(acString1,u8DeviceIndex);
                strcat(acLCDString[5],acString1);
                vNumberToString(acString1,sDiscovedZoneServers[u8DeviceIndex].u64IeeeAddrOfServer,E_ZCL_UINT64);
                strcpy(acLCDString[6],acString1);
                break;
            }

            default:
            break;
        }
        vLcdClear();

        /* As the number of lines to be displayed are more than MAXIMUM_NUMBER_OF_ROWS_PER_PANEL,
         * gu8CursorPosition is needed for navigation
         */
        if(E_LCD_ACE_MENU != eLCDState &&
                E_LCD_NETWORK_FORMED != eLCDState &&
                E_LCD_DISPLAY_SECURITY_DEVICES != eLCDState &&
                E_LCD_DISPLAY_CONFIG_PERMIT_ENROLLMENT_ZONES != eLCDState &&
                E_LCD_DISPLAY_ARM_DISARM_THE_SYSTEM != eLCDState &&
                E_LCD_DISPLAY_CONFIG_BYPASSED_LIST != eLCDState)
        {
            gu8CursorPosition = 0;
        }

        /* Write to LCD Panel */
        for(i = 0 ;i<MAXIMUM_NUMBER_OF_ROWS_PER_PANEL ;i++)
        {
            if(i == (u8MenuIndex - gu8CursorPosition))
                vLcdWriteInvertedText(acLCDString[i+gu8CursorPosition],i,0);
            else
                vLcdWriteText(acLCDString[i+gu8CursorPosition],i,0);
        }

        /* Last line to be always fixed to Menu   Up   Down  Mode */
        if(E_LCD_ACE_MENU != eLCDState)
        {
        	if(bAceMessage)
        	{
            	vLcdWriteText("Menu   Up   Down   Sel",MAXIMUM_NUMBER_OF_ROWS_PER_PANEL,0);
            	vLcdWriteInvertedText("C",MAXIMUM_NUMBER_OF_ROWS_PER_PANEL,120);
        	}
        	else
        	{
            	vLcdWriteText("Menu   Up   Down   Sel C",MAXIMUM_NUMBER_OF_ROWS_PER_PANEL,0);
			}
        }
        else
        {
            vLcdWriteText("Menu   Up   Down   Sel A",MAXIMUM_NUMBER_OF_ROWS_PER_PANEL,0);
		}

        vLcdRefreshAll();
}

/****************************************************************************
 *
 * NAME: vChangeLCDState
 *
 * DESCRIPTION:
 * Change the state of LCD based on button press
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vChangeLCDState(uint8 u8Key)
{
    switch (eLCDState)
    {
        case E_LCD_NETWORK_FORMED:
        {
            switch(u8Key)
            {
                case APP_E_BUTTONS_BUTTON_SW3:
                        /* Switches to Main Menu screen if SW1 button is pressed */
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                case APP_E_BUTTONS_BUTTON_SW2:
                        vMoveScreenUp(MINIMUM_INDEX_OF_SCREEN);
                    break;

                case APP_E_BUTTONS_BUTTON_SW1:
                        vMoveScreenDown((3 + u8Discovered));
                    break;

                case APP_E_BUTTONS_BUTTON_SW4:
                    break;
                default:
                    break;
            }
        break;
        }
        case E_LCD_ACE_MENU:
        {
            switch(u8Key)
            {
                case APP_E_BUTTONS_BUTTON_SW3:
                    /* Switches to Main Menu screen if SW1 button is pressed */
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                case APP_E_BUTTONS_BUTTON_SW2:
                        vMoveScreenUp(MINIMUM_INDEX_OF_SCREEN);
                    break;

                case APP_E_BUTTONS_BUTTON_SW1:
                        /* keep moving down till the last line */
                        vMoveScreenDown((MAXIMUM_NUMBER_OF_ROWS - 1));
                    break;

                case APP_E_BUTTONS_BUTTON_SW4:
                    break;

                case APP_E_BUTTONS_BUTTON_1:
                    /* Switches to Network formed screen if DIO8 button is pressed */
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_NETWORK_FORMED;
                break;

                default:
                    break;
            }
        break;
        }
        case E_LCD_CIE_MAIN_MENU:
        {
            switch(u8Key)
            {
                case APP_E_BUTTONS_BUTTON_SW3:
                /* Switches to Main Menu screen if SW1 button is pressed */
                    vSetIntialDisplayPostition();
                    eLCDState = E_LCD_CIE_MAIN_MENU;
                break;

                case APP_E_BUTTONS_BUTTON_SW2:
                    /* keep moving up till the second last line */
                    vMoveScreenUp(MINIMUM_INDEX_OF_SCREEN);
                    eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                case APP_E_BUTTONS_BUTTON_SW1:
                    /* keep moving down till the second last line */
                    vMoveScreenDown(NUMBER_OF_CIE_MAIN_MENU_OPTION);
                    eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                case APP_E_BUTTONS_BUTTON_SW4:
                    /* Select the Menu Option */
                        switch(u8MenuIndex)
                        {
                            case E_CIE_MAIN_MENU_SECURITY_DEVICES:
                                /* Switches to security devices display */
                                if(u8Discovered != 0)
                                {
                                    vSetIntialDisplayPostition();
                                    eLCDState = E_LCD_DISPLAY_SECURITY_DEVICES;
                                }
                            break;
                            case E_CIE_MAIN_MENU_CONFIGURATION:
                                /* Switches to Configuration screen for Permit Enrollment display */
                                if(u8Discovered != 0)
                                {
                                    vSetIntialDisplayPostition();
                                    eLCDState = E_LCD_DISPLAY_CONFIG_PERMIT_ENROLLMENT_ZONES;
                                }
                            break;
                            case E_CIE_MAIN_MENU_PERMIT_JOIN:
                                /* Toggles Enable/disable Permit Join */
                                if(ZPS_bGetPermitJoiningStatus())
                                    vEnablePermitJoin(0);
                                else
                                    vEnablePermitJoin(EZ_MODE_TIME * 60);
                            break;
                            case E_CIE_MAIN_MENU_ARM_DISARM:
                            {
                                vSetIntialDisplayPostition();
                                eLCDState = E_LCD_DISPLAY_ARM_DISARM_THE_SYSTEM;
                                break;
                            }
                            case E_CIE_MAIN_MENU_BYPASS_ZONES:
                                /* Switches to Bypass Zone display */
                                if(u8Discovered != 0)
                                {
                                    vSetIntialDisplayPostition();
                                    eLCDState = E_LCD_DISPLAY_CONFIG_BYPASSED_LIST;
                                }
                            break;
                            case E_CIE_MAIN_MENU_PANEL_STATUS:
                                /* Switches to Panel Status display */
                                vSetIntialDisplayPostition();
                                eLCDState = E_LCD_DISPLAY_PANEL_STATUS;
                            break;
                            default:
                                eLCDState = E_LCD_CIE_MAIN_MENU;
                            break;
                        }
                    break;

                case APP_E_BUTTONS_BUTTON_1:
                /* Switches to Network formed screen if DIO8 button is pressed */
                    vSetIntialDisplayPostition();
                    eLCDState = E_LCD_NETWORK_FORMED;
                break;

                default:
                    break;
            }
            break;
        }
        case E_LCD_DISPLAY_PANEL_STATUS:
        {
            switch(u8Key)
            {
                case APP_E_BUTTONS_BUTTON_SW3:
                /* Switches to Main Menu screen if SW1 button is pressed */
                    vSetIntialDisplayPostition();
                    eLCDState = E_LCD_CIE_MAIN_MENU;
                break;

                case APP_E_BUTTONS_BUTTON_1:
                /* Switches back to Main Menu screen if DI08 button is pressed */
                    vSetIntialDisplayPostition();
                    eLCDState = E_LCD_CIE_MAIN_MENU;
                break;

                default:
                    break;
            }
        break;
        }
        case E_LCD_DISPLAY_SECURITY_DEVICES:
        {
            switch(u8Key)
            {
                case APP_E_BUTTONS_BUTTON_SW3:
                        /* Switches to Main Menu screen if SW1 button is pressed */
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                case APP_E_BUTTONS_BUTTON_SW2:
                        vMoveScreenAndDeviceUp();
                        eLCDState = E_LCD_DISPLAY_SECURITY_DEVICES;
                    break;

                case APP_E_BUTTONS_BUTTON_SW1:
                        vMoveScreenAndDeviceDown();
                        eLCDState = E_LCD_DISPLAY_SECURITY_DEVICES;
                    break;

                case APP_E_BUTTONS_BUTTON_SW4:
                        /* switches to Zone Information display if pressed for less than one sec */
                        u8MenuIndex = MINIMUM_INDEX_OF_ZONE_INFO_SCREEN;
                        eLCDState = E_LCD_DISPLAY_ZONE_INFORMATION;
                    break;

                case APP_E_BUTTONS_BUTTON_1:
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                default:
                    break;
            }
            break;
        }

        case E_LCD_DISPLAY_ARM_DISARM_THE_SYSTEM:
         {
             switch(u8Key)
             {
                 case APP_E_BUTTONS_BUTTON_SW3:
                         vSetIntialDisplayPostition();
                         eLCDState = E_LCD_CIE_MAIN_MENU;
                     break;

                 case APP_E_BUTTONS_BUTTON_SW2:
                        /* keep moving up till the second last line */
                         vMoveScreenUp(MINIMUM_INDEX_OF_SCREEN);
                         eLCDState = E_LCD_DISPLAY_ARM_DISARM_THE_SYSTEM;
                     break;

                 case APP_E_BUTTONS_BUTTON_SW1:
                        /* keep moving down till the second last line */
                         vMoveScreenDown(MAXIMUM_INDEX_OF_ARM_SYSTEM_SCREEN);
                         eLCDState = E_LCD_DISPLAY_ARM_DISARM_THE_SYSTEM;
                     break;

                 case APP_E_BUTTONS_BUTTON_SW4:
                 {
                     uint8 u8PanelStatus = 0;
                     int Count;
                     eCLD_IASACEGetPanelParameter(CIE_EP,E_CLD_IASACE_PANEL_PARAMETER_PANEL_STATUS,&u8PanelStatus);
                     /* Move to disarm state only if you are not already disarmed*/
                     if(u8MenuIndex == 1)
                     {
                         if(u8PanelStatus != E_CLD_IASACE_PANEL_STATUS_PANEL_DISARMED)
                         {
                             /* Disarm the system */
                             /* Make sure to stop exit/entry delay timer & accept the request */
                             eCLD_IASACESetPanelParameter (
                                     CIE_EP,
                                     E_CLD_IASACE_PANEL_PARAMETER_SECONDS_REMAINING,
                                     0);
                             OS_eStopSWTimer(APP_EntryExitDelayTmr);
                             bStayInExitDelay = FALSE;
                             /* Cancels all the alarm & move the system to disarm state */
                             /* Stop any warning */
                             vStartWarning(DISABLE_WARNING,DISABLE_WARNING,DISABLE_WARNING,DISABLE_WARNING);
                             vSetPanelParamter(E_CLD_IASACE_PANEL_STATUS_PANEL_DISARMED,E_CLD_IASACE_ALARM_STATUS_NO_ALARM,E_CLD_IASACE_AUDIBLE_NOTIF_MUTE);
                             vStartSquawk(SQUAWK_MODE_STROBE_AND_LEVEL_DISARMED);
                             for(Count=0; Count < u8Discovered; Count++)
                             {
                                 if(sDiscovedZoneServers[Count].bValid == TRUE)
                                 {
                                     /* If arm bit/bypass bit is set , clear it and set zone parameter for ACE server cluster */
                                     ZCL_BIT_CLEAR(uint8,sDiscovedZoneServers[Count].u8ArmBypass,CLD_IASACE_ZONE_STATUS_FLAG_ARM);
                                     ZCL_BIT_CLEAR(uint8,sDiscovedZoneServers[Count].u8ArmBypass,CLD_IASACE_ZONE_STATUS_FLAG_BYPASS);
                                     eCLD_IASACESetZoneParameterValue (
                                                 CIE_EP,
                                                 E_CLD_IASACE_ZONE_PARAMETER_ZONE_STATUS_FLAG,
                                                 sDiscovedZoneServers[Count].u8ZoneId,
                                                 sDiscovedZoneServers[Count].u8ArmBypass);
                                 }
                             }
                         }
                     }
                     else
                     {
                         /* Move to arm state only if you are disarmed && no alarm on any zones*/
                         if(u8PanelStatus == E_CLD_IASACE_PANEL_STATUS_PANEL_DISARMED)
                         {
                                 switch(u8MenuIndex)
                                 {
                                     case 2:
                                         u8ConfigFlag = 0xFF;
                                         u8PanelStatus = E_CLD_IASACE_PANEL_STATUS_PANEL_ARMING_AWAY;
                                         break;
                                     case 3:
                                         u8ConfigFlag = CLD_IASACE_ZONE_CONFIG_FLAG_DAY_HOME;
                                         u8PanelStatus = E_CLD_IASACE_PANEL_STATUS_PANEL_ARMING_STAY;
                                         break;
                                     case 4:
                                         u8ConfigFlag = CLD_IASACE_ZONE_CONFIG_FLAG_NIGHT_SLEEP;
                                         u8PanelStatus = E_CLD_IASACE_PANEL_STATUS_PANEL_ARMING_NIGHT;
                                         break;
                                     default:
                                         break;
                                 }
                                 if(bCheckNotReadyToArm(u8ConfigFlag))
                                 {
                                    /* Not ready to arm as one of the zones is in alarm */
                                    u8PanelStatusB4NotReadyToArm = E_CLD_IASACE_PANEL_STATUS_PANEL_DISARMED;
                                    vSetPanelParamter(E_CLD_IASACE_PANEL_STATUS_PANEL_NOT_READY_TO_ARM,E_CLD_IASACE_ALARM_STATUS_NO_ALARM,E_CLD_IASACE_AUDIBLE_NOTIF_MUTE);
                                 }
                                 else
                                 {
                                    u8LastPanelStatus = u8PanelStatus;
                                    vSetPanelParamter(u8PanelStatus,E_CLD_IASACE_ALARM_STATUS_NO_ALARM,E_CLD_IASACE_AUDIBLE_NOTIF_MUTE);
                                    /* Check for types of zones and make ARM flag to true if the system is getting armed  */
                                     for(Count=0; Count < u8Discovered; Count++)
                                     {
                                         if((sDiscovedZoneServers[Count].bValid == TRUE) &&
                                                 (sDiscovedZoneServers[Count].u8Config & u8ConfigFlag || u8ConfigFlag == 0xFF) &&
                                                     (!(sDiscovedZoneServers[Count].u8ArmBypass & CLD_IASACE_ZONE_STATUS_FLAG_BYPASS)))
                                         {
                                             sDiscovedZoneServers[Count].u8ArmBypass |= CLD_IASACE_ZONE_STATUS_FLAG_ARM;
                                             eCLD_IASACESetZoneParameterValue (
                                                     CIE_EP,
                                                     E_CLD_IASACE_ZONE_PARAMETER_ZONE_STATUS_FLAG,
                                                     sDiscovedZoneServers[Count].u8ZoneId,
                                                     sDiscovedZoneServers[Count].u8ArmBypass);
                                         }
                                     }
                                     vStartArmingSystem();
                                 }
                         }
                     }
                     PDM_eSaveRecordData( PDM_ID_APP_IASACE_ZONE_PARAM,
                                          (tsCLD_IASACE_ZoneParameter *)&sDevice.sIASACEServerCustomDataStructure.asCLD_IASACE_ZoneParameter[0],
                                          sizeof(tsCLD_IASACE_ZoneParameter) * CLD_IASACE_ZONE_TABLE_SIZE);
                     PDM_eSaveRecordData( PDM_ID_APP_IASCIE_STRUCT,
                                          &sDiscovedZoneServers[0],
                                          sizeof(tsDiscovedZoneServers) * MAX_ZONE_SERVER_NODES);
                     eLCDState = E_LCD_DISPLAY_ARM_DISARM_THE_SYSTEM;
                     break;
                 }

                 case APP_E_BUTTONS_BUTTON_1:
                         vSetIntialDisplayPostition();
                         eLCDState = E_LCD_CIE_MAIN_MENU;
                     break;

                 default:
                     break;
             }
             break;
         }
        case E_LCD_DISPLAY_CONFIG_PERMIT_ENROLLMENT_ZONES:
        {
            switch(u8Key)
            {
                case APP_E_BUTTONS_BUTTON_SW3:
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                case APP_E_BUTTONS_BUTTON_SW2:
                        vMoveScreenAndDeviceUp();
                        eLCDState = E_LCD_DISPLAY_CONFIG_PERMIT_ENROLLMENT_ZONES;
                    break;

                case APP_E_BUTTONS_BUTTON_SW1:
                        vMoveScreenAndDeviceDown();
                        eLCDState = E_LCD_DISPLAY_CONFIG_PERMIT_ENROLLMENT_ZONES;
                    break;

                case APP_E_BUTTONS_BUTTON_SW4:
                       /* Toggles the enroll permit configuration */
                      sDiscovedZoneServers[u8DeviceIndex].u8PermitEnrol = !(sDiscovedZoneServers[u8DeviceIndex].u8PermitEnrol);
                      PDM_eSaveRecordData( PDM_ID_APP_IASCIE_STRUCT,
                                           &sDiscovedZoneServers[0],
                                           sizeof(tsDiscovedZoneServers) * MAX_ZONE_SERVER_NODES);
                      eLCDState = E_LCD_DISPLAY_CONFIG_PERMIT_ENROLLMENT_ZONES;
                    break;

                case APP_E_BUTTONS_BUTTON_1:
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                default:
                    break;
            }
            break;
        }

        case E_LCD_DISPLAY_CONFIG_BYPASSED_LIST:
        {
            switch(u8Key)
            {
                case APP_E_BUTTONS_BUTTON_SW3:
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                case APP_E_BUTTONS_BUTTON_SW2:
                        vMoveScreenAndDeviceUp();
                        eLCDState = E_LCD_DISPLAY_CONFIG_BYPASSED_LIST;
                    break;

                case APP_E_BUTTONS_BUTTON_SW1:
                        vMoveScreenAndDeviceDown();
                        eLCDState = E_LCD_DISPLAY_CONFIG_BYPASSED_LIST;
                    break;

                case APP_E_BUTTONS_BUTTON_SW4:
                    /* Toggles the Bypass list configuration  and writes to ACE cluster server zone parameter*/
                    if(ZCL_IS_BIT_SET(uint8,sDiscovedZoneServers[u8DeviceIndex].u8ArmBypass,CLD_IASACE_ZONE_STATUS_FLAG_BYPASS)){
                        ZCL_BIT_CLEAR(uint8,sDiscovedZoneServers[u8DeviceIndex].u8ArmBypass,CLD_IASACE_ZONE_STATUS_FLAG_BYPASS);
                        ZCL_BIT_CLEAR(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_BYPASS);
                    }
                    else{
                        ZCL_BIT_SET(uint8,sDiscovedZoneServers[u8DeviceIndex].u8ArmBypass,CLD_IASACE_ZONE_STATUS_FLAG_BYPASS);
                        ZCL_BIT_SET(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_BYPASS);
                        ZCL_BIT_CLEAR(uint8,sDiscovedZoneServers[u8DeviceIndex].u8ArmBypass,CLD_IASACE_ZONE_STATUS_FLAG_ARM);
                    }
                    eCLD_IASACESetZoneParameterValue (
                                CIE_EP,
                                E_CLD_IASACE_ZONE_PARAMETER_ZONE_STATUS_FLAG,
                                sDiscovedZoneServers[u8DeviceIndex].u8ZoneId,
                                sDiscovedZoneServers[u8DeviceIndex].u8ArmBypass);
                    eCLD_IASACESetZoneParameterValue (
                                CIE_EP,
                                E_CLD_IASACE_ZONE_PARAMETER_ZONE_CONFIG_FLAG,
                                sDiscovedZoneServers[u8DeviceIndex].u8ZoneId,
                                sDiscovedZoneServers[u8DeviceIndex].u8Config);

                    PDM_eSaveRecordData( PDM_ID_APP_IASACE_ZONE_PARAM,
                                         (tsCLD_IASACE_ZoneParameter *)&sDevice.sIASACEServerCustomDataStructure.asCLD_IASACE_ZoneParameter[0],
                                         sizeof(tsCLD_IASACE_ZoneParameter) * CLD_IASACE_ZONE_TABLE_SIZE);
                    PDM_eSaveRecordData( PDM_ID_APP_IASCIE_STRUCT,
                                         &sDiscovedZoneServers[0],
                                         sizeof(tsDiscovedZoneServers) * MAX_ZONE_SERVER_NODES);;
                    eLCDState = E_LCD_DISPLAY_CONFIG_BYPASSED_LIST;
                    break;

                case APP_E_BUTTONS_BUTTON_1:
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                default:
                    break;
            }
            break;
        }
        case E_LCD_DISPLAY_CONFIG_ARM_GROUP_ZONES:
        {
                switch(u8Key)
                {
                case APP_E_BUTTONS_BUTTON_SW3:
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_CIE_MAIN_MENU;
                    break;

                case APP_E_BUTTONS_BUTTON_SW2:
                        vMoveScreenUp(MINIMUM_INDEX_OF_ARM_GROUP_SCREEN);
                        eLCDState = E_LCD_DISPLAY_CONFIG_ARM_GROUP_ZONES;
                    break;

                case APP_E_BUTTONS_BUTTON_SW1:
                        vMoveScreenDown(MAXIMUM_INDEX_OF_ARM_GROUP_SCREEN);
                        eLCDState = E_LCD_DISPLAY_CONFIG_ARM_GROUP_ZONES;
                    break;

                case APP_E_BUTTONS_BUTTON_SW4:
                    /* Sets ARM group configuration  and writes to ACE cluster server zone parameter*/
                    switch(u8MenuIndex)
                    {
                    case 2:
                        if(ZCL_IS_BIT_SET(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_DAY_HOME))
                            ZCL_BIT_CLEAR(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_DAY_HOME);
                        else
                            ZCL_BIT_SET(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_DAY_HOME);
                    break;
                    case 3:
                        if(ZCL_IS_BIT_SET(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_NIGHT_SLEEP))
                            ZCL_BIT_CLEAR(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_NIGHT_SLEEP);
                        else
                            ZCL_BIT_SET(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_NIGHT_SLEEP);
                    break;
                    case 4:
                        if(ZCL_IS_BIT_SET(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_BYPASS))
                            ZCL_BIT_CLEAR(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_BYPASS);
                        else
                            ZCL_BIT_SET(uint8,sDiscovedZoneServers[u8DeviceIndex].u8Config,CLD_IASACE_ZONE_CONFIG_FLAG_BYPASS);
                    break;
                    }
                    eCLD_IASACESetZoneParameterValue (
                                CIE_EP,
                                E_CLD_IASACE_ZONE_PARAMETER_ZONE_CONFIG_FLAG,
                                sDiscovedZoneServers[u8DeviceIndex].u8ZoneId,
                                sDiscovedZoneServers[u8DeviceIndex].u8Config);

                    PDM_eSaveRecordData( PDM_ID_APP_IASACE_ZONE_PARAM,
                                         (tsCLD_IASACE_ZoneParameter *)&sDevice.sIASACEServerCustomDataStructure.asCLD_IASACE_ZoneParameter[0],
                                         sizeof(tsCLD_IASACE_ZoneParameter) * CLD_IASACE_ZONE_TABLE_SIZE);
                    PDM_eSaveRecordData( PDM_ID_APP_IASCIE_STRUCT,
                                         &sDiscovedZoneServers[0],
                                         sizeof(tsDiscovedZoneServers) * MAX_ZONE_SERVER_NODES);
                    eLCDState = E_LCD_DISPLAY_CONFIG_ARM_GROUP_ZONES;
                    break;

                case APP_E_BUTTONS_BUTTON_1:
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_DISPLAY_SECURITY_DEVICES;
                    break;

                default:
                    break;
            }
            break;
        }
        case E_LCD_DISPLAY_ZONE_INFORMATION:
        {
            switch(u8Key)
            {
                case APP_E_BUTTONS_BUTTON_SW3:
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_CIE_MAIN_MENU;
                break;

                case APP_E_BUTTONS_BUTTON_SW2:
                        u8MenuIndex = MINIMUM_INDEX_OF_ZONE_INFO_SCREEN;
                        eLCDState = E_LCD_DISPLAY_ZONE_INFORMATION;
                    break;

                case APP_E_BUTTONS_BUTTON_SW1:
                        u8MenuIndex = MAXIMUM_INDEX_OF_ZONE_INFO_SCREEN;
                        eLCDState = E_LCD_DISPLAY_ZONE_INFORMATION;
                    break;
                break;
                case APP_E_BUTTONS_BUTTON_SW4:
                        if(u8MenuIndex == MINIMUM_INDEX_OF_ZONE_INFO_SCREEN)
                        {
                            if(sDiscovedZoneServers[u8DeviceIndex].u8ZoneId != 0xFF){
                                vSendUnenrollReq(sDiscovedZoneServers[u8DeviceIndex].u8ZoneId);
                            }
                            else{
                                sDiscovedZoneServers[u8DeviceIndex].u8PermitEnrol = 1;
                                vSendAutoEnroll(u8DeviceIndex);
                            }
                            PDM_eSaveRecordData( PDM_ID_APP_IASCIE_STRUCT,
                                                 &sDiscovedZoneServers[0],
                                                 sizeof(tsDiscovedZoneServers) * MAX_ZONE_SERVER_NODES);
                            PDM_eSaveRecordData( PDM_ID_APP_IASACE_ZONE_TABLE,
                                                (tsCLD_IASACE_ZoneTable *)&sDevice.sIASACEServerCustomDataStructure.asCLD_IASACE_ZoneTable[0],
                                                sizeof(tsCLD_IASACE_ZoneTable) * CLD_IASACE_ZONE_TABLE_SIZE);
                            eLCDState = E_LCD_DISPLAY_ZONE_INFORMATION;
                        }else if(u8MenuIndex == MAXIMUM_INDEX_OF_ZONE_INFO_SCREEN)
                        {
                            u8MenuIndex = MINIMUM_INDEX_OF_ARM_GROUP_SCREEN;
                            eLCDState = E_LCD_DISPLAY_CONFIG_ARM_GROUP_ZONES;
                        }
                    break;
                case APP_E_BUTTONS_BUTTON_1:
                        vSetIntialDisplayPostition();
                        eLCDState = E_LCD_DISPLAY_SECURITY_DEVICES;
                    break;

                default:
                    break;
            }
            break;
        }
        default:
        break;
    }

    vLCDDisplay();
}

/****************************************************************************
 *
 * NAME: vACEPanelStatusDisplay
 *
 * DESCRIPTION:
 * Updated the ACE mode string for panel status display.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vACEPanelStatusDisplay(void)
{
    uint8 u8PanelStatus = 0;
    char acAceString[MAXIMUM_LENGTH_OF_STRING];

    bAceMessage = TRUE;

    eCLD_IASACEGetPanelParameter (
        CIE_EP,
        E_CLD_IASACE_PANEL_PARAMETER_PANEL_STATUS,
        &u8PanelStatus);

    vCheckForMenuIndex();
    vGetPanelStatusString(acAceString,u8PanelStatus);
    strcpy(acACELCDString[u8AceMenuIndex++],acAceString);

    vCheckForCursorPosition();
    vLCDDisplay();
}

/****************************************************************************
 *
 * NAME: vACEBypassResultDisplay
 *
 * DESCRIPTION:
 * Updated the ACE mode string for bypass result display.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vACEBypassResultDisplay(uint8 u8NumberOfZones,
                                    uint8 *pu8ZoneID)
{
    int i = 0;
    char acAceString[MAXIMUM_LENGTH_OF_STRING];
    uint8 u8ParameterLenght = 0,u8ConfigFlag = 0;
    tsCLD_IASACE_ZoneTable  *psZoneTable;

    bAceMessage = TRUE;

    vCheckForMenuIndex();
    strcpy(acACELCDString[u8AceMenuIndex++],"Adding bypass zones");

    for(i=0;i<u8NumberOfZones;i++)
    {
        eCLD_IASACEGetZoneParameter (
                        CIE_EP,
                        E_CLD_IASACE_ZONE_PARAMETER_ZONE_CONFIG_FLAG,
                        pu8ZoneID[i],
                        &u8ParameterLenght,
                        &u8ConfigFlag);

        vGetZoneIDString(acAceString,pu8ZoneID[i]);
        strcat(acAceString," - ");
        if(pu8ZoneID[i] >= CLD_IASACE_ZONE_TABLE_SIZE)
        {
            strcat(acAceString,"Invalid zone");
        }else if(eCLD_IASACEGetZoneTableEntry(CIE_EP,pu8ZoneID[i],&psZoneTable) != E_ZCL_CMDS_SUCCESS){
            strcat(acAceString,"Unknown zone");
        }else if(u8ConfigFlag & CLD_IASACE_ZONE_CONFIG_FLAG_NOT_BYPASSED){
            strcat(acAceString,"Not Bypassed");
        }else if(!(u8ConfigFlag & CLD_IASACE_ZONE_CONFIG_FLAG_BYPASS)){
            strcat(acAceString,"Not Allowed");
        }else{
            strcat(acAceString,"Bypassed");
        }
        vCheckForMenuIndex();
        strcpy(acACELCDString[u8AceMenuIndex++],acAceString);
    }

    vCheckForCursorPosition();
}

/****************************************************************************
 *
 * NAME: vACEZoneStatusDisplay
 *
 * DESCRIPTION:
 * Updated the ACE mode string for zone status display.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vACEZoneStatusDisplay(bool bZoneStatusMaskFlag)
{
    int i = 0;
    char acAceString[MAXIMUM_LENGTH_OF_STRING],acAceString1[MAXIMUM_NUMBER_OF_COLUMNS];
    bool bZoneInAlarm = FALSE;

    bAceMessage = TRUE;
    memset(acAceString1,0,MAXIMUM_NUMBER_OF_COLUMNS);
    if(bZoneStatusMaskFlag == FALSE)
    {
        for(i=0; i < u8Discovered; i++)
        {
            if((sDiscovedZoneServers[i].bValid == TRUE) &&
                    (sDiscovedZoneServers[i].u8ZoneId != 0xFF))
            {
                vCheckForMenuIndex();
                vGetZoneIDString(acAceString,sDiscovedZoneServers[i].u8ZoneId);
                strcpy(acACELCDString[u8AceMenuIndex],acAceString);
                strcat(acACELCDString[u8AceMenuIndex],"   ");
                vGetZoneTypeString(acAceString,sDiscovedZoneServers[i].u16ZoneType);
                strcat(acACELCDString[u8AceMenuIndex++],acAceString);
                vCheckForMenuIndex();
                vGetZoneLabelString(acAceString,sDiscovedZoneServers[i].u16ZoneType);
                strcpy(acACELCDString[u8AceMenuIndex++],acAceString);
                vCheckForMenuIndex();
                strcpy(acACELCDString[u8AceMenuIndex++],"1 2 T B S R F M");
                vCheckForMenuIndex();
                vGetZoneStatusBitString(acAceString,sDiscovedZoneServers[i].u16ZoneStatus);
                strcpy(acACELCDString[u8AceMenuIndex++],acAceString);
            }
        }
    }else{
        for(i=0; i < u8Discovered; i++)
        {
            if((sDiscovedZoneServers[i].bValid == TRUE) &&
                    (sDiscovedZoneServers[i].u8ZoneId != 0xFF))
            {
                if(sDiscovedZoneServers[i].u16ZoneStatus != 0 &&
                    (sDiscovedZoneServers[i].u8Config & u8ConfigFlag || u8ConfigFlag == 0xFF) &&
                    (!(sDiscovedZoneServers[i].u8ArmBypass & CLD_IASACE_ZONE_STATUS_FLAG_BYPASS)))
                {
                    bZoneInAlarm = TRUE;
                    vGetZoneIDString(acAceString,sDiscovedZoneServers[i].u8ZoneId);
                    strcat(acAceString," ");
                    strcat(acAceString1,acAceString);
                }
            }
        }
        vCheckForMenuIndex();
        if(bZoneInAlarm)
        {
            strcpy(acACELCDString[u8AceMenuIndex++],"Check Zones");
            vCheckForMenuIndex();
            strcpy(acACELCDString[u8AceMenuIndex++],acAceString1);
        }
        else
        {
            strcpy(acACELCDString[u8AceMenuIndex++],"Already Armed");
        }
    }

    vCheckForCursorPosition();

}

/****************************************************************************
 *
 * NAME: vACEZoneInfoDisplay
 *
 * DESCRIPTION:
 * Updated the ACE mode string for zone information display.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vACEZoneInfoDisplay(uint8 u8ZoneID)
{
    uint8 u8NumofZones = CLD_IASACE_ZONE_TABLE_SIZE,au8ZoneId[CLD_IASACE_ZONE_TABLE_SIZE];
    int i = 0;
    char acAceString[MAXIMUM_LENGTH_OF_STRING];

    eCLD_IASACEGetEnrolledZones(CIE_EP,
                                au8ZoneId,
                                &u8NumofZones);

    if(u8ZoneID == au8ZoneId[u8NumofZones-1])
    {
        bAceMessage = TRUE;
        for(i=0; i < u8Discovered; i++)
        {
            if((sDiscovedZoneServers[i].bValid == TRUE) &&
                    (sDiscovedZoneServers[i].u8ZoneId != 0xFF))
            {
                vCheckForMenuIndex();
                vGetZoneIDString(acAceString,sDiscovedZoneServers[i].u8ZoneId);
                strcpy(acACELCDString[u8AceMenuIndex],acAceString);
                strcat(acACELCDString[u8AceMenuIndex],"   ");
                vGetZoneTypeString(acAceString,sDiscovedZoneServers[i].u16ZoneType);
                strcat(acACELCDString[u8AceMenuIndex++],acAceString);
                vCheckForMenuIndex();
                vGetZoneLabelString(acAceString,sDiscovedZoneServers[i].u16ZoneType);
                strcpy(acACELCDString[u8AceMenuIndex++],acAceString);
                vCheckForMenuIndex();
                vNumberToString(acAceString,sDiscovedZoneServers[i].u64IeeeAddrOfServer,E_ZCL_UINT64);
                strcpy(acACELCDString[u8AceMenuIndex++],acAceString);
            }
        }
    }

    vCheckForCursorPosition();
}

/****************************************************************************
 *
 * NAME: vACEBypassedListDisplay
 *
 * DESCRIPTION:
 * Updated the ACE mode string for Bypassed list display.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vACEBypassedListDisplay(void)
{
    int i = 0;
    char acAceString[MAXIMUM_LENGTH_OF_STRING];

    bAceMessage = TRUE;

    vCheckForMenuIndex();
    strcpy(acACELCDString[u8AceMenuIndex++],"Bypassed Zones");

    vCheckForMenuIndex();
    memset(acACELCDString[u8AceMenuIndex],0,22);
    for(i=0; i < u8Discovered; i++)
    {
        if(sDiscovedZoneServers[i].bValid == TRUE && sDiscovedZoneServers[i].u8ZoneId != 0xFF )
        {
            if(ZCL_IS_BIT_SET(uint8,sDiscovedZoneServers[i].u8ArmBypass,CLD_IASACE_ZONE_STATUS_FLAG_BYPASS)){
                vGetZoneIDString(acAceString,sDiscovedZoneServers[i].u8ZoneId);
                strcat(acAceString," ");
                strcat(acACELCDString[u8AceMenuIndex],acAceString);
            }
        }
    }

    u8AceMenuIndex++;

    vCheckForCursorPosition();
}

/****************************************************************************
 *
 * NAME: vACEGetPanelStatusDisplay
 *
 * DESCRIPTION:
 * Updated the ACE mode string for Panel Status display.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vACEGetPanelStatusDisplay(void)
{
    char acAceString[MAXIMUM_LENGTH_OF_STRING];
    uint8 u8ParameterValue = 0;

    bAceMessage = TRUE;

    vCheckForMenuIndex();
    eCLD_IASACEGetPanelParameter(CIE_EP,E_CLD_IASACE_PANEL_PARAMETER_PANEL_STATUS,&u8ParameterValue);
    vGetPanelStatusString(acAceString,u8ParameterValue);
    strcpy(acACELCDString[u8AceMenuIndex], "Panel: ");
    strcat(acACELCDString[u8AceMenuIndex++],acAceString);
    vCheckForMenuIndex();
    eCLD_IASACEGetPanelParameter(CIE_EP,E_CLD_IASACE_PANEL_PARAMETER_ALARM_STATUS,&u8ParameterValue);
    vGetAlarmStatusString(acAceString,u8ParameterValue);
    strcpy(acACELCDString[u8AceMenuIndex],"Alarm: ");
    strcat(acACELCDString[u8AceMenuIndex++],acAceString);
    vCheckForMenuIndex();
    eCLD_IASACEGetPanelParameter(CIE_EP,E_CLD_IASACE_PANEL_PARAMETER_AUDIBLE_NOTIFICATION,&u8ParameterValue);
    vGetSoundString(acAceString,u8ParameterValue);
    strcpy(acACELCDString[u8AceMenuIndex],"Sound: ");
    strcat(acACELCDString[u8AceMenuIndex++],acAceString);
    vCheckForMenuIndex();
    eCLD_IASACEGetPanelParameter(CIE_EP,E_CLD_IASACE_PANEL_PARAMETER_SECONDS_REMAINING,&u8ParameterValue);
    vNumberToString(acAceString,(uint64)u8ParameterValue,E_ZCL_UINT8);
    strcpy(acACELCDString[u8AceMenuIndex],"Seconds Left: ");
    strcat(acACELCDString[u8AceMenuIndex++],acAceString);

    vCheckForCursorPosition();
}

/****************************************************************************
 *
 * NAME: vCheckForCursorPosition
 *
 * DESCRIPTION:
 * Checks for the cursor position and menu index for ACE mode display.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vCheckForCursorPosition(void)
{
    if(eLCDState == E_LCD_ACE_MENU)
    {
        if(u8AceMenuIndex > MAXIMUM_NUMBER_OF_ROWS_PER_PANEL)
        {
            gu8CursorPosition = u8AceMenuIndex - MAXIMUM_NUMBER_OF_ROWS_PER_PANEL;
            u8MenuIndex = u8AceMenuIndex - MAXIMUM_NUMBER_OF_ROWS_PER_PANEL;
        }else
        {
            vSetIntialDisplayPostition();
        }
    }
}

/****************************************************************************
 *
 * NAME: vHandleButtonPress
 *
 * DESCRIPTION:
 * Checks for the button pressed & if required starts delay timer.
 *
 * PARAMETERS:      Name              Usage
 *                  eType             Button event type
 *                  u8Button          Button pressed
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vHandleButtonPress(APP_teEventType eType,uint8 u8Button)
{
    if(eType == APP_E_EVENT_BUTTON_DOWN)
    {
        OS_eStopSWTimer(APP_DisplayModeSwitchTmr);
        if(u8Button != APP_E_BUTTONS_BUTTON_SW4)
            vChangeLCDState(u8Button);
        else
            OS_eStartSWTimer(APP_DisplayModeSwitchTmr, (APP_TIME_MS(2000)), NULL );
    }else if(eType == APP_E_EVENT_BUTTON_UP &&
            u8Button == APP_E_BUTTONS_BUTTON_SW4)
    {
        if(OS_eGetSWTimerStatus(APP_DisplayModeSwitchTmr) == OS_E_SWTIMER_RUNNING )
        {
            OS_eStopSWTimer(APP_DisplayModeSwitchTmr);
            vChangeLCDState(APP_E_BUTTONS_BUTTON_SW4);
        }
    }
}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vCheckForMenuIndex
 *
 * DESCRIPTION:
 * Checks for the menu index for ACE mode display for circular buffer implementation.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vCheckForMenuIndex(void)
{
    int i;
    if(u8AceMenuIndex > (MAXIMUM_NUMBER_OF_ROWS - 1))
    {
        for(i = 0; i < (MAXIMUM_NUMBER_OF_ROWS - 1) ;i++)
        {
            strcpy(acACELCDString[i],acACELCDString[i + 1]);
        }
        u8AceMenuIndex--;
    }
}

/****************************************************************************
 *
 * NAME: vSetIntialDisplayPostition
 *
 * DESCRIPTION:
 * Set the cursor position and device index.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vSetIntialDisplayPostition(void)
{
    u8MenuIndex = MINIMUM_INDEX_OF_SCREEN;
    gu8CursorPosition = 0;
    u8DeviceIndex = 0;
}

/****************************************************************************
 *
 * NAME: vMoveScreenUp
 *
 * DESCRIPTION:
 * Move the cursor position and screen up.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vMoveScreenUp(uint8 u8MinimumIndex)
{
    /* keep moving up till the first line */
    if(u8MenuIndex > u8MinimumIndex)
        u8MenuIndex--;

    /* keep switching between panels till the first panel */
    if(gu8CursorPosition > 0)
        gu8CursorPosition--;
}

/****************************************************************************
 *
 * NAME: vMoveScreenAndDeviceUp
 *
 * DESCRIPTION:
 * Move the cursor position , screen , and device index for selection up.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vMoveScreenAndDeviceUp(void)
{
    /* keep moving up till the first line */
    if(u8DeviceIndex > 0){
        u8DeviceIndex--;
        u8MenuIndex--;
    }
    /* keep switching between panels till the last panel */
    if(gu8CursorPosition > 0)
        gu8CursorPosition--;
}

/****************************************************************************
 *
 * NAME: vMoveScreenDown
 *
 * DESCRIPTION:
 * Move the cursor position , and screen down.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vMoveScreenDown(uint8 u8MaximumIndex)
{
    /* keep moving down till the last line */
    if(u8MenuIndex < u8MaximumIndex)
        u8MenuIndex++;

    /* keep switching between panels till the last panel */
    if(u8MenuIndex > ((MAXIMUM_NUMBER_OF_ROWS_PER_PANEL - 1)+gu8CursorPosition))
        gu8CursorPosition++;
}

/****************************************************************************
 *
 * NAME: vMoveScreenAndDeviceDown
 *
 * DESCRIPTION:
 * Move the cursor position , screen , and device index for selection down.
 *
 * PARAMETERS:      Name              Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vMoveScreenAndDeviceDown(void)
{
    /* Moves down till the first line */
    if(u8DeviceIndex < (u8Discovered - 1) && u8MenuIndex < (MAXIMUM_NUMBER_OF_ROWS - 1)){
        u8DeviceIndex++;
        u8MenuIndex++;
    }

    /* keep switching between panels till the last panel */
    if(u8MenuIndex > (MAXIMUM_NUMBER_OF_ROWS_PER_PANEL - 1)+gu8CursorPosition)
        gu8CursorPosition++;
}

/****************************************************************************
 *
 * NAME: APP_SwitchDisplayMode
 *
 * DESCRIPTION:
 * Switches between different display modes
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
OS_TASK(APP_DisplayModeSwitch)
{
    OS_eStopSWTimer(APP_DisplayModeSwitchTmr);
    if(eLCDState != E_LCD_ACE_MENU)
    {
        ePreviousLCDState = eLCDState;
        eLCDState = E_LCD_ACE_MENU;
        vCheckForCursorPosition();
    }else
    {
        u8MenuIndex = MINIMUM_INDEX_OF_SCREEN;
        gu8CursorPosition = 0;
        eLCDState = ePreviousLCDState;
    }
    vLCDDisplay();
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
