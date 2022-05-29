#pragma once
#include "../BaseInterfaceSerializable.h"
#include "../Asset/BaseAssetManager.h"
#include "../Resource/BaseResourceManager.h"

typedef enum class ComponentType
{
  COMPONENT_MESH,
  COMPONENT_LIGHT
} ComponentType;

class BaseComponent: public BaseInterfaceSerializable
{
public:
  ComponentType mComponentType;

  virtual void Update() {}

  virtual void OnEditorGUI(BaseAssetManager* assetManager, BaseResourceManager* resourceManager) {};
};

