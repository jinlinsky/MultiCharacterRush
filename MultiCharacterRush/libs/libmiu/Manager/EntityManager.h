//
//  EntityManager
//  libmiu
//
//  Created by jinlin on 13-3-8.
//
//

#ifndef __Miu__EntityManager__
#define __Miu__EntityManager__

#import <iostream>
#import <list>
#import "LHSprite.h"
#import "EntityBase.h"

namespace libmiu {

class EntityManager
{
public:
    typedef std::list<EntityBase*>           EntityList;
    typedef std::list<EntityBase*>::iterator EntityListIterator;
    
    typedef std::list<int>     TagList;
    typedef TagList::iterator  TagListIterator;
    
    static EntityManager& sGetEntityManager ();
    
    EntityManager();
    ~EntityManager();

    void                 Reset                    ();
    void                 Tick                     (bool gamePause);
    
//    void              CreateMius            (NSArray* lhSprites);
//    EntityBase*       CreateMiu             (LHSprite* lhSprite);
//    
//    void              RemoveMius            (NSArray* lhSprites);
//    void              RemoveMiu             (LHSprite* lhSprite);
    
    EntityListIterator   GetFirstMiuIterator      ();
    EntityListIterator   GetLastMiuIterator       ();
    
    EntityBase*          GetEntityBaseByLHSprite  (LHSprite* lhSprite);
    EntityList           GetEntityBasesByTag      (int tag);
    
    TagList&             GetUITagList             ();
    bool                 IsTagUI                  (int tag);
    
private:
    EntityList   mEntityList;
    EntityList   mMenuEntityList;
    
    TagList      mUITags;
    
    void DelEntity (EntityBase* EntityBase);
    void AddEntity (EntityBase* EntityBase);
};
    
}  // namespace libmiu 

#define GET_MIU_BY_LHSPRITE(x) \
EntityManager::sGetEntityManager().GetEntityBaseByLHSprite(x)

#endif /* defined(__Miu__EntityManager__) */
