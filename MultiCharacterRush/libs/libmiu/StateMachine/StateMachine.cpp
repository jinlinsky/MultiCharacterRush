//
//  StateMachine.cpp
//  Miu
//
//  Created by jinlin on 13-3-8.
//
//

#import "StateMachine.h"
#import "iState.h"
#import "iTransition.h"
#import "EntityBase.h"

namespace libmiu {
    
StateMachine::StateMachine(const std::string & name, EntityBase* entityBase)
: iState(name)
, mRootState(NULL)
, mActiveState(NULL)
, mPreState(NULL)
, mEntityBase(entityBase)
{

}

StateMachine::~StateMachine()
{
    StateMapIterator it = mStateMap.begin();
    while (it != mStateMap.end())
    {
        CC_SAFE_DELETE(it->second);
        ++it;
    }
    mStateMap.clear();
}

void StateMachine::Tick()
{
    //---------------------------------------------------------------------------------------------
	// Update
	//---------------------------------------------------------------------------------------------
	bool tickResult = updateActiveState();
	
    //---------------------------------------------------------------------------------------------
	// Evaluate
	//---------------------------------------------------------------------------------------------
	evaluateTransitions(tickResult);
}

bool StateMachine::AddState (iState* state, bool isRootState/* = false */)
{
    const std::string stateName = state->GetName();
	if (mStateMap.find(stateName) != mStateMap.end())
	{
		// State already added.
        assert(0);
		return false;
	}

	mStateMap.insert(StateMap::value_type(stateName, state));
	
	if (isRootState)
	{
		mRootState = state;
		//mActiveState->Enter(mMiuRef);
	}
	
	return true;
}

bool StateMachine::updateActiveState()
{
	if (mActiveState != NULL)
	{
        mActiveState->Tick(mEntityBase);

		return mActiveState->IsStateFinished();
	}
    
    return false;
}

void StateMachine::evaluateTransitions(bool tickResult)
{
	if (mActiveState != NULL)
	{
        iState::TransitionVector transitions = mActiveState->GetTransitions();
		
        iState::TransitionVectorIterator transitionIter = transitions.begin();
		iState::TransitionVectorIterator transitionIterEnd = transitions.end();
		
		while(transitionIter != transitionIterEnd)
		{
            iTransition* transition = (*transitionIter);
            
			if (transition != NULL)
			{
                transition->SetStateTickResult(tickResult);

                StateMapIterator it;
                
                if (transition->IsMeetCoodition(mEntityBase))
                {
                    const std::string nextState = transition->GetYesNextState();
                    it = mStateMap.find(nextState);
                }else
                {
                    const std::string nextState = transition->GetNoNextState();
                    it = mStateMap.find(nextState);
                }
				
				if (it != mStateMap.end())
				{
                    mActiveState->Exit(mEntityBase);
                    mPreState = mActiveState;
                    
					mActiveState = (it->second);
					mActiveState->Enter(mEntityBase);
                    
                    break;
				}
			}
			
			transitionIter++;
		}
	}
}

} // namespace libmiu