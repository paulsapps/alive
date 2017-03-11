#pragma once

#include <string>
#include "types.hpp"
#include "proxy_sol.hpp"
#include "proxy_sqrat.hpp"
#include "logger.hpp"

struct ObjRect
{
    s32 x;
    s32 y;
    s32 w;
    s32 h;

    static void RegisterScriptBindings(sol::state& state)
    {
        Sqrat::Class<ObjRect> c(Sqrat::DefaultVM::Get(), "ObjRect");
        c.Var("x", &ObjRect::x)
            .Var("y", &ObjRect::y)
            .Var("w", &ObjRect::h)
            .Var("h", &ObjRect::h)
            .Ctor();
        Sqrat::RootTable().Bind("ObjRect", c);

        state.new_usertype<ObjRect>("ObjRect",
            "x", &ObjRect::x,
            "y", &ObjRect::y,
            "w", &ObjRect::h,
            "h", &ObjRect::h);
    }
};

namespace Oddlib
{
    class IStream;
}

class InputState;
class Renderer;
class Animation;
struct GuiContext;
class ResourceLocator;

class MapObject
{
public:
    MapObject()
    {
        TRACE_ENTRYEXIT;
        LOG_INFO("this = " << std::hex << "0x" << static_cast<void*>(this));
    }
    MapObject(const MapObject&) = default;
    MapObject(MapObject&& other) = default;
   
    MapObject& operator = (const MapObject&) = default;
    MapObject& operator = (MapObject&& other) = default;
    ~MapObject();

    //MapObject(IMap& map, sol::state& luaState, ResourceLocator& locator, const ObjRect& rect);
    //MapObject(IMap& map, sol::state& luaState, ResourceLocator& locator, const std::string& scriptName);

    void SetScriptInstance(Sqrat::Object obj)
    {
        TRACE_ENTRYEXIT;
        mScriptObject = obj;
    }

    void Init(ResourceLocator& locator);
    void Update(const InputState& input);
    void Render(Renderer& rend, GuiContext& gui, int x, int y, float scale, int layer) const;
    void ReloadScript();
    static void RegisterScriptBindings(sol::state& state);

    bool ContainsPoint(s32 x, s32 y) const;
    const std::string& Name() const { return mName; }

    // TODO: Shouldn't be part of this object
    void SnapXToGrid();

    float mXPos = 50.0f;
    float mYPos = 100.0f;

    s32 Id() const { return mId; }
    void Activate(bool direction);
    bool WallCollision(f32 dx, f32 dy) const;
    bool CellingCollision(f32 dx, f32 dy) const;
    std::tuple<bool, f32, f32, f32> FloorCollision() const;
private:
    void ScriptLoadAnimations();
    //IMap& mMap;

    std::map<std::string, std::shared_ptr<Animation>> mAnims;
    Animation* mAnim = nullptr;
    //sol::state& mLuaState;
    sol::table mStates;

    void LoadScript();
private: // Actions
    bool AnimationComplete() const;
    void SetAnimation(const std::string& animation);
    void SetAnimationFrame(s32 frame);
    void SetAnimationAtFrame(const std::string& animation, u32 frame);
    bool FacingLeft() const { return mFlipX; }
    bool FacingRight() const { return !FacingLeft(); }
    void FlipXDirection() { mFlipX = !mFlipX; }
private:
    bool AnimUpdate();
    s32 FrameCounter() const;
    s32 NumberOfFrames() const;
    bool IsLastFrame() const;
    s32 FrameNumber() const;
public:
    bool mFlipX = false;
private:
    //ResourceLocator& mLocator;
    std::string mScriptName;
    std::string mName;
    s32 mId = 0;
   // ObjRect mRect;
    Sqrat::Object mScriptObject; // Derived script object instance
};