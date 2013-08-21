//
//  iTransition.h
//  libmiu
//
//  Created by jinlin on 13-3-8.
//
//

#ifndef __iTransition__
#define __iTransition__

#import <iostream>

namespace libmiu {

class EntityBase;

class iTransition
{
public:
	iTransition();
	virtual ~iTransition();
	
    virtual bool IsMeetCoodition (EntityBase* entityBase) = 0;
	
    void SetNoState (const std::string& state)
    {
        mNoNextState = state;
    }
    
	const std::string& GetNoNextState() const
	{
		return mNoNextState;
	}
    
    void SetYesState (const std::string& state)
    {
        mYesNextState = state;
    }
    
	const std::string& GetYesNextState() const
	{
		return mYesNextState;
	}
    
    void  SetStateTickResult (bool stateTickResult)
    {
        mStateTickResult = stateTickResult;
    }
    
    bool  GetStateTickResult ()
    {
        return mStateTickResult;
    }
	
protected:
    std::string mNoNextState;
    std::string mYesNextState;
    
    bool        mStateTickResult;
};
    
} // namespace libmiu

#endif /* defined(__iTransition__) */
