#include "cocos2d.h"
#include "GameDB.h"

USING_NS_CC;
using namespace ui;

static GameDB*    instance = NULL;

GameDB::GameDB()
{
    // costruttore dell'oggetto (lasciare vuoto)
}

GameDB* GameDB::getInstance()
{
    if( instance == NULL ) {
        instance = new GameDB();
    }
    return instance;
}

/*
    ritorna il massimo livello sbloccato
*/
int GameDB::getTotalUnlockedLevels()
{
    return UserDefault::getInstance()->getIntegerForKey( GAMEDB_TOTAL_UNLOCKED_LEVELS, 0 );
}

/*
    imposta il massimo livello sbloccato
*/
void GameDB::setTotalUnlockedLevels( int levels )
{
    UserDefault::getInstance()->setIntegerForKey( GAMEDB_TOTAL_UNLOCKED_LEVELS, levels );
}

/*
    ritorna la massima percentuale di completamento per il livello indicato
*/
int GameDB::getLevelProgress( int level )
{
    char tmpstr[ 50 ];
    sprintf( tmpstr, GAMEDB_LEVEL_PROGRESS, level );
    return UserDefault::getInstance()->getIntegerForKey( tmpstr, 0 );
}

/*
    imposta la massima percentuale di completamento per il livello indicato
*/
void GameDB::setLevelProgress( int level, int progress )
{
    char tmpstr[ 50 ];
    sprintf( tmpstr, GAMEDB_LEVEL_PROGRESS, level );
    return UserDefault::getInstance()->setIntegerForKey( tmpstr, progress );
}

