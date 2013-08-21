//
//  StateMachine.h
//  libmiu
//
//  Created by jinlin on 13-3-8.
//
//

#ifndef __Miu__StateMachine__
#define __Miu__StateMachine__

#import <iostream>
#import <map>
#import "iState.h"

class MiuEntity;
class iState;

namespace libmiu {

class StateMachine : public iState
{
public:
    typedef std::map<std::string, iState*> StateMap;
    typedef StateMap::iterator             StateMapIterator;
    
	StateMachine(const std::string & name, EntityBase* entityBase);
    virtual ~StateMachine();
    
    void Enter (EntityBase* entityBase)
    {
        mActiveState = mRootState;
        mActiveState->Enter(entityBase);
        
        mIsStateFinished = false;
    }
    
    void Tick (EntityBase* entityBase)
    {
        Tick();
        
        if (mActiveState != NULL)
        {
            if (mActiveState->GetTransitions().size() == 0 && mActiveState->IsStateFinished())
            {
                mIsStateFinished = true;
            }
        }
    }
    
    void Exit (EntityBase* entityBase)
    {
        mActiveState->Exit(entityBase);
        mActiveState = NULL;
        mPreState = NULL;
    }
	
    void    Tick           ();
	
	bool    AddState       (iState* state, bool isRootState = false);
    
    iState* GetRootState   () { return mRootState; }
    iState* GetActiveState () { return mActiveState; }
    iState* GetPreState    () { return mPreState; }
    
private:
	StateMap mStateMap;
	
	iState*  mRootState;
	iState*  mActiveState;
	iState*  mPreState;
    
    EntityBase* mEntityBase;
	
	bool updateActiveState   ();
	void evaluateTransitions (bool tickResult);
};
    
} // namespace libmiu

#endif /* defined(__Miu__StateMachine__) */
