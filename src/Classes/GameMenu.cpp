/*
    the Game Menu
*/

#include "cocos2d.h"
#include "ui/UILoadingBar.h"
#include "ui/UIButton.h"
#include "ui/UIImageView.h"
#include "GameMenu.h"
#include "GameWorldScene.h"
#include "GameDB.h"
#include "AnimationManager.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace ui;
using namespace CocosDenshion;


/*
    crea la scena contenente il menu
*/
Scene* GameMenu::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    // 'layer' is an autorelease object
    auto layer = GameMenu::create();
    // add layer as a child to scene
    scene->addChild( layer );
    // return the scene
    return scene;
}

#define MENU_TAG                1
#define BUTTON_LEVEL_BASE_TAG   100
#define BUTTON_LEVEL_INFO_TAG   200
#define COMPLETION_LABEL_TAG    300


// on "init" you need to initialize your instance
bool GameMenu::init()
{

    //////////////////////////////
    // 1. super init first
    if ( !LayerColor::initWithColor( Color4B( 199, 101, 255, 255 ) ) )
    {
        return false;
    }

    // creazione e posizionamento dell'oggetto menu contenente tutti i tasti
    auto menu = Menu::create();
    menu->setTag( MENU_TAG );
    this->addChild( menu );

    menu->setAnchorPoint( Vec2::ZERO );
    menu->setPosition( Vec2::ZERO );

    // ======================== pagina 0 =====================
    // TODO togliere!!!!! solo per debug
    //GameDB::getInstance()->setTotalUnlockedLevels( 29 );
    //log( "TotalUnlockedLevels %d", GameDB::getInstance()->getTotalUnlockedLevels() );


    // crea il titoletto
    auto littletitle = Sprite::create( "images/backgrounds/title_little.png" );
    littletitle->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    menu->addChild( littletitle );
    littletitle->setPosition( Vec2( 320, 426 ) );

    // crea il tasto di selezione livelli ("LEVELS")
    auto btn_levels = Button::create("images/buttons/button_hexgreen.png", "images/buttons/button_hexgreen.png" );
    btn_levels->setTitleText("Levels");
    btn_levels->setTitleFontName("fonts/Marker Felt.ttf");
    btn_levels->setTitleFontSize( 42 );
    btn_levels->setTitleColor( Color3B::WHITE );
    btn_levels->setPressedActionEnabled( true );
    btn_levels->getTitleRenderer()->enableShadow( Color4B::BLACK, Size( 2,-2 ) );
    btn_levels->addClickEventListener([=](Ref* sender) {
        // se non e' gia' in corso uno slittamento...
        if( sliding ) {
            return;
        }
        // imposta subito il flag di slide
        sliding = true;
        // file audio
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect( "audio/button.mp3", false, 1.0f, 1.0f, 1.0f );
        // sposta il menu alla prima pagina dei livelli sbloccati
        auto moveaction = MoveBy::create( 0.2f, Vec2( -SCREEN_WIDTH, 0 ) );
        auto callback   = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameMenu::endSliding ) );
        menu->runAction( Sequence::create( moveaction, callback, nullptr ) );
    });
    btn_levels->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    menu->addChild( btn_levels );
    btn_levels->setPosition( Vec2( 190, 280 ) );

    // crea il tasto di EXIT
    auto btn_exit = Button::create("images/buttons/button_hexviolet.png", "images/buttons/button_hexviolet.png" );
    btn_exit->setTitleText("Exit");
    btn_exit->setTitleFontName("fonts/Marker Felt.ttf");
    btn_exit->setTitleFontSize( 42 );
    btn_exit->setTitleColor( Color3B::WHITE );
    btn_exit->setPressedActionEnabled( true );
    btn_exit->getTitleRenderer()->enableShadow( Color4B::BLACK,Size(2,-2) );
    btn_exit->addClickEventListener([=](Ref* sender) {
        // file audio
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect( "audio/button.mp3", false, 1.0f, 1.0f, 1.0f );
        exitGame();
    });
    btn_exit->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    menu->addChild( btn_exit );
    btn_exit->setPosition( Vec2( 320, 150 ) );


    // crea il tasto di HELP
    auto btn_help = Button::create("images/buttons/button_hexblue.png", "images/buttons/button_hexblue.png" );
    btn_help->setTitleText("Help");
    btn_help->setTitleFontName("fonts/Marker Felt.ttf");
    btn_help->setTitleFontSize( 42 );
    btn_help->setTitleColor( Color3B::WHITE );
    btn_help->setPressedActionEnabled( true );
    btn_help->getTitleRenderer()->enableShadow( Color4B::BLACK, Size( 2,-2 ) );
    btn_help->addClickEventListener([=](Ref* sender) {
        // se non e' gia' in corso uno slittamento...
        if( sliding ) {
            return;
        }
        // imposta subito il flag di slide
        sliding = true;
        // file audio
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect( "audio/button.mp3", false, 1.0f, 1.0f, 1.0f );
        // sposta il menu alla prima pagina dei livelli sbloccati
        auto moveaction = MoveBy::create( 0.2f, Vec2( SCREEN_WIDTH, 0 ) );
        auto callback   = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameMenu::endSliding ) );
        menu->runAction( Sequence::create( moveaction, callback, nullptr ) );
    });
    btn_help->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    menu->addChild( btn_help );
    btn_help->setPosition( Vec2( 450, 280 ) );


    // crea la sprite con l'immagine contenente le istruzioni da visualizzare pagina di help
    auto infoimage = Sprite::create( "images/backgrounds/infopage.png" );
    infoimage->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    menu->addChild( infoimage );
    infoimage->setPosition( Vec2( -320, 240 ) );


    // label con il numero di oggetti piazzabili rimanenti
    auto completionlabel    = Label::createWithTTF( "0%", "fonts/Marker Felt.ttf", 36 );
	completionlabel->setHorizontalAlignment( TextHAlignment::LEFT );
	completionlabel->setTextColor( Color4B::WHITE );
	completionlabel->enableGlow( Color4B::YELLOW );
	completionlabel->enableShadow();
	completionlabel->setTag( COMPLETION_LABEL_TAG );
	completionlabel->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    menu->addChild( completionlabel );
	completionlabel->setPosition( Vec2( -330, 60 ) );

    // label con il disclaimer per audionautix.com
    auto audionautixlabel    = Label::createWithTTF( "Music by audionautix.com", "fonts/Marker Felt.ttf", 24 );
	audionautixlabel->setHorizontalAlignment( TextHAlignment::LEFT );
	audionautixlabel->setTextColor( Color4B::WHITE );
	audionautixlabel->enableGlow( Color4B::YELLOW );
	audionautixlabel->enableShadow();
	audionautixlabel->setTag( COMPLETION_LABEL_TAG );
	audionautixlabel->setAnchorPoint( Vec2( 0, 0.5f ) );
    menu->addChild( audionautixlabel );
	audionautixlabel->setPosition( Vec2( -600, 20 ) );

     // crea il tasto di uscita ("back") dalla pagina di help
    auto btn_help_back = Button::create("images/buttons/bottom_button.png", "images/buttons/bottom_button.png" );
    btn_help_back->setTitleText("Back");
    btn_help_back->setTitleFontName("fonts/Marker Felt.ttf");
    btn_help_back->setTitleFontSize( 38 );
    btn_help_back->setTitleColor( Color3B::WHITE );
    btn_help_back->setPressedActionEnabled( true );
    btn_help_back->getTitleRenderer()->enableShadow( Color4B::BLACK,Size(2,-2) );
    btn_help_back->addClickEventListener([=](Ref* sender) {
        // se non e' gia' in corso uno slittamento...
        if( sliding ) {
            return;
        }
        // imposta subito il flag di slide
        sliding = true;
        // file audio
        auto audio = SimpleAudioEngine::getInstance();
        audio->playEffect( "audio/button.mp3", false, 1.0f, 1.0f, 1.0f );
        // riporta il menu dalla pagina 1 alla pagina 0
        auto moveaction = MoveBy::create( 0.2f, Vec2( -SCREEN_WIDTH, 0 ) );
        auto callback   = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameMenu::endSliding ) );
        menu->runAction( Sequence::create( moveaction, callback, nullptr ) );
    });
    btn_help_back->setAnchorPoint( Vec2::ZERO );
    menu->addChild( btn_help_back );
    btn_help_back->setPosition( Vec2( -220, 5 ) );


    // numero pagine livelli in base al numero massimo di livelli
    #define MAX_LEVELS_PAGES ( MAX_LEVELS / 15 )

    // crea i tasti "next" e "back" per tutte le pagine di selezione dei livelli
    for(int page = 0; page < MAX_LEVELS_PAGES; page++)
    {
         // Create the Back/Forward buttons for the page.
         // Back arrow if there is a previous page.
        // crea il tasto di BACK (Pagina 1)
        auto btn_back = Button::create("images/buttons/bottom_button.png", "images/buttons/bottom_button.png" );
        btn_back->setTitleText("Back");
        btn_back->setTitleFontName("fonts/Marker Felt.ttf");
        btn_back->setTitleFontSize( 38 );
        btn_back->setTitleColor( Color3B::WHITE );
        btn_back->setPressedActionEnabled( true );
        btn_back->getTitleRenderer()->enableShadow( Color4B::BLACK,Size(2,-2) );
        btn_back->addClickEventListener([=](Ref* sender) {
            // se non e' gia' in corso uno slittamento...
            if( sliding ) {
                return;
            }
            // imposta subito il flag di slide
            sliding = true;
            // file audio
            auto audio = SimpleAudioEngine::getInstance();
            audio->playEffect( "audio/button.mp3", false, 1.0f, 1.0f, 1.0f );
            // riporta il menu dalla pagina 1 alla pagina 0
            auto moveaction = MoveBy::create( 0.2f, Vec2( SCREEN_WIDTH, 0 ) );
            auto callback   = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameMenu::endSliding ) );
            menu->runAction( Sequence::create( moveaction, callback, nullptr ) );
        });
        btn_back->setAnchorPoint( Vec2::ZERO );
        menu->addChild( btn_back );
        btn_back->setPosition( Vec2( ( SCREEN_WIDTH * ( page + 1 ) ) + 20, 5 ) );

        if( page < ( MAX_LEVELS_PAGES - 1 ) ) {
            auto btn_next = Button::create("images/buttons/bottom_button.png", "images/buttons/bottom_button.png" );
            btn_next->setTitleText("Next");
            btn_next->setTitleFontName("fonts/Marker Felt.ttf");
            btn_next->setTitleFontSize( 38 );
            btn_next->setTitleColor( Color3B::WHITE );
            btn_next->setPressedActionEnabled( true );
            btn_next->getTitleRenderer()->enableShadow( Color4B::BLACK,Size(2,-2) );
            btn_next->addClickEventListener([=](Ref* sender) {
                // se non e' gia' in corso uno slittamento...
                if( sliding ) {
                    return;
                }
                // imposta subito il flag di slide
                sliding = true;
                // file audio
                auto audio = SimpleAudioEngine::getInstance();
                audio->playEffect( "audio/button.mp3", false, 1.0f, 1.0f, 1.0f );
                // riporta il menu dalla pagina 1 alla pagina 0
                auto moveaction = MoveBy::create( 0.2f, Vec2( -SCREEN_WIDTH, 0 ) );
                auto callback   = CCCallFunc::create( this, CC_CALLFUNC_SELECTOR( GameMenu::endSliding ) );
                menu->runAction( Sequence::create( moveaction, callback, nullptr ) );
            });
            btn_next->setAnchorPoint( Vec2::ZERO );
            menu->addChild( btn_next );
            btn_next->setPosition( Vec2( ( SCREEN_WIDTH * ( page + 1 ) ) + 420, 5 ) );
        }
    }

    return true;
}

void GameMenu::endSliding()
{
    sliding = false;
}

void GameMenu::onEnter()
{
    Layer::onEnter();
    if( levels_created ) {
        // aggiorna i tasti e le info relativi ai livelli
        if( !updateLevelButtons() ) {
            // gestire l'errore
        }
    } else {
        // crea i tasti e le info relativi ai livelli
        if( createLevelButtons() ) {
            // gestire l'errore
            levels_created = true;
        } else {
            // gestire l'errore!
        }
    }
}


bool GameMenu::createLevelButtons()
{
    bool result = false;
    // recupera l'oggetto menu
    auto menu = this->getChildByTag( MENU_TAG );
    if( menu ) {
        // recupera il numero di livelli sbloccati
        int total_unlocked = GameDB::getInstance()->getTotalUnlockedLevels();
        int page = 1;
        int xpos = 30;
        int ypos = 105;
        for( int i = 0; i < MAX_LEVELS; i++ ) {
            auto btn_level = Button::create( button_level_normal, button_level_pressed, button_level_disabled );
            char levstr[ 10 ];
            sprintf( levstr, "%d", i + 1 );
            btn_level->setTitleText( levstr );
            btn_level->setTitleFontName("fonts/Marker Felt.ttf");
            btn_level->setTitleFontSize( 48 );
            btn_level->setTitleColor( Color3B( 0, 0, 255 ) );
            btn_level->setPressedActionEnabled( true );
            btn_level->getTitleRenderer()->enableShadow( Color4B::BLACK,Size(2,-2) );
            btn_level->addClickEventListener([=](Ref* sender) {
                // file audio
                auto audio = SimpleAudioEngine::getInstance();
                audio->playEffect( "audio/button.mp3", false, 1.0f, 1.0f, 1.0f );

                // fa partire il livello selezionato
                startFromLevel( i );

            });
            btn_level->setAnchorPoint( Vec2::ZERO );

            bool level_unlocked = ( i <= total_unlocked );
            // NOTE la setEnabled da sola non fa un cazzo (non aggiorna la texture del tasto)
            // bisogna forzare la ristampa usando la setBright()!
            btn_level->setEnabled( level_unlocked );
            btn_level->setBright( level_unlocked );

            menu->addChild( btn_level );
            btn_level->setPosition( Vec2( ( page * SCREEN_WIDTH ) + xpos, SCREEN_HEIGHT - ypos ) );
            // imposta il tag per poter recuperare il tasto e aggiornarlo
            btn_level->setTag( BUTTON_LEVEL_BASE_TAG + i );


            // il numero di stelle nell'immagine deve essere proporzionale
            // alla percentuale di completamento del livello
            int levelProgress   = GameDB::getInstance()->getLevelProgress( i );
            int imageId         = getStarsFromProgress( levelProgress );
            auto levelInfo      = ImageView::create( level_stars_images[ imageId ] );
            levelInfo->setAnchorPoint( Vec2::ZERO );
            levelInfo->setPosition( Vec2( ( page * SCREEN_WIDTH ) + xpos + 5, SCREEN_HEIGHT - ( ypos + 20 ) ) );
            levelInfo->setTag( BUTTON_LEVEL_INFO_TAG + i );
            menu->addChild( levelInfo );


            // aggiorna la posizione x, in teoria il prossimo viene stampato a destra del tasto corrente...
            xpos += 120;
            // ma al raggiungimento del quinto tasto...
            if( xpos > 510 ) {
                // si passa alla riga successiva, viene aggiornata la posizione y e resettata la posizione x
                xpos = 30;
                ypos += 122;
                // quando viene raggiunta la fine della terza riga si passa...
                if( ypos > 349 ) {
                    // si passa alla pagina successiva, viene resettata la posizione y
                    ypos = 105;
                    page += 1;
                }
            }
        }

        // aggiorna la percentuale di completamento di gioco
        auto completionlabel = (Label*) menu->getChildByTag( COMPLETION_LABEL_TAG );
        if( completionlabel != nullptr ) {
            //log( "getGameCompletion %d", getGameCompletion() );
            char tmpstr[ 10 ];
            sprintf( tmpstr, "%d%%", getGameCompletion() );
            ((Label*)completionlabel)->setString( tmpstr );
        }

        // esito positivo
        result = true;
    } else {
        // gestire l'errore
    }
    return result;
}

/*
    ritorna un indice corrispondente a
    0   ->  0 stelle
    1   ->  1 stella
    2   ->  2 stelle
    3   ->  3 stelle
*/
int  GameMenu::getStarsFromProgress( int progress )
{
    if( progress >= 100 ) {
        return 3;
    } else if ( progress >= 66 ) {
        return 2;
    } else if ( progress >= 33 ) {
        return 1;
    } else {
        return 0;
    }
}

int GameMenu::getGameCompletion()
{
    long    levelCompletion = 0;
    long    totalCompletion = 0;
    int     completion      = 0;

    totalCompletion = MAX_LEVELS * 100;
    for( int i = 0; i < MAX_LEVELS; i++ ) {
        levelCompletion += GameDB::getInstance()->getLevelProgress( i );
    }
    completion = ( levelCompletion * 100 / totalCompletion );
    return completion;
}

bool GameMenu::updateLevelButtons()
{
    bool result = false;
    // recupera l'oggetto menu
    auto menu = this->getChildByTag( MENU_TAG );
    if( menu ) {
        // recuperare il numero di livelli sbloccati
        int total_unlocked = GameDB::getInstance()->getTotalUnlockedLevels();
        for( int i = 0; i < MAX_LEVELS; i++ ) {
            auto btn_level = (Button*) menu->getChildByTag( BUTTON_LEVEL_BASE_TAG + i );
            if( btn_level ) {
                bool level_unlocked = ( i <= total_unlocked );
                // NOTE la setEnabled da sola non fa un cazzo (non aggiorna la texture del tasto)
                // bisogna forzare la ristampa usando la setBright()!
                btn_level->setEnabled( level_unlocked );
                btn_level->setBright( level_unlocked );
            }
            auto level_info = (ImageView*) menu->getChildByTag( BUTTON_LEVEL_INFO_TAG + i );
            if( level_info ) {
                // il numero di stelle nell'immagine deve essere proporzionale
                // alla percentuale di completatmento del livello
                int levelProgress   = GameDB::getInstance()->getLevelProgress( i );
                int imageId         = getStarsFromProgress( levelProgress );
                level_info->loadTexture( level_stars_images[ imageId ] );
            }
        }

        auto completionlabel = (Label*) menu->getChildByTag( COMPLETION_LABEL_TAG );
        if( completionlabel != nullptr ) {
            //log( "getGameCompletion %d", getGameCompletion() );
            char tmpstr[ 10 ];
            sprintf( tmpstr, "%d%%", getGameCompletion() );
            ((Label*)completionlabel)->setString( tmpstr );
        }
    }
    return result;
}

void GameMenu::exitGame()
{
    // in caso di uscita scarica tutte le animazioni
    //AnimationManager::getInstance()->unloadAll();
    // e chiude il gioco
    Director::getInstance()->end();
}

void GameMenu::startFromLevel( int level )
{
    // crea la scena di gioco
    auto scene = GameWorld::createScene();
    // e la fa diventare la scena corrente
    Director::getInstance()->pushScene( scene );
    // recupera l'oggetto scena...
    auto gameWorldLayer = scene->getChildByTag( tag_gamelayer );
    if( gameWorldLayer != nullptr ) {
        // ...per poter invocare la funzione che carica il livello desiderato
        if( ((GameWorld*)gameWorldLayer)->loadLevel( level ) == false ) {
            log( "Errore! Livello non caricabile...uscita!" );
        }
    } else {
        log( "Errore! Game World Layer non trovato!" );
    }

}
