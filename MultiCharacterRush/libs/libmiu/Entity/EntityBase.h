//
//  EntityBase.h
//  libmiu
//
//  Created by jinlin on 13-3-8.
//
//

#ifndef __EntityBase__
#define __EntityBase__

#include <iostream>
#include <list>
#include "Box2D.h"
#include "StateMachine.h"
#include "LHSprite.h"
#include "iTransition.h"
#include "iState.h"

namespace libmiu {

class EntityBase
{
public:
    
    typedef std::list<StateMachine*>    StateMachineList;
    typedef StateMachineList::iterator  StateMachineListIterator;
    
    static void sLinkState(iState* firstState, iTransition* transition, iState* secondYesState, iState* secondNoState);
    
    bool mShouldRemove;
    bool mFirstInCamera;
    
    EntityBase(LHSprite* sprite);
    virtual ~EntityBase();
    
    virtual void           SetupStateMachine         ();
    virtual void           Init                      ();
    
    virtual void           Tick                      ();
   
    int                    GetType                   ();
    
    LHSprite*              GetSprite                 ();
    b2Body*                GetBody                   ();
    
    bool                   IsAnimLastFrame           ();
//    bool                   IsAnimLoop                ();
    bool                   IsPlayingAnim             (AnimInfo& animInfo);
    
    void                   PlayAnim                  (AnimInfo& animInfo);
    
    void                   RecoverMasks              (int fixtureId = -1);
    void                   ClearCollisionMask        (int fixtureId = -1);
    
    void                   FirstEnter                ();
    
protected:
    int               mType;
    
    StateMachineList  mStateMachineList;
    
    LHSprite*         mSprite;
};

enum EntityType
{
    ENTITY_BASE = 0,
};
    
} // namespace libmiu

#define LINK_STATE(s1,t1,s2,s3) \
EntityBase::sLinkState(s1, t1, s2, s3)

#endif /* defined(__EntityBase__) */
