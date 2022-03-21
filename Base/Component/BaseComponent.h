#pragma once

typedef enum class BaseComponentType
{
  BASE_COMPONENT_MESH
} BaseComponentType;

class BaseComponent
{
public:
  BaseComponentType mComponentType;

  virtual void Update() {}
};

