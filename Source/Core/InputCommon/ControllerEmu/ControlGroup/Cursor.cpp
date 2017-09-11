// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "InputCommon/ControllerEmu/ControlGroup/Cursor.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

#include "Common/Common.h"
#include "Common/MathUtil.h"

#include "InputCommon/ControlReference/ControlReference.h"
#include "InputCommon/ControllerEmu/Control/Control.h"
#include "InputCommon/ControllerEmu/Control/Input.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerEmu/Setting/BooleanSetting.h"
#include "InputCommon/ControllerEmu/Setting/NumericSetting.h"

namespace ControllerEmu
{
Cursor::Cursor(const std::string& name_) : ControlGroup(name_, GroupType::Cursor)
{
  for (auto& named_direction : named_directions)
    controls.emplace_back(std::make_unique<Input>(named_direction));

  controls.emplace_back(std::make_unique<Input>(_trans("Forward")));
  controls.emplace_back(std::make_unique<Input>(_trans("Backward")));
  controls.emplace_back(std::make_unique<Input>("Range"));
  controls.emplace_back(std::make_unique<Input>(_trans("Hide")));
  controls.emplace_back(std::make_unique<Input>("Show"));
  controls.emplace_back(std::make_unique<Input>(_trans("Recenter")));

  numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Range"), 1.0, 0, 500));
  numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Center"), 0.5));
  numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Width"), 0.5));
  numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Height"), 0.5));
  numeric_settings.emplace_back(std::make_unique<NumericSetting>(_trans("Dead Zone"), 0, 0, 20));
  boolean_settings.emplace_back(std::make_unique<BooleanSetting>(_trans("Relative Input"), false));
}

void Cursor::GetState(ControlState* const x, ControlState* const y, ControlState* const z,
                      const bool adjusted, const bool relative, const bool step)
{
  std::lock_guard<std::mutex> lk(m_mutex);

  // smooth frames
  const u8 NUM_FRAMES = 10;
  // absolute conversion range
  const double ABS_RANGE = .2,
    // raw input range
    RAW_RANGE = .1;

  ControlState r = controls[C_RANGE_MODIFIER]->control_ref->State();
  if (!r)
    r = 1.0;

  for (unsigned int i = 0; i<3; i++)
  {
    //bool is_relative = controls[i * 2 + 1]->control_ref->IsRelative() || controls[i * 2]->control_ref->IsRelative();
    ControlState state = controls[i * 2 + 1]->control_ref->State() - controls[i * 2]->control_ref->State();

    // change sign
    if (i == 0)
      state = -state;

    // update absolute position
    //if (is_relative)
    //{
      // moving average smooth input
      m_list[i].push_back(state);
      double fsum = 0; int j = 0;
      if (m_list[i].size() > 0)
      {
        for (std::list<double>::reverse_iterator it = m_list[i].rbegin(); relative ? it != m_list[i].rend() : j < 1; it++, j++)
          fsum += *it;
        state = fsum / double(m_list[i].size());
      }
    if (m_list[i].size() >= NUM_FRAMES)
        m_list[i].pop_front();

      // update position
      m_absolute[i] += state * r * ABS_RANGE;

      m_absolute[i] = MathUtil::Trimrange(m_absolute[i], -1, 1);
      m_state[i] = m_absolute[i];
    /*}
    else
    {
      m_absolute[i] = MathUtil::Trim(state, -1, 1);
    }*/

    //if (relative)
    //{
      m_state[i] = (m_absolute[i] - m_last[i]) * RAW_RANGE;
      if (step)
        m_last[i] = m_absolute[i];
    /*}
    else
    {
      m_state[i] = m_absolute[i];
    }*/
  }

  *y = m_state[0];
  *x = m_state[1];
  *z = m_state[2];

    // adjust absolute cursor
  if (adjusted && !relative)
  {
    *x *= (numeric_settings[C_WIDTH]->GetValue() * 2.0);
    *y *= (numeric_settings[C_HEIGHT]->GetValue() * 2.0);
    *y += (numeric_settings[C_CENTER]->GetValue() - 0.5);
  }
  }
}  // namespace ControllerEmu
