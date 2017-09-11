// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "InputCommon/ControllerEmu/ControlGroup/Tilt.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

#include "Common/Common.h"
#include "InputCommon/ControlReference/ControlReference.h"
#include "InputCommon/ControllerEmu/Control/Control.h"
#include "InputCommon/ControllerEmu/Control/Input.h"
#include "InputCommon/ControllerEmu/Setting/NumericSetting.h"

namespace ControllerEmu
{
Tilt::Tilt(const std::string& name_, bool gyro) : m_has_gyro(gyro), ControlGroup(name_, GroupType::Tilt)
{
  controls.emplace_back(std::make_unique<Input>(_trans("Forward")));
  controls.emplace_back(std::make_unique<Input>(_trans("Backward")));
  controls.emplace_back(std::make_unique<Input>(_trans("Left")));
  controls.emplace_back(std::make_unique<Input>(_trans("Right")));
  controls.emplace_back(std::make_unique<Input>(_trans("Yaw Left")));
  controls.emplace_back(std::make_unique<Input>(_trans("Yaw Right")));

   //controls.emplace_back(std::make_unique<Input>(_trans("Modifier")));
  if (gyro)
  {
    controls.emplace_back(std::make_unique<Input>(_trans("Fast")));
    controls.emplace_back(std::make_unique<Input>(_trans("Acc Range")));
    controls.emplace_back(std::make_unique<Input>(_trans("Gyro Range 1")));
    controls.emplace_back(std::make_unique<Input>(_trans("Gyro Range 2")));
    numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Acc Range"), 1.0, 0, 500));
    numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Gyro Range"), 1.0, 0, 500));
    numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Gyro Settle"), 0.25, 0, 9999));
  }
  else
  {
    controls.emplace_back(std::make_unique<Input>(_trans("Range")));
    numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Range"), 1.0, 0, 500));
  }

  numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Dead Zone"), 0, 0, 50));
  numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Circle Stick"), 0));
  numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Angle"), 0.9, 0, 180));
}

void Tilt::GetState(ControlState* pitch, ControlState* roll, ControlState* yaw, bool gyro, bool step)
{
  ControlState state;
  ControlState r_acc = controls[m_has_gyro ? T_ACC_RANGE : T_RANGE]->control_ref->State();
  auto const r_acc_s = numeric_settings[m_has_gyro ? T_ACC_RANGE_S : T_RANGE]->GetValue();
  auto const deadzone = numeric_settings[m_has_gyro ? T_DEADZONE : T_DEADZONE_N]->GetValue();
  auto const circle = numeric_settings[m_has_gyro ? T_CIRCLESTICK : T_CIRCLESTICK_N]->GetValue();
  auto const angle = numeric_settings[m_has_gyro ? T_ANGLE : T_ANGLE_N]->GetValue() / 1.8;

  ControlState settle = 0
    , r_gyro_1 = 1.0
    , r_gyro_2 = 1.0
    , r_gyro_s = 1.0;
  if (m_has_gyro)
  {
    r_gyro_1 = controls[T_GYRO_RANGE_1]->control_ref->State();
    r_gyro_2 = controls[T_GYRO_RANGE_2]->control_ref->State();
    r_gyro_s = numeric_settings[T_GYRO_RANGE]->GetValue();
    settle = numeric_settings[T_GYRO_SETTLE]->GetValue();
  }

  for (unsigned int i = 0; i < 3; i++)
  {
    state = controls[i * 2 + 1]->control_ref->State() - controls[i * 2]->control_ref->State();

    // deadzone
    state *= fabs(state)>deadzone
      ? (fabs(state) - deadzone) / (1.0 - deadzone)
      : 0.0;

    if (step)
    {
      if (state == m_acc[i])
        m_settle[i]++;
      else
        m_settle[i] = 0;

      // step towards state
      if (state > m_acc[i])
        m_acc[i] = std::min(m_acc[i] + 0.1, state);
      else if (state < m_acc[i])
        m_acc[i] = std::max(m_acc[i] - 0.1, state);

      // step gyro towards 0
      if (m_settle[i] > settle * 100.0)
      {
        if (0 > m_gyro[i])
          m_gyro[i] = std::min(m_gyro[i] + 0.05, 0.0);
        else if (0 < m_gyro[i])
          m_gyro[i] = std::max(m_gyro[i] - 0.05, 0.0);
      }

      // step gyro towards state
      else
      {
        if (m_acc[i] > m_gyro[i])
          m_gyro[i] = std::min(m_gyro[i] + 0.05, m_acc[i]);
        else if (m_acc[i] < m_gyro[i])
          m_gyro[i] = std::max(m_gyro[i] - 0.05, m_acc[i]);
      }
    }
  }

  ControlState y = m_acc[0];
  ControlState x = m_acc[1];

  // circle stick
  if (circle)
  {
    ControlState ang = atan2(y, x);
    ControlState ang_sin = sin(ang);
    ControlState ang_cos = cos(ang);
    ControlState rad = sqrt(x*x + y*y);

  // the amt a full square stick would have at current angle
    ControlState square_full = std::min(ang_sin ? 1 / fabs(ang_sin) : 2, ang_cos ? 1 / fabs(ang_cos) : 2);

  // the amt a full stick would have that was (user setting circular) at current angle
  // I think this is more like a pointed circle rather than a rounded square like it should be
  ControlState stick_full = (square_full * (1 - circle)) + (circle);

  // dead zone
  rad = std::max(0.0, rad - deadzone * stick_full);
  rad /= (1.0 - deadzone);

  // circle stick
  ControlState amt = rad / stick_full;
  rad += (square_full - 1) * amt * circle;

  y = ang_sin * rad;
  x = ang_cos * rad;
  }

  m_acc[0] = y;
  m_acc[1] = x;

  if (gyro)
  {
    if (!r_gyro_1) r_gyro_1 = 1.0;
    if (!r_gyro_2) r_gyro_2 = 1.0;
    *pitch = m_gyro[0] * angle * r_gyro_1 * r_gyro_2 * r_gyro_s;
    *roll = m_gyro[1] * angle * r_gyro_1 * r_gyro_2 * r_gyro_s;
    *yaw = m_gyro[2] * angle * r_gyro_1 * r_gyro_2 * r_gyro_s;
  }
  else
  {
    if (!r_acc) r_acc = 1.0;
    *pitch = m_acc[0] * angle * r_acc * r_acc_s;
    *roll = m_acc[1] * angle * r_acc * r_acc_s;
    *yaw = m_acc[2] * angle * r_acc * r_acc_s;
  }
}
}  // namespace ControllerEmu
