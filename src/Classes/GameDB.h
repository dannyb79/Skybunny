#ifndef __GAMEDB_H__
#define __GAMEDB_H__

#include "cocos2d.h"

USING_NS_CC;

class GameDB
{
public:
    // recupero istanza dell'oggetto (singletons)
    static GameDB*      getInstance();

    // ritorna il numero di livelli sbloccati
    int                 getTotalUnlockedLevels();
    // imposta il numero di livelli sbloccati (solo debug)
    void                setTotalUnlockedLevels( int );

    // ritorna la percentuale di progressione del livello
    int                 getLevelProgress( int );
    // imposta la percentuale di progressione del livello
    void                setLevelProgress( int , int );


private:
    // costruttore
    GameDB();

    // chiavi per salvataggio dati
    const char          *GAMEDB_TOTAL_UNLOCKED_LEVELS   =  "GAMEDB_TOTAL_UNLOCKED_LEVELS";
    const char          *GAMEDB_LEVEL_PROGRESS          =  "GAMEDB_LEVEL_PROGRESS_%d";

};

#endif // __GAMEDB_H__
