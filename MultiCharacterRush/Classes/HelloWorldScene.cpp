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
#include "LHSettings.h"

using namespace cocos2d;
using namespace CocosDenshion;

int EnvFootstepLength[] =
{
    2, // diff level 0
    0, // diff level 1
    0, // diff level 2
    0, // diff level 3
};

int EnvGroundLength[] =
{
    1, // diff level 0
    0, // diff level 1
    0, // diff level 2
    0, // diff level 3
};

#define DEBUG_DRAW() 0

#define PTM_RATIO 32

#define JUMP01_VELOCITY 26
#define JUMP02_VELOCITY 26
#define RUN_VELOCITY 12

#define PLAY01_GRAVITY_SCALE 1

#define TOTAL_SCREEN_WIDTH() ((int)(WIN_SIZE_W*mScreenCount))

#define WIN_SIZE_W \
CCDirector::sharedDirector()->getWinSize().width

#define WIN_SIZE_H \
CCDirector::sharedDirector()->getWinSize().height

#define WIN_SIZE \
CCDirector::sharedDirector()->getWinSize()

#define BG01_CONTENT_WIDTH 413.75
#define BG01_CONTENT_HEIGHT 155.25

#define BG02_CONTENT_WIDTH 331.25
#define BG02_CONTENT_HEIGHT 266.75

#define ENV_BASE_BLOCK_SIZE 32.0f
#define ENV_FOOTSTEP_BLOCK_SIZE 32.0f
#define ENV_GROUND_BLOCK_SIZE 96.0f

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
, mScreenCount(0)
, mLevelDifficulty(0)
, mIsCameraMoving(false)
, mPlayer01Jump02Count(0)
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

bool HelloWorld::init()
{
    return initWithColor(ccc4(255, 0, 0, 255));
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
    if (DEBUG_DRAW())
    {
        CCLayer::draw();
        
        ccGLEnableVertexAttribs( kCCVertexAttribFlag_Position );
        
        kmGLPushMatrix();
        
        world->DrawDebugData();
        
        kmGLPopMatrix();
    }
}


void HelloWorld::LoadLevel()
{
    if (mLevelHelperLoader != NULL)
    {
        delete mLevelHelperLoader;
        mLevelHelperLoader = NULL;
    }
    
    // init
    mScreenCount = 0;
    mIsCameraMoving = false;
    mLevelDifficulty = 0;
    mPlayer01Jump02Count = 1;
    
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
    mLevelHelperLoader->registerBeginOrEndCollisionCallbackBetweenTagA(TAG_PLAYER_01, TAG_ENV_FOOTSTEP, this, callfuncO_selector(HelloWorld::beginOrEndCollisionBetweenMarioAndCoin));
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
    // BG update
    //--------------------------------------------------------------
    updateBG();
    
    //--------------------------------------------------------------
    // player01 update
    //--------------------------------------------------------------
    updatePlayer01(dt);
    
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
    
    updateFootstep();
}

void HelloWorld::ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent)
{
    if (mPlayer01State == PS_RUN || mPlayer01State == PS_LAND)
    {
        player01Jump01();
    }else if (mPlayer01State == PS_JUMP01 || mPlayer01State == PS_JUMP02 || mPlayer01State == PS_FALLING)
    {
        if (mPlayer01Jump02Count != 0)
        {
            player01Jump02();
        }else
        {
            player01Holding();
        }
    }
}

void HelloWorld::ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent)
{
//    if (mPlayer01State == PS_RUN || mPlayer01State == PS_LAND)
//    {
//        player01Jump01();
//    }else if (mPlayer01State == PS_JUMP01)
//    {
//        player01Jump02();
//    }
}

void HelloWorld::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
    if (mPlayer01State == PS_HOLDING)
    {
        player01Falling();
    }
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
        if (mPlayer01State == PS_FALLING || mPlayer01State == PS_HOLDING)
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

void HelloWorld::EnableCollision(LHSprite* sprite)
{
    b2Fixture* curFix = sprite->getBody()->GetFixtureList();
    while (curFix)
    {
        b2Filter filter;
        filter.categoryBits = 1;
        filter.maskBits     = 65535;
        filter.groupIndex   = 0;
        
        curFix->SetFilterData(filter);
        curFix = curFix->GetNext();
    }
}

void HelloWorld::DisableCollision(LHSprite* sprite)
{
    b2Fixture* curFix = sprite->getBody()->GetFixtureList();
    while (curFix)
    {
        b2Filter filter;
        filter.categoryBits = 0;
        filter.maskBits     = 0;
        filter.groupIndex   = 0;
        
        curFix->SetFilterData(filter);
        curFix = curFix->GetNext();
    }
}

void HelloWorld::updateBG()
{
    if (!mIsCameraMoving)
        return;
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    b2Vec2 BGVelocity = player01->getBody()->GetLinearVelocity();
    BGVelocity.y = 0;
    
    CCArray* sprites = mLevelHelperLoader->spritesWithTag(TAG_BG_01);
    for (int i=0; i<sprites->count(); ++i)
    {
        ((LHSprite*)sprites->objectAtIndex(i))->getBody()->SetLinearVelocity(0.75*BGVelocity);
    }
    sprites = mLevelHelperLoader->spritesWithTag(TAG_BG_02);
    for (int i=0; i<sprites->count(); ++i)
    {
        ((LHSprite*)sprites->objectAtIndex(i))->getBody()->SetLinearVelocity(1.0*BGVelocity);
    }
}

void HelloWorld::updatePlayer01(float dt)
{
    // velocity
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    b2Vec2 desireVelocity = b2Vec2(RUN_VELOCITY,0);
    b2Vec2 currentVelocity = player01->getBody()->GetLinearVelocity();
    
    b2Vec2 deltaVelocity = desireVelocity - currentVelocity;
    b2Vec2 acceleration = dt * deltaVelocity;
    
    b2Vec2 newVelocity = currentVelocity+2.0*acceleration;
    
    player01->getBody()->SetLinearVelocity(newVelocity);
    
    // state
    if (mPlayer01State == PS_JUMP01 || mPlayer01State == PS_JUMP02)
    {
        if (player01->getBody()->GetLinearVelocity().y <= 0)
        {
            player01Falling();
        }
    }
}

void HelloWorld::updateFootstep()
{
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    CCArray* sprites = mLevelHelperLoader->spritesWithTag(TAG_ENV_FOOTSTEP);
    for (int i=0; i<sprites->count(); ++i)
    {
        LHSprite* sprite = (LHSprite*)sprites->objectAtIndex(i);
        if (player01->getPosition().y > sprite->getPosition().y + player01->getContentSize().height/2 + 2/*offset*/)
        {
            EnableCollision(sprite);
        }else
        {
            DisableCollision(sprite);
        }
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
    
    if (!mIsCameraMoving)
        mIsCameraMoving = true;
    
    CCPoint cameraSpritePoint;
    cameraSpritePoint.x = WIN_SIZE_W/2 + offsetXToWinCenter;
    cameraSpritePoint.y = WIN_SIZE_H/2;
    
    mSpriteCamera->transformPosition(cameraSpritePoint);
    
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

void HelloWorld::gnerateTriggers(float startX)
{
    LHSprite* newSpriteMarker = mLevelHelperLoader->createSpriteWithName("marker", "common", "GameData.pshs");
    newSpriteMarker->setTag(TAG_MAP_MARKER);
    newSpriteMarker->transformPosition(CCPoint(startX+WIN_SIZE_W, WIN_SIZE_H/2));
    newSpriteMarker->setVisible(true);
    
    LHSprite* newSpriteGhost = mLevelHelperLoader->createSpriteWithName("ghost", "common", "GameData.pshs");
    newSpriteGhost->setTag(TAG_GHOST_ZONE);
    newSpriteGhost->transformPosition(CCPoint(startX+WIN_SIZE_W/2, 0));
    newSpriteGhost->transformScaleY(10);
    newSpriteGhost->transformScaleX(200);
    newSpriteGhost->setVisible(false);
}

void HelloWorld::generateBG(float startX)
{
    if (DEBUG_DRAW())
        return;
    
    int index = 0;
    
    float lastBG01PointX = getLastBG01Point().x;
    float lastBG01PointY = getLastBG01Point().y;
    while (lastBG01PointX < TOTAL_SCREEN_WIDTH())
    {
        LHSprite* newSpriteBG01 = mLevelHelperLoader->createSpriteWithName("bg01", "bg01", "GameData.pshs");
        
        /*offset 2 to avoid blank space*/
        float bgX = lastBG01PointX + BG01_CONTENT_WIDTH - 2;
        float bgY = lastBG01PointY;
        
        newSpriteBG01->transformPosition(CCPoint(bgX,bgY));
        newSpriteBG01->setTag(TAG_BG_01);
        newSpriteBG01->setZOrder(-1);
        
        lastBG01PointX = newSpriteBG01->getPosition().x;
        ++index;
    }
    
    index = 0;
    
    float lastBG02PointX = getLastBG02Point().x;
    float lastBG02PointY = getLastBG02Point().y;
    while (lastBG02PointX < TOTAL_SCREEN_WIDTH())
    {
        LHSprite* newSpriteBG02 = mLevelHelperLoader->createSpriteWithName("bg02", "bg01", "GameData.pshs");
        
        /*offset 2 to avoid blank space*/
        float bgX = lastBG02PointX + BG02_CONTENT_WIDTH - 2;
        float bgY = lastBG02PointY;
        
        newSpriteBG02->transformPosition(CCPoint(bgX,bgY));
        newSpriteBG02->setTag(TAG_BG_02);
        newSpriteBG02->setZOrder(-2);
        
        lastBG02PointX = newSpriteBG02->getPosition().x;
        ++index;
    }
}

void HelloWorld::generateFootsetp(float startX)
{
    int randNumber = -mMapBlockSize + (arc4random()%(mMapBlockSize*2));
    
    float height = WIN_SIZE_H/2;
    
    CCPoint spriteHeadPos;
    spriteHeadPos.x = (startX+WIN_SIZE_W/2) + randNumber;
    spriteHeadPos.y = height;
    
    LHSprite* spriteHead = mLevelHelperLoader->createSpriteWithName("land03_01", "scene01", "GameData.pshs");
    spriteHead->setTag(TAG_ENV_FOOTSTEP);
    spriteHead->transformPosition(spriteHeadPos);
    
    for (int i=0; i<EnvFootstepLength[mLevelDifficulty]; ++i)
    {
        CCPoint spriteBodyPos;
        spriteBodyPos.x = spriteHeadPos.x+ENV_FOOTSTEP_BLOCK_SIZE+ENV_FOOTSTEP_BLOCK_SIZE*i;
        spriteBodyPos.y = height;
        
        LHSprite* spriteBody = mLevelHelperLoader->createSpriteWithName("land03_02", "scene01", "GameData.pshs");
        spriteBody->setTag(TAG_ENV_FOOTSTEP);
        spriteBody->transformPosition(spriteBodyPos);
    }
    
    CCPoint spriteEndPos;
    spriteEndPos.x = spriteHeadPos.x+ENV_FOOTSTEP_BLOCK_SIZE*(EnvFootstepLength[mLevelDifficulty]+1);
    spriteEndPos.y = height;
    
    LHSprite* spriteEnd = mLevelHelperLoader->createSpriteWithName("land03_03", "scene01", "GameData.pshs");
    spriteEnd->setTag(TAG_ENV_FOOTSTEP);
    spriteEnd->transformPosition(spriteEndPos);
}

void HelloWorld::generateGround(float startX)
{
    static int lastType = -1;
    
    float lastEnvGroundPointX = getLastEnvGroundPoint().x;
    while (lastEnvGroundPointX < TOTAL_SCREEN_WIDTH())
    {
        int type = arc4random()%3;
        while (lastType == type)
            type = arc4random()%3;
        
        lastType = type;
        
        float height = WIN_SIZE_H/2 - ENV_BASE_BLOCK_SIZE*6 + ENV_GROUND_BLOCK_SIZE/2 + type*ENV_BASE_BLOCK_SIZE;
        
        // head
        CCPoint spriteHeadPos;
        spriteHeadPos.x = lastEnvGroundPointX + ENV_GROUND_BLOCK_SIZE + ENV_BASE_BLOCK_SIZE*(2+arc4random()%2);
        spriteHeadPos.y = height;
        
        LHSprite* spriteHead = mLevelHelperLoader->createSpriteWithName("land01_01", "scene01", "GameData.pshs");
        spriteHead->setTag(TAG_ENV_GROUND);
        spriteHead->transformPosition(spriteHeadPos);
        
        for (int i=0; i<type; ++i)
        {
            LHSprite* spriteFoot = mLevelHelperLoader->createSpriteWithName("land02_02", "scene01", "GameData.pshs");
            spriteFoot->setTag(TAG_ENV_GROUND);
            spriteFoot->transformPosition(CCPoint(spriteHeadPos.x, spriteHeadPos.y-ENV_GROUND_BLOCK_SIZE/2-ENV_BASE_BLOCK_SIZE/2 - ENV_BASE_BLOCK_SIZE*i));
        }
        
        // body
        for (int i=0; i<EnvGroundLength[mLevelDifficulty]; ++i)
        {
            CCPoint spriteBodyPos;
            spriteBodyPos.x = spriteHeadPos.x+ENV_GROUND_BLOCK_SIZE+ENV_GROUND_BLOCK_SIZE*i;
            spriteBodyPos.y = height;
            
            LHSprite* spriteBody = mLevelHelperLoader->createSpriteWithName("land01_02", "scene01", "GameData.pshs");
            spriteBody->setTag(TAG_ENV_GROUND);
            spriteBody->transformPosition(spriteBodyPos);
            
            for (int i=0; i<type; ++i)
            {
                LHSprite* spriteFoot = mLevelHelperLoader->createSpriteWithName("land02_04", "scene01", "GameData.pshs");
                spriteFoot->setTag(TAG_ENV_GROUND);
                spriteFoot->transformPosition(CCPoint(spriteBodyPos.x, spriteBodyPos.y-ENV_GROUND_BLOCK_SIZE/2-ENV_BASE_BLOCK_SIZE/2 - ENV_BASE_BLOCK_SIZE*i));
            }
        }

        // end
        CCPoint spriteEndPos;
        spriteEndPos.x = spriteHeadPos.x+ENV_GROUND_BLOCK_SIZE*(EnvGroundLength[mLevelDifficulty]+1);
        spriteEndPos.y = height;
        
        LHSprite* spriteEnd = mLevelHelperLoader->createSpriteWithName("land01_03", "scene01", "GameData.pshs");
        spriteEnd->setTag(TAG_ENV_GROUND);
        spriteEnd->transformPosition(spriteEndPos);
        
        for (int i=0; i<type; ++i)
        {
            LHSprite* spriteFoot = mLevelHelperLoader->createSpriteWithName("land02_03", "scene01", "GameData.pshs");
            spriteFoot->setTag(TAG_ENV_GROUND);
            spriteFoot->transformPosition(CCPoint(spriteEndPos.x, spriteEndPos.y-ENV_GROUND_BLOCK_SIZE/2-ENV_BASE_BLOCK_SIZE/2 - ENV_BASE_BLOCK_SIZE*i));
        }

        lastEnvGroundPointX = spriteEndPos.x;
    }
}

void HelloWorld::generateNextScreenMap(float startX)
{
    cleanOutofScreenMap();
    
    CCLOG("win w=%f, h%f", WIN_SIZE_W, WIN_SIZE_H);
    
    mScreenCount++;
    
    CCLog("startX=%f", startX);
    
    gnerateTriggers(startX);
    
    generateBG(startX);
    
    generateFootsetp(startX);
    
    generateGround(startX);
}

void HelloWorld::cleanOutofScreenMap()
{
    CCArray* sprites = mLevelHelperLoader->allSprites();
    for (int i=0; i<sprites->count(); ++i)
    {
        LHSprite* sprite = (LHSprite*)sprites->objectAtIndex(i);
        
        if (sprite->getPosition().x < mSpriteCamera->getPosition().x - WIN_SIZE_W*2)
        {
            sprite->removeSelf();
        }
    }
}

CCPoint HelloWorld::getLastBG01Point()
{
    CCPoint result;
    result.x = -WIN_SIZE_W;
    result.y = WIN_SIZE_H - BG01_CONTENT_HEIGHT - (BG02_CONTENT_HEIGHT-BG01_CONTENT_HEIGHT)/2;
    
    CCArray* sprites = mLevelHelperLoader->spritesWithTag(TAG_BG_01);
    for (int i=0; i<sprites->count(); ++i)
    {
        CCPoint point = ((LHSprite*)sprites->objectAtIndex(i))->getPosition();
        if (point.x >= result.x)
        {
            result = point;
        }
    }
    
    return result;
}

CCPoint HelloWorld::getLastBG02Point()
{
    CCPoint result;
    result.x = -WIN_SIZE_W;
    result.y = WIN_SIZE_H - BG02_CONTENT_HEIGHT/2;
    
    CCArray* sprites = mLevelHelperLoader->spritesWithTag(TAG_BG_02);
    for (int i=0; i<sprites->count(); ++i)
    {
        CCPoint point = ((LHSprite*)sprites->objectAtIndex(i))->getPosition();
        if (point.x >= result.x)
        {
            result = point;
        }
    }
    
    return result;
}

CCPoint HelloWorld::getLastEnvGroundPoint()
{
    CCPoint result;
    result.x = -WIN_SIZE_W;
    result.y = WIN_SIZE_H - BG02_CONTENT_HEIGHT/2;
    
    CCArray* sprites = mLevelHelperLoader->spritesWithTag(TAG_ENV_GROUND);
    for (int i=0; i<sprites->count(); ++i)
    {
        CCPoint point = ((LHSprite*)sprites->objectAtIndex(i))->getPosition();
        if (point.x >= result.x)
        {
            result = point;
        }
    }
    
    return result;
}

void HelloWorld::player01Run()
{
    mPlayer01State = PS_RUN;
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    player01->getBody()->SetGravityScale(PLAY01_GRAVITY_SCALE);
    
    player01->stopAnimation();
    player01->prepareAnimationNamed("char01_run", "GameData.pshs");
    player01->playAnimation();
    
    CCLOG("RUN~~~~~~~~~~~");
}

void HelloWorld::player01Jump01()
{
    mPlayer01State = PS_JUMP01;
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    player01->getBody()->SetGravityScale(PLAY01_GRAVITY_SCALE);
    
    player01->getBody()->ApplyLinearImpulse(b2Vec2(0,JUMP01_VELOCITY), player01->getBody()->GetWorldCenter());
    
    player01->stopAnimation();
    player01->prepareAnimationNamed("char01_jump01", "GameData.pshs");
    player01->playAnimation();
    
    CCLOG("JUMP01!!!!!!!!!!!!");
}

void HelloWorld::player01Jump02()
{
    mPlayer01State = PS_JUMP02;
    mPlayer01Jump02Count--;
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    player01->getBody()->SetGravityScale(PLAY01_GRAVITY_SCALE);
    
    float jump02Velocity = -player01->getBody()->GetLinearVelocity().y+JUMP02_VELOCITY;
    player01->getBody()->ApplyLinearImpulse(b2Vec2(0,jump02Velocity), player01->getBody()->GetWorldCenter());
    
    player01->stopAnimation();
    player01->prepareAnimationNamed("char01_jump02", "GameData.pshs");
    player01->playAnimation();
    
    CCLOG("JUMP02!!!!!!!!!!!!");
}

void HelloWorld::player01Falling()
{
    mPlayer01State = PS_FALLING;
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    player01->getBody()->SetGravityScale(PLAY01_GRAVITY_SCALE);
    
//    player01->stopAnimation();
//    player01->prepareAnimationNamed("char01_fall", "GameData.pshs");
//    player01->playAnimation();
    
    CCLOG("FALL!!!!!!!!!!!!");
}

void HelloWorld::player01Land()
{
    mPlayer01State = PS_LAND;
    
    mPlayer01Jump02Count=1;
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    player01->getBody()->SetGravityScale(PLAY01_GRAVITY_SCALE);
    
    player01->stopAnimation();
    player01->prepareAnimationNamed("char01_land", "GameData.pshs");
    player01->playAnimation();
    
    CCLOG("LAND!!!!!!!!!!!!");
}

void HelloWorld::player01Holding()
{
    mPlayer01State = PS_HOLDING;
    
    LHSprite* player01 = mLevelHelperLoader->spriteWithUniqueName("player01");
    
    player01->getBody()->SetGravityScale(PLAY01_GRAVITY_SCALE*0.2);
    
    player01->stopAnimation();
    player01->prepareAnimationNamed("char01_holding", "GameData.pshs");
    player01->playAnimation();
    
    CCLOG("HOLDING!!!!!!!!!!!!");
}

void HelloWorld::player02Run()
{
    
}

void HelloWorld::player02Jump()
{
    
}
