//
//  HelloWorldScene.cpp
//  MultiCharacterRush
//
//  Created by jinlin on 13-8-13.
//  Copyright __MyCompanyName__ 2013年. All rights reserved.
//
#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "GLES-Render.h"
#include "CCNotificationCenter.h"

using namespace cocos2d;
using namespace CocosDenshion;

#define PTM_RATIO 32

#define JUMP_VELOCITY 10
#define RUN_VELOCITY 5

#define WIN_SIZE_W \
CCDirector::sharedDirector()->getWinSize().width

#define WIN_SIZE_H \
CCDirector::sharedDirector()->getWinSize().height

#define WIN_SIZE \ 
CCDirector::sharedDirector()->getWinSize()

enum {
    kTagParentNode = 1,
};

PhysicsSprite::PhysicsSprite()
: m_pBody(NULL)
{

}

void PhysicsSprite::setPhysicsBody(b2Body * body)
{
    m_pBody = body;
}

// this method will only get called if the sprite is batched.
// return YES if the physics values (angles, position ) changed
// If you return NO, then nodeToParentTransform won't be called.
bool PhysicsSprite::isDirty(void)
{
    return true;
}

// returns the transform matrix according the Chipmunk Body values
CCAffineTransform PhysicsSprite::nodeToParentTransform(void)
{
    b2Vec2 pos  = m_pBody->GetPosition();

    float x = pos.x * PTM_RATIO;
    float y = pos.y * PTM_RATIO;

    if ( isIgnoreAnchorPointForPosition() ) {
        x += m_obAnchorPointInPoints.x;
        y += m_obAnchorPointInPoints.y;
    }

    // Make matrix
    float radians = m_pBody->GetAngle();
    float c = cosf(radians);
    float s = sinf(radians);

    if( ! m_obAnchorPointInPoints.equals(CCPointZero) ){
        x += c*-m_obAnchorPointInPoints.x + -s*-m_obAnchorPointInPoints.y;
        y += s*-m_obAnchorPointInPoints.x + c*-m_obAnchorPointInPoints.y;
    }

    // Rot, Translate Matrix
    m_sTransform = CCAffineTransformMake( c,  s,
        -s,    c,
        x,    y );

    return m_sTransform;
}

HelloWorld::HelloWorld()
{
    setTouchEnabled( true );
    setAccelerometerEnabled( true );

    CCSize s = CCDirector::sharedDirector()->getWinSize();
    // init physics
    this->initPhysics();

    CCSpriteBatchNode *parent = CCSpriteBatchNode::create("blocks.png", 100);
    m_pSpriteTexture = parent->getTexture();

    addChild(parent, 0, kTagParentNode);
    
    mLevelHelperLoader = new LevelHelperLoader("TestLevel.plhs");
    mLevelHelperLoader->addObjectsToWorld(world, this);
    mLevelHelperLoader->createPhysicBoundaries(world);
    mLevelHelperLoader->useLevelHelperCollisionHandling();
    
    mSpriteCamera = mLevelHelperLoader->spriteWithUniqueName("camera");
    
    player01Run();
    
    mOriginalPosition = mLevelHelperLoader->spriteWithUniqueName("player")->getBody()->GetPosition();
    
    ::CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
                                                           callfuncO_selector(HelloWorld::AnimationEndedNotification),
                                                           LHAnimationHasEndedNotification,
                                                           NULL);
    
    
    mLevelHelperLoader->registerBeginOrEndCollisionCallbackBetweenTagA(TAG_PLAYER_01, TAG_ENV_GROUND, this, callfuncO_selector(HelloWorld::beginOrEndCollisionBetweenMarioAndCoin));
    mLevelHelperLoader->registerBeginOrEndCollisionCallbackBetweenTagA(TAG_CAMERA, TAG_MAP_MARKER, this, callfuncO_selector(HelloWorld::beginOrEndCollisionBetweenCameraAndMarke));
    
//    CCArray* grounds = mLevelHelperLoader->spritesWithTag(TAG_ENV_GROUND);
//    for (int i=0; i<grounds->count(); ++i)
//    {
//        LHSprite* sprite = (LHSprite*)grounds->objectAtIndex(i);
//        sprite->getBody()->SetLinearVelocity(b2Vec2(-2,0));
//    }

    
//    cs::ArmatureDataManager::sharedArmatureDataManager()->addArmatureFileInfo("Armature/Dragon.png", "Armature/Dragon.plist", "Armature/Dragon.xml");
//    
//    cs::Armature *armature = cs::Armature::create("Dragon");
//	armature->getAnimation()->playByIndex(1);
//	armature->getAnimation()->setAnimationScale(0.1f);
//	armature->setPosition(s.width/2,0);
//	addChild(armature);
    
    generateNextScreenMap(0);
    
    scheduleUpdate();
}

HelloWorld::~HelloWorld()
{
    mLevelHelperLoader->cancelBeginOrEndCollisionCallbackBetweenTagA(TAG_PLAYER_01, TAG_ENV_GROUND);
    mLevelHelperLoader->cancelBeginOrEndCollisionCallbackBetweenTagA(TAG_CAMERA, TAG_MAP_MARKER);
    
    delete world;
    world = NULL;
    
    delete mLevelHelperLoader;
    mLevelHelperLoader = NULL;
    
    delete m_debugDraw;
}

void HelloWorld::initPhysics()
{
    CCSize s = CCDirector::sharedDirector()->getWinSize();

    b2Vec2 gravity;
    gravity.Set(0.0f, -15.0f);
    world = new b2World(gravity);

    // Do we want to let bodies sleep?
    world->SetAllowSleeping(true);

    world->SetContinuousPhysics(true);

    m_debugDraw = new GLESDebugDraw( LevelHelperLoader::pointsToMeterRatio() );
    world->SetDebugDraw(m_debugDraw);

    uint32 flags = 0;
    flags += b2Draw::e_shapeBit;
    flags += b2Draw::e_jointBit;
    flags += b2Draw::e_aabbBit;
    flags += b2Draw::e_pairBit;
    flags += b2Draw::e_centerOfMassBit;
    m_debugDraw->SetFlags(flags);
}

void HelloWorld::draw()
{
    //
    // IMPORTANT:
    // This is only for debug purposes
    // It is recommend to disable it
    //
    CCLayer::draw();

    ccGLEnableVertexAttribs( kCCVertexAttribFlag_Position );

    kmGLPushMatrix();

    world->DrawDebugData();

    kmGLPopMatrix();
}

void HelloWorld::update(float dt)
{
    //It is recommended that a fixed time step is used with Box2D for stability
    //of the simulation, however, we are using a variable time step here.
    //You need to make an informed choice, the following URL is useful
    //http://gafferongames.com/game-physics/fix-your-timestep/
    
    int velocityIterations = 8;
    int positionIterations = 1;
    
    world->Step(dt, velocityIterations, positionIterations);
    
    //Iterate over the bodies in the physics world
    for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
    {
        if (b->GetUserData() != NULL) {
            //Synchronize the AtlasSprites position and rotation with the corresponding body
            CCSprite* myActor = (CCSprite*)b->GetUserData();
            myActor->setPosition(LevelHelperLoader::metersToPoints(b->GetPosition()));
            myActor->setRotation( -1 * CC_RADIANS_TO_DEGREES(b->GetAngle()) );
        }    
    }
    
    //--------------------------------------------------------------
    // player update
    //--------------------------------------------------------------
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player");
    
    b2Vec2 desireVelocity = b2Vec2(RUN_VELOCITY,0);
    b2Vec2 currentVelocity = player01->getBody()->GetLinearVelocity();
    
    b2Vec2 deltaVelocity = desireVelocity - currentVelocity;
    b2Vec2 acceleration = dt * deltaVelocity;
    
    player01->getBody()->SetLinearVelocity(currentVelocity+acceleration);
    
    //--------------------------------------------------------------
    // camera update
    //--------------------------------------------------------------
    updateCamera(this->getCamera());
    
    //--------------------------------------------------------------
    // map update
    //--------------------------------------------------------------
    if (mIsRequestNextScreenMap)
    {
        generateNextScreenMap(mNextScreenMapStartX);
        mIsRequestNextScreenMap = false;
    }
}

void HelloWorld::ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent)
{
    if (mPlayer01State == PS_RUN)
    {
        player01Jump();
    }
}

void HelloWorld::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
}

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // add layer as a child to scene
    CCLayer* layer = new HelloWorld();
    scene->addChild(layer);
    layer->release();
    
    return scene;
}

void HelloWorld::beginOrEndCollisionBetweenMarioAndCoin(LHContactInfo* contact)
{
    //CCLog("BEGIN OR END Mario ... Coin");
    
    if (contact->contactType == LH_BEGIN_CONTACT)
    {
        if (mPlayer01State == PS_JUMP)
        {
            player01Land();
        }
    }
}

void HelloWorld::beginOrEndCollisionBetweenCameraAndMarke(LHContactInfo* contact)
{
    if (contact->contactType == LH_BEGIN_CONTACT)
    {
        LHSprite* markeSprite = contact->spriteA()->getTag() == TAG_MAP_MARKER ? contact->spriteA() : contact->spriteB();
        
        float startX = markeSprite->getPosition().x;
        requestNextScreenMap(startX);
    }
}

void HelloWorld::AnimationEndedNotification(LHSprite* sprite)
{
//    CCLog("Animation has ended");
//    CCLog("Sprite Name %s", sprite->getUniqueName().c_str());
//    CCLog("Animation Name %s", sprite->animationName().c_str());
//    CCLog("..............................................................");
    
    if (mPlayer01State == PS_LAND && sprite->animationName() == "char01_land")
    {
        player01Run();
    }
}

void HelloWorld::updateCamera(CCCamera* camera)
{
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player");
    
    float offsetXToWinCenter = player01->getPosition().x - winSize.width/2;
    
    if (offsetXToWinCenter < 0)
        return;
    
    CCPoint cameraSpritePoint;
    cameraSpritePoint.x = WIN_SIZE_W/2 + offsetXToWinCenter;
    cameraSpritePoint.y = WIN_SIZE_H/2;
    
    mSpriteCamera->setUsesOverloadedTransformations(true);
    mSpriteCamera->setPosition(cameraSpritePoint);
    
    float realCameraX = -WIN_SIZE_W/2+mSpriteCamera->getPosition().x;
    float realCameraY = -WIN_SIZE_H/2+mSpriteCamera->getPosition().y;
    
    camera->setCenterXYZ(realCameraX, realCameraY, 0);
    camera->setEyeXYZ(realCameraX, realCameraY, 1);
}

void HelloWorld::requestNextScreenMap(float startX)
{
    mNextScreenMapStartX = startX;
    mIsRequestNextScreenMap = true;
}

void HelloWorld::generateNextScreenMap(float startX)
{
    for (int i=0; i<4; ++i)
    {
        char name[32];
        sprintf(name, "land01_0%d", i%2+1);
        
        LHSprite* newSprite = mLevelHelperLoader->createSpriteWithName(name, "scene01", "TestData_Miu.pshs");
        newSprite->setTag(TAG_ENV_GROUND);
        
        CCPoint newSpritePoint;
        newSpritePoint.x = startX + newSprite->getContentSize().width*i + (i>=2 ? WIN_SIZE_W/15*2 : 0);
        newSpritePoint.y = newSprite->getContentSize().height/2;
        
        newSprite->transformPosition(newSpritePoint);
    }
    
    LHSprite* newSpriteMarker = mLevelHelperLoader->createSpriteWithName("marker", "common", "TestData_Miu.pshs");
    newSpriteMarker->setTag(TAG_MAP_MARKER);
    newSpriteMarker->transformPosition(CCPoint(startX+WIN_SIZE_W, WIN_SIZE_H/2));
    newSpriteMarker->setVisible(false);
}

void HelloWorld::player01Run()
{
    mPlayer01State = PS_RUN;
    
    LHSprite* sprite = mLevelHelperLoader->spriteWithUniqueName("player");
    
    sprite->prepareAnimationNamed("char01_run", "TestData_Miu.pshs");
    sprite->playAnimation();
}

void HelloWorld::player01Jump()
{
    mPlayer01State = PS_JUMP;
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player");
    
    player01->getBody()->ApplyLinearImpulse(b2Vec2(0,JUMP_VELOCITY), player01->getBody()->GetWorldCenter());
    
    player01->prepareAnimationNamed("char01_jump", "TestData_Miu.pshs");
    player01->playAnimation();
}

void HelloWorld::player01Land()
{
    mPlayer01State = PS_LAND;
    
    LHSprite* sprite = mLevelHelperLoader->spriteWithUniqueName("player");
    
    sprite->prepareAnimationNamed("char01_land", "TestData_Miu.pshs");
    sprite->playAnimation();
}

void HelloWorld::player02Run()
{
    
}

void HelloWorld::player02Jump()
{
    
}
