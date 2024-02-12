/*
    bunny object
*/
#include "Bunny.h"

using namespace cocos2d;

/*
    costruttore
*/
Bunny::Bunny() {}

/*
    distruttore
*/
Bunny::~Bunny() {}

/*
    creazione di un nuovo coniglietto
*/
Bunny* Bunny::create( int tile_x, int tile_y, int orientation, int tileCenterYOffset )
{
    Bunny*  bunny;

    // istanzia un nuovo personaggio
    bunny = new Bunny();

    // se l'allocazione va a buon fine...
    if( bunny != nullptr ) {

        // inizializza la sprite del coniglietto con l'immagine di default
        if( bunny->initWithFile( "images/bunny/default.png" ) ) {
            // rilascio automatico della memoria
            bunny->autorelease();
            // imposta il punto di ancoraggio che e' sempre in basso al centro
            bunny->setAnchorPoint( Vec2( 0.5f, 0 ) );
            // per default il coniglietto salta sul posto
            bunny->status = bunnystate_idle;
            // imposta le coordinate della tile occupata
            bunny->tile_x = tile_x;
            bunny->tile_y = tile_y;
            // imposta l'orientamento del coniglietto
            bunny->orientation = orientation;
            // imposta l'offset verticale per centrare il coniglietto nella tile
            bunny->tileCenterYOffset = tileCenterYOffset;

        } else {
            CC_SAFE_DELETE( bunny );
        }
    }
    return bunny;
}

/*
    imposta l'orientamento (su, giu', destra, sinistra)
*/
void Bunny::setOrientation( int orientation )
{
    this->orientation = orientation;
}

/*
    ritorna l'orientamento
*/
int Bunny::getOrientation()
{
    return this->orientation;
}

/*
    imposta la cella della mappa da occupare
*/
void Bunny::setTile( int tile_x, int tile_y )
{
    this->tile_x = tile_x;
    this->tile_y = tile_y;
}

/*
    ritorna la cella della mappa occupata
*/
void Bunny::getTile( int *tile_x, int *tile_y )
{
    *tile_x = this->tile_x;
    *tile_y = this->tile_y;
}

/*
    ritorna la coordinata x della cella occupata
*/
int Bunny::getTileX()
{
    return this->tile_x;
}

/*
    ritorna la coordinata y della cella occupata
*/
int Bunny::getTileY()
{
    return this->tile_y;
}


/*
    imposta lo stato di movimento
*/
void Bunny::setStatus( int status )
{
    this->status = status;
}

/*
    ritorna la cella della mappa occupata
*/
bool Bunny::getStatus()
{
    return this->status;
}

/*
    imposta lo scostamento y dal centro della cella
*/
void Bunny::setTileCenterYOffset( int offset )
{
    this->tileCenterYOffset = offset;
}

/*
    ritorna lo scostamento y dal centro della cella
*/
int Bunny::getTileCenterYOffset()
{
    return this->tileCenterYOffset;
}





