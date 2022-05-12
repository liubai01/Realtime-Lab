#pragma once

typedef enum class ComponentType
{
  COMPONENT_MESH
} ComponentType;

class BaseComponent
{
public:
  ComponentType mComponentType;

  virtual void Update() {}
};

