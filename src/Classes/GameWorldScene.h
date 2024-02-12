#ifndef __GAMEWORLD_SCENE_H__
#define __GAMEWORLD_SCENE_H__

#include "cocos2d.h"
#include "Bunny.h"
#include "Enemy.h"

USING_NS_CC;

// dimensione dello schermo
#define SCREEN_WIDTH                640
#define SCREEN_HEIGHT               480

// numero massimo di livelli (visibile anche dal menu)
#define MAX_LEVELS  30


/*
    tag degli oggetti figli della scena
*/
enum {
    tag_gamelayer   = 0,    // layer di gioco
    tag_background,         // immagine di sfondo dietro la mappa
    tag_clouds,             // immagine nuvole per cambio livello
    tag_menu,               // menu (pulsanti di gioco)
	tag_tilemap,            // mappa tmx
	tag_debuglabel,         // label di debug
	tag_gameinfolabel,      // label di gioco
	tag_tutoriallabel,      // label per tutorial
	tag_rabbit,             // coniglio
	tag_endlevelsprite,     // immagine a fine livello, sopra le nuvole
	tag_endlevelsprite_2nd, // seconda immagine (se presente) a fine livello, sopra le nuvole
};

/*
    tag degli oggetti figli del menu di gioco
*/
enum {
    tag_obstacles_button     = 100,     // tasto di gioco
    tag_obstacles_number,               // numero di ostacoli piazzabili
    tag_obstacles_icon                  // icona ostacoli piazzabili
};


/*
    stati di gioco
*/
typedef enum {
    gs_initializing,
    gs_loading_level,
    gs_get_ready,
    gs_running,
    gs_level_complete,
    gs_game_over,
    gs_final_scene,
    gs_max
} gamestate_t;

/*
    orientamento personaggio
*/
typedef enum {
    orientation_up,
    orientation_right,
    orientation_down,
    orientation_left,
    orientation_max
} orientation_t;

/*
    contenuto della cella nella matrice walkable
*/
typedef enum {
    walktile_vacuum,
    walktile_walkable,
    walktile_obstacle,
    // !!da qui in poi tutti i tipo di scale!!
    walktile_stairs_x2_up,
    walktile_stairs_x2_down,
    walktile_stairs_x2_right,
    walktile_stairs_x2_left,
    walktile_stairs_x4_up,
    walktile_stairs_x4_down,
    walktile_stairs_x4_right,
    walktile_stairs_x4_left,
    walktile_max
} walktile_t;

/*
    coordinate cella
*/
typedef struct {
    int     x;
    int     y;
} tilecoord_t;

/*
    tipi di oggetti collezionabili
    devono essere nell'ordine in cui si trovano nel file tiles_64x64.png
*/
typedef enum {
    pickabletype_coin,
    pickabletype_ice_cream,
    pickabletype_watermelon,
    pickabletype_bomb,
    pickabletype_ananas,
    pickabletype_bee_hive,
    pickabletype_hamburger,
    pickabletype_blue_stone,
    pickabletype_red_stone,
    pickabletype_snake,
    pickabletype_cheese,
    pickabletype_carrots,
    pickabletype_grape,
    pickabletype_strawberry,
    pickabletype_white_grape,
    pickabletype_teleport,
    pickabletype_max
} pickabletype_t;

/*
    tipi di starter
*/
typedef enum {
    startergid_bunny_down,
    startergid_bunny_left,
    startergid_bunny_right,
    startergid_bunny_up,
    startergid_porcupine_down,
    startergid_porcupine_up,
    startergid_porcupine_left,
    startergid_porcupine_right,
    startergid_butterfly_down,
    startergid_butterfly_left,
    startergid_butterfly_up,
    startergid_butterfly_right,
    startergid_snail_down,
    startergid_snail_left,
    startergid_snail_right,
    startergid_snail_up,
    startergid_max
} startergid_t;

/*
    dati oggetto collezionabile
*/
typedef struct {
    int     type;
    int     tag;
} pickable_t;

/*
    motivo game over
*/
enum {
    gameover_explode,
    gameover_fall,
    gameover_block,
    gameover_prick,
    gameover_max
} gameover_t;

/*
    tipi di personaggi animati
*/
typedef enum {
    character_bunny,
    character_porcupine,
    character_butterfly,
    character_snail,
    character_max
} character_t;

/*
    struttura dati posizione di partenza e direzione dei personaggi
*/
typedef struct {
    int     tile_x;
    int     tile_y;
    int     orientation;
    int     character;
} characterstart_t;

/*
    tipo di ostacolo del giocatore
*/
typedef enum {
    userobstacle_mushroom,
    userobstacle_flower_pot,
    userobstacle_max
} userobstacle_t;



// numero massimo tiles orizzontali
#define MAX_H_TILES     100
// numero massimo tiles verticali
#define MAX_V_TILES     100
// numero massimo di personaggi animati (coniglietti, porcospini, ecc.)
#define MAX_CHARACTERS  10


/*
    informazioni mappa
*/
typedef struct {
    float               total_h_tiles;          // numero totale di celle orizzontali
    float               total_v_tiles;          // numero totale di celle verticali
    float               tile_width;             // larghezza della singola tile in pixel
    float               tile_height;            // altezza della singola tile in pixel
    float               half_tile_width;        // meta' larghezza della singola tile in pixel (usata per spostamenti)
    float               half_tile_height;       // meta' altezza della singola tile in pixel (usata per spostamenti)
    float               map_width;              // larghezza totale della mappa (= tile width * numero di tile orizzontali )
    float               map_height;             // altezza totale della mappa (= tile height * numero di tile verticali )
    int                 walkables[ MAX_V_TILES ][ MAX_H_TILES ];    // info tile percorribili, scale e vuoti
    pickable_t          pickables[ MAX_V_TILES ][ MAX_H_TILES ];    // info oggetti collezionabili
    int                 total_pickables_tag;
    int                 total_pickables;
    int                 total_picked;
    tilecoord_t         target_tile;            // coordinate cella target
    int                 user_obstacle_type;     // tipo di ostacolo del giocatore: funghetto, vaso, ecc..
    int                 total_user_obstacles;   // numero totale di oggetti piazzabili dal giocatore
    int                 total_characters;       // numero totale di personaggi animati (conigli, ricci, ecc.)
    characterstart_t    characters[ MAX_CHARACTERS ];   // posizioni di partenza dei personaggi animati
} mapinfo_t;

/*
    combinazioni colore gradiente per sfondo di gioco
*/
typedef enum {
    gradient_blue_pink,
    gradient_blue_green,
    gradient_blue_white,
    gradient_blue_fuchsia,
    gradient_yellow_pink,
    gradient_max
} gradient_t;


class GameWorld : public cocos2d::LayerGradient
{
public:

    // crea la scena di gioco
    static          cocos2d::Scene* createScene();
    // inizializzazione scena di gioco
    virtual bool    init();
    virtual void    onExit() override;

    // implement the "static create()" method manually
    CREATE_FUNC(GameWorld);

    // callback dei tasti di gioco
    void            button_debug_callback(cocos2d::Ref* pSender);
    void            button_close_callback(cocos2d::Ref* pSender);
    void            button_reload_callback(cocos2d::Ref* pSender);


    // touch callback
	void            getTouchedTile( float x, float y, int *tile_x, int *tile_y );
	void            onTouchesBegan(const std::vector<Touch*>& touches, Event  *event);
	void            onTouchesMoved(const std::vector<Touch*>& touches, Event  *event);
	void            onTouchesEnded(const std::vector<Touch*>& touches, Event  *event);
	void            doubleTap( float x, float y );
	void            zoomIn( float, float );
	void            zoomOut( float, float );
    void            zoomEnd();

    // selezione cella
    void            addUserObstacle( int, int );

    // info mappa
    mapinfo_t       mapinfo;

    // ritorna la posizione relativa alla mappa della cella indicata come argomento
    cocos2d::Vec2   getTilePosition( int, int, int );
    int             getTileDepth( int, int );
    bool            isTileInMap( int, int );
    bool            isTileWalkable( int, int );
    bool            isTileVacuum( int tile_x, int tile_y );
    bool            isTileSelectable( int, int );

    void            updateBunny( Node *node );
    void            updateBunnyZOrder( Node *node );
    void            pickObject( Bunny *bunny );
    void            bunnyExplode( Bunny *bunny );
    void            endOfBunnyExplosion();
    int             getBunnyAction( Bunny *bunny );
    void            jumpBunny( Bunny*, int, int, int, int );
    void            teleportBunny( Bunny *bunny );

    void            getNextTile( int, int, int, int*, int* );
    void            getNextTileStairs( int, int, int, int, int*, int* );

    void            createEnemy( TMXTiledMap *, characterstart_t *, int, int );
    void            enemyIdle( Enemy *enemy );
    void            enemyMove( Enemy *enemy );
    void            enemyExplode( Enemy *enemy );
    void            enemyExplosionEnd( Node *node );
    void            enemyLocked( Enemy *enemy );
    void            updateEnemy( Node *node );
    void            updateEnemyZOrder( Node *node );
    int             getEnemyAction( Enemy *enemy );

    void            setDebugLabelText( const char *text );
    void            setTutorialLabelText( const char *text );
    void            setGameInfoLabelText( const char *text, bool );
    void            setObstaclesNumberLabelText( int );

    void            setBackgroundColor( const Color3B& top, const Color3B& bottom );
    void            setGradientIndex( int );
    void            resetCloud( Node *node );
    float           getCloudRandTime();
    float           getCloudStartTime( float posx );
    float           getCloudRandYPos();
    bool            loadLevel( int level );
    void            goToNewLevel();
    void            gameOver( int );
    void            removeEndLevelSprite();


    int             gameState = 0;
    void            enterGameState( int );

    void            getReady();
    void            startGame();
    void            loadNextLevel();
    void            reloadLevel();
    void            startFinalCutScene();
    void            cutSceneCallback();


private:

    bool    zooming             = false;

    // coordinate ultima cella selezionata per posizionamento oggetti
    int     last_tile_x         = -1;
    int     last_tile_y         = -1;

    // load a level
    int     currentLevel        = 0;

    bool    teleporting         = false;
    int     cutSceneStatus      = 0;
    int     gradient_id         = 0;
};

#endif // __GAMEWORLD_SCENE_H__
