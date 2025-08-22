/**
 * @file
 * @brief Common error code definitions.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-07-02
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_ERR_ERRCODE_H_
#define NTURT_ERR_ERRCODE_H_

/**
 * @addtogroup err_code Error Codes
 * @brief Common error code definitions.
 *
 * @ingroup err
 * @{
 */

enum err_code {
  ERR_CODE_STEER,

  ERR_CODE_ACCEL,
  ERR_CODE_APPS1,
  ERR_CODE_APPS2,

  ERR_CODE_BRAKE,
  ERR_CODE_BSE1,
  ERR_CODE_BSE2,

  ERR_CODE_PEDAL_PLAUS,

  ERR_CODE_INV_FL,
  ERR_CODE_INV_FR,
  ERR_CODE_INV_RL,
  ERR_CODE_INV_RR,

  ERR_CODE_INV_FL_HV_LOW,
  ERR_CODE_INV_FR_HV_LOW,
  ERR_CODE_INV_RL_HV_LOW,
  ERR_CODE_INV_RR_HV_LOW,

  ERR_CODE_EMCY_STOP,

  ERR_CODE_CANBUS,
  ERR_CODE_HB_VCU,
  ERR_CODE_HB_SENSORS,
  ERR_CODE_HB_RPI,
  ERR_CODE_HB_IMU,
  ERR_CODE_HB_GPS,
  ERR_CODE_HB_ACC,
  ERR_CODE_HB_INV_FL,
  ERR_CODE_HB_INV_FR,
  ERR_CODE_HB_INV_RL,
  ERR_CODE_HB_INV_RR,
};

/**
 * @} // err_code
 */

#endif  // NTURT_ERR_ERRCODE_H_
