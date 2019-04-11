/*****************************************************************************
 *
 * MODULE:             JN-AN-1201
 *
 * COMPONENT:          app_CIE_display.h
 *
 * DESCRIPTION:        CIE display file header
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
 * Copyright NXP B.V. 2013. All rights reserved
 *
 ***************************************************************************/

#ifndef ZHA_CIE_DISPLAY_H_
#define ZHA_CIE_DISPLAY_H_

#include "app_buttons.h"
#include "app_events.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
    E_LCD_INTIAL_DISPLAY,
    E_LCD_NETWORK_FORMATION,
    E_LCD_NETWORK_FORMED,
    E_LCD_CIE_MAIN_MENU,
    E_LCD_ACE_MENU,
    E_LCD_DISPLAY_SECURITY_DEVICES,
    E_LCD_DISPLAY_CONFIG_PERMIT_ENROLLMENT_ZONES,
    E_LCD_DISPLAY_ARM_DISARM_THE_SYSTEM,
    E_LCD_DISPLAY_PANEL_STATUS,
    E_LCD_DISPLAY_CONFIG_BYPASSED_LIST,
    E_LCD_DISPLAY_CONFIG_ARM_GROUP_ZONES,
    E_LCD_DISPLAY_ZONE_INFORMATION
}teLCDState;
typedef enum
{
    E_CIE_MAIN_MENU_SECURITY_DEVICES = 1,
    E_CIE_MAIN_MENU_CONFIGURATION,
    E_CIE_MAIN_MENU_PERMIT_JOIN,
    E_CIE_MAIN_MENU_ARM_DISARM,
    E_CIE_MAIN_MENU_BYPASS_ZONES,
    E_CIE_MAIN_MENU_PANEL_STATUS
}teCIEMainMenuOption;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void vChangeLCDState(uint8 u8Key);
PUBLIC void vLCDDisplay(void);
PUBLIC void vHandleButtonPress(APP_teEventType eType,uint8 u8Button);
PUBLIC void vGetZoneTypeString(char *str, uint16 u16ZoneType);
PUBLIC void vGetActualZoneTypeString(char *str, uint16 u16ZoneType);
PUBLIC void vGetZoneIDString(char *str, uint8 u8ZoneId);
PUBLIC void vGetPanelStatusString(char *str,uint8 u8PanelStatus);
PUBLIC void vGetAlarmStatusString(char *str,uint8 u8AlarmStatus);
PUBLIC void vGetSoundString(char *str,uint8 u8Sound);
PUBLIC void vNumberToString(char * str, uint64 u64Num, teZCL_ZCLAttributeType eType);
PUBLIC void vDecToString(char * str, uint8 u8Num);
PUBLIC void vGetZoneConfigString(char *str,uint8 u8DeviceIndex);
PUBLIC void vGetZoneLabelString(char *str,uint16 u16ZoneType);
PUBLIC void vIEEEToString(char * str, uint64 u64IEEEAddr);
PUBLIC void vGetZoneStatusBitString(char *str,uint16 u16ZoneStatus);
PUBLIC void vACEPanelStatusDisplay(void);
PUBLIC void vACEBypassResultDisplay(uint8 u8NumberOfZones,
                                    uint8 *pu8ZoneID);
PUBLIC void vACEZoneStatusDisplay(bool bZoneStatusMaskFlag);
PUBLIC void vACEZoneInfoDisplay(uint8 u8ZoneID);
PUBLIC void vACEBypassedListDisplay(void);
PUBLIC void vACEGetPanelStatusDisplay(void);
PUBLIC void vCheckForCursorPosition(void);
/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
extern teLCDState eLCDState;
extern uint8 u8DeviceIndex;
extern uint8 u8MenuIndex;
extern uint8 gu8CursorPosition;
extern bool bAceMessage;
extern uint8 u8AceMenuIndex;
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#endif /*ZHA_CIE_DISPLAY_H_*/
