#include "Engine/Core/CoreApp.h"
#include "Engine/Core/CoreGeometry.h"
#include "Engine/Core/Component/CoreMeshComponent.h"
#include "Engine/Core/CoreGeometryUtils.h"

#include "Engine/Base/Resource/BaseResourceImage.h"
#include "Engine/Base/BaseProject.h"


class MyApp : public CoreApp {
public:
    MyApp(HINSTANCE hInstance, BaseProject* proj) : CoreApp(hInstance, proj) {};
    

    void Start() {
        // textured tower
        shared_ptr<BaseObject> tower = mNowScene->CreateObject("Wood Tower");
        shared_ptr<CoreMeshComponent> meshComponent;

        tower->mTransform.SetPos(0.0f, -3.0f, 0.0f);

        meshComponent = mMeshLoader->MakeComponent(mResourceManager->LoadByURL<BaseResourceMesh>("models\\WoodTower.obj"));

        BaseResourceImage* colorTexture = mResourceManager->LoadByURL<BaseResourceImage>("Wood_Tower_Col.jpg");
        BaseResourceImage* normalTexture = mResourceManager->LoadByURL<BaseResourceImage>("Wood_Tower_Nor.jpg");

        CoreResourceMaterial* towerMat = mResourceManager->LoadByURL<CoreResourceMaterial>("matereials\\Tower.mat");
        meshComponent->mMat[0] = towerMat;
        meshComponent->mMat[0]->SetDiffuseColorTextured(colorTexture);
        meshComponent->mMat[0]->SetNormalTextured(normalTexture);
        meshComponent->mMat[0]->SetNormalStrength(0.2f);

        json j = meshComponent->Serialize();
        dout::printf("Mesh Component %s\n", j.dump().c_str());

        tower->AddComponent(meshComponent);

        // light GO
        shared_ptr<BaseObject> light = mNowScene->CreateObject("Directional Light");
        shared_ptr<CoreLightComponent> lightComponent = mLightManager->MakeLightComponent();
        light->AddComponent(lightComponent);

        //json j = lightComponent->Serialize();
        //lightComponent->Deserialize(j);

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

BaseProject* GetProject(const string& projectPath)
{
    BaseProject* proj = new BaseProject(projectPath);

    // Should be removed after scene could be serialized
    BaseAssetNode* node = proj->mAssetManager->RegisterAsset("models\\WoodTower.obj", "ExampleProject\\Asset\\models\\WoodTower.obj");
    node = proj->mAssetManager->RegisterAsset("Wood_Tower_Col.jpg", "ExampleProject\\Asset\\Wood_Tower_Col.jpg");
    node = proj->mAssetManager->RegisterAsset("Wood_Tower_Nor.jpg", "ExampleProject\\Asset\\Wood_Tower_Nor.jpg");
    node = proj->mAssetManager->RegisterAsset("matereials\\Tower.mat", "ExampleProject\\Asset\\matereials\\Tower.mat");

    // Editor resources
    node = proj->mAssetManager->RegisterAsset("EditorAsset", "ExampleProject\\Asset\\EditorAsset");
    node->SetHidden(true); // forbid users to modify resources under editor asset
    // The editor font
    node = proj->mAssetManager->RegisterAsset("EditorAsset\\Cousine-Regular.ttf", "ExampleProject\\Asset\\EditorAsset\\Cousine-Regular.ttf");
    // The default material
    node = proj->mAssetManager->RegisterAsset("EditorAsset\\Default.mat", "ExampleProject\\Asset\\EditorAsset\\Default.mat");
    // Editor icons
    node = proj->mAssetManager->RegisterAsset("EditorAsset\\icon\\folder.png", "ExampleProject\\Asset\\EditorAsset\\icon\\folder.png");

    return proj;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nShowCmd)
{
    string projectPath = "ExampleProject";
    BaseProject* proj = GetProject(projectPath);
    
    MyApp app = MyApp(hInstance, proj);
    
    try
    {
        app.Run();
    }
    catch (dout::DxException e) {
        dout::printf("%s\n", e.ToString().c_str());
    }

    delete proj;

    return 0;
}