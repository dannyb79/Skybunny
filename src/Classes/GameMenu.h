#ifndef __GAMEMENU_SCENE_H__
#define __GAMEMENU_SCENE_H__

#include "cocos2d.h"

USING_NS_CC;


class GameMenu : public cocos2d::LayerColor
{
public:

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();

    // implement the "static create()" method manually
    CREATE_FUNC( GameMenu );

    virtual void onEnter() override;

private:

    const char  *button_level_normal        = "images/buttons/button_level_normal.png";
    const char  *button_level_pressed       = "images/buttons/button_level_pressed.png";
    const char  *button_level_disabled      = "images/buttons/button_level_disabled.png";

    #define MAX_LEVEL_STARTS_IMAGES         4

    const char  *level_stars_images[ MAX_LEVEL_STARTS_IMAGES ] = {
        "images/buttons/level_stars_0.png",
        "images/buttons/level_stars_1.png",
        "images/buttons/level_stars_2.png",
        "images/buttons/level_stars_3.png",
    };

    bool        levels_created = false;
    bool        createLevelButtons();
    bool        updateLevelButtons();
    int         getStarsFromProgress( int );
    int         getGameCompletion();

    void        startFromLevel( int );
    void        exitGame();

    bool        sliding = false;
    void        endSliding();
};

#endif // __GAMEMENU_SCENE_H__
