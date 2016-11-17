#pragma once

#include <memory>
#include <vector>
#include <map>
#include <deque>
#include "core/audiobuffer.hpp"
#include "proxy_nanovg.h"
#include "oddlib/path.hpp"
#include "fsm.hpp"
#include "proxy_sol.hpp"
#include "renderer.hpp"
#include "collisionline.hpp"

struct GuiContext;
class Renderer;
class ResourceLocator;
class InputState;

namespace Oddlib { class LvlArchive; class IBits; }


class Animation;


struct ObjRect
{
    s32 x;
    s32 y;
    s32 w;
    s32 h;

    static void RegisterLuaBindings(sol::state& state)
    {
        state.new_usertype<ObjRect>("ObjRect",
            "x", &ObjRect::x,
            "y", &ObjRect::y,
            "w", &ObjRect::h,
            "h", &ObjRect::h);
    }
};

class IMap
{
public:
    virtual ~IMap() = default;
    virtual const CollisionLines& Lines() const = 0;
};

class MapObject
{
public:
    MapObject(MapObject&&) = delete;
    MapObject& operator = (MapObject&&) = delete;
    MapObject(IMap& map, sol::state& luaState, ResourceLocator& locator, const ObjRect& rect);
    MapObject(IMap& map, sol::state& luaState, ResourceLocator& locator, const std::string& scriptName);
    void Init();
    void GetName();
    void Init(const ObjRect& rect, Oddlib::IStream& objData);
    void Update(const InputState& input);
    void Render(Renderer& rend, GuiContext& gui, int x, int y, float scale) const;
    void ReloadScript();
    static void RegisterLuaBindings(sol::state& state);

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
    IMap& mMap;

    std::map<std::string, std::unique_ptr<Animation>> mAnims;
    Animation* mAnim = nullptr;
    sol::state& mLuaState;
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
    ResourceLocator& mLocator;
    std::string mScriptName;
    std::string mName;
    s32 mId = 0;
    ObjRect mRect;
};

class Level
{
public:
    Level(Level&&) = delete;
    Level& operator = (Level&&) = delete;
    Level(IAudioController& audioController, ResourceLocator& locator, sol::state& luaState, Renderer& render);
    void Update(const InputState& input, CoordinateSpace& coords);
    void Render(Renderer& rend, GuiContext& gui, int screenW, int screenH);
    void EnterState();
private:
    void RenderDebugPathSelection(Renderer& rend, GuiContext& gui);
    std::unique_ptr<class GridMap> mMap;
    ResourceLocator& mLocator;
    sol::state& mLuaState;
};

class GridScreen
{
public:
    GridScreen(const GridScreen&) = delete;
    GridScreen& operator = (const GridScreen&) = delete;
    GridScreen(const std::string& lvlName, const Oddlib::Path::Camera& camera, Renderer& rend, ResourceLocator& locator);
    ~GridScreen();
    const std::string& FileName() const { return mFileName; }
    int getTexHandle();
    bool hasTexture() const;
    const Oddlib::Path::Camera &getCamera() const { return mCamera; }
private:
    std::string mLvlName;
    std::string mFileName;
    int mTexHandle;

    // TODO: This is not the in-game format
    Oddlib::Path::Camera mCamera;

    // Temp hack to prevent constant reloading of LVLs
    std::unique_ptr<Oddlib::IBits> mCam;

    ResourceLocator& mLocator;
    Renderer& mRend;
};

class ICommand
{
public:
    virtual void Redo() = 0;
    virtual void Undo() = 0;
    virtual std::string Message() = 0;
    virtual ~ICommand() = default;
};

class Selection
{
public:
    std::set<s32> Clear(CollisionLines& items);
    void Select(CollisionLines& items, s32 idx, bool select);
    bool HasSelection() const { return mSelectedLines.empty() == false; }
    const std::set<s32>& SelectedLines() const { return mSelectedLines; }
private:
    std::set<s32> mSelectedLines;
};

class CommandSelectOrDeselectLine : public ICommand
{
public:
    CommandSelectOrDeselectLine(CollisionLines& lines, Selection& selection, s32 idx, bool select) 
        : mLines(lines), mSelection(selection), mIdx(idx), mSelect(select) { }

    virtual void Redo() final
    {
        mSelection.Select(mLines, mIdx, mSelect);
    }

    virtual void Undo() final
    {
        mSelection.Select(mLines, mIdx, !mSelect);
    }

    virtual std::string Message() final
    {
        if (mSelect)
        {
            return "Select line " + std::to_string(mIdx);
        }
        else
        {
            return "De select line " + std::to_string(mIdx);
        }
    }
private:
    CollisionLines& mLines;
    Selection& mSelection;
    const s32 mIdx;
    const bool mSelect;
};

class CommandClearSelection : public ICommand
{
public:
    CommandClearSelection(CollisionLines& lines, Selection& selection)
        : mLines(lines), mSelection(selection) { }

    virtual void Redo() final
    {
        mOldSelection = mSelection.Clear(mLines);
    }

    virtual void Undo() final
    {
        for (s32 idx : mOldSelection)
        {
            mSelection.Select(mLines, idx, true);
        }
    }

    virtual std::string Message() final
    {
        return "Clear selection";
    }

private:
    CollisionLines& mLines;
    Selection& mSelection;
    std::set<s32> mOldSelection;
};

class CommandMoveLinePoint : public ICommand
{
public:
    CommandMoveLinePoint(CollisionLines& lines, Selection& selection, const glm::vec2& newPos, bool moveP1)
        : mLines(lines), mSelection(selection), mNewPos(newPos), mApplyToP1(moveP1)
    {
        const auto& line = mLines[*mSelection.SelectedLines().begin()];
        mOldPos = mApplyToP1 ? line->mLine.mP1 : line->mLine.mP2;
    }

    virtual void Redo() final
    {
        const auto& line = mLines[*mSelection.SelectedLines().begin()];
        (mApplyToP1 ? line->mLine.mP1 : line->mLine.mP2) = mNewPos;
    }

    virtual void Undo() final
    {
        const auto& line = mLines[*mSelection.SelectedLines().begin()];
        (mApplyToP1 ? line->mLine.mP1 : line->mLine.mP2) = mOldPos;
    }

    virtual std::string Message() final
    {
        return "Move line point " + std::to_string(mApplyToP1 ? 1 : 2) +
            " from (" + std::to_string(mOldPos.x) + "," + std::to_string(mOldPos.y)
            + ") to (" + std::to_string(mNewPos.x) + "," + std::to_string(mNewPos.y) + ")";
    }
private:
    CollisionLines& mLines;
    Selection& mSelection;
    glm::vec2 mOldPos;
    const glm::vec2 mNewPos;
    bool mApplyToP1;
};


class MoveSelection : public ICommand
{
public:
    MoveSelection(CollisionLines& lines, Selection& selection, const glm::vec2& delta)
        : mLines(lines), mSelection(selection), mDelta(delta)
    {

    }

    virtual void Redo() final
    {
        for (s32 idx : mSelection.SelectedLines())
        {
            mLines[idx]->mLine.mP1 += mDelta;
            mLines[idx]->mLine.mP2 += mDelta;
        }
    }

    virtual void Undo() final
    {
        for (s32 idx : mSelection.SelectedLines())
        {
            mLines[idx]->mLine.mP1 -= mDelta;
            mLines[idx]->mLine.mP2 -= mDelta;
        }
    }

    virtual std::string Message() final
    {
        return "Move selection by " + std::to_string(mDelta.x) + "," + std::to_string(mDelta.y);
    }

private:
    CollisionLines& mLines;
    Selection& mSelection;
    const glm::vec2 mDelta;
};

// TODO: Implement stack limit
class UndoStack
{
public:
    template<class T, class... Args>
    void Push(Args&&... args)
    {
        auto cmd = std::make_unique<T>(std::forward<Args>(args)...);

        if (mCommandIndex != Count())
        {
            mUndoStack.erase(mUndoStack.begin() + mCommandIndex, mUndoStack.end());
        }

        cmd->Redo();
        mUndoStack.emplace_back(std::move(cmd));
        mCommandIndex++;
    }

    void Undo();
    void Redo();
    void Clear();
    u32 Count() const { return static_cast<u32>(mUndoStack.size()); }
    void DebugRenderCommandList(GuiContext& gui) const;
private:
    std::vector<std::unique_ptr<ICommand>> mUndoStack;
    u32 mCommandIndex = 0;
};

class GridMap : public IMap
{
public:
    GridMap(const GridMap&) = delete;
    GridMap& operator = (const GridMap&) = delete;
    GridMap(Oddlib::Path& path, ResourceLocator& locator, sol::state& luaState, Renderer& rend);
    void Update(const InputState& input, CoordinateSpace& rend);
    void Render(Renderer& rend, GuiContext& gui) const;
private:
    MapObject* GetMapObject(s32 x, s32 y, const char* type);
    void ActivateObjectsWithId(MapObject* from, s32 id, bool direction);
    void RenderDebug(Renderer& rend) const;
    void RenderEditor(Renderer& rend, GuiContext& gui) const;
    void RenderGame(Renderer& rend, GuiContext& gui) const;

    virtual const CollisionLines& Lines() const override final { return mCollisionItems; }

    void DebugRayCast(Renderer& rend, const glm::vec2& from, const glm::vec2& to, u32 collisionType, const glm::vec2& fromDrawOffset = glm::vec2()) const;



    std::deque<std::deque<std::unique_ptr<GridScreen>>> mScreens;
    
    std::string mLvlName;

    // Editor stuff
    int mEditorCamZoom = 5;
    const int mEditorGridSizeX = 25;
    const int mEditorGridSizeY = 20;

    Selection mSelection;
    UndoStack mUndoStack;
    bool mDraggingItems = false;
    glm::vec2 mLastMousePos;
    enum class eSelectedArea
    {
        eP1,
        eP2,
        eMiddle
    };
    eSelectedArea mSelectedArea = eSelectedArea::eP1;

    // CollisionLine contains raw pointers to other CollisionLine objects. Hence the vector
    // has unique_ptrs so that adding or removing to this vector won't cause the raw pointers to dangle.
    CollisionLines mCollisionItems;

    bool mIsAo;

    MapObject mPlayer;
    std::vector<std::unique_ptr<MapObject>> mObjs;

    enum class eStates
    {
        eInGame,
        eEditor
    };
    eStates mState = eStates::eInGame;

    void ConvertCollisionItems(const std::vector<Oddlib::Path::CollisionItem>& items);

    glm::vec2 mCameraPosition;

};