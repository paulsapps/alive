
class Switch
{
    mScale = 0;
    mId = 0;
    mLastAnim = "";
    mBase = 0;

    static kAnimationResources =
    [
        "SwitchIdle",
        "SwitchActivateLeft",
        "SwitchDeactivateLeft",
        "SwitchActivateRight",
        "SwitchDeactivateRight"
    ];

    static kSoundResources =
    [
        "FX_LEVER"
    ];

    constructor(mapObj, rect, stream)
    {
        mBase = mapObj;

        mBase.mName = "Switch";
    
        log_info("Switch ctor");
        
        mBase.mXPos = rect.x + 37;
        mBase.mYPos = rect.y + rect.h - 5;
  
        log_info("Switch ctor stream");
     
        mScale = IStream.ReadU32(stream);
        if (mScale == 1)
        {
            log_warning("Half scale not supported")
        }
  
        log_info("Switch ctor stream extra");
    
        local onSound = IStream.ReadU16(stream);
        local offSound = IStream.ReadU16(stream);
        local soundDirection = IStream.ReadU16(stream);

        // The ID of the object that this switch will apply "targetAction" to
        mId = IStream.ReadU16(stream)
    }

    function SetAnimation(anim)
    {
        if (mLastAnim != anim)
        {
            mBase.SetAnimation(anim);
            mLastAnim = anim;
        }
    }

    function Activate(direction)
    {
        log_info("TODO: Activate");
        // TODO: Activate objects with self.mId
    }

    // TODO: Make this work
    function Update(actions)
    {
        SetAnimation("SwitchIdle");
        // SwitchActivateLeft
        // SwitchDeactivateLeft
        // SwitchActivateRight
        // SwitchDeactivateRight
        // PlaySoundEffect("FX_LEVER");
        mBase.AnimUpdate();
    }
}
