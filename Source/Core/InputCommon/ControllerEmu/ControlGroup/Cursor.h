// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <string>
#include "InputCommon/ControllerEmu/ControlGroup/ControlGroup.h"
#include "InputCommon/ControllerInterface/Device.h"

namespace ControllerEmu
{
class Cursor : public ControlGroup
{
public:
enum
  {
    C_UP,
    C_DOWN,
    C_LEFT,
    C_RIGHT,
    C_FORWARD,
    C_BACKWARD,
    C_RANGE_MODIFIER,
    C_HIDE,
    C_SHOW,
  };
  enum
  {
    C_RANGE,
    C_CENTER,
    C_WIDTH,
    C_HEIGHT,
  };
  explicit Cursor(const std::string& name);

  void GetState(ControlState* x, ControlState* y, ControlState* z, bool adjusted = false, const bool relative = false, const bool step = false);

private:
  // This is used to reduce the cursor speed for relative input
  // to something that makes sense with the default range.
  static constexpr double SPEED_MULTIPLIER = 0.04;

  ControlState m_x = 0.0;
  ControlState m_y = 0.0;
  ControlState m_z = 0.0;
  std::array<ControlState, 3> m_state{}, m_absolute{}, m_last{};
  std::list<double> m_list[3];
  std::mutex m_mutex;
};
}  // namespace ControllerEmu
