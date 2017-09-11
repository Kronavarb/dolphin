// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <array>
#include "Core/HW/WiimoteEmu/Attachment/Attachment.h"

namespace ControllerEmu
{
class AnalogStick;
class Buttons;
class ControlGroup;
class Force;
class Tilt;
}

namespace WiimoteEmu
{
enum class NunchukGroup;
struct ExtensionReg;

/* Default calibration for the nunchuck. It should be written to 0x20 - 0x3f of the
extension register. 0x80 is the neutral x and y accelerators and 0xb3 is the
neutral z accelerometer that is adjusted for gravity. */
static const u8 nunchuck_calibration[] =
{
  0x80,0x80,0x80,0x00, // accelerometer x, y, z neutral
  0x9a,0x9a,0x9a,0x00, //  x, y, z g-force values

  0xff, 0x00, 0x80, 0xff, // 0xff max, 0x00 min, 0x80 = analog stick x and y axis center
  0x00, 0x80, 0xa1, 0xf6	// checksum on the last two bytes
};

class Nunchuk : public Attachment
{
public:
  explicit Nunchuk(ExtensionReg& reg);

  void GetState(u8* const data) override;
  bool IsButtonPressed() const override;

  ControllerEmu::ControlGroup* GetGroup(NunchukGroup group);

  enum
  {
    BUTTON_C = 0x02,
    BUTTON_Z = 0x01,
  };

  enum
  {
    ACCEL_ZERO_G = 0x80,
    ACCEL_ONE_G = 0xB3,
    ACCEL_RANGE = (ACCEL_ONE_G - ACCEL_ZERO_G),
  };

  enum
  {
    STICK_CENTER = 0x80,
    STICK_RADIUS = 0x7F,
  };

  void LoadDefaults(const ControllerInterface& ciface) override;

private:
  ControllerEmu::Tilt* m_tilt;
  ControllerEmu::Force* m_swing;

  ControllerEmu::Buttons* m_shake;

  ControllerEmu::Buttons* m_buttons;
  ControllerEmu::AnalogStick* m_stick;

  std::array<u8, 3> m_shake_step{};
};
}
