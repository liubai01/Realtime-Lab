#include "Core/CoreApp.h"
#include "Core/CoreGeometry.h"
#include "Core/Component/CoreMeshComponent.h"
#include "Core/CoreGeometryUtils.h"

#include "Base/Resource/BaseResourceImage.h"


class MyApp : public CoreApp {
public:
    MyApp(HINSTANCE hInstance, const string& ProjectPath) : CoreApp(hInstance, ProjectPath) {};
    

    void Start() {
        shared_ptr<BaseObject> tower = mNowScene->CreateObject("Wood Tower");
        shared_ptr<CoreMeshComponent> meshComponent;

        tower->mTransform.SetPos(0.0f, -3.0f, 0.0f);

        meshComponent = mMeshLoader->MakeComponent(mResourceManager->LoadMesh("models\\WoodTower.obj"));

        BaseResourceImage* colorTexture = mResourceManager->LoadImage("Wood_Tower_Col.jpg");
        BaseResourceImage* normalTexture = mResourceManager->LoadImage("Wood_Tower_Nor.jpg");

        meshComponent->mMat[0]->SetDiffuseColorTextured(colorTexture);
        meshComponent->mMat[0]->SetNormalTextured(normalTexture);

        tower->AddComponent(meshComponent);

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

        //ImGui::Begin("Normal strength");
        //ImGui::DragFloat("normalMap", &meshComponent2->mMat[0]->mBuffer.mData.NormalStrength, 0.01f, 0.0f, 1.0f, "%.02f");
        //ImGui::End(); // end of docker space

        //mLightManager->SetLightDir(x, y, z);

        //mMainCamera->SetPos(mRadius * x, mRadius * y, mRadius * z);

        //shared_ptr<BaseObject> cubeObj = GetObject("Cube Red");
        //cubeObj->mTransform.SetRot(timer, timer * 0.5f, timer);
    }
};


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd)
{
    string projectPath = "ExampleProject";
    MyApp app = MyApp(hInstance, projectPath);


    try
    {
        app.Run();
    }
    catch (dout::DxException e) {
        dout::printf("%s\n", e.ToString().c_str());
    }

    return 0;
}