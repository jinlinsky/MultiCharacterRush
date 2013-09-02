//
//  HelloWorldScene.h
//  MultiCharacterRush
//
//  Created by jinlin on 13-8-13.
//  Copyright __MyCompanyName__ 2013年. All rights reserved.
//
#ifndef __HELLO_WORLD_H__
#define __HELLO_WORLD_H__

// When you import this file, you import all the cocos2d classes
#include "cocos2d.h"
#include "Box2D.h"
#include "LevelHelperLoader.h"
#include "CSArmature.h"
#include "CSArmatureDataManager.h"

enum PlayerState
{
    PS_RUN = 0,
    PS_JUMP = 1,
    PS_LAND = 2,
};

class PhysicsSprite : public cocos2d::CCSprite
{
public:
    PhysicsSprite();
    void setPhysicsBody(b2Body * body);
    virtual bool isDirty(void);
    virtual cocos2d::CCAffineTransform nodeToParentTransform(void);
private:
    b2Body* m_pBody;    // strong ref
};

class HelloWorld : public cocos2d::CCLayer {
public:
    ~HelloWorld();
    HelloWorld();
    
    // returns a Scene that contains the HelloWorld as the only child
    static cocos2d::CCScene* scene();
    
    void initPhysics();

    virtual void draw();
    void update(float dt);
    
    virtual void ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);
    virtual void ccTouchesEnded(cocos2d::CCSet* touches, cocos2d::CCEvent* event);
    
    void beginOrEndCollisionBetweenMarioAndCoin(LHContactInfo* contact);
    void beginOrEndCollisionBetweenPlayer01AndGhostZone(LHContactInfo* contact);
    void beginOrEndCollisionBetweenCameraAndMarke(LHContactInfo* contact);
    
    void AnimationEndedNotification(LHSprite* sprite);
    
    void updateCamera(CCCamera* camera);
    void updateBG();
    
    void requestNextScreenMap(float startX);
    void generateNextScreenMap(float startX);
    void cleanOutofScreenMap();
    
    CCPoint getBG01StartPoint();
    CCPoint getBG02StartPoint();
    
    void player01Run();
    void player01Jump();
    void player01Land();
    
    void player02Run();
    void player02Jump();
    
    LevelHelperLoader* mLevelHelperLoader;
    
    // player01
    int mPlayer01State;
    b2Vec2 mOriginalPosition;
    b2Vec2 mPlayer01Velocity;
    
    // camera
    LHSprite* mSpriteCamera;
    bool mIsCameraMoving;
    
    // map
    bool mIsRequestNextScreenMap;
    float mNextScreenMapStartX;
    
    float mMapBlockSize;
    
    int mScreenCount;
    
    int mBG01Count;
    int mBG02Count;
    
    // level
    bool mRequestLoadLevel;
    
    void LoadLevel();
    
private:
    b2World* world;
    cocos2d::CCTexture2D* m_pSpriteTexture; // weak ref
    b2Draw* m_debugDraw;
};

#endif // __HELLO_WORLD_H__
