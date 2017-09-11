// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "InputCommon/ControllerEmu/ControlGroup/Force.h"

#include <cmath>
#include <memory>
#include <string>

#include "Common/Common.h"
#include "Common/CommonTypes.h"
#include "InputCommon/ControlReference/ControlReference.h"
#include "InputCommon/ControllerEmu/Control/Input.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerEmu/Setting/NumericSetting.h"

namespace ControllerEmu
{
Force::Force(const std::string& name_) : ControlGroup(name_, GroupType::Force)
{
  controls.emplace_back(std::make_unique<Input>(_trans("Up")));
  controls.emplace_back(std::make_unique<Input>(_trans("Down")));
  controls.emplace_back(std::make_unique<Input>(_trans("Left")));
  controls.emplace_back(std::make_unique<Input>(_trans("Right")));
  controls.emplace_back(std::make_unique<Input>(_trans("Forward")));
  controls.emplace_back(std::make_unique<Input>(_trans("Backward")));

  numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Range"), 1.0, 0, 500));
  numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Dead Zone"), 0, 0, 50));
}

void Force::GetState(ControlState* axis, bool step)
{
  const ControlState deadzone = numeric_settings[0]->GetValue();

  for (unsigned int i = 0; i < 3; i++)
  {
    double dz = 0;
    const double state = controls[i * 2 + 1]->control_ref->State() - controls[i * 2]->control_ref->State();
    if (fabs(state) > deadzone)
      dz = ((state - (deadzone * sign(state))) / (1 - deadzone));

    if (step)
    {
      if (state > m_swing[i])
        m_swing[i] = std::min(m_swing[i] + 0.1, state);
      else if (state < m_swing[i])
        m_swing[i] = std::max(m_swing[i] - 0.1, state);
    }

    *axis++ = (ControlState)((abs(m_swing[i]) >= 0.7
      ? -2 * sign(state) + m_swing[i] * 2
      : m_swing[i]) * sign(state));
  }
}
}  // namespace ControllerEmu
