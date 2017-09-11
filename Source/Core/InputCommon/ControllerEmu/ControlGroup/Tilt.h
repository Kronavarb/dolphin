// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <string>
#include "InputCommon/ControllerEmu/ControlGroup/ControlGroup.h"
#include "InputCommon/ControllerInterface/Device.h"

namespace ControllerEmu
{
class Tilt : public ControlGroup
{
public:
  enum
  {
    T_FORWARD,
    T_BACKWARD,
    T_LEFT,
    T_RIGHT,
    T_UP,
    T_DOWN,
  };
  enum
  {
    T_FAST = 6,
    T_ACC_RANGE,
    T_GYRO_RANGE_1,
    T_GYRO_RANGE_2,
  };
  enum
  {
    T_MODIFIER = 6,
  };
  enum
  {
    T_ACC_RANGE_S,
    T_GYRO_RANGE,
    T_GYRO_SETTLE,
    T_DEADZONE,
    T_CIRCLESTICK,
    T_ANGLE,
  };
  enum
  {
    T_RANGE,
    T_DEADZONE_N,
    T_CIRCLESTICK_N,
    T_ANGLE_N,
  };

  explicit Tilt(const std::string& name, bool gyro = false);

  bool HasGyro() { return m_has_gyro; }

  void GetState(ControlState* pitch, ControlState* roll, ControlState* yaw, bool gyro = false, bool step = true);

private:
  std::array<ControlState, 3> m_acc{}, m_gyro{};
  int m_settle[3];
  bool m_has_gyro;
};
}  // namespace ControllerEmu
