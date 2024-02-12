/*
    enemy object
*/
#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "cocos2d.h"

typedef enum {
    enemystate_idle,
    enemystate_moving,
    enemystate_locked,
    enemystate_stop,
    enemystate_max
} enemystate_t;

class Enemy : public cocos2d::Sprite
{
public:
    Enemy();
    ~Enemy();
    static Enemy* create( int, const char*, int, int, int, int );


    void    setOrientation( int orientation );
    int     getOrientation();

    void    setType( int );
    int     getType();

    void    setTile( int, int );
    void    getTile( int*, int* );
    int     getTileX();
    int     getTileY();

    void    setPreviousTile( int, int );
    void    getPreviousTile( int*, int* );
    int     getPreviousTileX();
    int     getPreviousTileY();

    void    setTileCenterYOffset( int offset );
    int     getTileCenterYOffset();

    void    setStatus( int );
    bool    getStatus();

private:

    int     type;

    int     tileCenterYOffset;
    int     orientation;
    int     tile_x;
    int     tile_y;
    int     previous_tile_x;
    int     previous_tile_y;

    int     status;

};

#endif // __ENEMY
