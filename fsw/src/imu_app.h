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
*******************************************************************************/

/**
 * @file
 *
 * Main header file for the IMU application
 */

#ifndef IMU_APP_H
#define IMU_APP_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"

#include "mpu9dof_lib.h"

#include "imu_app_perfids.h"
#include "imu_app_msgids.h"
#include "imu_app_msg.h"

/***********************************************************************/
#define IMU_APP_PIPE_DEPTH 32 /* Depth of the Command Pipe for Application */

#define IMU_APP_NUMBER_OF_TABLES 1 /* Number of Table(s) */

/* Define filenames of default data images for tables */
#define IMU_APP_TABLE_FILE "/cf/imu_app_tbl.tbl"

#define IMU_APP_TABLE_OUT_OF_RANGE_ERR_CODE -1

#define IMU_APP_TBL_ELEMENT_1_MAX 10
/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct
{
    /*
    ** Command interface counters...
    */
    uint8 CmdCounter;
    uint8 ErrCounter;
    
    mpu9dof_t mpu9dof;
    int16_t Accel_x;
    int16_t Accel_y;
    int16_t Accel_z;
    
    /*
    ** Housekeeping telemetry packet...
    */
    IMU_APP_HkTlm_t HkTlm;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;

    /*
    ** Initialization data (not reported in housekeeping)...
    */
    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

    CFE_EVS_BinFilter_t EventFilters[IMU_APP_EVENT_COUNTS];
    CFE_TBL_Handle_t    TblHandles[IMU_APP_NUMBER_OF_TABLES];

} IMU_APP_Data_t;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (IMU_APP_Main), these
**       functions are not called from any other source module.
*/
void  IMU_APP_Main(void);
int32 IMU_APP_Init(void);
void  IMU_APP_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr);
void  IMU_APP_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr);
int32 IMU_APP_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg);
int32 IMU_APP_ResetCounters(const IMU_APP_ResetCountersCmd_t *Msg);
int32 IMU_APP_Process(const IMU_APP_ProcessCmd_t *Msg);
int32 IMU_APP_Noop(const IMU_APP_NoopCmd_t *Msg);
void  IMU_APP_GetCrc(const char *TableName);

int32 IMU_APP_TblValidationFunc(void *TblData);

bool IMU_APP_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

#endif /* IMU_APP_H */
