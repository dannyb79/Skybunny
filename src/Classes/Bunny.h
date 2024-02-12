/*
    bunny object
*/
#ifndef __BUNNY_H__
#define __BUNNY_H__

#include "cocos2d.h"

typedef enum {
    bunnystate_idle,
    bunnystate_moving,
    bunnystate_stop,
    bunnystate_max
} bunnystate_t;

class Bunny : public cocos2d::Sprite
{
public:
    Bunny();
    ~Bunny();
    static Bunny* create( int, int, int, int );


    void    setOrientation( int orientation );
    int     getOrientation();

    void    setTile( int tile_x, int tile_y );
    void    getTile( int *tile_x, int *tile_y );
    int     getTileX();
    int     getTileY();

    void    setTileCenterYOffset( int offset );
    int     getTileCenterYOffset();

    void    setStatus( int );
    bool    getStatus();

private:

    int     orientation;
    int     tile_x;
    int     tile_y;
    int     tileCenterYOffset;

    int     status;

};

#endif // __BUNNY_H__
