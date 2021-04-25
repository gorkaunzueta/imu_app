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
 * Define IMU App  Messages and info
 */

#ifndef IMU_APP_MSG_H
#define IMU_APP_MSG_H

/*
** IMU App command codes
*/
#define IMU_APP_NOOP_CC           0
#define IMU_APP_RESET_COUNTERS_CC 1
#define IMU_APP_PROCESS_CC        2

/*************************************************************************/

/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */
} IMU_APP_NoArgsCmd_t;

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
typedef IMU_APP_NoArgsCmd_t IMU_APP_NoopCmd_t;
typedef IMU_APP_NoArgsCmd_t IMU_APP_ResetCountersCmd_t;
typedef IMU_APP_NoArgsCmd_t IMU_APP_ProcessCmd_t;

/*************************************************************************/
/*
** Type definition (IMU App housekeeping)
*/

typedef struct
{
    uint8   CommandErrorCounter;
    uint8   CommandCounter;
    int16_t Accel_x;
    int16_t Accel_y;
    int16_t Accel_z;
    uint8   spare[2];
} IMU_APP_HkTlm_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t  TlmHeader; /**< \brief Telemetry header */
    IMU_APP_HkTlm_Payload_t Payload;   /**< \brief Telemetry payload */
} IMU_APP_HkTlm_t;

#endif /* IMU_APP_MSG_H */
