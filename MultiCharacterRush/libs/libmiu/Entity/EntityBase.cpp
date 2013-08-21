//
//  EntityBase.cpp
//  libmiu
//
//  Created by jinlin on 13-3-8.
//
//

#include "EntityBase.h"
#include "LHFixture.h"
#include "EntityManager.h"

namespace libmiu {

void EntityBase::sLinkState(iState* firstState, iTransition* transition, iState* secondYesState, iState* secondNoState)
{
    if (secondYesState != NULL)
    {
        transition->SetYesState(secondYesState->GetName());
    }

    if (secondNoState != NULL)
    {
        transition->SetNoState(secondNoState->GetName());
    }
    
    firstState->AddTransition(transition);
}

EntityBase::EntityBase(LHSprite* sprite)
: mSprite(sprite)
{
    mType = ENTITY_BASE;
}

EntityBase::~EntityBase()
{
    StateMachineListIterator it = mStateMachineList.begin();
    while (it != mStateMachineList.end())
    {
        CC_SAFE_DELETE(*it);
        
        ++it;
    }
}

void EntityBase::SetupStateMachine()
{
    
}

void EntityBase::Init()
{
}

void EntityBase::Tick ()
{
    StateMachineListIterator it = mStateMachineList.begin();
    while (it != mStateMachineList.end())
    {
        (*it)->Tick();
        
        ++it;
    }
}

int EntityBase::GetType()
{
    return mType;
}

LHSprite* EntityBase::GetSprite()
{
    return mSprite;
}

b2Body* EntityBase::GetBody ()
{
    return mSprite->getBody();
}


bool EntityBase::IsAnimLastFrame()
{
    return mSprite != NULL ? mSprite->isAtLastFrame() : true;
}

//bool EntityBase::IsAnimLoop()
//{
//    return mSprite->isAnimationLoop;
//}

bool EntityBase::IsPlayingAnim(AnimInfo& animInfo)
{
    return GetSprite()->animationName() == animInfo.mName;
}

void EntityBase::PlayAnim(AnimInfo& animInfo)
{
    if (animInfo.mName == "Default" || animInfo.mSheetName == "Default")
        return;
    
    GetSprite()->prepareAnimationNamed(animInfo.mName, animInfo.mSheetName);
    GetSprite()->playAnimation();
}

void EntityBase::FirstEnter()
{
    StateMachineListIterator it = mStateMachineList.begin();
    while (it != mStateMachineList.end())
    {
        (*it)->Enter(this);
        
        ++it;
    }
}

} // namespace libmiu




