//
//  MultiCharacterRushAppDelegate.cpp
//  MultiCharacterRush
//
//  Created by jinlin on 13-8-13.
//  Copyright __MyCompanyName__ 2013年. All rights reserved.
//

#include "AppDelegate.h"

#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "HelloWorldScene.h"

USING_NS_CC;
using namespace CocosDenshion;

AppDelegate::AppDelegate()
{

}

AppDelegate::~AppDelegate()
{
}

bool AppDelegate::applicationDidFinishLaunching()
{
    // initialize director
    CCDirector *pDirector = CCDirector::sharedDirector();
    pDirector->setOpenGLView(CCEGLView::sharedOpenGLView());

    // turn on display FPS
    pDirector->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    pDirector->setAnimationInterval(1.0 / 60);
    
    float scrWidth = CCEGLView::sharedOpenGLView()->getFrameSize().width;
    float scrHeight= CCEGLView::sharedOpenGLView()->getFrameSize().height;
    
    std::vector<std::string> res;
    res.push_back("./GameData/Cooked");
    
    if(scrWidth == 480 || scrHeight == 480){
        
        res.push_back("./GameData/Cooked/sd");
        
        CCFileUtils::sharedFileUtils()->setSearchPaths(res);
        CCDirector::sharedDirector()->setContentScaleFactor(1.0f);
    }
    else if(scrWidth == 640 || scrHeight == 640){ //check for both iphone4/4S and iphone5
        res.push_back("./GameData/Cooked/hd");
        
        CCFileUtils::sharedFileUtils()->setSearchPaths(res);
        CCDirector::sharedDirector()->setContentScaleFactor(2.0f);
    }
    else if(scrWidth == 1024 || scrHeight == 1024){
        
        res.push_back("./GameData/Cooked/hd");
        CCFileUtils::sharedFileUtils()->setSearchPaths(res);
        // on ipad the rect (textture coordinate) * 2 already, to check the following function
        // CCRect LHSettings::transformedTextureRect(CCRect rect, const std::string& image)
        CCDirector::sharedDirector()->setContentScaleFactor(1.0f); 
    }
    else if(scrWidth == 2048 || scrHeight == 2048){
        
        res.push_back("./GameData/Cooked/ipadhd");
        
        CCFileUtils::sharedFileUtils()->setSearchPaths(res);
        // on ipad the rect (textture coordinate) * 2 already, to check the following function
        // CCRect LHSettings::transformedTextureRect(CCRect rect, const std::string& image)
        CCDirector::sharedDirector()->setContentScaleFactor(2.0f);
    }

    // create a scene. it's an autorelease object
    CCScene *pScene = HelloWorld::scene();

    // run
    pDirector->runWithScene(pScene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground()
{
    CCDirector::sharedDirector()->stopAnimation();
    SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
    SimpleAudioEngine::sharedEngine()->pauseAllEffects();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
    CCDirector::sharedDirector()->startAnimation();
    SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
    SimpleAudioEngine::sharedEngine()->resumeAllEffects();
}
