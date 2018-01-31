#pragma once

#include <memory>
#include <vector>
#include <map>
#include <deque>
#include <iomanip>
#include <sstream>
#include "core/audiobuffer.hpp"
#include "oddlib/path.hpp"
#include "fsm.hpp"
#include "abstractrenderer.hpp"
#include "collisionline.hpp"
#include "imgui/imgui.h"
#include "iterativeforloop.hpp"
#include "core/entitymanager.hpp"

class AbstractRenderer;
class ResourceLocator;
class InputState;

namespace Oddlib
{ 
    class LvlArchive; 
    class IBits;
}

class Animation;

class IMap
{
public:
    virtual ~IMap() = default;
    virtual const CollisionLines& Lines() const = 0;
};

class Sound;

class GridScreen
{
public:
    GridScreen(const GridScreen&) = delete;
    GridScreen& operator = (const GridScreen&) = delete;
    GridScreen(const Oddlib::Path::Camera& camera, ResourceLocator& locator);
    ~GridScreen();
    const std::string& FileName() const { return mFileName; }
    void LoadTextures(AbstractRenderer& rend);
    void UnLoadTextures(AbstractRenderer& rend);
    bool hasTexture() const;
    const Oddlib::Path::Camera &getCamera() const { return mCamera; }
    void Render(AbstractRenderer& rend, float x, float y, float w, float h);
private:
    std::string mFileName;
    TextureHandle mCameraTexture;
    TextureHandle mFG1Texture;

    // TODO: This is not the in-game format
    Oddlib::Path::Camera mCamera;

    // Temp hack to prevent constant reloading of LVLs
    std::unique_ptr<Oddlib::IBits> mCam;

    ResourceLocator& mLocator;
};


#define NO_MOVE_OR_MOVE_ASSIGN(x)  x(x&&) = delete; x& operator = (x&&) = delete

constexpr u32 kSwitchTimeMs = 300;

class WorldState;

class GridMap : public IMap
{
public:
    GridMap(const GridMap&) = delete;
    GridMap& operator = (const GridMap&) = delete;
    GridMap(CoordinateSpace& coords, WorldState& state);
    ~GridMap();
    bool LoadMap(const Oddlib::Path& path, ResourceLocator& locator, const InputState& input); // TODO: Input wired here
private:
    class Loader
    {
    public:
        Loader(GridMap& gm);
        bool Load(const Oddlib::Path& path, ResourceLocator& locator, const InputState& input); // TODO: Input wired here
    private:
        void SetupSystems(ResourceLocator& locator, const InputState& state);
        void SetupAndConvertCollisionItems(const Oddlib::Path& path);
        void HandleAllocateCameraMemory(const Oddlib::Path& path);
        void HandleLoadCameras(const Oddlib::Path& path, ResourceLocator& locator);
        void HandleLoadEntities(const Oddlib::Path& path);

        GridMap& mGm;
        enum class LoaderStates
        {
            eInit,
            eSetupSystems,
            eSetupAndConvertCollisionItems,
            eAllocateCameraMemory,
            eLoadCameras,
            eLoadEntities,
        };

        enum ObjectTypesAe : u8
        {
            eHoist = 2,
            eDoor = 5,
            eBackgroundAnimation = 13,
            eSwitch = 17,
            eMine = 24,
            eElectricWall = 38,
            eSlamDoor = 85
        };
        
        LoaderStates mState = LoaderStates::eInit;
        IterativeForLoopU32 mXForLoop;
        IterativeForLoopU32 mYForLoop;
        IterativeForLoopU32 mIForLoop;

        void SetState(LoaderStates state);
    };
    Loader mLoader;
   
    virtual const CollisionLines& Lines() const override final;

    void ConvertCollisionItems(const std::vector<Oddlib::Path::CollisionItem>& items);
   
public:
    void UnloadMap(AbstractRenderer& renderer);
    EntityManager mRoot;
private:
    WorldState& mWorldState;
};
