/*
    the Game World
*/


#include "Bunny.h"
#include "Enemy.h"
#include "GameWorldScene.h"
#include "ui/UIButton.h"
#include "AnimationManager.h"
#include "GameDB.h"
#include "SimpleAudioEngine.h"


USING_NS_CC;
using namespace ui;
using namespace CocosDenshion;



/*
    livello di profondita' dei figli dell'oggetto layer (GameWorld)

    OGGETTO                         TAG                 Z ORDER
    sprite animazione fine livello  tag_endlevelsprite  1001
    nuvole per cambio livello       tag_clouds          1000
    menu (tasti giocatore)          tag_menu            1
    label info gioco                tag_gameinfolabel   1
    label debug                     tag_debuglabel      1
    mappa                           tag_tilemap         0
    sfondo animato dietro la mappa  tag_background      -1000

    dati dei figli dell'oggetto mappa TMX

    OGGETTO                         TAG         Z ORDER
    layer "basement"                0           0
    layer "walkable"                1           1
    layer "objects"                 2           2   (rimosso dopo posizionamento)
    sprite selettore                5           (total_h_tiles * total_v_tiles)
    sprite oggetto target           6           1+  (profondita' tile occupata)
    sprites ostacoli giocatore      7           1+  (profondita' tile occupata)
    coniglietti (max 1)             10          1+  (profondita' tile occupata)
    oggetti collezionabili (max 18) 12~29       1+  (profondita' tile occupata)
    ...
*/

#define SPRITE_TAG_TARGET           6
#define SPRITE_TAG_USER_OBSTACLE    7
#define SPRITE_TAG_MAP_OBSTACLE     8
#define BUNNY_TAG                   10
#define PICKABLE_TAG_MIN            12
#define PICKABLE_TAG_MAX            29
#define ENEMY_TAG_MIN               30
#define ENEMY_TAG_MAX               39

#define STATIC_WALL_TAG_MIN         100


/*
    intervalli GID
    1   ~   10      ostacoli fissi
    11  ~   20      oggetti collezionabili scala x2 sinistra
    21  ~   30      scala x2 su
    31  ~   40      targets
    41  ~   50      piastrelle camminabili
    51  ~   60      mura
    61  ~   70      oggetti collezionabili
    ...
    ATTENZION! la GID 0 non e' utilizzabile, e' riservata alle tile vuote!!
*/
#define OBSTACLE_GID_MIN            1
#define OBSTACLE_GID_MAX            10
#define PICKABLE_GID_MIN            11
#define PICKABLE_GID_MAX            30
#define STAIRS_X2_LEFT_GID_MIN      31
#define STAIRS_X2_LEFT_GID_MAX      40
#define STAIRS_X2_UP_GID_MIN        41
#define STAIRS_X2_UP_GID_MAX        50
#define STARTER_GID_MIN             51
#define STARTER_GID_MAX             70
#define TARGETS_GID_MIN             71
#define TARGETS_GID_MAX             80
#define WALKABLE_GID_MIN            81
#define WALKABLE_GID_MAX            90
#define WALL_GID_MIN                91
#define WALL_GID_MAX                100



#define TIPS_FONT                   "fonts/Marker Felt.ttf"



/*
    creazione della scena di gioco
*/
Scene* GameWorld::createScene()
{
    // crea l'oggetto Scene che dovra' essere passato al Director
    auto scene = Scene::create();
    // crea l'oggetto Layer (GameWorld eridita Layer) e invoca la init()
    auto layer = GameWorld::create();
    // aggiunge l'oggetto GameWorld alla scena
    scene->addChild( layer, 0, tag_gamelayer );
    // return the scene
    return scene;
}

/*
    uscita dalla scena di gioco
*/
void GameWorld::onExit()
{
    LayerGradient::onExit();
    auto audio = SimpleAudioEngine::getInstance();
    audio->stopBackgroundMusic();
}

/*
    imposta il colore gradiente dello sfondo
*/
void GameWorld::setBackgroundColor( const Color3B& top, const Color3B& bottom )
{
    // colore della parte superiore dello sfondo che sfuma...
    this->setStartColor( top );
    // ...nel colore della parte inferiore dello sfondo
    this->setEndColor( bottom );
}

/*
    imposta il colore gradiente di fondo secondo l'indice passato come argomento
*/
void GameWorld::setGradientIndex( int id )
{
    switch( id ) {
        // azzurro - rosa
        case gradient_blue_pink:
            setBackgroundColor( Color3B( 0, 219, 255 ), Color3B( 255, 219, 255 ) );
            break;
        // azzurro - verde
        case gradient_blue_green:
            setBackgroundColor( Color3B( 167, 255, 255 ), Color3B( 167, 255, 0 ) );
            break;
        // azzurro - bianco
        case gradient_blue_white:
            setBackgroundColor( Color3B( 25, 234, 242 ), Color3B( 255, 255, 255 ) );
            break;
        // blu - fucsia
        case gradient_blue_fuchsia:
            setBackgroundColor( Color3B( 50, 78, 247 ), Color3B( 255, 78, 247 ) );
            break;
        // giallo - rosa
        case gradient_yellow_pink:
            setBackgroundColor( Color3B( 245, 172, 255 ), Color3B( 255, 255, 0 ) );
            break;
        // aggiungere qui altre composizioni di gradienti
        default:
            setBackgroundColor( Color3B( 245, 172, 255 ), Color3B( 255, 255, 0 ) );
            break;
    }
}


// tempo base di una nuvola per l'attraversamento dello schermo
#define CLOUD_BASE_TIME (40.0f)

/*
    ritorna l'intervallo di tempo in cui la nuvola deve uscire dallo schermo alla creazione
*/
float GameWorld::getCloudStartTime( float posx )
{
    return (float)( posx * CLOUD_BASE_TIME / SCREEN_WIDTH );
}

/*
    ritorna l'intervallo di tempo in cui la nuvola deve attraversare lo schermo
*/
float GameWorld::getCloudRandTime()
{
    return (float)( CLOUD_BASE_TIME + rand()%5 );
}

/*
    ritorna una posizione verticale casuale per una nuvola tenendo presente la dimensione dell'immagine
*/
float GameWorld::getCloudRandYPos()
{
    return (float)( rand()%( SCREEN_HEIGHT - 50 ) );
}

/*
    reimposta la posizione della nuvola
*/
void GameWorld::resetCloud( Node *node )
{
    float cloud_y = getCloudRandYPos();
    node->setPosition( Vec2( 740, cloud_y ) );
    auto movecloud  = MoveTo::create( getCloudRandTime(), Vec2( -100, cloud_y ) );
    auto callback   = CCCallFuncN::create( this, CC_CALLFUNCN_SELECTOR( GameWorld::resetCloud ) );
    node->runAction( Sequence::create( movecloud, callback, nullptr )  );
}

/*
    inizializzazione scena di gioco
*/
bool GameWorld::init()
{
    // inizializzazione del layer
    if ( !LayerGradient::init() ) {
        return false;
    }

    // posizione x tasti nella scena di gioco
    #define BUTTONS_POSX        576

    // label con il numero di oggetti piazzabili rimanenti
    auto obstacles_number    = Label::createWithTTF( "", TIPS_FONT, 32 );
	obstacles_number->setAnchorPoint( Vec2( 0.5f, 0 ) );
	obstacles_number->setHorizontalAlignment( TextHAlignment::CENTER );
	obstacles_number->setPosition( Vec2( BUTTONS_POSX + 32, 360 ) );
	obstacles_number->setTextColor( Color4B::MAGENTA );
	obstacles_number->enableGlow( Color4B::YELLOW );
	obstacles_number->enableShadow();
    // imposta il tag per recuperare l'oggetto
	obstacles_number->setTag( tag_obstacles_number );

    // icona oggetti piazzabili dal giocatore
    auto obstacles_icon     = MenuItemSprite::create(
        Sprite::create( "images/thumbs/thumb_mushroom.png" ),
        Sprite::create( "images/thumbs/thumb_mushroom.png" ),
        Sprite::create( "images/thumbs/thumb_mushroom.png" ) );
	obstacles_icon->setAnchorPoint( Vec2::ZERO );
	obstacles_icon->setPosition( Vec2( BUTTONS_POSX - 32, 360 ) );
	obstacles_icon->setTag( tag_obstacles_icon );

/*
    // tasto di debug
    auto button_debug       = MenuItemImage::create(
        "images/buttons/debug_normal.png",
        "images/buttons/debug_normal.png",
        CC_CALLBACK_1(GameWorld::button_debug_callback, this));
	button_debug->setAnchorPoint( Vec2::ZERO );
	button_debug->setPosition( Vec2( BUTTONS_POSX, 128 ) );
*/

    // tasto "chiudi", ritorno al menu
    auto button_close   = MenuItemImage::create(
        "images/buttons/button_closegame.png",
        "images/buttons/button_closegame.png",
        CC_CALLBACK_1(GameWorld::button_close_callback, this));

    button_close->setAnchorPoint( Vec2::ZERO );
	button_close->setPosition( Vec2( BUTTONS_POSX, 0 ) );

    // crea il menu di gioco, ovvero i tasti con cui interagisce il giocatore
    auto menu = Menu::create( /*button_debug,*/ button_close, obstacles_number, obstacles_icon, NULL );
    menu->setPosition( Vec2::ZERO );
    this->addChild( menu, 1, tag_menu );

    // imposta il colore gradiente dello sfondo
    //setBackgroundColor( Color3B::BLUE, Color3B( 50, 210, 200 ) );
    setBackgroundColor( Color3B( 245, 177, 255 ), Color3B( 245, 177, 0 ) );

    // crea le sprite delle nuvole dietro la mappa
    for( int i = 0; i < 20; i++ ) {
        auto cloud = Sprite::create( rand()%2 ? "images/backgrounds/cloud_big.png" : "images/backgrounds/cloud_small.png" );
        cloud->setAnchorPoint( Vec2( Vec2::ZERO ) );
        float cloud_x = rand()%SCREEN_WIDTH;
        float cloud_y = getCloudRandYPos();
        cloud->setPosition( Vec2( cloud_x, cloud_y ) );
        this->addChild( cloud );
        auto movecloud  = MoveTo::create( getCloudStartTime( cloud_x ), Vec2( -100, cloud_y ) );
        auto callback   = CCCallFuncN::create( this, CC_CALLFUNCN_SELECTOR( GameWorld::resetCloud ) );
        cloud->runAction( Sequence::create( movecloud, callback, nullptr )  );
    }


    // crea l'immagine delle nuvole per il cambio di livello
    auto clouds = Sprite::create( "images/backgrounds/clouds.png" );
	clouds->setAnchorPoint( Vec2( 0, 0 ) );
	clouds->setPosition( Vec2( 0, 0 ) );
	this->addChild( clouds, 1000, tag_clouds );

    // listener per gli eventi del touch screen
    auto listener = EventListenerTouchAllAtOnce::create();
    listener->onTouchesMoved = CC_CALLBACK_2( GameWorld::onTouchesMoved, this );
    listener->onTouchesBegan = CC_CALLBACK_2( GameWorld::onTouchesBegan, this );
    listener->onTouchesEnded = CC_CALLBACK_2( GameWorld::onTouchesEnded, this );
    _eventDispatcher->addEventListenerWithSceneGraphPriority( listener, this );


    // crea la label di info gioco ("Get ready...", "Go...", ecc) e la posizione al centro dello schermo
    auto gameLabel = Label::createWithTTF( "", TIPS_FONT, 56 );
    gameLabel->setPosition( Vec2(  SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 ) );
    gameLabel->setTextColor( Color4B::MAGENTA );
    gameLabel->enableShadow();
    this->addChild( gameLabel, 1, tag_gameinfolabel );

    // crea la label per i tips e la posiziona in alto allo schermo
    auto tutLabel = Label::createWithTTF( "", TIPS_FONT, 48 );
    tutLabel->setPosition( Vec2(  SCREEN_WIDTH / 2, 450 ) );
    tutLabel->setTextColor( Color4B::GREEN );
    tutLabel->enableShadow();
    this->addChild( tutLabel, 1, tag_tutoriallabel );

/*
    // crea la label di debug in alto al centro
    auto label = Label::createWithTTF( "Hello World", TIPS_FONT, 24 );
    label->setPosition(Vec2( SCREEN_WIDTH / 2, SCREEN_HEIGHT - label->getContentSize().height ) );
    this->addChild( label, 1, tag_debuglabel );
*/

    // musica sottofondo
    auto audio = SimpleAudioEngine::getInstance();
    audio->playBackgroundMusic( "audio/loop_1.mp3", true );

    return true;
}

/*
    imposta lo stato generale di gioco
*/
void GameWorld::enterGameState( int newState )
{
    const char *gameStateNames[ gs_max ] =
    {
        "Initializing",
        "Loading level",
        "Get Ready",
        "Running",
        "Level complete",
        "Game over",
        "Final scene"
    };

    if( newState >= 0 && newState <= gs_max ) {
        // imposta lo stato generale di gioco con il nuovo stato
        this->gameState = newState;

        // add more action
        switch( newState ) {
            default: break;
        }

        //log( "nuovo stato: %s", gameStateNames[ newState ] );
    }
}

void GameWorld::createEnemy( TMXTiledMap *tilemap, characterstart_t *enemyData, int tileCenterYOffset, int total_enemies )
{
    char defaultImage[ 100 ];
    switch( enemyData->character ) {
        case character_porcupine:   sprintf( defaultImage, "images/porcupine/default.png" );    break;
        case character_snail:       sprintf( defaultImage, "images/snail/default.png" );        break;
        case character_butterfly:   sprintf( defaultImage, "images/butterfly/default.png" );    break;
        default:                    sprintf( defaultImage, "images/porcupine/default.png" );    break;
    }

    // crea l'oggetto "nemico"
    auto enemy = Enemy::create( enemyData->character, defaultImage, enemyData->tile_x, enemyData->tile_y, enemyData->orientation, tileCenterYOffset );
    if( enemy != nullptr ) {
        // ... la aggiunge alla mappa con Z order stabilito dalla profondita'...
        tilemap->addChild( enemy, getTileDepth( enemyData->tile_x, enemyData->tile_y ), ENEMY_TAG_MIN + total_enemies );
        // ... e la posiziona in base alla tile occupata
        enemy->setPosition( getTilePosition( enemyData->tile_x, enemyData->tile_y, tileCenterYOffset ) );
        // anima il nemico secondo lo stato corrente
        updateEnemy( enemy );
    }
}

/*
    carica il livello indicato dal parametro
*/
bool GameWorld::loadLevel( int level )
{
    char        tmpstr[ 50 ];
    Size        visibleSize    = Director::getInstance()->getVisibleSize();
    Vec2        origin         = Director::getInstance()->getVisibleOrigin();
    uint32_t    tileGID;
    int         walkable_max_x  = 0;
    int         walkable_max_y  = 0;
    int         check_tile_x    = 0;
    int         check_tile_y    = 0;

    log( "Loading level %d...", level );

    // imposta lo stato generale di gioco (caricamento livello)
    enterGameState( gs_loading_level );
    // svuota la label di debug
    setDebugLabelText( "" );
    // svuota la label info di gioco
    setGameInfoLabelText( "", false );
    // memorizza il livello corrente
    currentLevel = level;

    // il nome del file tmx legato al livello e' level_X.tmx
    // dove X e' il numero del livello a partire da 0
    sprintf( tmpstr, "maps/level_%d.tmx", level );
    // crea l'oggetto mappa
    auto tilemap = TMXTiledMap::create( tmpstr );
    if( tilemap == nullptr ) {
        log( "Impossibile creare la mappa dal file %s", tmpstr );
        // ritorna al menu
        Director::getInstance()->popScene();
        return false;
    }
    // imposta il punto di ancoraggio della mappa
    tilemap->setAnchorPoint( Vec2::ZERO );


    // !!!!!!! Recupero informazioni dalla mappa !!!!!!!

    // azzera tutti i dati relativi alla mappa corrente
    memset( &mapinfo, 0, sizeof( mapinfo_t ) );
    // recupera tutte le informazioni contenute nella mappa
    // larghezza e altezza della mappa
    mapinfo.map_width           = tilemap->getContentSize().width;
    mapinfo.map_height          = tilemap->getContentSize().height;
    // numero di celle orizzontali e verticali
    mapinfo.total_h_tiles       = tilemap->getMapSize().width;
    mapinfo.total_v_tiles       = tilemap->getMapSize().height;
    // dimensione di ogni cella
    mapinfo.tile_width          = tilemap->getTileSize().width;
    mapinfo.tile_height         = tilemap->getTileSize().height;
    // meta' delle dimensioni di ogni cella (usata per gli spostamenti)
    mapinfo.half_tile_width     = mapinfo.tile_width / 2;
    mapinfo.half_tile_height    = mapinfo.tile_height / 2;
    // con il lato piu' grande viene calcolato il fattore di scala per visualizzare
    // completamente la mappa centrata nello schermo
    if( mapinfo.map_width > mapinfo.map_height ) {
        //log( "mapinfo.map_width > mapinfo.map_height" );
        tilemap->setScale( SCREEN_WIDTH / mapinfo.map_width );
        //log( "initial scale %f",  SCREEN_WIDTH / mapinfo.map_width );
        //tilemap->setPosition( Vec2( 0, origin.y + ( visibleSize.height - mapinfo.map_height * tilemap->getScale() ) / 2 ) );
        tilemap->setAnchorPoint( Vec2::ZERO );
        tilemap->setPosition( Vec2( 0, ( SCREEN_HEIGHT - ( mapinfo.map_height * tilemap->getScale() ) ) / 2 ) );
    } else {
        //log( "mapinfo.map_width < mapinfo.map_height" );
        tilemap->setScale( SCREEN_HEIGHT / mapinfo.map_height );
        tilemap->setPosition( Vec2( origin.x + ( visibleSize.width - mapinfo.map_width * tilemap->getScale() ) / 2, 0 ) );
    }

    /*
        Questi sono i layer che ogni mappa deve avere:
        - objects   -> per disposizione degli oggetti (personaggi, trappole, nemici, ecc.)
        - walkable  -> per info zone percorribili da personaggi
        - basement  -> per costruzioni sotto le zone percorribili
    */

    /*
        ogni mappa dovrebbe avere un layer denominato "basements" contenente le immagini
        delle costruzioni al di sotto delle zone percorribili
    */
    auto basement_layer = tilemap->getLayer( "basement" );
#ifdef COCOS2D_DEBUG
    if( basement_layer != nullptr ) {
        // OK!
        log( "La mappa ha un layer \"basement\" (tag %d)", basement_layer->getTag() );
        // NOTE il contenuto del layer basement e' irrilevante, serve a creare
        // visivamente le basi su cui appoggiano le tile percorribili
    } else {
        // segnala l'errore nel log
        log( "Errore! la mappa dovrebbe avere un layer \"basement\"" );
    }
#endif

    /*
        ogni mappa deve avere un layer denominato "walkable" contenente le informazioni
        sulle tiles dove i personaggi posso muoversi, su eventuali scale e sulle
        posizioni di vuoto
    */
    auto walkable_layer = tilemap->getLayer( "walkable" );
    if( walkable_layer != nullptr ) {
        // scansione di tutte le tiles nel layer "walkable" e impostazione matrice walkables di mapinfo
        // come default, essendo tutti i valori a zero, tutte le celle sono walktile_vacuum
        //log( "Scansione tiles del layer \"walkable\" (tag %d)...", walkable_layer->getTag() );
        for( int i = 0; i < mapinfo.total_v_tiles; i++ ) {
            for( int j = 0; j < mapinfo.total_h_tiles; j++ ) {
                // GID della tile corrente
                tileGID = walkable_layer->getTileGIDAt( Vec2( j, i ) );

                // tiles con GID relativo alle piastrelle calpestabili
                if( tileGID >= WALKABLE_GID_MIN && tileGID <= WALKABLE_GID_MAX ) {
                    // la tile viene marcata come percorribile
                    mapinfo.walkables[ i ][ j ] = walktile_walkable;
                }
                // tiles con GID relativo alle scale verso l'alto
                if( tileGID >= STAIRS_X2_UP_GID_MIN && tileGID <= STAIRS_X2_UP_GID_MAX ) {
                    // controlla se la scala (x2) e' la fine di una scala x4: per saperlo bisogna
                    // controllare se dati x,y correnti la cella x-1, y-2 contiene una scala x2
                    check_tile_x = j - 1;
                    check_tile_y = i - 2;
                    if( isTileInMap( check_tile_x, check_tile_y ) && ( mapinfo.walkables[ check_tile_y ][ check_tile_x ] == walktile_stairs_x2_up ) ) {
                        // e' la fine di una scala x4! imposta la cella corrente...
                        mapinfo.walkables[ i ][ j ]                         = walktile_stairs_x4_up;
                        // azzera quella che era l'inizio della scala x2 (al centro della scala)
                        mapinfo.walkables[ check_tile_y ][ check_tile_x ]   = walktile_vacuum;
                        // reimposta l'inizio della scala
                        mapinfo.walkables[ i - 3 ][ j - 2 ]                 = walktile_stairs_x4_down;
                    } else {
                        // e' una scala x2
                        mapinfo.walkables[ i ][ j ] = walktile_stairs_x2_up;
                        // la cella sopra deve essere per la discesa
                        if( ( ( i - 1 ) >= 0 ) && ( ( j - 1 ) >= 0 ) ) {
                            mapinfo.walkables[ i - 1 ][ j - 1 ] = walktile_stairs_x2_down;
                        }
                    }
                }
                // tiles con GID relativo alle scale verso sinistra
                if( tileGID >= STAIRS_X2_LEFT_GID_MIN && tileGID <= STAIRS_X2_LEFT_GID_MAX ) {
                    // controlla se la scala (x2) e' la fine di una scala x4: per saperlo bisogna
                    // controllare se dati x,y correnti la cella x-2, y-1 contiene una scala
                    check_tile_x = j - 2;
                    check_tile_y = i - 1;
                    if( isTileInMap( check_tile_x, check_tile_y ) && ( mapinfo.walkables[ check_tile_y ][ check_tile_x ] == walktile_stairs_x2_left ) ) {
                        // e' la fine di una scala x4! imposta la cella corrente...
                        mapinfo.walkables[ i ][ j ]                         = walktile_stairs_x4_left;
                        // azzera quella che era l'inizio della scala x2 (al centro della scala)
                        mapinfo.walkables[ check_tile_y ][ check_tile_x ]   = walktile_vacuum;
                        // reimposta l'inizio della scala
                        mapinfo.walkables[ i - 2 ][ j - 3 ]                 = walktile_stairs_x4_right;
                    } else {
                        // e' una scala x2
                        mapinfo.walkables[ i ][ j ] = walktile_stairs_x2_left;
                        // la cella sopra deve essere per scendere a destra
                        if( ( ( i - 1 ) >= 0 ) && ( ( j - 1 ) >= 0 ) ) {
                            mapinfo.walkables[ i - 1 ][ j - 1 ] = walktile_stairs_x2_right;
                        }
                    }
                }
                // tiles con GID relativo ai muri
                if( tileGID >= WALL_GID_MIN && tileGID <= WALL_GID_MAX ) {
                    // la tile viene marcata come non percorribile (ostacolo)
                    mapinfo.walkables[ i ][ j ] = walktile_obstacle;
                }
                // memorizza le posizioni orizzontali estreme per il riscalamento della mappa
                if( tileGID > 0 ) {
                    if( j > walkable_max_x ) {
                        walkable_max_x = j;
                    }
                    if( i > walkable_max_y ) {
                        walkable_max_y = i;
                    }
                }
            }
        }
    } else {
        // segnala l'errore nel log
        //log( "Errore! la mappa deve avere un layer \"walkable\"" );
    }

    /*
        ogni mappa deve avere un layer denominato "objects" contenente le informazioni
        sugli oggetti/personaggi che devono essere piazzati sulla mappa; poiche' la profondita'
        deve essere gestita a runtime utilizzando (reorderChild) gli oggetti vengono creati
        e aggiunti alla mappa qui;
        al termine del posizionamento il layer "objects" DEVE ESSERE ELIMINATO!
    */
    // recupera il layer "objects"
    auto objects_layer = tilemap->getLayer( "objects" );
    if( objects_layer != nullptr ) {
        // scansione di tutte le tiles nel layer "objects"
        //log( "Scansione tiles del layer \"objects\" (tag %d)...", objects_layer->getTag() );
        for( int i = 0; i < mapinfo.total_v_tiles; i++ ) {
            for( int j = 0; j < mapinfo.total_h_tiles; j++ ) {
                tileGID = objects_layer->getTileGIDAt( Vec2( j, i ) );

                // se la GID equivale all'oggetto target (palloncino)...
                if( ( tileGID >= TARGETS_GID_MIN ) && ( tileGID <= TARGETS_GID_MAX ) ) {
                    //log( "\tTarget GID %d in posizione %d, %d", tileGID, j, i );
                    // memorizza nella struttura mapinfo la posizione della cella target
                    mapinfo.target_tile.x   = j;
                    mapinfo.target_tile.y   = i;
                    // ricicla la sprite contenuta nel layer per creare la sprite del target
                    auto target_sprite = (Sprite*) objects_layer->getTileAt( Vec2( j, i ) );
                    if( target_sprite ) {
                        // rimuove la sprite dall'oggetto padre (il layer che verra' rimosso)...
                        target_sprite->removeFromParentAndCleanup( false );
                        // ...e la aggiunge alla mappa diventandone figlia
                        tilemap->addChild( target_sprite, getTileDepth( j, i ), SPRITE_TAG_TARGET );
                        target_sprite->setAnchorPoint( Vec2( 0.5f, 0 ) );
                        target_sprite->setPosition( getTilePosition( j, i, -( mapinfo.half_tile_height ) ) );
                    }
                }

                // se la GID equivale a un ostacolo fisso
                if( ( tileGID >= OBSTACLE_GID_MIN ) && ( tileGID <= OBSTACLE_GID_MAX ) ) {
                    //log( "\tOstacolo GID %d in posizione %d, %d", tileGID, j, i );
                    // aggiorna la matrice walkables
                    mapinfo.walkables[ i ][ j ] = walktile_obstacle;
                    // ricicla la sprite contenuta nel layer per creare la sprite dell'ostacolo
                    auto obstacle_sprite = (Sprite*) objects_layer->getTileAt( Vec2( j, i ) );
                    if( obstacle_sprite ) {
                        // rimuove la sprite dall'oggetto padre (il layer che verra' rimosso)...
                        obstacle_sprite->removeFromParentAndCleanup( false );
                        // ...e la aggiunge alla mappa diventandone figlia
                        tilemap->addChild( obstacle_sprite, getTileDepth( j, i ), SPRITE_TAG_MAP_OBSTACLE );
                        obstacle_sprite->setAnchorPoint( Vec2( 0.5f, 0 ) );
                        obstacle_sprite->setPosition( getTilePosition( j, i, -( mapinfo.half_tile_height ) ) );
                    } else {
                        //log( "Errore! impossibile recuperare sprite ostacolo" );
                    }
                }

                // se la GID equivale a un oggetto collezionabile...
                if( ( tileGID >= PICKABLE_GID_MIN ) && ( tileGID <= PICKABLE_GID_MAX ) ) {
                    //log( "\tOggetto collezionabile GID %d tag %d in posizione %d, %d", tileGID, PICKABLE_TAG_MIN + mapinfo.total_pickables_tag, j, i );
                    // aggiorna la matrice pickables
                    // il tipo di oggetto e' relativo al valore della GID:
                    // PICKABLE_GID_MIN         -> tipo 0
                    // PICKABLE_GID_MIN + 1     -> tipo 1 ...
                    mapinfo.pickables[ i ][ j ].type    = tileGID - PICKABLE_GID_MIN;
                    // il tag deve variare per permettere il recupero della singola sprite ed e' calcolato
                    // sommando al valore minimo del tag il numero di oggetti collezionabili trovati
                    mapinfo.pickables[ i ][ j ].tag     = PICKABLE_TAG_MIN + mapinfo.total_pickables_tag;
                    // ricicla la sprite contenuta nel layer per creare la sprite dell'ostacolo
                    auto pickable_sprite = (Sprite*) objects_layer->getTileAt( Vec2( j, i ) );
                    if( pickable_sprite ) {
                        // rimuove la sprite dall'oggetto padre (il layer che verra' rimosso)...
                        pickable_sprite->removeFromParentAndCleanup( false );
                        // ...e la aggiunge alla mappa diventandone figlia
                        tilemap->addChild( pickable_sprite, getTileDepth( j, i ), PICKABLE_TAG_MIN + mapinfo.total_pickables_tag );
                        pickable_sprite->setAnchorPoint( Vec2( 0.5f, 0 ) );
                        pickable_sprite->setPosition( getTilePosition( j, i, -( mapinfo.half_tile_height ) ) );

                        // aggiorna il numero totale di oggetti da collezionare
                        mapinfo.total_pickables_tag += 1;
                    } else {
                        //log( "Errore! impossibile recuperare sprite ostacolo" );
                    }
                }
                // tiles con GID relativo agli starter
                if( tileGID >= STARTER_GID_MIN && tileGID <= STARTER_GID_MAX ) {
                    // il tipo di starter e' dato dall'indice della gid
                    int starter = tileGID - STARTER_GID_MIN;
                    // memorizza le coordinate della tile di partenza
                    mapinfo.characters[ mapinfo.total_characters ].tile_x       = j;
                    mapinfo.characters[ mapinfo.total_characters ].tile_y       = i;
                    // la GID identifica la posizione di partenza di un coniglietto
                    if( starter >= startergid_bunny_down && starter <= startergid_bunny_up ) {
                        // le informazioni sono riferite a un coniglietto
                        mapinfo.characters[ mapinfo.total_characters ].character    = character_bunny;
                        // memorizza l'orientamento
                        switch( starter ) {
                            case startergid_bunny_down:     mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_down;     break;
                            case startergid_bunny_up:       mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_up;       break;
                            case startergid_bunny_left:     mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_left;     break;
                            case startergid_bunny_right:    mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_right;    break;
                        }
                        // incrementa il numero di personaggi animati
                        mapinfo.total_characters += 1;
                    }
                    // la GID identifica la posizione di partenza di un porcospino
                    if( starter >= startergid_porcupine_down && starter <= startergid_porcupine_right ) {
                        mapinfo.characters[ mapinfo.total_characters ].character    = character_porcupine;
                        // memorizza l'orientamento
                        switch( starter ) {
                            case startergid_porcupine_down:     mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_down;     break;
                            case startergid_porcupine_up:       mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_up;       break;
                            case startergid_porcupine_left:     mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_left;     break;
                            case startergid_porcupine_right:    mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_right;    break;
                        }
                        // incrementa il numero di personaggi animati
                        mapinfo.total_characters += 1;
                    }
                    // la GID identifica la posizione di partenza di una farfalla
                    if( starter >= startergid_butterfly_down && starter <= startergid_butterfly_right ) {
                        mapinfo.characters[ mapinfo.total_characters ].character    = character_butterfly;
                        // memorizza l'orientamento
                        switch( starter ) {
                            case startergid_butterfly_down:     mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_down;     break;
                            case startergid_butterfly_up:       mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_up;       break;
                            case startergid_butterfly_left:     mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_left;     break;
                            case startergid_butterfly_right:    mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_right;    break;
                        }
                        // incrementa il numero di personaggi animati
                        mapinfo.total_characters += 1;
                    }
                    // la GID identifica la posizione di partenza di una lumaca
                    if( starter >= startergid_snail_down && starter <= startergid_snail_up ) {
                        mapinfo.characters[ mapinfo.total_characters ].character    = character_snail;
                        // memorizza l'orientamento
                        switch( starter ) {
                            case startergid_snail_down:     mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_down;     break;
                            case startergid_snail_up:       mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_up;       break;
                            case startergid_snail_left:     mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_left;     break;
                            case startergid_snail_right:    mapinfo.characters[ mapinfo.total_characters ].orientation  = orientation_right;    break;
                        }
                        // incrementa il numero di personaggi animati
                        mapinfo.total_characters += 1;
                    }

                }

            }
        }
        // rimozione del layer "objects"
        tilemap->removeChild( objects_layer, true );
    } else {
        // segnala l'errore nel log
        //log( "Errore! la mappa deve avere un layer \"objects\"" );
    }

    // recupera il layer "walls"
    auto walls_layer = tilemap->getLayer( "walls" );
    if( walls_layer != nullptr ) {
        TMXTilesetInfo* tileSetInfo = walls_layer->getTileSet();
        //log( "tileSetInfo %f %f", tileSetInfo->_tileSize.width, tileSetInfo->_tileSize.height );
        // le tile da 128x 128 devono essere centrate nella tile (posizione x - 32)
        float drawingOffsetX = 0;
        if( tileSetInfo->_tileSize.width == 128 ) {
            drawingOffsetX = -32;
        }
        // scansione di tutte le tiles nel layer "walls"
        //log( "Scansione tiles del layer \"walls\" (tag %d)...", walls_layer->getTag() );
        for( int i = 0; i < mapinfo.total_v_tiles; i++ ) {
            for( int j = 0; j < mapinfo.total_h_tiles; j++ ) {
                // recupera il GID della tile
                tileGID = walls_layer->getTileGIDAt( Vec2( j, i ) );
                // se e' presente un muro...
                if( tileGID > 0 ) {
                    // la tile non e' percorribile ne dal coniglio ne dai nemici
                    mapinfo.walkables[ i ][ j ] = walktile_obstacle;
                    // viene recuperata la sprite (gia' esistente)
                    auto wall_sprite = (Sprite*) walls_layer->getTileAt( Vec2( j, i ) );
                    if( wall_sprite ) {
                        // rimuove la sprite dall'oggetto padre (il layer che verra' rimosso)...
                        wall_sprite->removeFromParentAndCleanup( false );
                        // ...e la aggiunge alla mappa diventandone figlia
                        tilemap->addChild( wall_sprite, getTileDepth( j, i ), STATIC_WALL_TAG_MIN );
                        wall_sprite->setAnchorPoint( Vec2( 0.5f, 0 ) );
                        Vec2 sprite_position = getTilePosition( j, i, -( mapinfo.tile_height ) );
                        sprite_position.x + drawingOffsetX;
                        wall_sprite->setPosition( sprite_position );
                    } else {
                        //log( "Errore! impossibile recuperare sprite del muro" );
                    }
                }
            }
        }
        // rimozione del layer "walls"
        tilemap->removeChild( walls_layer, true );

    } else {
        // segnala nel log che manca il layer "walls"
        //log( "La mappa non ha un layer \"walls\"" );
    }


    // animazione oggetto target (palloncino)
    auto target_sprite = tilemap->getChildByTag( SPRITE_TAG_TARGET );
    if( target_sprite != nullptr ) {
        auto target_move    = MoveBy::create( 2.0f, Vec2( 0, mapinfo.tile_height ) );
        auto target_back    = MoveBy::create( 2.0f, Vec2( 0, -mapinfo.tile_height ) );
        auto target_seq     = Sequence::create( target_move, target_back, nullptr );
        target_sprite->runAction( RepeatForever::create( target_seq ) );
    } else {
        //log( "Errore! oggetto con tag SPRITE_TAG_TARGET non trovato!" );
    }

    // ogni mappa deve avere delle proprieta':
    // - numero di oggetti piazzabili a disposizione del giocatore (-1 = infinito, default)
    mapinfo.total_user_obstacles = -1;
    Value user_obstacles = tilemap->getProperty( "user_obstacles" );
    if( !user_obstacles.isNull() ) {
        // memorizza il numero di ostacoli piazzabili
        mapinfo.total_user_obstacles = user_obstacles.asInt();
    }
    // (se necessario) lo visualizza
    setObstaclesNumberLabelText( mapinfo.total_user_obstacles );
    // info colore di sfondo se la mappa contiene l'informazione altrimenti casuale
    Value background = tilemap->getProperty( "background" );
    if( !background.isNull() ) {
        gradient_id = background.asInt();
    } else {
        gradient_id = rand()%gradient_max;
    }
    setGradientIndex( gradient_id );

    // info colore di sfondo
    auto menu = this->getChildByTag( tag_menu );
    auto icon = (MenuItemImage*)menu->getChildByTag( tag_obstacles_icon );
    Value obstacle_type = tilemap->getProperty( "obstacle_type" );
    if( !obstacle_type.isNull() ) {
        // memorizza il numero di ostacoli piazzabili
        switch( obstacle_type.asInt() ) {
            // vaso di fiori
            case 1:
                mapinfo.user_obstacle_type = userobstacle_flower_pot;
                icon->setNormalImage( Sprite::create( "images/thumbs/thumb_flower.png" ) );
                icon->setSelectedImage( Sprite::create( "images/thumbs/thumb_flower.png" ) );
            break;
            // funghetto
            default:
                mapinfo.user_obstacle_type = userobstacle_mushroom;
                icon->setNormalImage( Sprite::create( "images/thumbs/thumb_mushroom.png" ) );
                icon->setSelectedImage( Sprite::create( "images/thumbs/thumb_mushroom.png" ) );
            break;
        }
    }


    // aggiunge la mappa al layer GameWorld
    addChild( tilemap, 0, tag_tilemap );


    // crea le animazioni per gli oggetti collezionabili
    int pickable_animation_id = 0;
    for( int i = 0; i < mapinfo.total_v_tiles; i++ ) {
        for( int j = 0; j < mapinfo.total_h_tiles; j++ ) {
            if( mapinfo.pickables[ i ][ j ].tag > 0 ) {
                auto pickable_sprite = ( Sprite* ) tilemap->getChildByTag( mapinfo.pickables[ i ][ j ].tag );
                if( pickable_sprite != nullptr ) {
                    // indice animazione secondo il tipo di oggetto collezionabile
                    bool update_total_pickable = false;
                    switch( mapinfo.pickables[ i ][ j ].type ) {
                        case pickabletype_coin:         pickable_animation_id = animation_bunny_coin;   update_total_pickable = true;  break;
                        case pickabletype_ice_cream:    pickable_animation_id = animation_ice_cream;    update_total_pickable = true;  break;
                        case pickabletype_watermelon:   pickable_animation_id = animation_watermelon;   update_total_pickable = true;  break;
                        case pickabletype_ananas:       pickable_animation_id = animation_ananas;       update_total_pickable = true;  break;
                        case pickabletype_bomb:         pickable_animation_id = animation_bomb;         update_total_pickable = false;  break;
                        case pickabletype_bee_hive:     pickable_animation_id = animation_bee_hive;     update_total_pickable = false;  break;
                        case pickabletype_hamburger:    pickable_animation_id = animation_hamburger;    update_total_pickable = true;  break;
                        case pickabletype_blue_stone:   pickable_animation_id = animation_blue_stone;   update_total_pickable = true;  break;
                        case pickabletype_red_stone:    pickable_animation_id = animation_red_stone;    update_total_pickable = true;  break;
                        case pickabletype_snake:        pickable_animation_id = animation_snake;        update_total_pickable = false;  break;
                        case pickabletype_cheese:       pickable_animation_id = animation_cheese;       update_total_pickable = true;  break;
                        case pickabletype_carrots:      pickable_animation_id = animation_carrots;      update_total_pickable = true;  break;
                        case pickabletype_strawberry:   pickable_animation_id = animation_strawberry;   update_total_pickable = true;  break;
                        case pickabletype_grape:        pickable_animation_id = animation_grape;        update_total_pickable = true;  break;
                        case pickabletype_white_grape:  pickable_animation_id = animation_white_grape;  update_total_pickable = true;  break;
                        case pickabletype_teleport:     pickable_animation_id = animation_teleport;     update_total_pickable = false; break;
                        // altri oggetti, ciliegie pere, mele
                        default:                        pickable_animation_id = animation_bunny_coin;   break;
                    }
                    // aggiorna il numero totale di oggetti realmente collezionabili, ovvero vengono
                    // esclusi gli animali statici, per il calcolo della percentuale di livello completata
                    if( update_total_pickable ) {
                        mapinfo.total_pickables += 1;
                    }

                    // crea l'animazione infinita per la sprite
                    auto pickable_animation = Animate::create( AnimationManager::getInstance()->animations[ pickable_animation_id ] );
                    if( pickable_animation ) {
                        pickable_sprite->runAction( RepeatForever::create( pickable_animation ) );
                    } else {
                        //log( "Errore recupero animazione pickable object" );
                    }
                } else {
                    //log( "Errore! oggetto collezionabile con tag %d non trovato", mapinfo.pickables[ i ][ j ].tag );
                }
            }
        }
    }

    // offset per centramento verticale nella tile, questo valore puo' variare
    // a seconda del personaggio per il corretto centramento delle immagini
    int tileCenterYOffset   = 0;
    int total_enemies       = 0;
    // scansione dell'array characters contenente le informazioni sul punto di partenza
    // di tutti i personaggi animati (coniglietti, porcospini, ecc)
    for( int i = 0; i < mapinfo.total_characters; i++ ) {
        // se il personaggio da creare e' un coniglietto...
        if( mapinfo.characters[ i ].character == character_bunny ) {
            // il coniglietto ha questo offset verticale
            tileCenterYOffset = -16;
            // crea la sprite del coniglietto...
            auto bunny = Bunny::create( mapinfo.characters[ i ].tile_x, mapinfo.characters[ i ].tile_y, mapinfo.characters[ i ].orientation, tileCenterYOffset );
            if( bunny != nullptr ) {
                // ... la aggiunge alla mappa con Z order stabilito dalla profondita'...
                tilemap->addChild( bunny, getTileDepth( mapinfo.characters[ i ].tile_x, mapinfo.characters[ i ].tile_y ), BUNNY_TAG );
                // ... e la posiziona in base alla tile occupata
                bunny->setPosition( getTilePosition( mapinfo.characters[ i ].tile_x, mapinfo.characters[ i ].tile_y, tileCenterYOffset ) );
                // anima il coniglietto
                updateBunny( bunny );
            }
        }
        // se il personaggio da creare e' un porcospino...
        if( mapinfo.characters[ i ].character == character_porcupine ) {
            // crea la sprite del porcospino
            createEnemy( tilemap, &mapinfo.characters[ i ], -24, total_enemies );
            // aggiorna il numero di nemici per il calcolo del tag
            total_enemies += 1;
        }
        // se il personaggio da creare e' una farfalla...
        if( mapinfo.characters[ i ].character == character_butterfly ) {
            // crea la sprite della farfalla
            createEnemy( tilemap, &mapinfo.characters[ i ], -24, total_enemies );
            // aggiorna il numero di nemici per il calcolo del tag
            total_enemies += 1;
        }
        // se il personaggio da creare e' una lumaca...
        if( mapinfo.characters[ i ].character == character_snail ) {
            // crea la sprite della lumaca
            createEnemy( tilemap, &mapinfo.characters[ i ], -24, total_enemies );
            // aggiorna il numero di nemici per il calcolo del tag
            total_enemies += 1;
        }
    }

#ifdef COCOS2D_DEBUG

    // debug info mappa
    char logstr[ 500 ];
    for( int i = 0; i < mapinfo.total_v_tiles; i++ ) {
        logstr[ 0 ] = 0;
        for( int j = 0; j < mapinfo.total_h_tiles; j++ ) {
            sprintf( tmpstr, "%2d", mapinfo.walkables[ i ][ j ] );
            strcat( logstr, tmpstr );
        }
        strcat( logstr, "  " );
        for( int j = 0; j < mapinfo.total_h_tiles; j++ ) {
            sprintf( tmpstr, "%2d", mapinfo.pickables[ i ][ j ].tag );
            strcat( logstr, tmpstr );
        }
        log( "%s", logstr );
    }

    log( "mapinfo.map_width            : %f", mapinfo.map_width );
    log( "mapinfo.map_height           : %f", mapinfo.map_height );
    log( "mapinfo.total_h_tiles        : %f", mapinfo.total_h_tiles );
    log( "mapinfo.total_v_tiles        : %f", mapinfo.total_v_tiles );
    log( "mapinfo.tile_width           : %f", mapinfo.tile_width );
    log( "mapinfo.tile_height          : %f", mapinfo.tile_height );
    log( "mapinfo.total_user_obstacles : %d", mapinfo.total_user_obstacles );

    // debug tag degli oggetti figli di tilemap
    log( "Info figli di tilemap" );
    int child_counter = 0;
    for( const auto& child: tilemap->getChildren() ) {
        log( "child # %d : tag %d z order %d", child_counter++, child->getTag(), child->getLocalZOrder() );
    }

    // debug personaggi
    for( int i = 0; i < mapinfo.total_characters; i++ ) {
        log( "character %d at tile %2d,%2d orientation %d",
            mapinfo.characters[ i ].character,
            mapinfo.characters[ i ].tile_x,
            mapinfo.characters[ i ].tile_y,
            mapinfo.characters[ i ].orientation );
    }

#endif


    // recupera la sprite con le nuvole per animazione ingresso nel livello
    auto clouds = this->getChildByTag( tag_clouds );
    // animazione fade out delle nuvole
    auto fadeout_animation = FadeOut::create( 1.0f );
    // crea la funzione di callback da chiamare ad animazione finita
    auto getready_func  = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::getReady ) );
    // crea la funzione di callback da chiamare ad animazione finita
    auto start_func     = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::startGame ) );
    // crea la sequenza di operazioni da eseguire
    auto actions        = Sequence::create( fadeout_animation, getready_func, DelayTime::create( 2.0f ), start_func, nullptr );
    // avvia animazione
    clouds->runAction( actions );

    // ATTENZIONE! il riposizionamento della mappa va fatto qui per via delle tile multidimensionali
    // re-imposta il punto di ancoraggio della mappa
    tilemap->setAnchorPoint( Vec2::ZERO );
    // re-imposta la posizione della mappa
    tilemap->setPosition( Vec2( 0, ( SCREEN_HEIGHT - ( mapinfo.map_height * tilemap->getScale() ) ) / 2 ) );

    return true;
}

/*
    fine animazione caricamento livello, avvisa il giocatore di prepararsi
*/
void GameWorld::getReady()
{
    auto audio = SimpleAudioEngine::getInstance();
    auto tutLabel = this->getChildByTag( tag_tutoriallabel );
    // se e' un livello di tutorial (uno dei primi 5) visualizza il tip
    // nella parte superiore dello schermo
    switch( currentLevel ) {
        case 0:     audio->playEffect( "audio/tutorial/tut_1.mp3", false, 1.0f, 1.0f, 1.0f );
                    setTutorialLabelText( "Help bunny to fly away" );
        break;
        case 1:     audio->playEffect( "audio/tutorial/tut_2.mp3", false, 1.0f, 1.0f, 1.0f );
                    setTutorialLabelText( "Bunny always turns clockwise" );
        break;
        case 2:     audio->playEffect( "audio/tutorial/tut_3.mp3", false, 1.0f, 1.0f, 1.0f );
                    setTutorialLabelText( "Double tap to place obstacles" );
        break;
        case 3:     audio->playEffect( "audio/tutorial/tut_4.mp3", false, 1.0f, 1.0f, 1.0f );
                    setTutorialLabelText( "Try it yourself" );
        break;
        case 4:     audio->playEffect( "audio/tutorial/tut_5.mp3", false, 1.0f, 1.0f, 1.0f );
                    setTutorialLabelText( "Pick objects but avoid animals" );
        break;
        default:    setTutorialLabelText( "" );
        break;
    }

    setDebugLabelText( "Get ready..." );
    setGameInfoLabelText( "Get ready...", false );
    enterGameState( gs_get_ready );
    // piccolo zoom in della mappa ???
}

/*
    avvia il gioco
*/
void GameWorld::startGame()
{
    setDebugLabelText( "...Go!" );
    setGameInfoLabelText( "...Go!", true );
    enterGameState( gs_running );

    // cerca il coniglietto e i nemici (figli dell'oggetto mappa) e li fa partire modificandone lo stato
    auto tilemap = getChildByTag( tag_tilemap );

    // recupera l'oggetto "coniglio" per modificarne lo stato
    auto bunny = (Bunny*)tilemap->getChildByTag( BUNNY_TAG );
    if( bunny != nullptr ) {
        bunny->setStatus( bunnystate_moving );
    }
    // cerca tutti i nemici (porcospini, farfalle, ecc.) e li fa partire modificandone lo stato
    for( int i = ENEMY_TAG_MIN; i <= ENEMY_TAG_MAX; i++ ) {
        auto enemy = (Enemy*)tilemap->getChildByTag( i );
        if( enemy != nullptr ) {
            enemy->setStatus( enemystate_moving );
        }
    }
}

/*
    questa e' la callback finale di un'animazione:
    distrugge la mappa corrente e carica il nuovo livello
*/
void GameWorld::loadNextLevel()
{
    // cancella la tile map corrente e ogni traccia delle animazioni legate ad essa
    auto tilemap = this->getChildByTag( tag_tilemap );
    this->removeChild( tilemap, true );

    // aggiorare la percentuale di completamento del livello
    int curLevelProgress = 0;
    int levelProgress = GameDB::getInstance()->getLevelProgress( currentLevel );
    // calcola curLevelProgress in base al numero totale di oggetti raccolti
    if( mapinfo.total_pickables == 0 ) {
        curLevelProgress = 100;
    } else {
        curLevelProgress = ( mapinfo.total_picked * 100 ) / mapinfo.total_pickables;
    }
    //log( "curLevelProgress %d ( picked %d / total pickable %d )", curLevelProgress, mapinfo.total_picked, mapinfo.total_pickables );
    // se la percentuale e' maggiore dell'ultima salavata viene aggiornata
    if( curLevelProgress > levelProgress ) {
        GameDB::getInstance()->setLevelProgress( currentLevel, curLevelProgress );
    }

    // indice del livello successivo
    currentLevel += 1;

    // salva il numero corrente di livelli sbloccati
    int totalUnlockedLevel = GameDB::getInstance()->getTotalUnlockedLevels();
    if( currentLevel > totalUnlockedLevel ) {
        GameDB::getInstance()->setTotalUnlockedLevels( currentLevel );
    }

    // carica il nuovo livello
    loadLevel( currentLevel );
}

/*
    questa e' la callback finale di un'animazione:
    distrugge la mappa corrente e ricarica il livello
*/
void GameWorld::reloadLevel()
{
    // cancella la tile map corrente e ogni traccia delle animazioni legate ad essa
    auto tilemap = this->getChildByTag( tag_tilemap );
    this->removeChild( tilemap, true );
    // carica il nuovo livello
    loadLevel( currentLevel );
}

/*
    imposta testo nella label di debug
*/
void GameWorld::setObstaclesNumberLabelText( int number )
{
    char tmpstr[ 10 ];
    auto menu   = this->getChildByTag( tag_menu );
    auto label  = menu->getChildByTag( tag_obstacles_number );
    if( number >= 0 ) {
        sprintf( tmpstr, "%d", number );
    } else {
        sprintf( tmpstr, " " );
    }
    ((Label*)label)->setString( tmpstr );
}

/*
    imposta testo nella label di debug
*/
void GameWorld::setGameInfoLabelText( const char *text, bool fadeout )
{
    auto label = this->getChildByTag( tag_gameinfolabel );
    ((Label*)label)->setString( text );
    if( fadeout ) {
        auto fade = FadeOut::create( 2.0f );
        label->runAction( fade );
    } else {
        auto fade = FadeIn::create( 0.2f );
        label->runAction( fade );
    }
}

/*
    imposta testo nella label di debug
*/
void GameWorld::setDebugLabelText( const char *text )
{
    // this is for debug/development only
/*
    auto label = this->getChildByTag( tag_debuglabel );
    ((Label*)label)->setString( text );
*/
}

/*
    imposta testo nella label di tutorial
*/
void GameWorld::setTutorialLabelText( const char *text )
{
    auto label = this->getChildByTag( tag_tutoriallabel );
    ((Label*)label)->setString( text );
}

/*
    ritorna le coordinate del centro della cella indicata come argomento,
    la posizione e' relativa alla mappa
*/
Vec2 GameWorld::getTilePosition( int x, int y, int tileCenterYOffset )
{
    Vec2 result;
    result.x    = ( x - y ) * ( mapinfo.tile_width     / 2 );
    result.y    = ( x + y ) * ( mapinfo.tile_height    / 2 );
    result.x    += ( mapinfo.map_width / 2 );
    result.y    = mapinfo.map_height - result.y;
    result.y    += tileCenterYOffset;
    return result;
}

/*
    ritorna la profondita' secondo la posizione della tile
*/
int GameWorld::getTileDepth( int tile_x, int tile_y )
{
    return ( tile_x + tile_y + 2 );
}


/*
    tipi di salto del coniglietto
*/
typedef enum {
    jumptype_default,              // salto normale
    jumptype_stairs_x2,            // salta su scala x2
    jumptype_stairs_x4,            // salta su scala x4
    jumptype_max,
} jumptype_t;

/*
    aggiorna la posizione z (profondita') del coniglietto
*/
void GameWorld::updateBunnyZOrder( Node *node )
{
    Bunny *bunny = (Bunny*)node;
    // recupera l'oggetto mappa per l'assegnazione del nuovo z order
    auto    tilemap = getChildByTag( tag_tilemap );
    tilemap->reorderChild( bunny, getTileDepth( bunny->getTileX(), bunny->getTileY() ) );
}

/*
    avvia l'animazione del salto (jumptype) sul coniglietto (bunny)
*/
void GameWorld::jumpBunny( Bunny *bunny, int jumptype, int target_tile_x, int target_tile_y, int orientation )
{
    int     animation_id    = 0;
    int     jumps           = 1;    // default = 1 salto solo
    float   duration        = 1.0f; // default = 1 secondo

    Vec2    new_position    = getTilePosition( target_tile_x, target_tile_y, bunny->getTileCenterYOffset() );

    // tipo di salto
    switch( jumptype ) {
        // normale (da tile a tile)
        case jumptype_default:
            jumps           = 1;
            duration        = 1.0f;
            break;
        // scale x2
        case jumptype_stairs_x2:
            jumps           = 2;
            duration        = 2.0f;
            break;
        // scale x4
        case jumptype_stairs_x4:
            jumps           = 4;
            duration        = 4.0f;
            break;
    }

    //log( "bunny cur_position %.2f,%.2f new_position %.2f,%.2f", cur_position.x, cur_position.y, new_position.x, new_position.y );

    // imposta l'indice dell'animazione secondo l'orientamento corrente
    switch( orientation ) {
        case orientation_up:    animation_id = animation_bunny_up;       break;
        case orientation_right: animation_id = animation_bunny_right;    break;
        case orientation_down:  animation_id = animation_bunny_down;     break;
        case orientation_left:  animation_id = animation_bunny_left;    break;
        default:                animation_id = animation_bunny_up;       break;
    }

    // crea l'azione di salto della sprite
    auto jump       = JumpTo::create( duration, new_position, 32.0f, jumps );
    // crea l'animazione secondo l'orientamento
    auto animation  = Animate::create( AnimationManager::getInstance()->animations[ animation_id ] );
    // il salto e l'animazione devono essere eseguite contemporaneamnte
    auto comb       = Spawn::create( jump, animation, nullptr );
    // crea la funzione di callback da chiamare ad animazione finita
    auto func       = CCCallFuncN::create( this, CC_CALLFUNCN_SELECTOR( GameWorld::updateBunny ) );


    // se la direzione e' verso l'alto o verso sinistra significa il coniglietto sta procedendo verso
    // il fondo: l'aggiornamento del livello di profondita' deve essere eseguito immediatamente
    // affinche' il coniglietto possa scomparire immediatamente dietro agli oggetti
    if( orientation == orientation_left || orientation == orientation_up ) {
        // aggiorna immediatamente il livello di profondita' del coniglietto
        updateBunnyZOrder( bunny );
        // crea la sequenza di operazioni da eseguire
        auto actions    = Sequence::create( comb, func, nullptr );
        // avvia la sequenza di azioni
        bunny->runAction( actions );
    } else {
        // se la direzione e' verso il basso o verso destra significa il coniglio sta procedendo in avanti:
        // l'aggiornamento del livello di profondita' deve essere eseguito immediatamente
        // dopo lo spostamento sulla nuova cella affinche' il coniglio possa apparire da dietro agli oggetti

        // crea la funzione di callback (updateBunnyZOrder) da chiamare a spostamento ultimato
        auto zordfunc   = CCCallFuncN::create( this, CC_CALLFUNCN_SELECTOR( GameWorld::updateBunnyZOrder ) );
        // crea la sequenza di operazioni da eseguire
        auto actions    = Sequence::create( comb, zordfunc, func, nullptr );
        // avvia la sequenza di azioni
        bunny->runAction( actions );
    }
}

/*
    tipo di azioni del coniglietto
*/
typedef enum {
    bunnyaction_idle,
    bunnyaction_move,
    bunnyaction_move_stairs_x2,
    bunnyaction_move_stairs_x4,
    bunnyaction_pick_object,
    bunnyaction_explode,
    bunnyaction_fall,
    bunnyaction_fly_away,
    bunnyaction_prick,
    bunnyaction_block,
    bunnyaction_teleport,
    bunnyaction_max,
} bunnyaction_t;



/*
    controllo validita' coordinate della cella:
    - x deve essere tra 0 (incluso) e il numero totale di celle orizzontali (escluso)
    - y deve essere tra 0 (incluso) e il numero totale di celle verticali (escluso)
*/
bool GameWorld::isTileInMap( int tile_x, int tile_y )
{
    bool result = false;
    if( ( tile_x >= 0 ) && ( tile_x <= mapinfo.total_h_tiles ) && ( tile_y >= 0 ) && ( tile_y <= mapinfo.total_v_tiles ) ) {
        result = true;
    }
    return result;
}

/*
    la cella e' percorribile?
*/
bool GameWorld::isTileWalkable( int tile_x, int tile_y )
{
    bool result = false;
    // controllo validita' coordinate
    if( isTileInMap( tile_x, tile_y ) ) {
        // la cella e' marcata come walkable ?
        if( mapinfo.walkables[ tile_y ][ tile_x ] == walktile_walkable ) {
            result = true;
        }
    }
    return result;
}

/*
    la cella e' nel vuoto
*/
bool GameWorld::isTileVacuum( int tile_x, int tile_y )
{
    bool result = false;
    // controllo validita' coordinate
    if( isTileInMap( tile_x, tile_y ) ) {
        // la cella e' marcata come walkable ?
        if( mapinfo.walkables[ tile_y ][ tile_x ] == walktile_vacuum ) {
            result = true;
        }
    }
    return result;
}


#define MAX_NEIGHBOUR_OFFSETS   4

tilecoord_t neighbour_offset[ MAX_NEIGHBOUR_OFFSETS ] =
{
    //  x   y
    {   0,  -1  },  // up
    {   0,  +1  },  // down
    {   1,  0   },  // right
    {   -1, 0   },  // left
};

/*
    la cella e' selezionabile per piazzare un ostacolo?
*/
bool GameWorld::isTileSelectable( int tile_x, int tile_y )
{
    int     test_tile_x, test_tile_y;
    bool    stairs  = false;
    bool    result  = false;

    // controllo validita' coordinate
    if( isTileInMap( tile_x, tile_y ) ) {
        // controllo le condizioni per cui una cella puo' essere selezionata
        do {
            // nella matrice walkables la cella deve essere contrassegnata come walktile_walkable
            if( mapinfo.walkables[ tile_y ][ tile_x ] != walktile_walkable ) {
                break;
            }

            // controlla le 4 posizioni raggiungibili dalla cella corrente
            for( int i = 0; i < MAX_NEIGHBOUR_OFFSETS; i++ ) {
                test_tile_x = tile_x + neighbour_offset[ i ].x;
                test_tile_y = tile_y + neighbour_offset[ i ].y;
                // controllo validita' coordinate
                if( isTileInMap( test_tile_x, test_tile_y ) ) {
                    if( mapinfo.walkables[ test_tile_y ][ test_tile_x ] >= walktile_stairs_x2_up ) {
                        // trovata una scala! non controlla nemmeno le posizioni rimanenti
                        stairs = true;
                        break;
                    }
                }
            }
            // se ci sono delle scale nelle adiacenze, la cella non e' selezionabile
            if( stairs ) {
                break;
            }

            // la cella non deve essere occupa da un coniglietto
            auto    tilemap         = getChildByTag( tag_tilemap );
            int     bunny_tile_x    = 0;
            int     bunny_tile_y    = 0;
            bool    bunnies         = false;
            auto    bunny = (Bunny*)tilemap->getChildByTag( BUNNY_TAG );
            if( bunny != nullptr ) {
                bunny->getTile( &bunny_tile_x, &bunny_tile_y );
                if( ( tile_x == bunny_tile_x ) && ( tile_y == bunny_tile_y ) ) {
                    bunnies = true;
                }
            }
            if( bunnies ) {
                break;
            }

            // la tile selezionata non deve essere contenere l'oggetto target
            if( ( tile_x == mapinfo.target_tile.x ) && ( tile_y == mapinfo.target_tile.y ) ) {
                break;
            }

            // non deve esserci un oggetto collezionabile
            if( mapinfo.pickables[ tile_y ][ tile_x ].tag > 0 ) {
                break;
            }


            // ok! la cella e' selezionabile
            result = true;
        } while( 0 );
    }
    return result;
}

/*
    ritorna le coordinate (next_tile_x, next_tile_y) della prossima cella secondo
    la cella attuale (tile_x, tile_y) e la direzione (orientation) con uno spostamento
    normale
*/
void GameWorld::getNextTile( int tile_x, int tile_y, int orientation, int *next_tile_x, int *next_tile_y )
{
    *next_tile_x = tile_x;
    *next_tile_y = tile_y;
    switch( orientation ) {
        case orientation_up:    *next_tile_y -= 1;     break;
        case orientation_down:  *next_tile_y += 1;     break;
        case orientation_left:  *next_tile_x -= 1;     break;
        case orientation_right: *next_tile_x += 1;     break;
    }
}

/*
    ritorna le coordinate (next_tile_x, next_tile_y) della prossima cella secondo
    la cella attuale (tile_x, tile_y) e la direzione (orientation) quando deve essere
    percorsa una scala
*/
void GameWorld::getNextTileStairs( int stairs_lenght, int tile_x, int tile_y, int orientation, int *next_tile_x, int *next_tile_y )
{
    int offset_x = 0;
    int offset_y = 0;
    // assegna la posizione corrente
    *next_tile_x = tile_x;
    *next_tile_y = tile_y;

    switch( stairs_lenght ) {
        case 4:
            switch( orientation ) {
                case orientation_up:    offset_x    =   -2; offset_y   =   -4;  break;
                case orientation_down:  offset_x    =   2;  offset_y   =    4;  break;
                case orientation_left:  offset_x    =   -4; offset_y   =   -2;  break;
                case orientation_right: offset_x    =   4;  offset_y   =    2;  break;
            }
        break;
        default: // 2
            switch( orientation ) {
                case orientation_up:    offset_x    =   -1; offset_y   =   -3;  break;
                case orientation_down:  offset_x    =   1;  offset_y   =    3;  break;
                case orientation_left:  offset_x    =   -3; offset_y   =   -1;  break;
                case orientation_right: offset_x    =   3;  offset_y   =    1;  break;
            }
        break;
    }

    // aggiorna la posizione secondo l'offset
    *next_tile_x += offset_x;
    *next_tile_y += offset_y;
}


/*
    ritorna l'indice dell'azione che il coniglietto deve compiere in base
*/
int GameWorld::getBunnyAction( Bunny *bunny )
{
    int action              = 0;
    int orientation         = 0;
    int start_orientation   = 0;
    int tile_x              = 0;
    int tile_y              = 0;
    int next_tile_x         = 0;
    int next_tile_y         = 0;

//#define BUNNY_FALL

    // il gioco e' in stato di "get ready"...
    if( bunny->getStatus() == bunnystate_idle ) {

        // il coniglietto deve saltare sul posto
        action = bunnyaction_idle;

    } else if( bunny->getStatus() == bunnystate_moving ) {

        // recupera le coordinate della cella attualmente occupata dal coniglietto
        bunny->getTile( &tile_x, &tile_y );

        // controlla cosa c'e' nella posizione corrente

        // il coniglietto ha raggiunto la tile target?
        if( ( tile_x == mapinfo.target_tile.x ) && ( tile_y == mapinfo.target_tile.y ) ) {
            // target raggiunto, il coniglietto vola via
            return bunnyaction_fly_away;
        }

        // nella posizione attuale c'e' una bomba?
        if( mapinfo.pickables[ tile_y ][ tile_x ].type == pickabletype_bomb ) {
            // il coniglietto salta in aria!!!
            return bunnyaction_explode;
        }
        // nella posizione attuale c'e' un alveare o un serpente?
        if( ( mapinfo.pickables[ tile_y ][ tile_x ].type == pickabletype_bee_hive ) ||
           ( mapinfo.pickables[ tile_y ][ tile_x ].type == pickabletype_snake ) ) {
            // game over: il coniglietto si punge!
            return bunnyaction_prick;
        }

        // nella posizione attuale c'e' il teletrasporto?
        if( mapinfo.pickables[ tile_y ][ tile_x ].type == pickabletype_teleport ) {
            if( teleporting ) {
                teleporting = false;
            } else {
                // game over: il coniglietto si teletrasporta!
                return bunnyaction_teleport;
            }
        }


        // nella posizione attuale c'e' un nemico?
        auto tilemap = getChildByTag( tag_tilemap );
        for( int i = 0; i < mapinfo.total_characters; i++ ) {
            auto enemy = (Enemy*)tilemap->getChildByTag( ENEMY_TAG_MIN + i );
            if( enemy != nullptr ) {
                if( ( ( tile_x == enemy->getTileX() ) && ( tile_y == enemy->getTileY() ) ) ||
                    ( ( tile_x == enemy->getPreviousTileX() ) && ( tile_y == enemy->getPreviousTileY() ) ) ) {
                    // ferma l'animale con cui si e' scontrato il coniglietto
                    enemy->cleanup();
                    enemy->setStatus( enemystate_stop );
                    // il coniglietto e' stato "beccato"!
                    return bunnyaction_prick;
                }
            }
        }

        // nella posizione attuale c'e' un oggetto da raccogliere?
        if( mapinfo.pickables[ tile_y ][ tile_x ].tag > 0 ) {
            // se l'oggetto non e' il portale del teletrasporto...
            if( mapinfo.pickables[ tile_y ][ tile_x ].type != pickabletype_teleport ) {
                // il coniglietto raccoglie l'oggetto
                return bunnyaction_pick_object;
            }
        }

        // valutare se far cadere il coniglio o no
#ifdef BUNNY_FALL
        // nella posizione attuale c'e' il vuoto?
        if( mapinfo.walkables[ tile_y ][ tile_x ] == walktile_vacuum ) {
            // il coniglietto cade
            return bunnyaction_fall;
        }
#endif // BUNNY_FALL

        // recupera la direzione corrente del coniglio
        orientation = bunny->getOrientation();
        // memorizza la direzione iniziale (per controllo dead loop)
        start_orientation = orientation;

        do {
            // recupera le coordinate della prossima cella secondo la direzione
            getNextTile( tile_x, tile_y, orientation, &next_tile_x, &next_tile_y );
            // la prossima cella e' percorribile?
            if( isTileWalkable( next_tile_x, next_tile_y ) ) {
                bunny->setTile( next_tile_x, next_tile_y );
                action = bunnyaction_move;
                break;
            }

#ifdef BUNNY_FALL
            // la prossima cella e' nel vuoto? il coniglio cadra'...
            if( isTileVacuum( next_tile_x, next_tile_y ) ) {
                bunny->setTile( next_tile_x, next_tile_y );
                action = bunnyaction_move;
                break;
            }
#endif // BUNNY_FALL

            // la prossima cella e' una scala?
            //log( "current tile %d %d next tile %d %d content %d", tile_x, tile_y, next_tile_x, next_tile_y, mapinfo.walkables[ next_tile_y ][ next_tile_x ] );
            switch( mapinfo.walkables[ next_tile_y ][ next_tile_x ] ) {
                case walktile_stairs_x2_left:
                case walktile_stairs_x2_up:
                case walktile_stairs_x2_down:
                case walktile_stairs_x2_right:
                    // recupera le coordinate della prossima cella dopo le scale
                    getNextTileStairs( 2, tile_x, tile_y, orientation, &next_tile_x, &next_tile_y );
                    bunny->setTile( next_tile_x, next_tile_y );
                    return bunnyaction_move_stairs_x2;
                    break;
                case walktile_stairs_x4_left:
                case walktile_stairs_x4_up:
                case walktile_stairs_x4_down:
                case walktile_stairs_x4_right:
                    // recupera le coordinate della prossima cella dopo le scale
                    getNextTileStairs( 4, tile_x, tile_y, orientation, &next_tile_x, &next_tile_y );
                    bunny->setTile( next_tile_x, next_tile_y );
                    return bunnyaction_move_stairs_x4;
                    break;
            }
            // prova con una nuova direzione in senso orario
            orientation += 1;
            if( orientation >= orientation_max ) {
                orientation = 0;
            }
            bunny->setOrientation( orientation );
            // controllo di un eventuale dead loop se il coniglietto e' imprigionato
            if( orientation == start_orientation ) {
                return bunnyaction_block;
            }

        } while( 1 );

    }

    return action;
}

/*
    colleziona l'oggetto in cui si trova il coniglietto
*/
void GameWorld::pickObject( Bunny* bunny )
{
    int tile_x, tile_y;

    // aggiorna il numero di oggetti collezionati dal giocatore
    // a fine livello serve a determinare se la percentuale di completamento
    mapinfo.total_picked += 1;

    // recupera le coordinate della tile in cui si trova il coniglio
    tile_x = bunny->getTileX();
    tile_y = bunny->getTileY();
    // rimuove la sprite dell'oggetto collezionabile, il tag si trova
    // nella matrice pickables di mapinfo
    auto tilemap = getChildByTag( tag_tilemap );
    tilemap->removeChildByTag( mapinfo.pickables[ tile_y ][ tile_x ].tag );
    // rimuove il tag dell'oggetto collezionabile dalla matrice pickables
    mapinfo.pickables[ tile_y ][ tile_x ].tag = 0;

    // file audio
    auto audio = SimpleAudioEngine::getInstance();
    audio->playEffect( "audio/pick.mp3", false, 1.0f, 1.0f, 1.0f );

    // animazione stelline
    auto _emitter = ParticleFlower::create();
    _emitter->setTexture( Director::getInstance()->getTextureCache()->addImage( "images/objects/star_1.png" ) );
    //_emitter->setPosition( getTilePosition( tile_x, tile_y, -( mapinfo.half_tile_height ) ) );
    _emitter->setPosition( getTilePosition( tile_x, tile_y, mapinfo.half_tile_height ) );
    _emitter->setAnchorPoint( Vec2( 0.5f, 0 ) );
    _emitter->setEmissionRate( 5.0f );
    _emitter->setDuration( 1.0f );
    _emitter->setAutoRemoveOnFinish( true );
    tilemap->addChild( _emitter, 999, getTileDepth( tile_x, tile_y ) );
}

/*
    callback al termine dell'esplosione del coniglio
*/
void GameWorld::endOfBunnyExplosion()
{
    // game over, ricomincia il livello da capo
    gameOver( gameover_explode );
}

/*
    esplosione del coniglio sulla bomba
*/
void GameWorld::bunnyExplode( Bunny* bunny )
{
    int tile_x = 0, tile_y = 0;
    // recupera la tile occupata dal coniglio
    bunny->getTile( &tile_x, &tile_y );
    // rende il coniglio invisibile
    bunny->setVisible( false );
    // la bomba e' un oggetto collezionabile quindi il tag per recupare
    // la sprite si trova nella matrice pickables
    auto tilemap        = getChildByTag( tag_tilemap );
    if( tilemap != nullptr ) {
        auto bomb_sprite = tilemap->getChildByTag( mapinfo.pickables[ tile_y ][ tile_x ].tag );
        if( bomb_sprite != nullptr ) {
            bomb_sprite->cleanup();
            // file audio
            auto audio = SimpleAudioEngine::getInstance();
            audio->playEffect( "audio/explosion.mp3", false, 1.0f, 1.0f, 1.0f );

            auto explosion  = Animate::create( AnimationManager::getInstance()->animations[ animation_explosion ] );
            auto callback   = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::endOfBunnyExplosion ) );
            bomb_sprite->runAction( Sequence::create( explosion, callback, nullptr)  );
        }
    }
}

/*
    teletrasporto del coniglio
*/
void GameWorld::teleportBunny( Bunny* bunny )
{
    int tile_x = 0, tile_y = 0;
    // coordinate della tile attualmente occupata dal coniglio
    bunny->getTile( &tile_x, &tile_y );
    // cerca la posizione in cui si trova l'altro portale
    for( int i = 0; i < mapinfo.total_v_tiles; i++ ) {
        for( int j = 0; j < mapinfo.total_h_tiles; j++ ) {
            // nella posizione attuale c'e' il teletrasporto?
            if( mapinfo.pickables[ i ][ j ].type == pickabletype_teleport ) {
                if( ( i != tile_y ) && ( j != tile_x ) ) {
                    //log( "Portale teletrasporto alla coordinata %d,%d", j,i );
                    // imposta la nuova posizione del coniglio
                    bunny->setTile( j, i );
                    bunny->setPosition( getTilePosition( j, i, bunny->getTileCenterYOffset() ) );
                    // aggiorna immediatamente il livello di profondita' del coniglietto
                    updateBunnyZOrder( bunny );
                    // il flag teleporting serve a evitare che il coniglio vada in loop di teletrasporto
                    teleporting = true;
                    // file audio
                    auto audio = SimpleAudioEngine::getInstance();
                    audio->playEffect( "audio/teleport.mp3", false, 1.0f, 1.0f, 1.0f );
                }
            }
        }
    }
}

/*
    aggiornamento stato del coniglietto
*/
void GameWorld::updateBunny( Node* node )
{
    Bunny   *bunny  = ( ( Bunny* ) node );

    int action = getBunnyAction( bunny );

    auto tilemap = getChildByTag( tag_tilemap );

    //log( "bunny action %d", action );

    // logica di gioco del coniglio
    switch( action ) {
        // stato di idle: il coniglietto deve saltare sul posto
        case bunnyaction_idle:
            // salto
            jumpBunny( bunny, jumptype_default, bunny->getTileX(), bunny->getTileY(), bunny->getOrientation() );
            break;
        // spostamento da tile a tile
        case bunnyaction_move:
            // salto
            jumpBunny( bunny, jumptype_default, bunny->getTileX(), bunny->getTileY(), bunny->getOrientation() );
            break;
        // spostamento sulle scale x2
        case bunnyaction_move_stairs_x2:
            // salto
            jumpBunny( bunny, jumptype_stairs_x2, bunny->getTileX(), bunny->getTileY(), bunny->getOrientation() );
            // riassegna il livello di profondita'
            break;
        case bunnyaction_move_stairs_x4:
            // salto
            jumpBunny( bunny, jumptype_stairs_x4, bunny->getTileX(), bunny->getTileY(), bunny->getOrientation() );
            break;
        // colleziona l'oggetto
        case bunnyaction_pick_object:
            pickObject( bunny );
            updateBunny( bunny );
            break;
        // il coniglio salta in aria
        case bunnyaction_explode:
            // imposta lo stato del coniglietto (stop)
            bunny->setStatus( bunnystate_stop );
            // esplosione del coniglio
            bunnyExplode( bunny );
            break;
        // il coniglio precipita
        case bunnyaction_fall:
            // imposta lo stato del coniglietto (stop)
            bunny->setStatus( bunnystate_stop );
            // game over, ricominica il livello da capo
            gameOver( gameover_fall );
            break;
        // il coniglio e' imprigionato
        case bunnyaction_block:
            // imposta lo stato del coniglietto (stop)
            bunny->setStatus( bunnystate_stop );
            // game over, ricominica il livello da capo
            gameOver( gameover_block );
            break;
        // livello completato: animazione del coniglietto che vola e passaggio al prossimo livello
        case bunnyaction_fly_away:
            // imposta lo stato del coniglietto (stop)
            bunny->setStatus( bunnystate_stop );
            // livello completato, passa al prossimo
            goToNewLevel();
            break;
        // il coniglietto ha urtato un porcospino
        case bunnyaction_prick:
            // imposta lo stato del coniglietto (stop)
            bunny->setStatus( bunnystate_stop );
            // game over, ricominica il livello da capo
            gameOver( gameover_prick );
            break;
        // il coniglietto si teletrasporta
        case bunnyaction_teleport:
            // riposizionamento del coniglio nell'altro portale
            teleportBunny( bunny );
            updateBunny( bunny );
            break;
    }
}


/*
    tipo di azioni del nemico
*/
typedef enum {
    enemyaction_idle,
    enemyaction_move,
    enemyaction_locked,
    enemyaction_explode,
    enemyaction_max,
} enemyaction_t;

/*
    ritorna il tipo di azione del nemico secondo lo stato o la posizione corrente
*/
int GameWorld::getEnemyAction( Enemy *enemy )
{
    int action              = 0;
    int orientation         = 0;
    int start_orientation   = 0;
    int tile_x              = 0;
    int tile_y              = 0;
    int next_tile_x         = 0;
    int next_tile_y         = 0;

    //log( "Enemy status %d", enemy->getStatus() );

    // il gioco e' in stato di "get ready"...
    if( enemy->getStatus() == enemystate_idle ) {
        // il nemico deve stare fermo e ricontrollare il proprio stato dopo un certo delay
        action = enemyaction_idle;
    } else if( enemy->getStatus() == enemystate_moving ) {

        // recupera le coordinate della cella attualmente occupata dal nemico
        enemy->getTile( &tile_x, &tile_y );

        // controlla cosa c'e' nella posizione corrente

        // nella posizione attuale c'e' una bomba? in teoria questo controllo non server perche'
        // la bomba viene controlla in anticipo prima di finire sulla stessa cella...
        if( mapinfo.pickables[ tile_y ][ tile_x ].type == pickabletype_bomb ) {
            // il nemico salta in aria!!!
            return enemyaction_explode;
        }

        // se non c'e' nessun ostacolo cerca la prossima tile da occupare secondo la direzione corrente,
        // ma prima memorizza la cella corrente per il rilevamento della collisione con il coniglietto
        enemy->setPreviousTile( tile_x, tile_y );
        // recupera la direzione corrente del nemico
        orientation = enemy->getOrientation();
        // memorizza la direzione iniziale (per controllo dead loop)
        start_orientation = orientation;

        do {
            // recupera le coordinate della prossima cella secondo la direzione
            getNextTile( tile_x, tile_y, orientation, &next_tile_x, &next_tile_y );

            // il nemico puo' occupare la prossima cella se:
            // - e' percorribile (isTileWalkable)
            // - non contiene un oggetto collezionabile (eccetto la bomba)
            bool pickable   = false;
            // nella prossima cella c'e' un oggetto da raccogliere?
            if( mapinfo.pickables[ next_tile_y ][ next_tile_x ].tag > 0 ) {
                // l'oggetto da raccogliere e' una bomba ?
                if( mapinfo.pickables[ next_tile_y ][ next_tile_x ].type == pickabletype_bomb ) {
                    enemy->setTile( next_tile_x, next_tile_y );
                    // il personaggio deve esplodere
                    return enemyaction_explode;
                } else {
                    pickable = true;
                }
            }
            //log( "porcupine %2d,%2d or %d next %2d %2d walk %d", tile_x, tile_y, orientation, next_tile_x, next_tile_y, isTileWalkable( next_tile_x, next_tile_y ) );
            // la prossima cella e' percorribile e non contiene oggetti collezionabili
            if( ( isTileWalkable( next_tile_x, next_tile_y ) ) && ( !pickable ) ) {
                enemy->setTile( next_tile_x, next_tile_y );
                action = enemyaction_move;
                break;
            }

            // prova con una nuova direzione in senso orario
            orientation += 1;
            if( orientation >= orientation_max ) {
                orientation = 0;
            }
            enemy->setOrientation( orientation );
            // controllo di un eventuale dead loop se il nemico e' imprigionato
            if( orientation == start_orientation ) {
                return enemyaction_locked;
            }

        } while( 1 );

    } else if( enemy->getStatus() == enemystate_stop ) {
        // il nemico deve stare fermo e ricontrollare il proprio stato dopo un certo delay
        action = enemyaction_idle;
    }

    return action;
}

/*
    stato di attesa (inizio partita) del nemico
*/
void GameWorld::enemyIdle( Enemy *enemy )
{
    // crea la funzione di callback (updateEnemy) da chiamare trascorso il delay
    auto func       = CCCallFuncN::create( this, CC_CALLFUNCN_SELECTOR( GameWorld::updateEnemy ) );
    // crea la sequenza di operazioni da eseguire (attesa e ricontrollo stato)
    auto actions    = Sequence::create( DelayTime::create( 1.0f), func, nullptr );
    // avvia la sequenza di azioni
    enemy->runAction( actions );
}

/*
    aggiorna la posizione z (profondita') del nemico
*/
void GameWorld::updateEnemyZOrder( Node *node )
{
    Enemy *enemy = (Enemy*)node;
    // recupera l'oggetto mappa per l'assegnazione del nuovo z order
    auto    tilemap = getChildByTag( tag_tilemap );
    tilemap->reorderChild( enemy, getTileDepth( enemy->getTileX(), enemy->getTileY() ) );
}

/*
    spostamento del nemico da una tile all'altra
*/
void GameWorld::enemyMove( Enemy *enemy )
{
    int     animation_id    = 0;
    float   duration        = 1.0;
    Vec2    new_position    = getTilePosition( enemy->getTileX(), enemy->getTileY(), enemy->getTileCenterYOffset() );


    // imposta l'indice dell'animazione secondo il tipo di animale e l'orientamento corrente
    if( enemy->getType() == character_porcupine ) {
        // l'animazione varia secondo la direzione
        switch( enemy->getOrientation() ) {
            case orientation_up:    animation_id = animation_porcupine_up;       break;
            case orientation_right: animation_id = animation_porcupine_right;    break;
            case orientation_down:  animation_id = animation_porcupine_down;     break;
            case orientation_left:  animation_id = animation_porcupine_left;    break;
            default:                animation_id = animation_porcupine_up;       break;
        }
        // il riccio ha la stessa velocita' di spostamento del coniglio: 1 secondo
        auto jump       = JumpTo::create( 1.0f, new_position, 5, 2 );
        // crea l'animazione secondo l'orientamento
        auto animation  = Animate::create( AnimationManager::getInstance()->animations[ animation_id ] );
        // il salto e l'animazione devono essere eseguite contemporaneamnte
        auto comb       = Spawn::create( jump, animation, nullptr );
        // crea la funzione di callback da chiamare ad animazione finita
        auto func       = CCCallFuncN::create( this, CC_CALLFUNCN_SELECTOR( GameWorld::updateEnemy ) );
        // se la direzione e' verso l'alto o verso sinistra significa il nemico sta procedendo verso
        // il fondo: l'aggiornamento del livello di profondita' deve essere eseguito immediatamente
        // affinche' il nemico possa scomparire immediatamente dietro agli oggetti
        if( enemy->getOrientation() == orientation_left || enemy->getOrientation() == orientation_up ) {
            // aggiorna immediatamente il livello di profondita' del nemico
            updateEnemyZOrder( enemy );
            // crea la sequenza di operazioni da eseguire
            auto actions    = Sequence::create( comb, func, nullptr );
            // avvia la sequenza di azioni
            enemy->runAction( actions );
        } else {
            // se la direzione e' verso il basso o verso destra significa il nemico sta procedendo in avanti:
            // l'aggiornamento del livello di profondita' deve essere eseguito immediatamente
            // dopo lo spostamento sulla nuova cella affinche' il nemico possa apparire da dietro agli oggetti

            // crea la funzione di callback (updateEnemyZOrder) da chiamare a spostamento ultimato
            auto zordfunc   = CCCallFuncN::create( this, CC_CALLFUNCN_SELECTOR( GameWorld::updateEnemyZOrder ) );
            // crea la sequenza di operazioni da eseguire
            auto actions    = Sequence::create( comb, zordfunc, func, nullptr );
            // avvia la sequenza di azioni
            enemy->runAction( actions );
        }

    } else if( enemy->getType() == character_butterfly || enemy->getType() == character_snail ) {
        if( enemy->getType() == character_butterfly ) {
            // la farfalla e' veloce nello spostamento, ci mette mezzo secondo
            duration = 0.5;
            switch( enemy->getOrientation() ) {
                case orientation_up:    animation_id = animation_butterfly_up;       break;
                case orientation_right: animation_id = animation_butterfly_right;    break;
                case orientation_down:  animation_id = animation_butterfly_down;     break;
                case orientation_left:  animation_id = animation_butterfly_left;    break;
                default:                animation_id = animation_butterfly_up;       break;
            }
        } else {
            // la lumaca e' lenta nello spostamento, ci mette un secondo e mezzo
            duration = 1.5;
            switch( enemy->getOrientation() ) {
                case orientation_up:    animation_id = animation_snail_up;       break;
                case orientation_right: animation_id = animation_snail_right;    break;
                case orientation_down:  animation_id = animation_snail_down;     break;
                case orientation_left:  animation_id = animation_snail_left;    break;
                default:                animation_id = animation_snail_up;       break;
            }
        }

        // crea l'azione di salto della sprite
        //auto jump       = JumpTo::create( 1.0f, new_position, 5, 2 );
        auto step       = MoveTo::create( duration, new_position );
        // crea l'animazione secondo l'orientamento
        auto animation  = Animate::create( AnimationManager::getInstance()->animations[ animation_id ] );
        // il salto e l'animazione devono essere eseguite contemporaneamnte
        auto comb       = Spawn::create( step, animation, nullptr );
        // crea la funzione di callback da chiamare ad animazione finita
        auto func       = CCCallFuncN::create( this, CC_CALLFUNCN_SELECTOR( GameWorld::updateEnemy ) );
        // se la direzione e' verso l'alto o verso sinistra significa il nemico sta procedendo verso
        // il fondo: l'aggiornamento del livello di profondita' deve essere eseguito immediatamente
        // affinche' il nemico possa scomparire immediatamente dietro agli oggetti
        if( enemy->getOrientation() == orientation_left || enemy->getOrientation() == orientation_up ) {
            // aggiorna immediatamente il livello di profondita' del nemico
            updateEnemyZOrder( enemy );
            // crea la sequenza di operazioni da eseguire
            auto actions    = Sequence::create( comb, func, nullptr );
            // avvia la sequenza di azioni
            enemy->runAction( actions );
        } else {
            // se la direzione e' verso il basso o verso destra significa il nemico sta procedendo in avanti:
            // l'aggiornamento del livello di profondita' deve essere eseguito immediatamente
            // dopo lo spostamento sulla nuova cella affinche' il nemico possa apparire da dietro agli oggetti

            // crea la funzione di callback (updateEnemyZOrder) da chiamare a spostamento ultimato
            auto zordfunc   = CCCallFuncN::create( this, CC_CALLFUNCN_SELECTOR( GameWorld::updateEnemyZOrder ) );
            // crea la sequenza di operazioni da eseguire
            auto actions    = Sequence::create( comb, zordfunc, func, nullptr );
            // avvia la sequenza di azioni
            enemy->runAction( actions );
        }

    } else {
        // aggiungere altri animali qui
    }
}

/*
    fine esplosione del nemico,
    il nodo chiamante e' la sprite della bomba che deve essere rimossa
*/
void GameWorld::enemyExplosionEnd( Node* node )
{
    int bombsprite_tag = node->getTag();
    // recupera l'oggetto mappa
    auto tilemap = getChildByTag( tag_tilemap );
    // rimozione sprite della bomba
    tilemap->removeChildByTag( bombsprite_tag );
}


/*
    contatto del nemico con una bomba
*/
void GameWorld::enemyExplode( Enemy *enemy )
{
    int tile_x = 0, tile_y = 0;
    // recupera le coordinate della tile occupata dal nemico,
    // le coordinate sono le stesse in cui si trova la bomba
    enemy->getTile( &tile_x, &tile_y );
    // recupera l'oggetto mappa
    auto tilemap = getChildByTag( tag_tilemap );
    if( tilemap != nullptr ) {
        // la sprite del nemico deve essere distrutta
        tilemap->removeChildByTag( enemy->getTag() );
        // la sprite da utilizzare per l'animazione dell'esplosione e' la stessa della bomba,
        // per recuperarla bisogna utilizzare il tag contenuto nella matrice pickables
        auto bomb_sprite = tilemap->getChildByTag( mapinfo.pickables[ tile_y ][ tile_x ].tag );
        // la matrice pickable deve essere aggiornata togliendo la bomba
        mapinfo.pickables[ tile_y ][ tile_x ].tag   = 0;
        mapinfo.pickables[ tile_y ][ tile_x ].type  = 0;
        // una volta recuperata la sprite della bomba...
        if( bomb_sprite != nullptr ) {
            // ...viene cancellata l'animazione corrente (bomba che gira)
            bomb_sprite->cleanup();
            // file audio
            auto audio = SimpleAudioEngine::getInstance();
            audio->playEffect( "audio/explosion.mp3", false, 1.0f, 1.0f, 1.0f );

            // viene creata l'animazione dell'esplosione e la callback da chiamare al termine
            auto explosion  = Animate::create( AnimationManager::getInstance()->animations[ animation_explosion ] );
            auto callback   = CCCallFuncN::create( this, CC_CALLFUNCN_SELECTOR( GameWorld::enemyExplosionEnd ) );
            bomb_sprite->runAction( Sequence::create( explosion, callback, nullptr)  );
        }
    }
}

/*
    il nemico e' circondato da ostacoli e non puo' piu' muoversi
*/
void GameWorld::enemyLocked( Enemy *enemy )
{
    int tile_x, tile_y;
    // imposta lo stato
    enemy->setStatus( enemystate_locked );

    // quando il nemico non si puo' piu' muovere scompare
    // il comportamento potrebbe variare a seconda del tipo di animale:

    // la sprite del nemico viene eliminata
    auto tilemap = getChildByTag( tag_tilemap );
    if( tilemap != nullptr ) {
        // prima di eliminare l'oggetto ne recupera la posizione
        enemy->getTile( &tile_x, &tile_y );
        // rimuove l'oggetto
        tilemap->removeChildByTag( enemy->getTag() );
         // animazione stelline
        auto _emitter = ParticleFlower::create();
        _emitter->setTexture( Director::getInstance()->getTextureCache()->addImage( "images/objects/star_2.png" ) );
        _emitter->setPosition( getTilePosition( tile_x, tile_y, mapinfo.half_tile_height ) );
        _emitter->setAnchorPoint( Vec2( 0.5f, 0 ) );
        _emitter->setEmissionRate( 5.0f );
        _emitter->setDuration( 1.0f );
        _emitter->setAutoRemoveOnFinish( true );
        tilemap->addChild( _emitter, 999, getTileDepth( tile_x, tile_y ) );
    }
}

/*
    aggiornamento stato di un nemico
*/
void GameWorld::updateEnemy( Node* node )
{
    Enemy   *enemy = ( ( Enemy* ) node );

    // valutazione del tipo di azione che deve intraprendere il nemico
    int action = getEnemyAction( enemy );

    //log( "Enemy action %d", action );

    // esegue il tipo di azione
    switch( action ) {
        // stato di idle: il nemico deve stazionare e ricontrollare il proprio stato dopo un certo delay
        case enemyaction_idle:
            enemyIdle( enemy );
            break;
        // spostamento da tile a tile
        case enemyaction_move:
            enemyMove( enemy );
            break;
        // il nemico ha calpestato una bomba, deve saltare in aria
        case enemyaction_explode:
            enemyExplode( enemy );
            break;
        // il nemico e' circondato da ostacoli e non puo' muoversi
        case enemyaction_locked:
            enemyLocked( enemy );
            break;
    }
}


/*
    conversione coordinate touch negli indici della cella corrispondente
*/
void GameWorld::getTouchedTile( float x, float y, int *tile_x, int *tile_y )
{
    float   touch_x, touch_y;
    float   map_w, map_h;
    float   map_x, map_y;
    float   tile0_x, tile0_y;
    float   maptouch_x, maptouch_y;
    float   map_scale   = 0;
    float   tile_w, tile_h;

    auto    tilemap = getChildByTag( tag_tilemap );

    // coordinate del tocco sullo schermo, la y e' invertita
    touch_x     = x;
    touch_y     = SCREEN_HEIGHT - y;
    // posizione e dimensioni correnti della mappa
    map_scale   = tilemap->getScale();
    map_w       = tilemap->getContentSize().width * map_scale;
    map_h       = tilemap->getContentSize().height * map_scale;
    map_x       = tilemap->getPosition().x;
    map_y       = SCREEN_HEIGHT - tilemap->getPosition().y - map_h;

    //log( "width %f height %f scale %f position %f",tilemap->getContentSize().width,tilemap->getContentSize().height,map_scale, tilemap->getPosition().x );

    // calcola dimensione delle tiles
    tile_w      = map_w / mapinfo.total_v_tiles;
    tile_h      = map_h / mapinfo.total_h_tiles;
    // coordinate delle cella 0,0 secondo la posizione e la dimensione della mappa
    // (e implicitamente lo scale factor)
    tile0_x     = map_x + ( map_w / 2 );
    tile0_y     = map_y;
    // calcola la differenza tra la posizione teoricamente toccata sulla mappa e
    // la posizione della cella 0,0
    maptouch_x  = touch_x - tile0_x;
    maptouch_y  = touch_y - tile0_y;
    // calcola gli indici x e y della cella corrispondente al tocco
    *tile_x     = (int)( ( maptouch_x / ( tile_w / 2 ) + maptouch_y / ( tile_h / 2 ) ) / 2 );
    *tile_y     = (int)( ( maptouch_y / ( tile_h / 2 ) - maptouch_x / ( tile_w / 2 ) ) / 2 );
}


/*
    struttura dati del pinch
*/
typedef struct {
    bool    started;
    long    last_distance;
} pinch_t;

// dati del pinch
pinch_t     pinch;

/*
    struttura dati per rilevazione doppio tap
*/
typedef struct {
    Vec2    position;
    double  time;
} toucheventdata_t;

// dati ultimo tocco per rilevazione doppo tap
toucheventdata_t    last_touchevent;
bool                last_touchevent_setup = false;

/*
    calcolo distanza tra due punti (senza radice quadrata per velocizzare)
*/
float getPinchDistance( Vec2 a, Vec2 b )
{
    float dx = ( b.x - a.x );
    float dy = ( b.y - a.y );
    float dist = dx * dx + dy * dy;
    return dist;
}

/*
    il giocatore ha fatto doppio tap sullo cherma alla posizione x, y
    ATTENZIONE le coordinate sono nell'ordine delle cocos2d: la y non e' invertita
*/
void GameWorld::doubleTap( float x, float y )
{
    int tile_x = 0, tile_y = 0;

    // durante il gioco vero e proprio...
    if( this->gameState == gs_running ) {
        // il doppio tap significa che il giocatore vuole piazzare un ostacolo
        // converte la posizione del touch nelle coordinate della cella selezionata
        getTouchedTile( x, y, &tile_x, &tile_y );
        // se la tile e' selezionabile e il giocatore ha ancora oggetti posizionabili oppure
        // il giocatore ha un numero illimitato di oggetti posizionabili
        if( isTileSelectable( tile_x, tile_y ) && ( mapinfo.total_user_obstacles != 0 ) ) {
            // file audio
            auto audio = SimpleAudioEngine::getInstance();
            audio->playEffect( "audio/plop.mp3", false, 1.0f, 1.0f, 1.0f );
            // aggiorna la matrice walkable modificando la tile da walktile_walkable a walktile_obstacle
            mapinfo.walkables[ tile_y ][ tile_x ] = walktile_obstacle;
            // aggiunge la sprite animata dell'ostacolo alla mappa
            addUserObstacle( tile_x, tile_y );
        } else {
            // avvisa con un suono che sulla tile non e' possibile inserire l'ostacolo
            // nella posizione selezionata
            auto audio = SimpleAudioEngine::getInstance();
            audio->playEffect( "audio/wrong.mp3", false, 1.0f, 1.0f, 1.0f );
        }
    }
}

/*
    calcola la differenza di tempo tra due variabili valorizzate con la clock(),
    il tempo ritornato dovrebbe essere in millisecondi
*/
static double diff_time( clock_t clock1, clock_t clock2 )
{
    double diff_ticks = clock1 - clock2;
    double diff_ms = diff_ticks / ( CLOCKS_PER_SEC / 1000 );
    return diff_ms;
}

/*
    callback all'inizio del tocco sul touchscreen
*/
void GameWorld::onTouchesBegan(const std::vector<Touch*>& touches, Event  *event)
{
    auto    touch1st = touches[ 0 ];
    auto    touch2nd = touches[ 1 ];
    int     tile_x  = 0;
    int     tile_y  = 0;

    // a ogni inizio tocco (primo o secondo) azzera i dati del pinch
    memset( &pinch, 0, sizeof( pinch_t ) );

    // controllo double tap solo in caso di un singolo touch
    if( touches.size() == 1 ) {
        // recupera il timestamp corrente
        clock_t cur_time = clock();
        // se e' gia' stato registrato l'ultimo evento...
        if( last_touchevent_setup ) {
            // se la differenza tra un tap e l'altro puo' essere un doppio tap (< 300ms)
            if( diff_time( cur_time, last_touchevent.time ) < 300 ) {

                // controlla se le coordinate del secondo tap sono nel margine di tolleranza
                float diff_x, diff_y;
                if( touch1st->getLocation().x > last_touchevent.position.x ) {
                    diff_x = touch1st->getLocation().x - last_touchevent.position.x;
                } else {
                    diff_x = last_touchevent.position.x - touch1st->getLocation().x;
                }
                if( touch1st->getLocation().y > last_touchevent.position.y ) {
                    diff_y = touch1st->getLocation().y - last_touchevent.position.y;
                } else {
                    diff_y = last_touchevent.position.y - touch1st->getLocation().y;
                }
                // tarare il margine di tolleranza dei due tap (attualmente 20 px)
                if( diff_x < 30 && diff_y < 30 ) {
                    // indica al gioco che e' stato eseguito un doppio tap sullo schermo
                    doubleTap( touch1st->getLocation().x, touch1st->getLocation().y );
                }
            }
            // memorizza la posizione e il timestamp dell'ultimo tocco
            last_touchevent.position    = touch1st->getLocation();
            last_touchevent.time        = cur_time;
        } else {
            // memorizza la posizione e il timestamp dell'ultimo tocco
            last_touchevent.position    = touch1st->getLocation();
            last_touchevent.time        = cur_time;
            // questo controllo evita che il primo tocco venga interpretato come doppio touch
            last_touchevent_setup       = true;
        }
    }
}

/*
    callback al trascinamento del dito sul touchscreen
*/
void GameWorld::onTouchesMoved(const std::vector<Touch*>& touches, Event  *event)
{
    auto    touch   = touches[0];
    int     tile_x  = 0;
    int     tile_y  = 0;

/*
    log( "touches size %d", touches.size() );
    auto touch2nd = touches[1];
    log( "1st touch %f %f", touch->getLocation().x, touch->getLocation().y );
    if( touch2nd ) {
        log( "2nd touch %f %f", touch2nd->getLocation().x, touch2nd->getLocation().y );
    }
*/
    // tocco singolo
    if( touches.size() == 1 ) {

        // con un tocco singolo sposta la mappa
        // andrebbe limitato lo spostamento per non far uscire la mappa dallo schermo
        auto diff = touch->getDelta();
        auto node = getChildByTag( tag_tilemap );
        auto currentPos = node->getPosition();
        node->setPosition( currentPos + diff );

    } else if( touches.size() == 2 ) {
        // doppio tocco: recupera info seconda posizione di tocco
        auto touch2nd = touches[ 1 ];

        // il pinch deve essere inizializzato al primo tocco altrimenti non e' possibile calcolare
        // la distanza iniziale tra il primo punto e il secondo punto toccato
        if( pinch.started == false ) {
            pinch.started = true;
            pinch.last_distance = getPinchDistance( touch->getLocation(), touch2nd->getLocation() );
        } else {
            // calcola la distanza tra i due punti attualmente toccati
            float new_distance = getPinchDistance( touch->getLocation(), touch2nd->getLocation() );
            //log( "new_distance %f pinch.last_distance %f", new_distance, pinch.last_distance );
            // se la distanza e' maggiore significa che la distanza e' cresciuta: zoom in
            if( pinch.last_distance < new_distance ) {
                // se e' gia' in corso un operazione di zoom non viene eseguita alcuna operazione
                if( this->zooming == false ) {
                    // ...altrimenti calcolo il fattore di scalo in base alla differenza
                    // tra nuova distanza e vecchia distanza
                    float scale_factor = ( new_distance - pinch.last_distance ) / 100000;
                    // avvia l'operazione di zoom con durata 0ms
                    zoomIn( 1 + scale_factor, 0 );
                    // memorizza l'ultima distanza (quella corrente)
                    pinch.last_distance = new_distance;
                }
            } else {
                // se la distanza e' minore significa che la distanza e' diminuita: zoom out
                // se e' gia' in corso un operazione di zoom non viene eseguita alcuna operazione
                if( this->zooming == false ) {
                    // ...altrimenti calcolo il fattore di scalo in base alla differenza
                    // tra nuova distanza e vecchia distanza
                    float scale_factor = ( pinch.last_distance - new_distance ) / 100000;
                    // avvia l'operazione di zoom con durata 0ms
                    zoomOut( 1 - scale_factor, 0 );
                    // memorizza l'ultima distanza (quella corrente)
                    pinch.last_distance = new_distance;
                }
            }
        }
    }
}

/*
    aggiunge un ostacolo sulla mappa nella tile scelta dal giocatore
*/
void GameWorld::addUserObstacle( int tile_x, int tile_y )
{
    auto tilemap = (TMXTiledMap*)getChildByTag( tag_tilemap );


    // crea la sprite per l'ostacolo con un immagine di default,
    // l'immagine viene immediatamente sostituita dall'animazione
    auto obstacle = Sprite::create( "images/objects/flower/flower_default.png" );
    obstacle->setAnchorPoint( Vec2( 0.5f, 0 ) );
    obstacle->setPosition( getTilePosition( tile_x, tile_y, -( mapinfo.half_tile_height ) ) );
    tilemap->addChild( obstacle, getTileDepth( tile_x, tile_y ), SPRITE_TAG_USER_OBSTACLE );

    if( mapinfo.user_obstacle_type == userobstacle_flower_pot ) {
        // vaso di fiori
        auto animate        = Animate::create( AnimationManager::getInstance()->animations[ animation_flower ] );
        auto animate_back   = animate->reverse();
        auto action         = RepeatForever::create( Sequence::create( animate, animate_back, nullptr ) );
        obstacle->runAction( action );
    } else {
        // funghetto
        auto animate        = Animate::create( AnimationManager::getInstance()->animations[ animation_mushroom ] );
        auto action         = RepeatForever::create( animate );
        obstacle->runAction( action );
    }

    // aggiorna il numero di ostacoli disponibili
    mapinfo.total_user_obstacles -= 1;
    setObstaclesNumberLabelText( mapinfo.total_user_obstacles );
}

/*
    callback al rilascio del dito dal touchscreen
*/
void GameWorld::onTouchesEnded(const std::vector<Touch*>& touches, Event  *event)
{
    // niente da fare
}

/*
    fine operazione di zoom
*/
void GameWorld::zoomEnd()
{
    this->zooming = false;
}

/*
    avvio operazione di zoom in sulla mappa, in realta' viene aumentata la dimensione
    della mappa, gli oggetti figli vengono ingranditi automaticamente
*/
void GameWorld::zoomIn( float scale_factor, float duration )
{
    //log( "zoomIn scale_factor %f duration %d", scale_factor, duration );

    // oggetto mappa
    auto    tilemap    = getChildByTag( tag_tilemap );

    // fattore massimo di scala, attualmente 2x
    if( tilemap->getScale() > 2.0f ) {
        return;
    }
    // flag operazione di zoom in corso (verra' resettato dalla zoomEnd())
    this->zooming = true;
    // crea l'azione di riscalamento
    auto    scale      = ScaleBy::create( duration, scale_factor );
    // correzione della posizione della mappa (a causa del riscalamento)
    float   curw = mapinfo.map_width * tilemap->getScale();
    float   neww = curw * scale_factor;
    float   wdiff = ( neww - curw ) / 2;
    float   curh = mapinfo.map_height * tilemap->getScale();
    float   newh = curh * scale_factor;
    float   hdiff = ( newh - curh ) / 2;
    auto    adjustpos  = MoveBy::create( duration, Vec2( -wdiff, -hdiff ) );
    // la scalamento e lo spostamento devono avvenire contemporaneamente
    auto    zoomaction = Spawn::create( scale, adjustpos, nullptr );
    // funzione di callback al termine dell'operazione di zoom
    auto    callback    = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::zoomEnd ) );
    // avvia la sequenza di azioni + callback
    tilemap->runAction( Sequence::create( zoomaction, callback, nullptr ) );
}


/*
    avvio operazione di zoom out sulla mappa, in realta' viene ridotta la dimensione
    della mappa, gli oggetti figli vengono ridotti automaticamente
*/
void GameWorld::zoomOut( float scale_factor, float duration )
{
    // oggetto mappa
    auto    tilemap    = getChildByTag( tag_tilemap );

    // fattore minimo di scala, attualmente 1x
    if( tilemap->getScale() < 1.0f ) {
        return;
    }
    // flag operazione di zoom in corso (verra' resettato dalla zoomEnd())
    this->zooming = true;
    // crea l'azione di riscalamento
    auto    scale      = ScaleBy::create( duration, scale_factor );
    // correzione della posizione della mappa (a causa del riscalamento)
    float   curw = mapinfo.map_width * tilemap->getScale();
    float   neww = curw * scale_factor;
    float   wdiff = ( curw - neww ) / 2;
    float   curh = mapinfo.map_height * tilemap->getScale();
    float   newh = curh * scale_factor;
    float   hdiff = ( curh - newh ) / 2;
    auto    adjustpos  = MoveBy::create( duration, Vec2( wdiff, hdiff ) );
    // la scalamento e lo spostamento devono avvenire contemporaneamente
    auto    zoomaction = Spawn::create( scale, adjustpos, nullptr );
    // funzione di callback al termine dell'operazione di zoom
    auto    callback    = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::zoomEnd ) );
    // avvia la sequenza di azioni + callback
    tilemap->runAction( Sequence::create( zoomaction, callback, nullptr ) );
}




/*
    rimuove la sprite per l'animazione di fine livello
*/
void GameWorld::removeEndLevelSprite()
{
    this->removeChildByTag( tag_endlevelsprite );
    this->removeChildByTag( tag_endlevelsprite_2nd );
}

/*
    animazione passaggio al livello successivo
*/
void GameWorld::goToNewLevel()
{
    setTutorialLabelText( "" );
    setDebugLabelText( "Go to new level" );
    // imposta lo stato generale di gioco
    enterGameState( gs_level_complete );

    // se e' l'ultimo livello visualizza il cut scene finale
    if( currentLevel == ( MAX_LEVELS - 1 ) ) {
        enterGameState( gs_final_scene );
        startFinalCutScene();
        return;
    }

    // file audio
    auto audio = SimpleAudioEngine::getInstance();
    audio->playEffect( "audio/level_complete.mp3", false, 1.0f, 1.0f, 1.0f );

    // recupera la sprite con le nuvole per animazione ingresso nel livello
    auto clouds = this->getChildByTag( tag_clouds );
    // animazione fade in delle nuvole
    auto fadein     = FadeIn::create( 1.0f );
    // crea la funzione di callback da chiamare ad animazione finita
    auto func1      = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::loadNextLevel ) );
    // crea la sequenza di operazioni da eseguire
    auto actions    = Sequence::create( fadein, DelayTime::create( 1.0f ), func1, nullptr );
    // avvia animazione
    clouds->runAction( actions );

    // crea la sprite per l'animazione del coniglietto che vola appeso ai palloni
    auto endLevelSprite = Sprite::create( "images/bunny/bunny_baloon.png" );
    endLevelSprite->setAnchorPoint( Vec2( 0.5f, 0 ) );
    endLevelSprite->setPosition( Vec2( 320, -( endLevelSprite->getContentSize().height ) ) );
    this->addChild( endLevelSprite, 1001, tag_endlevelsprite );
    auto bunny_move = MoveBy::create( 2.0f, Vec2( 0, 480 + getContentSize().height ) );
    auto func2      = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::removeEndLevelSprite ) );
    endLevelSprite->runAction( Sequence::create( bunny_move, func2, nullptr ) );
}

/*
    animazione game over + ricomincia il livello da capo
*/
void GameWorld::gameOver( int reason )
{
    // imposta lo stato generale di gioco
    enterGameState( gs_game_over );

    // file audio
    auto audio = SimpleAudioEngine::getInstance();

    // recupera la sprite con le nuvole per animazione uscida dal livello
    auto clouds = this->getChildByTag( tag_clouds );
    // animazione fade in delle nuvole
    auto fadein     = FadeIn::create( 1.0f );
    // crea la funzione di callback da chiamare ad animazione finita (per ricaricare lo stesso livello)
    auto func1      = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::reloadLevel ) );
    // crea la sequenza di operazioni da eseguire
    auto actions    = Sequence::create( fadein, DelayTime::create( 1.0f ), func1, nullptr );
    // avvia animazione
    clouds->runAction( actions );

    // a seconda del motivo per cui termina il livello c'e' una specifica animazione...
    if( reason == gameover_explode ) {
        // crea la sprite per l'animazione del coniglietto esplode
        auto endLevelSprite = Sprite::create();
        endLevelSprite->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
        endLevelSprite->setPosition( Vec2( 320, 240 ) );
        this->addChild( endLevelSprite, 1001, tag_endlevelsprite );
        auto endLevelSprite_animation = Animate::create( AnimationManager::getInstance()->animations[ animation_bunny_burning ] );
        auto func2      = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::removeEndLevelSprite ) );
        endLevelSprite->runAction( Sequence::create( endLevelSprite_animation, func2, nullptr ) );
        audio->playEffect( "audio/game_over.mp3", false, 1.0f, 1.0f, 1.0f );

    } else if( reason == gameover_block ) {

        // crea la sprite del coniglietto indeciso
        audio->playEffect( "audio/game_over.mp3", false, 1.0f, 1.0f, 1.0f );

        auto endLevelSprite = Sprite::create( "images/bunny/bunny_undecided.png" );
        endLevelSprite->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
        endLevelSprite->setPosition( Vec2( 320, 240 ) );
        this->addChild( endLevelSprite, 1001, tag_endlevelsprite );
        auto func2      = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::removeEndLevelSprite ) );
        endLevelSprite->runAction( Sequence::create( DelayTime::create( 1.0f ), func2, nullptr ) );

    } else if( reason == gameover_prick ) {

        // crea la sprite per il timbro con scritto "beccato!"
        auto endLevelSprite = Sprite::create( "images/bunny/stamp.png" );
        endLevelSprite->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
        endLevelSprite->setPosition( Vec2( 320, 174 ) );
        endLevelSprite->setScale( 2.0f, 2.0f );
        // crea la sprite per il coniglio da timbrare
        auto endLevelSprite2nd = Sprite::create( "images/bunny/bunny_thinking.png" );
        endLevelSprite2nd->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
        endLevelSprite2nd->setPosition( Vec2( 320, 240 ) );
        // il timbro deve stare sopra al coniglio quindi ha un valore piu' alto (1002>1001)
        this->addChild( endLevelSprite, 1002, tag_endlevelsprite );
        this->addChild( endLevelSprite2nd, 1001, tag_endlevelsprite_2nd );

        auto stamp_resize = ScaleBy::create( 0.3f, 0.5f, 0.5f );
        auto func2      = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::removeEndLevelSprite ) );
        endLevelSprite->runAction( Sequence::create( stamp_resize, DelayTime::create( 1.0f ), func2, nullptr ) );
        audio->playEffect( "audio/stamp.mp3", false, 1.0f, 1.0f, 1.0f );
    }
}

/*
    passaggio automatico al livello successivo (debug)
*/
void GameWorld::button_debug_callback( Ref* pSender )
{
#if 0
    goToNewLevel();
#endif
}

/*
    stato della scena finale
*/
typedef enum {
    css_none,
    css_rocket_flight,
    css_bunny_on_the_moon,
    css_last_fade,
    css_max
} cutscenestatus_t;

/*
    la scena finale di divide nelle seguenti fasi:
    - fade in nuvole + partenza del razzo
    - fade out nuvole + background finale con animazione cuori
    - fade in nuvole
    - uscita (ritorno al menu di gioco)
*/
void GameWorld::cutSceneCallback()
{
    if( cutSceneStatus == css_rocket_flight ) {

        // imposta lo stato corrente (volo del razzo) dell'animazione (per la callback)
        cutSceneStatus = css_bunny_on_the_moon;

        // crea sprite per sfondo finale
        auto moon_background = Sprite::create( "images/end/moon_background.png" );
        moon_background->setAnchorPoint( Vec2( 0, 0 ) );
        moon_background->setPosition( Vec2( 0, 0 ) );
        this->addChild( moon_background, 998 );

        // animazione cuoricini
        auto _emitter = ParticleFlower::create();
        _emitter->setEmitterMode( ParticleSystem::Mode::RADIUS );
        _emitter->setTexture( Director::getInstance()->getTextureCache()->addImage( "images/end/heart.png" ) );
        _emitter->setPosition( Vec2( 320, 240 ) );
        _emitter->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
        _emitter->setEmissionRate( 10.0f );
        _emitter->setDuration( 5.0f );
        _emitter->setEndRadius( 300 );
        _emitter->setAutoRemoveOnFinish( true );
        this->addChild( _emitter, 999 );

        // recupera la sprite con le nuvole per animazione ingresso nel livello
        auto clouds = this->getChildByTag( tag_clouds );
        // animazione fade out delle nuvole
        auto fadeout_animation = FadeOut::create( 1.0f );
        // crea la funzione di callback da chiamare ad animazione finita
        auto func       = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::cutSceneCallback ) );
        // crea la sequenza di operazioni da eseguire: fade out nuvole + attesa 10 secondi + callback (=ritorno al menu)
        auto actions    = Sequence::create( fadeout_animation, DelayTime::create( 7.0f ), func, nullptr );
        // avvia animazione
        clouds->runAction( actions );

    } else if ( cutSceneStatus == css_bunny_on_the_moon ) {
         // imposta lo stato corrente (volo del razzo) dell'animazione (per la callback)
        cutSceneStatus = css_last_fade;
        // recupera la sprite con le nuvole per animazione di uscita dal livello
        auto clouds = this->getChildByTag( tag_clouds );
        // crea l'animazione di tipo fade in della durata di 1 secondo
        auto fadein     = FadeIn::create( 1.0f );
        // crea la funzione di callback da chiamare ad animazione finita
        auto func       = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::cutSceneCallback ) );
        // crea la sequenza di operazioni da eseguire: fadein -> attesa 1 sec -> chiamata alla callback
        auto actions    = Sequence::create( fadein, DelayTime::create( 0.5f ), func, nullptr );
        // avvia animazione del fade in delle nuvole
        clouds->runAction( actions );

    } else {
        // uscita dal gioco
        Director::getInstance()->popScene();
    }
}


/*
    scena finale = gioco completato
*/
void GameWorld::startFinalCutScene()
{
    // imposta lo stato corrente (volo del razzo) dell'animazione (per la callback)
    cutSceneStatus = css_rocket_flight;

    // avvio musica finale
    auto audio = SimpleAudioEngine::getInstance();
    audio->stopBackgroundMusic();
    audio->playBackgroundMusic( "audio/final.mp3", false );

    // recupera la sprite con le nuvole per animazione di uscita dal livello
    auto clouds = this->getChildByTag( tag_clouds );
    // crea l'animazione di tipo fade in della durata di 1 secondo
    auto fadein     = FadeIn::create( 1.0f );
    // crea la funzione di callback da chiamare ad animazione finita
    auto func       = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::cutSceneCallback ) );
    // crea la sequenza di operazioni da eseguire: fadein -> attesa 1 sec -> chiamata alla callback
    auto actions    = Sequence::create( fadein, DelayTime::create( 2.4f ), func, nullptr );
    // avvia animazione del fade in delle nuvole
    clouds->runAction( actions );

    // crea la sprite per il razzo che porta il coniglietto sulla luna
    auto rocket = Sprite::create( "images/end/rocket.png");
    rocket->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    rocket->setPosition( Vec2( 0, 0 ) );
    this->addChild( rocket, 1001, tag_endlevelsprite );

    auto rocket_move        = MoveBy::create( 2.3f, Vec2( 800, 480 ) );
    auto rocket_rotate      = RotateBy::create( 2.3f, 70 );
    auto rocket_actions     = Spawn::create( rocket_move, rocket_rotate, nullptr );
    auto rocket_autoremove  = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameWorld::removeEndLevelSprite ) );
    rocket->runAction( Sequence::create( rocket_actions, rocket_autoremove, nullptr ) );
}

/*
    torna al menu
*/
void GameWorld::button_close_callback(Ref* pSender)
{
    // controllo stato: il tasto di uscita deve essere abilitato in fase di running
    if( gameState == gs_running ) {
        Director::getInstance()->popScene();
    }
}
