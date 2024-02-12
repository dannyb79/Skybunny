/*
    the Game World
*/

#include "cocos2d.h"
#include "LoadingScene.h"
#include "ui/UILoadingBar.h"
#include "GameWorldScene.h"
#include "GameMenu.h"
#include "AnimationManager.h"

USING_NS_CC;
using namespace ui;


Scene* LoadingScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    // 'layer' is an autorelease object
    auto layer = LoadingScene::create();
    // add layer as a child to scene
    scene->addChild( layer );
    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool LoadingScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

    Size visibleSize    = Director::getInstance()->getVisibleSize();
    Vec2 origin         = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // create background image (640x480)
    auto background = Sprite::create( "images/backgrounds/loading.png" );
	background->setAnchorPoint( Vec2( 0, 0 ) );
	background->setPosition( Vec2( 0, 0 ) );
	this->addChild( background, -1000 );


    // create label for debug
/*
    auto label = Label::createWithTTF("Loading...", "fonts/Marker Felt.ttf", 24);
    label->setPosition(Vec2(origin.x + visibleSize.width/2, origin.y + visibleSize.height - label->getContentSize().height));
    this->addChild( label, 1 );
*/

    // Create the loading bar
    LoadingBar* loadingBar = LoadingBar::create("images/progress_slider.png");
    loadingBar->setTag( 0 );
    loadingBar->setPosition( Vec2( 320, 54 ) );
    loadingBar->setDirection( LoadingBar::Direction::LEFT );
    this->addChild( loadingBar, 1 );

    // nella prima fase vengono caricate le animazioni
    loading_state   = loadingstate_animations;
    // azzera l'indice dell'animazione corrente da caricare
    animation_id    = 0;
    // azzera il valore di avanzamento della progress bar
    progress        = 0;

    scheduleUpdate();

    return true;
}

/*
    aggiorna la progress bar
*/
void LoadingScene::updateProgressBar( int progress )
{
    LoadingBar* loadingBar = static_cast<LoadingBar*>( this->getChildByTag( 0 ) );
    loadingBar->setPercent( progress );
}

/*
    aggiorna lo stato del caricamento
*/
void LoadingScene::update( float delta )
{
    switch( loading_state ) {
        // caricamento delle animazioni
        case loadingstate_animations:

            // carica l'animazione secondo l'indice contenuton nella variabile animation_id
            AnimationManager::getInstance()->loadAnimation( animation_id );
            // indice della prossima animazione
            animation_id += 1;
            // una volta caricate tutte le animazioni...
            if( animation_id >= animation_max ) {
                // il caricamento e' completato al 100%
                progress = 100;
                // passa al caricamento del menu
                loading_state = loadingstate_complete;
            } else {
                // per il momento il valore e' calcolato sulle animazioni caricate
                // aggiorna il valore della progress bar
                progress = animation_id * 100 / animation_max;
            }
            // aggiorna la progress bar
            updateProgressBar( progress );

        break;
        // ( loadingstate_complete ) caricamento terminato
        default:

            // caricamento menu di gioco
            auto scene = GameMenu::createScene();
            Director::getInstance()->replaceScene( scene );

        break;
    }
}

