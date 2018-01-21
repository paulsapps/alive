#pragma once

#include <memory>
#include <vector>

#include "../abstractrenderer.hpp"

class Entity;

enum class ComponentIdentifier
{
    None,
    Transform,
    Animation,
    Physics,
    AbeMovementController,
    PlayerController
};

class Component
{
public:
    using UPtr = std::unique_ptr<Component>;
public:
    virtual ~Component() = default;
public:
    virtual void Update();
    virtual void Render(AbstractRenderer&) const;
public:
    void SetEntity(Entity*);
    void SetId(ComponentIdentifier);
    ComponentIdentifier GetId() const;
protected:
    Entity* mEntity = nullptr;
    ComponentIdentifier mId = ComponentIdentifier::None;
};