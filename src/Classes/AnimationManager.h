#ifndef __ANIMATIONMANAGER_H__
#define __ANIMATIONMANAGER_H__

#include "cocos2d.h"

USING_NS_CC;

/*
    indici animazioni
*/
typedef enum {
    animation_bunny_up,
    animation_bunny_right,
    animation_bunny_down,
    animation_bunny_left,
    animation_flower,
    animation_mushroom,
    animation_bunny_coin,
    animation_ice_cream,
    animation_strawberry,
    animation_grape,
    animation_white_grape,
    animation_watermelon,
    animation_teleport,
    animation_bomb,
    animation_ananas,
    animation_bee_hive,
    animation_hamburger,
    animation_blue_stone,
    animation_red_stone,
    animation_carrots,
    animation_snake,
    animation_cheese,
    animation_bunny_burning,
    animation_explosion,
    animation_porcupine_up,
    animation_porcupine_down,
    animation_porcupine_left,
    animation_porcupine_right,
    animation_butterfly_up,
    animation_butterfly_down,
    animation_butterfly_left,
    animation_butterfly_right,
    animation_snail_up,
    animation_snail_down,
    animation_snail_left,
    animation_snail_right,
    animation_max,
} animation_t;

class AnimationManager
{
public:
    // recupero istanza dell'oggetto (singletons)
    static AnimationManager*    getInstance();

    // array contenente le animazioni caricate
    cocos2d::Animation*         animations[ animation_max ];

    // carica un'animazione
    bool                        loadAnimation( int animation_id );

    // scarica tutte le animazioni
    void                        unloadAll();

private:
    // costruttore
    AnimationManager();

    // carica da file plist
    bool                        loadFromPlist( int, const char* , const char*, int, int, float );
};

#endif // __ANIMATIONMANAGER_H__
