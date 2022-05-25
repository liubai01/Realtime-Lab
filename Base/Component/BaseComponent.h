#pragma once

typedef enum class ComponentType
{
  COMPONENT_MESH,
  COMPONENT_LIGHT
} ComponentType;

class BaseComponent
{
public:
  ComponentType mComponentType;

  virtual void Update() {}

  virtual void OnEditorGUI() {};
};

