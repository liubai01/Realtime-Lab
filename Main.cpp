#include "Core/CoreApp.h"
#include "Core/CoreGeometry.h"
#include "Core/Component/CoreMeshComponent.h"
#include "Core/CoreGeometryUtils.h"



class MyApp : public CoreApp {
public:
    MyApp(HINSTANCE hInstance) : CoreApp(hInstance) {};

    void Start() {

        shared_ptr<BaseObject> tower = mGOManager->CreateObject("Wood Tower");
        tower->mTransform.SetPos(0.0f, -3.0f, 0.0f);
        shared_ptr<CoreMeshComponent> meshComponent2 = mMeshLoader->LoadObjMesh(".\\Asset\\WoodTower.obj", "WoodTower");
        tower->AddComponent(meshComponent2);

        mMainCamera->SetPos(10.0f, 10.0f, -10.0f);
    }

    void Update()
    {
        static float timer = 0.0f;
        timer += mTimeDelta;
        float mRadius = 20.0f;

        static float mTheta = 1.5f * XM_PI;
        float mPhi = XM_PIDIV4;
        
        mTheta += XM_PI * mTimeDelta * 0.1f;
        
        // Convert Spherical to Cartesian coordinates.
        float x = sinf(mPhi) * cosf(mTheta);
        float z = sinf(mPhi) * sinf(mTheta);
        float y = cosf(mPhi);

        //mLightManager->SetLightDir(x, y, z);

        //mMainCamera->SetPos(mRadius * x, mRadius * y, mRadius * z);

        //shared_ptr<BaseObject> cubeObj = GetObject("Cube Red");
        //cubeObj->mTransform.SetRot(timer, timer * 0.5f, timer);
    }
};


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd)
{
  MyApp app = MyApp(hInstance);

  try
  {
    app.Run();
  }
  catch (dout::DxException e) {
    dout::printf("%s\n", e.ToString().c_str());
  }


  return 0;
}