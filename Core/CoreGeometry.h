#pragma once
#include "../Base/BaseGeometry.h"

struct CoreVertex {
  XMFLOAT3 pos;
  XMFLOAT3 normal;
  XMFLOAT2 texCoord;
};

class CoreGeometry : public BaseGeometry<CoreVertex>
{
};

