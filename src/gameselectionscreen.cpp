#include "gameselectionscreen.hpp"
#include "developerscreen.hpp"
#include "gui.h"

void GameSelectionScreen::Update()
{

}

void GameSelectionScreen::Render(int /*w*/, int /*h*/, Renderer& /*renderer*/)
{
    gui_begin_window(mGui, "Select game");

    for (size_t idx = 0; idx < mVisibleGameDefinitions.size(); idx++)
    {
        const GameDefinition& gd = *mVisibleGameDefinitions[idx];
        if (gui_radiobutton(mGui, gd.Name().c_str(), mSelectedGameDefintionIndex == idx))
        {
            mSelectedGameDefintionIndex = idx;
        }

    }

    bool gotoDevMode = false;
    if (gui_button(mGui, "Start game"))
    {
        const GameDefinition& gd = *mVisibleGameDefinitions[mSelectedGameDefintionIndex];

        // TODO: Validate we have all of the required data sets to launch the game

        if (gd.DataSetName() == "Developer")
        {
            gotoDevMode = true;
        }
    }

    gui_end_window(mGui);
    if (gotoDevMode)
    {
        // "this" will be deleted after this call, so must be last call in the function

        mStateChanger.ToState(std::make_unique<DevloperScreen>(mGui, mStateChanger, mFmv, mSound, mLevel, mFsOld));
    }
}