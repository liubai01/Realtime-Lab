#pragma once
#include "../../Base/BaseGUIWidget.h"
#include "../../Base/BaseCamera.h"

class CoreSceneWidget : public BaseGUIWidget
{
public:
	CoreSceneWidget(BaseCamera* camera);
	void Update();
	void Start(ImGuiID& dockspace_id);
private:
	BaseCamera* mCamera;
};
