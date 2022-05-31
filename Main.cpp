#include "Engine/Core/CoreApp.h"
#include "Engine/Core/CoreGeometry.h"
#include "Engine/Core/Component/CoreMeshComponent.h"
#include "Engine/Core/CoreGeometryUtils.h"

#include "Engine/Base/Resource/BaseResourceImage.h"
#include "Engine/Base/BaseProject.h"

class MyApp : public CoreApp {
public:
    MyApp(HINSTANCE hInstance, BaseProject* proj) : CoreApp(hInstance, proj) {};
    
    // TBD: move those logic into scene start & update
    //      for the sake of scene switch
    void Start() {
        // textured tower 
        std::shared_ptr<BaseObject> tower = mNowScene->CreateObject("Wood Tower");
        std::shared_ptr<CoreMeshComponent> meshComponent;

        tower->mTransform.SetPos(0.0f, -3.0f, 0.0f);

        meshComponent = mMeshLoader->MakeComponent(mResourceManager->LoadByURL<BaseResourceMesh>("models\\WoodTower.obj"));

        BaseResourceImage* colorTexture = mResourceManager->LoadByURL<BaseResourceImage>("Wood_Tower_Col.jpg");
        BaseResourceImage* normalTexture = mResourceManager->LoadByURL<BaseResourceImage>("Wood_Tower_Nor.jpg");

        CoreResourceMaterial* towerMat = mResourceManager->LoadByURL<CoreResourceMaterial>("matereials\\Tower.mat");
        meshComponent->mMat[0] = towerMat;

        //json j = meshComponent->Serialize();
        //dout::printf("Mesh Component %s\n", j.dump().c_str());
        tower->AddComponent(meshComponent);
        
        // light GO
        std::shared_ptr<BaseObject> light = mNowScene->CreateObject("Directional Light");
        std::shared_ptr<CoreLightComponent> lightComponent = mLightManager->MakeLightComponent();
        light->AddComponent(lightComponent);

        std::shared_ptr<BaseObject> lightdirarrow = mNowScene->CreateObject("Light Arrow");
        std::shared_ptr<CoreMeshComponent> meshComponentArrow;
        meshComponentArrow = mMeshLoader->MakeComponent(mResourceManager->LoadByURL<BaseResourceMesh>("models\\Arrow.obj"));
        lightdirarrow->AddComponent(meshComponentArrow);
        lightdirarrow->SetParent(light.get());

        light->mTransform.SetPos(3.0f, 3.0f, -6.0f);
        lightdirarrow->mTransform.SetRot(0.0f, DirectX::XM_PI / 2.0f, 0.0f);
        lightdirarrow->mTransform.SetScale(0.1f, 0.1f, 0.1f);

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

BaseProject* GetProject(const std::string& projectPath)
{
    BaseProject* proj = new BaseProject(projectPath);
    proj->mAssetManager->ScanAssetFromRoot();

    // modify the visibility of editor directory that could help you
    // add asset into it.
    BaseAssetNode* editorDir = proj->mAssetManager->LoadAsset("EditorAsset");
    editorDir->SetHidden(true);

    return proj;
}


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd)
{
    std::string projectPath = "ExampleProject";
    BaseProject* proj = GetProject(projectPath);

    MyApp* app = new MyApp(hInstance, proj);
    try
    {
        app->Run();
    }
    catch (dout::DxException e) {
        dout::printf("%s\n", e.ToString().c_str());
    }

    delete proj;
    delete app;

    return 0;
}