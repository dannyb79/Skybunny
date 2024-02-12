#ifndef __LOADINGSCENE_SCENE_H__
#define __LOADINGSCENE_SCENE_H__

#include "cocos2d.h"

USING_NS_CC;

typedef enum {
    loadingstate_animations,
    loadingstate_complete,
    loadingstate_max
} loadingstate_t;

class LoadingScene : public cocos2d::Layer
{
public:

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();

    // implement the "static create()" method manually
    CREATE_FUNC( LoadingScene );

    void update( float delta );

    void updateProgressBar( int );


private:

    int     loading_state;
    int     animation_id;
    int     progress;

};

#endif // __GAMEWORLD_SCENE_H__
