//
//  iState.cpp
//  Miu
//
//  Created by jinlin on 13-3-8.
//
//

#import "iState.h"
#import "EntityBase.h"
#import "LevelHelperLoader.h"

namespace libmiu {

iState::iState(const std::string & name)
: mName(name)
, mIsStateFinished(false)
{

}

iState::~iState()
{
    TransitionVectorIterator it = mTransitions.begin();
    while (it != mTransitions.end())
    {
        CC_SAFE_DELETE(*it);
        ++it;
    }
    mTransitions.clear();
}

void iState::AddTransition (iTransition* transition)
{
    TransitionVectorIterator it = mTransitions.begin();
    while (it != mTransitions.end())
    {
        if (*it == transition)
        {
            assert(0);
            return;
        }
        
        ++it;
    }
    
    mTransitions.push_back(transition);
}

const iState::TransitionVector& iState::GetTransitions () const
{
    return mTransitions;
}

void iState::AddAnim(const char* name, const char* sheetName)
{
    if (mAnims.size() == ANIM_NUM)
    {
        assert(0);
        return;
    }
    
    if (name == NULL || sheetName == NULL)
        return;
    
    AnimInfo animInfo;
    animInfo.mName = name;
    animInfo.mSheetName = sheetName;
    
    mAnims.push_back(animInfo);
}

bool iState::PlayAnim(EntityBase* entityBase, int index)
{
    int animNum = mAnims.size()-1;
    if (index > animNum)
    {
        return false;
    }
    
    entityBase->PlayAnim(mAnims[index]);
    return true;
}

bool iState::IsPlayingAnim(EntityBase* entityBase, AnimInfo& animInfo)
{
    return entityBase->IsPlayingAnim(animInfo);
}

bool iState::IsAnimLastFrame(EntityBase* entityBase)
{
    return entityBase->IsAnimLastFrame();
}

//bool iState::IsAnimLoop(EntityBase* entityBase)
//{
//    return entityBase->IsAnimLoop();
//}
    
} // namespace libmiu
