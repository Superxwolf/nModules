#pragma once

#include "../nUtilities/Windows.h"

#include <ShObjIdl.h>

struct TaskData {
  UINT monitor;
  TBPFLAG progressState;
  USHORT progress;
  bool flashing;
  HICON overlayIcon;
};
