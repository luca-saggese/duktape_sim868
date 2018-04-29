/********************************************************************
 *                Copyright Simcom(shanghai)co. Ltd.                               *
 *---------------------------------------------------------------------
 * FileName      :   eat_soft_sim.h
 * version       :   0.10
 * Description   :   
 * Authors       :   Maobin  
 * Notes         :
 *---------------------------------------------------------------------
 *
 *    HISTORY OF CHANGES
 *---------------------------------------------------------------------
 *0.10  2013-01-11, maobin, create originally.
 *
 ********************************************************************/
#ifndef _EAT_SOFT_SIM_H_
#define _EAT_SOFT_SIM_H_
/********************************************************************
 * Include Files
 ********************************************************************/
#include "eat_type.h"
/********************************************************************
 * Macros
 ********************************************************************/

/********************************************************************
 * Types
 ********************************************************************/
typedef enum{
    EAT_SOFTSIM_UNKNOWN_POWER_CLASS = 0,
    EAT_SOFTSIM_CLASS_A_50V = 1,
    EAT_SOFTSIM_CLASS_B_30V = 2,
    EAT_SOFTSIM_CLASS_AB    = 3,
    EAT_SOFTSIM_CLASS_C_18V = 4,
    EAT_SOFTSIM_ClASS_BC        = 6,
    EAT_SOFTSIM_CLASS_ABC   = 7
}EAT_SOFTSIM_POWER;


typedef struct
{
    u8  txData[261];
    u32  txSize;
}EatAPDU_Tx_Data_st;



typedef struct SIMCARDAPDU_DATATag
{
	u16  v_len;
	u8   a_RData[258];
	u8   v_unused1;
	u8   v_unused2;
	u8   v_unused3;
	u8   v_unused4;
}SIMCARDAPDU_DATA;

typedef struct SIMCARDRESET_CNFTag
{
    u8	v_VoltageValue;
    u8	a_AnswerToReset[33];
    u8	unused1;
    u8	unused2;
}SIMCARDRESET_CNF;

typedef struct
{
    EAT_SOFTSIM_POWER  voltage;
}SIMCARDRESET_REQ;

/*****************************************************************************
 * Function :eat_SetModemAPDUToFL
 * Description: Control the direction of the APDU data from the core.
 * Parameters :
 *  	destination  : [IN] TRUE: Send the APDU data from core to embedded application.
 *                             FALSE: It is directed to the SIM card.                        
 * Returns:
 * 	Nothing	
* NOTE
*     Nothing
 *****************************************************************************/
extern void (*const eat_SetModemAPDUToFL)(eat_bool destination);
/*****************************************************************************
 * Function :eat_SoftSendAPDUCnfToModem
 * Description:  Send the APDU data which is received from the soft SIM card back to the core.
 * Parameters :
 *  	rx_data  : [IN] The APDU data which is received from the soft SIM card. 
 * Returns:
 * 	Nothing	
* NOTE
*     Nothing
 *****************************************************************************/
extern void (*const eat_SoftSendAPDUCnfToModem)(SIMCARDAPDU_DATA rx_data);
/*****************************************************************************
 * Function :eat_SendSIMCardResetCnfToModem
 * Description:  Send the reset confirmation to the Modem.
 * Parameters :
 *  	resetCnf  : [IN] The reset confirmation . 
 * Returns:
 * 	Nothing	
* NOTE
*     Nothing
 *****************************************************************************/
extern void (*const eat_SendSIMCardResetCnfToModem)(SIMCARDRESET_CNF resetCnf);

/*****************************************************************************
 * Function :eat_SoftSIM_GetCodeFingerprint
 * Description:  SoftSim Code check sum.
 * Parameters :
 *  	void
 * Returns:
 * 	u64 	the check sum value
* NOTE
*     Nothing
 *****************************************************************************/
extern u64 (*const eat_SoftSIM_GetCodeFingerprint)(void);
#endif

