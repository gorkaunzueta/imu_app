/*******************************************************************************
**
**      GSC-18128-1, "Core Flight Executive Version 6.7"
**
**      Copyright (c) 2006-2019 United States Government as represented by
**      the Administrator of the National Aeronautics and Space Administration.
**      All Rights Reserved.
**
**      Licensed under the Apache License, Version 2.0 (the "License");
**      you may not use this file except in compliance with the License.
**      You may obtain a copy of the License at
**
**        http://www.apache.org/licenses/LICENSE-2.0
**
**      Unless required by applicable law or agreed to in writing, software
**      distributed under the License is distributed on an "AS IS" BASIS,
**      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**      See the License for the specific language governing permissions and
**      limitations under the License.
**
** File: imu_app.c
**
** Purpose:
**   This file contains the source code for the IMU App.
**
*******************************************************************************/

/*
** Include Files:
*/
#include "imu_app_events.h"
#include "imu_app_version.h"
#include "imu_app.h"
#include "imu_app_table.h"



/*
** global data
*/
IMU_APP_Data_t IMU_APP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/* IMU_APP_Main() -- Application entry point and main process loop         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void IMU_APP_Main(void)
{
    int32            status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(IMU_APP_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = IMU_APP_Init();
    if (status != CFE_SUCCESS)
    {
        IMU_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** IMU Runloop
    */
    while (CFE_ES_RunLoop(&IMU_APP_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(IMU_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, IMU_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(IMU_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            IMU_APP_ProcessCommandPacket(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(IMU_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "IMU APP: SB Pipe Read Error, App Will Exit");

            IMU_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(IMU_APP_PERF_ID);

    CFE_ES_ExitApp(IMU_APP_Data.RunStatus);

} /* End of IMU_APP_Main() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* IMU_APP_Init() --  initialization                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 IMU_APP_Init(void)
{
    int32 status;

    IMU_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app command execution counters
    */
    IMU_APP_Data.CmdCounter = 0;
    IMU_APP_Data.ErrCounter = 0;

    /*
    ** Initialize app configuration data
    */
    IMU_APP_Data.PipeDepth = IMU_APP_PIPE_DEPTH;

    strncpy(IMU_APP_Data.PipeName, "IMU_APP_CMD_PIPE", sizeof(IMU_APP_Data.PipeName));
    IMU_APP_Data.PipeName[sizeof(IMU_APP_Data.PipeName) - 1] = 0;

    /*
    ** Initialize event filter table...
    */
    IMU_APP_Data.EventFilters[0].EventID = IMU_APP_STARTUP_INF_EID;
    IMU_APP_Data.EventFilters[0].Mask    = 0x0000;
    IMU_APP_Data.EventFilters[1].EventID = IMU_APP_COMMAND_ERR_EID;
    IMU_APP_Data.EventFilters[1].Mask    = 0x0000;
    IMU_APP_Data.EventFilters[2].EventID = IMU_APP_COMMANDNOP_INF_EID;
    IMU_APP_Data.EventFilters[2].Mask    = 0x0000;
    IMU_APP_Data.EventFilters[3].EventID = IMU_APP_COMMANDRST_INF_EID;
    IMU_APP_Data.EventFilters[3].Mask    = 0x0000;
    IMU_APP_Data.EventFilters[4].EventID = IMU_APP_INVALID_MSGID_ERR_EID;
    IMU_APP_Data.EventFilters[4].Mask    = 0x0000;
    IMU_APP_Data.EventFilters[5].EventID = IMU_APP_LEN_ERR_EID;
    IMU_APP_Data.EventFilters[5].Mask    = 0x0000;
    IMU_APP_Data.EventFilters[6].EventID = IMU_APP_PIPE_ERR_EID;
    IMU_APP_Data.EventFilters[6].Mask    = 0x0000;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(IMU_APP_Data.EventFilters, IMU_APP_EVENT_COUNTS, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("IMU App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_MSG_Init(&IMU_APP_Data.HkTlm.TlmHeader.Msg, IMU_APP_HK_TLM_MID, sizeof(IMU_APP_Data.HkTlm));

    /*
    ** Create Software Bus message pipe.
    */
    status = CFE_SB_CreatePipe(&IMU_APP_Data.CommandPipe, IMU_APP_Data.PipeDepth, IMU_APP_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("IMU App: Error creating pipe, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    status = CFE_SB_Subscribe(IMU_APP_SEND_HK_MID, IMU_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("IMU App: Error Subscribing to HK request, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to ground command packets
    */
    status = CFE_SB_Subscribe(IMU_APP_CMD_MID, IMU_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("IMU App: Error Subscribing to Command, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }

    /*
    ** Register Table(s)
    */
    status = CFE_TBL_Register(&IMU_APP_Data.TblHandles[0], "IMUAppTable", sizeof(IMU_APP_Table_t),
                              CFE_TBL_OPT_DEFAULT, IMU_APP_TblValidationFunc);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("IMU App: Error Registering Table, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }
    else
    {
        status = CFE_TBL_Load(IMU_APP_Data.TblHandles[0], CFE_TBL_SRC_FILE, IMU_APP_TABLE_FILE);
    }

    CFE_EVS_SendEvent(IMU_APP_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "IMU App Initialized.%s",
                      IMU_APP_VERSION_STRING);

    return (CFE_SUCCESS);

} /* End of IMU_APP_Init() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  IMU_APP_ProcessCommandPacket                                       */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the IMU       */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void IMU_APP_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (MsgId)
    {
        case IMU_APP_CMD_MID:
            IMU_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case IMU_APP_SEND_HK_MID:
            IMU_APP_ReportHousekeeping((CFE_MSG_CommandHeader_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(IMU_APP_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "IMU: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }

    return;

} /* End IMU_APP_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* IMU_APP_ProcessGroundCommand() -- IMU ground commands                      */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void IMU_APP_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process "known" IMU app ground commands
    */
    switch (CommandCode)
    {
        case IMU_APP_NOOP_CC:
            if (IMU_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(IMU_APP_NoopCmd_t)))
            {
                IMU_APP_Noop((IMU_APP_NoopCmd_t *)SBBufPtr);
            }

            break;

        case IMU_APP_RESET_COUNTERS_CC:
            if (IMU_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(IMU_APP_ResetCountersCmd_t)))
            {
                IMU_APP_ResetCounters((IMU_APP_ResetCountersCmd_t *)SBBufPtr);
            }

            break;

        case IMU_APP_PROCESS_CC:
            if (IMU_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(IMU_APP_ProcessCmd_t)))
            {
                IMU_APP_Process((IMU_APP_ProcessCmd_t *)SBBufPtr);
            }

            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(IMU_APP_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid ground command code: CC = %d", CommandCode);
            break;
    }

    return;

} /* End of IMU_APP_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  IMU_APP_ReportHousekeeping                                         */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 IMU_APP_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    IMU_APP_Data.HkTlm.Payload.CommandErrorCounter = IMU_APP_Data.ErrCounter;
    IMU_APP_Data.HkTlm.Payload.CommandCounter      = IMU_APP_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(&IMU_APP_Data.HkTlm.TlmHeader.Msg);
    CFE_SB_TransmitMsg(&IMU_APP_Data.HkTlm.TlmHeader.Msg, true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < IMU_APP_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(IMU_APP_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;

} /* End of IMU_APP_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* IMU_APP_Noop -- IMU NOOP commands                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 IMU_APP_Noop(const IMU_APP_NoopCmd_t *Msg)
{

    IMU_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(IMU_APP_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION, "IMU: NOOP command %s",
                      IMU_APP_VERSION);

    return CFE_SUCCESS;

} /* End of IMU_APP_Noop */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  IMU_APP_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 IMU_APP_ResetCounters(const IMU_APP_ResetCountersCmd_t *Msg)
{

    IMU_APP_Data.CmdCounter = 0;
    IMU_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(IMU_APP_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "IMU: RESET command");

    return CFE_SUCCESS;

} /* End of IMU_APP_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  IMU_APP_Process                                                     */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 IMU_APP_Process(const IMU_APP_ProcessCmd_t *Msg)
{
    int32               status;
    IMU_APP_Table_t *TblPtr;
    const char *        TableName = "IMU_APP.IMUAppTable";

    /* IMU Use of Table */

    status = CFE_TBL_GetAddress((void *)&TblPtr, IMU_APP_Data.TblHandles[0]);

    if (status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("IMU App: Fail to get table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    CFE_ES_WriteToSysLog("IMU App: Table Value 1: %d  Value 2: %d", TblPtr->Int1, TblPtr->Int2);

    IMU_APP_GetCrc(TableName);

    status = CFE_TBL_ReleaseAddress(IMU_APP_Data.TblHandles[0]);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("IMU App: Fail to release table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;

} /* End of IMU_APP_ProcessCC */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* IMU_APP_VerifyCmdLength() -- Verify command packet length                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool IMU_APP_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(IMU_APP_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        IMU_APP_Data.ErrCounter++;
    }

    return (result);

} /* End of IMU_APP_VerifyCmdLength() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* IMU_APP_TblValidationFunc -- Verify contents of First Table      */
/* buffer contents                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 IMU_APP_TblValidationFunc(void *TblData)
{
    int32               ReturnCode = CFE_SUCCESS;
    IMU_APP_Table_t *TblDataPtr = (IMU_APP_Table_t *)TblData;

    /*
    ** IMU Table Validation
    */
    if (TblDataPtr->Int1 > IMU_APP_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = IMU_APP_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;

} /* End of IMU_APP_TBLValidationFunc() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* IMU_APP_GetCrc -- Output CRC                                     */
/*                                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void IMU_APP_GetCrc(const char *TableName)
{
    int32          status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("IMU App: Error Getting Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("IMU App: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }

    return;

} /* End of IMU_APP_GetCrc */
