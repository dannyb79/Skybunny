#include "cocos2d.h"
#include "AnimationManager.h"

USING_NS_CC;
using namespace ui;



static AnimationManager*    instance = NULL;

AnimationManager::AnimationManager()
{
    // costruttore dell'oggetto (lasciare vuoto)
}

AnimationManager* AnimationManager::getInstance()
{
    if( instance == NULL ) {
        instance = new AnimationManager();
    }
    return instance;
}

/*
    carica animazione da file plist e file immagine sprite sheet
*/
bool AnimationManager::loadFromPlist( int animation_id, const char *plistfilepath, const char *framepattern, int start_frame, int end_frame, float duration )
{
    char    filename[ 50 ];
    bool    result          = false;
    int     total_frames    = 0;
    // crea l'oggetto animazione
    animations[ animation_id ] = Animation::create();
    // se l'oggetto e' stato creato con successo...
    if( animations[ animation_id ] != nullptr ) {
        // aggiunge alla cache il file plist
        auto cache = SpriteFrameCache::getInstance();
        cache->addSpriteFramesWithFile( plistfilepath );
        // scansiona tutti i frame
        for( int i = start_frame; i <= end_frame; i++ ) {
            // crea il nome del frame secondo il pattern
            sprintf( filename, framepattern, i );
            // crea lo sprite frame da aggiungere alla cache
            SpriteFrame *spriteFrame = cache->getSpriteFrameByName( filename );
            // se lo sprite frame e' stato creato con successo...
            if( spriteFrame != nullptr ) {
                // lo aggiunge all'animazione corrente
                animations[ animation_id ]->addSpriteFrame( spriteFrame );
                // rimuove lo sprite frame dalla cache
                cache->removeSpriteFrameByName( filename );
            } else {
                log( "Errore! sprite frame %s non caricato", filename );
            }
        }
        // calcolo numero totale di frames per il calcolo del delay di ogni frame
        total_frames = end_frame - start_frame;
        animations[ animation_id ]->setDelayPerUnit( duration / total_frames );
        animations[ animation_id ]->setRestoreOriginalFrame(false);
        animations[ animation_id ]->retain();
        result = true;
    }
    return result;
}

bool AnimationManager::loadAnimation( int id )
{
    char filename[ 50 ];
    bool result = false;

    switch( id ) {

        case animation_bunny_left:
            result = loadFromPlist( id, "images/bunny/jump/left/bunny_left.plist", "bunny_left_%02d.png", 1, 33, 1.0f );
            break;
        case animation_bunny_down:
            result = loadFromPlist( id, "images/bunny/jump/down/bunny_down.plist", "bunny_down_%02d.png", 1, 33, 1.0f );
            break;
        case animation_bunny_right:
            result = loadFromPlist( id, "images/bunny/jump/right/bunny_right.plist", "bunny_right_%02d.png", 1, 33, 1.0f );
            break;
        case animation_bunny_up:
            result = loadFromPlist( id, "images/bunny/jump/up/bunny_up.plist", "bunny_up_%02d.png", 1, 33, 1.0f );
            break;
        case animation_bunny_burning:
            result = loadFromPlist( id, "images/bunny/burning/bunny_burning.plist", "bunnyburning_00%02d.png", 0, 20, 1.0f );
        break;

        case animation_teleport:
            result = loadFromPlist( id, "images/objects/teleport/teleport.plist", "teleport_%02d.png", 0, 19, 1.2f );
        break;
        case animation_carrots:
            result = loadFromPlist( id, "images/objects/carrots/carrots.plist", "carrots_%02d.png", 0, 16, 1.0f );
        break;
        case animation_cheese:
            result = loadFromPlist( id, "images/objects/cheese/cheese.plist", "cheese_%02d.png", 0, 16, 1.0f );
        break;
        case animation_snake:
            result = loadFromPlist( id, "images/objects/snake/snake.plist", "snake_%02d.png", 0, 19, 1.0f );
        break;
        case animation_red_stone:
            result = loadFromPlist( id, "images/objects/red_stone/red_stone.plist", "red_stone_%02d.png", 0, 19, 1.0f );
        break;
        case animation_blue_stone:
            result = loadFromPlist( id, "images/objects/blue_stone/blue_stone.plist", "blue_stone_%02d.png", 0, 19, 1.0f );
        break;
        case animation_hamburger:
            result = loadFromPlist( id, "images/objects/hamburger/hamburger.plist", "hamburger_%02d.png", 0, 17, 1.0f );
        break;
        case animation_bee_hive:
            result = loadFromPlist( id, "images/objects/bee_hive/bee_hive.plist", "bee_hive_%02d.png", 0, 37, 2.0f );
        break;
        case animation_ananas:
            result = loadFromPlist( id, "images/objects/ananas/ananas.plist", "ananas_%02d.png", 0, 10, 1.0f );
        break;
        case animation_flower:
            result = loadFromPlist( id, "images/objects/flower/flower.plist", "flower_%02d.png", 0, 15, 0.8f );
        break;
        case animation_mushroom:
            result = loadFromPlist( id, "images/objects/mushroom/mushroom.plist", "mushroom_%02d.png", 0, 20, 1.4f );
            break;
        case animation_bunny_coin:
            result = loadFromPlist( id, "images/objects/coin/bunnycoin_spritesheet.plist", "bunnycoin00%02d.png", 0, 16, 0.8f );
        break;
        case animation_ice_cream:
            result = loadFromPlist( id, "images/objects/ice_cream/icecream.plist", "icecream_00%02d.png", 1, 13, 0.8f );
        break;
        case animation_strawberry:
            result = loadFromPlist( id, "images/objects/strawberry/strawberry.plist", "strawberry_%02d.png", 1, 14, 1.0f );
        break;
        case animation_grape:
            result = loadFromPlist( id, "images/objects/grape/grape.plist", "grape_%02d.png", 0, 19, 1.0f );
        break;
        case animation_white_grape:
            result = loadFromPlist( id, "images/objects/white_grape/white_grape.plist", "white_grape_%02d.png", 0, 14, 1.0f );
        break;
        case animation_watermelon:
            result = loadFromPlist( id, "images/objects/watermelon/watermelon.plist", "watermelon_%02d.png", 1, 14, 0.8f );
            break;
        case animation_bomb:
            result = loadFromPlist( id, "images/objects/bomb/bomb.plist", "bomba_00%02d.png", 1, 13, 1.0f );
            break;
        case animation_explosion:
            result = loadFromPlist( id, "images/objects/explosion/explosion.plist", "explosion_%02d.png", 1, 16, 0.5f );
            break;
        case animation_porcupine_down:
            result = loadFromPlist( id, "images/porcupine/porcupine_down.plist", "porcupine_down_%02d.png", 0, 12, 1.0f );
            break;
        case animation_porcupine_up:
            result = loadFromPlist( id, "images/porcupine/porcupine_up.plist", "porcupine_up_%02d.png", 0, 12, 1.0f );
            break;
        case animation_porcupine_right:
            result = loadFromPlist( id, "images/porcupine/porcupine_right.plist", "porcupine_right_%02d.png", 0, 12, 1.0f );
            break;
        case animation_porcupine_left:
            result = loadFromPlist( id, "images/porcupine/porcupine_left.plist", "porcupine_left_%02d.png", 0, 12, 1.0f );
            break;

        case animation_butterfly_up:
            result = loadFromPlist( id, "images/butterfly/butterfly_up.plist", "butterfly_up_%02d.png", 0, 10, 0.5f );
            break;
        case animation_butterfly_down:
            result = loadFromPlist( id, "images/butterfly/butterfly_down.plist", "butterfly_down_%02d.png", 0, 10, 0.5f );
            break;
        case animation_butterfly_left:
            result = loadFromPlist( id, "images/butterfly/butterfly_left.plist", "butterfly_left_%02d.png", 0, 10, 0.5f );
            break;
        case animation_butterfly_right:
            result = loadFromPlist( id, "images/butterfly/butterfly_right.plist", "butterfly_right_%02d.png", 0, 10, 0.5f );
            break;

        case animation_snail_up:
            result = loadFromPlist( id, "images/snail/snail_up.plist", "snail_up_%02d.png", 0, 10, 1.5f );
            break;
        case animation_snail_down:
            result = loadFromPlist( id, "images/snail/snail_down.plist", "snail_down_%02d.png", 0, 10, 1.5f );
            break;
        case animation_snail_left:
            result = loadFromPlist( id, "images/snail/snail_left.plist", "snail_left_%02d.png", 0, 10, 1.5f );
            break;
        case animation_snail_right:
            result = loadFromPlist( id, "images/snail/snail_right.plist", "snail_right_%02d.png", 0, 10, 1.5f );
            break;

    }
    return result;
}

/*
    scarica tutte le animazioni
*/
void AnimationManager::unloadAll()
{
    for( int i = 0; i <= animation_max; i++ ) {
        if( animations[ i ] != nullptr ) {
            animations[ i ]->release();
        }
    }
}
