 //
//  EntityManager.cpp
//  libmiu
//
//  Created by jinlin on 13-3-8.
//
//

#import "EntityManager.h"

namespace libmiu {

#define ENTITY_MAX_NUM 500

EntityManager& EntityManager::sGetEntityManager()
{
    static EntityManager sEntityManager;
    return sEntityManager;
}

EntityManager::EntityManager()
{
}

EntityManager::~EntityManager()
{
}

void EntityManager::Reset()
{
    EntityListIterator it = mEntityList.begin();
    while (it != mEntityList.end())
    {
        CC_SAFE_DELETE(*it);
        ++it;
    }
    mEntityList.clear();
}

void EntityManager::Tick(bool gamePause)
{
    EntityListIterator it = mEntityList.begin();

    while (it != mEntityList.end())
    {
        EntityBase* element = *it;
        
        if (element != NULL)
        {
            if (element->mShouldRemove)
            {
                CC_SAFE_DELETE(*it);
                it = mEntityList.erase(it);
                continue;
            }
            
            if (gamePause)
            {
                if (IsTagUI(element->GetSprite()->getTag()))
                {
                    element->Tick();
                }
            }else
            {
                element->Tick();
            }
        }
    }
}

//void EntityManager::CreateMius(NSArray* lhSprites)
//{
//    NSArray* mius = lhSprites;
//    for (int i=0; i<[mius count]; ++i)
//    {
//        LHSprite* lhSprite = [mius objectAtIndex:i];
//        
//        EntityManager::sGetEntityManager().CreateMiu(lhSprite);
//    }
//}
//
//void EntityManager::RemoveMius(NSArray* lhSprites)
//{
//    NSArray* mius = lhSprites;
//    for (int i=0; i<[mius count]; ++i)
//    {
//        LHSprite* lhSprite = [mius objectAtIndex:i];
//        
//        EntityManager::sGetEntityManager().RemoveMiu(lhSprite);
//    }
//}

//void EntityManager::RemoveMiu(LHSprite* lhSprite)
//{
//    DelMiu(GET_MIU_BY_LHSPRITE(lhSprite));
//}
//
//EntityBase* EntityManager::CreateMiu(LHSprite* lhSprite)
//{
//    EntityBase* miu = NULL;
//
//    if (0)
//    {
//        
//    }else
//    {
//        miu = new EntityBase(lhSprite);
//    }
//    
//    if (miu != NULL)
//    {
//        miu->Init();
//        miu->SetupStateMachine();
//        miu->FirstEnter();
//        
//        AddMiu(miu);
//    }
//    
//    return miu;
//}

EntityManager::EntityListIterator EntityManager::GetFirstMiuIterator ()
{
    return mEntityList.begin();
}

EntityManager::EntityListIterator EntityManager::GetLastMiuIterator ()
{
    return mEntityList.end();
}

void EntityManager::DelEntity(EntityBase *EntityBase)
{
    EntityListIterator it = mEntityList.begin();
    while (it != mEntityList.end())
    {
        if ((*it) == EntityBase)
        {
            CC_SAFE_DELETE(*it);
            mEntityList.erase(it);
            return;
        }
        
        ++it;
    }
}

void EntityManager::AddEntity(EntityBase *EntityBase)
{
    assert(mEntityList.size() <= ENTITY_MAX_NUM);
    
    EntityListIterator it = mEntityList.begin();
    while (it != mEntityList.end())
    {
        if (*it == EntityBase)
        {
            return;
        }
        
        ++it;
    }
    
    mEntityList.push_back(EntityBase);
}

EntityBase* EntityManager::GetEntityBaseByLHSprite(LHSprite* lhSprite)
{
    EntityListIterator it = mEntityList.begin();
    while (it != mEntityList.end())
    {
        if ((*it)->GetSprite() == lhSprite)
        {
            return (*it);
        }
        
        ++it;
    }
    
    return NULL;
}

EntityManager::EntityList EntityManager::GetEntityBasesByTag(int tag)
{
    EntityList list;
    
    EntityListIterator it = mEntityList.begin();
    while (it != mEntityList.end())
    {
        if ((*it)->GetSprite()->getTag() == tag)
        {
            list.push_back(*it);
        }
        
        ++it;
    }
    
    return list;
}

EntityManager::TagList& EntityManager::GetUITagList()
{
    return mUITags;
}

bool EntityManager::IsTagUI(int tag)
{
    TagListIterator it = mUITags.begin();
    while (it != mUITags.end())
    {
        if (tag == *it)
            return true;
        
        ++it;
    }
    
    return false;
}
    
} // namespace libmiu