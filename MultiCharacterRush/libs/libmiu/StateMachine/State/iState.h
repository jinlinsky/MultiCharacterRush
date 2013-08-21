//
//  iState.h
//  libmiu
//
//  Created by jinlin on 13-3-8.
//
//

#ifndef __Miu__iState__
#define __Miu__iState__

#import <iostream>
#import <vector>

namespace libmiu {

class iTransition;
class EntityBase;

struct AnimInfo
{
public:
    AnimInfo(){}
    AnimInfo(const char* name, const char* sheetName)
    : mName(name)
    , mSheetName(sheetName)
    {
        
    }
    
    std::string mName;
    std::string mSheetName;
};

class iState
{
public:
	typedef std::vector<iTransition*>  TransitionVector;
	typedef TransitionVector::iterator TransitionVectorIterator;
    typedef std::vector<AnimInfo>      AnimVector;
    typedef AnimVector::iterator       AnimVectorIterator;
    
    iState(const std::string & name);
    virtual ~iState();
    
    virtual void Enter  (EntityBase* entityBase) = 0;
    virtual void Tick   (EntityBase* entityBase) = 0;
    virtual void Exit   (EntityBase* entityBase) = 0;
    
    void                     AddTransition   (iTransition* transition);
	const TransitionVector&  GetTransitions  () const;
	
	const std::string&       GetName         () const { return mName; }
    
    void                     AddAnim         (const char* name, const char* sheetName);
    bool                     PlayAnim        (EntityBase* entityBase, int index);
    
    bool                     IsPlayingAnim   (EntityBase* entityBase, AnimInfo& animInfo);
    bool                     IsAnimLastFrame (EntityBase* entityBase);
//    bool                     IsAnimLoop      (EntityBase* entityBase);
    
    bool                     IsStateFinished   () { return mIsStateFinished; }
    
protected:
	std::string      mName;
    
    AnimVector       mAnims;
	TransitionVector mTransitions;
    
    bool             mIsStateFinished;
};

enum AnimIndex
{
    ANIM_BASE    = 0,
    ANIM_EXTRA_0 = 1,
    ANIM_EXTRA_1 = 2,
    ANIM_EXTRA_2 = 3,
    ANIM_EXTRA_3 = 4,
    
    ANIM_NUM
};
    
} // namespace libmiu

#endif /* defined(__Miu__iState__) */
