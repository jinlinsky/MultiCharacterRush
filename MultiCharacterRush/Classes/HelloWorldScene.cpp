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

#define JUMP_VELOCITY 20
#define RUN_VELOCITY 12

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
: mLevelHelperLoader(NULL)
{
    setTouchEnabled( true );
    setAccelerometerEnabled( true );
    
    // init physics
    this->initPhysics();
    
    if(WIN_SIZE_W == 480 || WIN_SIZE_H == 480)
    {
        mMapBlockSize = 32;
    }
    else if(WIN_SIZE_W == 640 || WIN_SIZE_H == 640)
    {
        mMapBlockSize = 64;
    }
    else if(WIN_SIZE_W == 1024 || WIN_SIZE_H == 1024)
    {
        mMapBlockSize = 64;
    }
    else if(WIN_SIZE_W == 2048 || WIN_SIZE_H == 2048)
    {
        mMapBlockSize = 128;
    }
    
    LevelHelperLoader::dontStretchArt();
    
    mRequestLoadLevel = true;
    
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
    b2Vec2 gravity;
    gravity.Set(0.0f, -35.0f);
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
//    CCLayer::draw();
//
//    ccGLEnableVertexAttribs( kCCVertexAttribFlag_Position );
//
//    kmGLPushMatrix();
//
//    world->DrawDebugData();
//
//    kmGLPopMatrix();
}


void HelloWorld::LoadLevel()
{
    if (mLevelHelperLoader != NULL)
    {
        delete mLevelHelperLoader;
        mLevelHelperLoader = NULL;
    }
    
    mLevelHelperLoader = new LevelHelperLoader("./GameData/Cooked/FirstPlayable.plhs");
    mLevelHelperLoader->addObjectsToWorld(world, this);
    mLevelHelperLoader->createPhysicBoundaries(world);
    mLevelHelperLoader->useLevelHelperCollisionHandling();
    
    mSpriteCamera = mLevelHelperLoader->spriteWithUniqueName("camera");
    
    player01Run();
    
    mOriginalPosition = mLevelHelperLoader->spriteWithUniqueName("player01")->getBody()->GetPosition();
    
    ::CCNotificationCenter::sharedNotificationCenter()->addObserver(this,
                                                                    callfuncO_selector(HelloWorld::AnimationEndedNotification),
                                                                    LHAnimationHasEndedNotification,
                                                                    NULL);
    
    
    mLevelHelperLoader->registerBeginOrEndCollisionCallbackBetweenTagA(TAG_PLAYER_01, TAG_ENV_GROUND, this, callfuncO_selector(HelloWorld::beginOrEndCollisionBetweenMarioAndCoin));
    mLevelHelperLoader->registerBeginOrEndCollisionCallbackBetweenTagA(TAG_PLAYER_01, TAG_GHOST_ZONE, this, callfuncO_selector(HelloWorld::beginOrEndCollisionBetweenPlayer01AndGhostZone));
    mLevelHelperLoader->registerBeginOrEndCollisionCallbackBetweenTagA(TAG_CAMERA, TAG_MAP_MARKER, this, callfuncO_selector(HelloWorld::beginOrEndCollisionBetweenCameraAndMarke));
    
    generateNextScreenMap(0);
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
    
    if (mRequestLoadLevel)
    {
        LoadLevel();
        
        mRequestLoadLevel = false;
    }
    
    //--------------------------------------------------------------
    // player update
    //--------------------------------------------------------------
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    b2Vec2 desireVelocity = b2Vec2(RUN_VELOCITY,0);
    b2Vec2 currentVelocity = player01->getBody()->GetLinearVelocity();
    
    b2Vec2 deltaVelocity = desireVelocity - currentVelocity;
    b2Vec2 acceleration = dt * deltaVelocity;
    
    player01->getBody()->SetLinearVelocity(currentVelocity+2.0*acceleration);
    
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
    if (mPlayer01State == PS_RUN || mPlayer01State == PS_LAND)
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

void HelloWorld::beginOrEndCollisionBetweenPlayer01AndGhostZone(LHContactInfo* contact)
{
    if (contact->contactType == LH_BEGIN_CONTACT)
    {
        mRequestLoadLevel = true;
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
    camera->setCenterXYZ(0, 0, 0);
    camera->setEyeXYZ(0, 0, 1);
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    float offsetXToWinCenter = player01->getPosition().x - WIN_SIZE_W/2;
    
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
#include "LHSettings.h"
void HelloWorld::generateNextScreenMap(float startX)
{
    cleanOutofScreenMap();
    
    CCLOG("win w=%f, h%f", WIN_SIZE_W, WIN_SIZE_H);
    
    for (int i=0; i<4; ++i)
    {
        char name[32];
        sprintf(name, "land01_0%d", i%2+1);
        
        LHSprite* newSprite = mLevelHelperLoader->createSpriteWithName(name, "scene01", "GameData.pshs");
        newSprite->setTag(TAG_ENV_GROUND);
        
        CCLOG("content w=%f h=%f", newSprite->getContentSize().width, newSprite->getContentSize().height);
        
        CCLOG("ptm radio=%f", LHSettings::sharedInstance()->lhPtmRatio());
        
        CCPoint newSpritePoint;
        newSpritePoint.x = startX + WIN_SIZE_W/2 - mMapBlockSize*6 + mMapBlockSize*3*i;// + (i >=2 ? mMapBlockSize*2 : 0);
        newSpritePoint.y = WIN_SIZE_H/2 - mMapBlockSize*3;
        
        newSprite->transformPosition(newSpritePoint);
    }
    
    LHSprite* newSpriteMarker = mLevelHelperLoader->createSpriteWithName("marker", "common", "GameData.pshs");
    newSpriteMarker->setTag(TAG_MAP_MARKER);
    newSpriteMarker->transformPosition(CCPoint(startX+WIN_SIZE_W, WIN_SIZE_H/2));
    newSpriteMarker->setVisible(true);
    
    LHSprite* newSpriteGhost = mLevelHelperLoader->createSpriteWithName("ghost", "common", "GameData.pshs");
    newSpriteGhost->setTag(TAG_GHOST_ZONE);
    newSpriteGhost->transformPosition(CCPoint(startX+WIN_SIZE_W/2, WIN_SIZE_H/2 - mMapBlockSize*4));
    newSpriteGhost->transformScaleY(10);
    newSpriteGhost->transformScaleX(200);
    newSpriteGhost->setVisible(false);
}

void HelloWorld::cleanOutofScreenMap()
{
    CCArray* sprites = mLevelHelperLoader->spritesWithTag(TAG_ENV_GROUND);
    for (int i=0; i<sprites->count(); ++i)
    {
        LHSprite* sprite = (LHSprite*)sprites->objectAtIndex(i);
        
        if (sprite->getPosition().x < mSpriteCamera->getPosition().x - WIN_SIZE_W/2)
        {
            sprite->removeSelf();
        }
    }
}

void HelloWorld::player01Run()
{
    mPlayer01State = PS_RUN;
    
    LHSprite* sprite = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    sprite->prepareAnimationNamed("char01_run", "GameData.pshs");
    sprite->playAnimation();
    
    CCLOG("RUN~~~~~~~~~~~");
}

void HelloWorld::player01Jump()
{
    mPlayer01State = PS_JUMP;
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    player01->getBody()->ApplyLinearImpulse(b2Vec2(0,JUMP_VELOCITY), player01->getBody()->GetWorldCenter());
    
    player01->prepareAnimationNamed("char01_jump", "GameData.pshs");
    player01->playAnimation();
    
    CCLOG("JUMP!!!!!!!!!!!!");
}

void HelloWorld::player01Land()
{
    mPlayer01State = PS_LAND;
    
    LHSprite* sprite = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    sprite->prepareAnimationNamed("char01_land", "GameData.pshs");
    sprite->playAnimation();
}

void HelloWorld::player02Run()
{
    
}

void HelloWorld::player02Jump()
{
    
}
