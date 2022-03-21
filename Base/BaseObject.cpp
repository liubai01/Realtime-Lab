#include "BaseObject.h"
#include "../MathUtils.h"

void BaseObject::AddComponent(shared_ptr<BaseComponent> component)
{
  mComponents.push_back(component);
}