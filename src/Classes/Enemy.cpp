/*
    enemy object
*/
#include "Enemy.h"

using namespace cocos2d;

/*
    costruttore
*/
Enemy::Enemy() {}

/*
    distruttore
*/
Enemy::~Enemy() {}

/*
    creazione di un nuovo coniglietto
*/
Enemy* Enemy::create( int type, const char* defaultImage, int tile_x, int tile_y, int orientation, int tileCenterYOffset )
{
    Enemy*      enemy;

    if( !defaultImage ) {
        return 0;
    }

    // istanzia un nuovo personaggio
    enemy = new Enemy();

    // se l'allocazione va a buon fine...
    if( enemy != nullptr ) {

        // inizializza la sprite del coniglietto con l'immagine di default
        if( enemy->initWithFile( defaultImage ) ) {
            // rilascio automatico della memoria
            enemy->autorelease();
            // imposta il punto di ancoraggio che e' sempre in basso al centro
            enemy->setAnchorPoint( Vec2( 0.5f, 0 ) );
            // imposta il tipo di nemico
            enemy->type         = type;
            // per default il nemico ha lo stato di idle
            enemy->status       = enemystate_idle;
            // imposta le coordinate della tile occupata
            enemy->tile_x           = tile_x;
            enemy->tile_y           = tile_y;
            // le coordinate della tile occupata precedentemente coincidono con quella di partenza
            enemy->previous_tile_x  = tile_x;
            enemy->previous_tile_y  = tile_y;
            // imposta l'orientamento del coniglietto
            enemy->orientation  = orientation;
            // imposta l'offset verticale per centrare il coniglietto nella tile
            enemy->tileCenterYOffset = tileCenterYOffset;

        } else {
            CC_SAFE_DELETE( enemy );
        }
    }
    return enemy;
}

/*
    imposta il tipo di nemico (porcospino, farfalla, ecc)
*/
void Enemy::setType( int type )
{
    this->type = type;
}

/*
    ritorna il tipo di nemico (porcospino, farfalla, ecc)
*/
int Enemy::getType()
{
    return this->type;
}


/*
    imposta l'orientamento (su, giu', destra, sinistra)
*/
void Enemy::setOrientation( int orientation )
{
    this->orientation = orientation;
}

/*
    ritorna l'orientamento
*/
int Enemy::getOrientation()
{
    return this->orientation;
}

/*
    imposta la cella della mappa da occupare
*/
void Enemy::setTile( int tile_x, int tile_y )
{
    this->tile_x = tile_x;
    this->tile_y = tile_y;
}

/*
    ritorna la cella della mappa occupata
*/
void Enemy::getTile( int *tile_x, int *tile_y )
{
    *tile_x = this->tile_x;
    *tile_y = this->tile_y;
}

/*
    ritorna la coordinata x della cella occupata
*/
int Enemy::getTileX()
{
    return this->tile_x;
}

/*
    ritorna la coordinata y della cella occupata
*/
int Enemy::getTileY()
{
    return this->tile_y;
}

/*
    imposta la cella occupata prima dello spostamento
*/
void Enemy::setPreviousTile( int tile_x, int tile_y )
{
    this->previous_tile_x = tile_x;
    this->previous_tile_y = tile_y;
}

/*
    ritorna la cella occupata prima dello spostamento
*/
void Enemy::getPreviousTile( int *tile_x, int *tile_y )
{
    *tile_x = this->previous_tile_x;
    *tile_y = this->previous_tile_y;
}

/*
    ritorna la coordinata x della cella occupata precedentemente allo spostamento
*/
int Enemy::getPreviousTileX()
{
    return this->previous_tile_x;
}

/*
    ritorna la coordinata y della cella occupata precedentemente allo spostamento
*/
int Enemy::getPreviousTileY()
{
    return this->previous_tile_y;
}


/*
    imposta lo stato di movimento
*/
void Enemy::setStatus( int status )
{
    log( "Enemy new status %d", status );
    this->status = status;
}

/*
    ritorna la cella della mappa occupata
*/
bool Enemy::getStatus()
{
    return this->status;
}

/*
    imposta lo scostamento y dal centro della cella
*/
void Enemy::setTileCenterYOffset( int offset )
{
    this->tileCenterYOffset = offset;
}

/*
    ritorna lo scostamento y dal centro della cella
*/
int Enemy::getTileCenterYOffset()
{
    return this->tileCenterYOffset;
}





