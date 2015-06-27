#include <QMenu>
#include <QMenuBar>
#include "WindowMenu.hpp"

void WindowMenu::Init( QWidget* mainWindow )
{
    connect(mainWindow, SIGNAL(GameObjectSelected(std::list< ae3d::GameObject* >)),
            this, SLOT(GameObjectSelected(std::list< ae3d::GameObject* >)));
    menuBar = new QMenuBar( mainWindow );

    fileMenu = menuBar->addMenu( "&File" );
    fileMenu->addAction( "Save Scene", mainWindow, SLOT(SaveScene()), QKeySequence("Ctrl+S"));
    fileMenu->addAction( "Open Scene", mainWindow, SLOT(LoadScene()), QKeySequence("Ctrl+O"));
    //fileMenu->addSeparator();

    editMenu = menuBar->addMenu( "&Edit" );
    editMenu->addAction( "Undo", mainWindow, SLOT(Undo()), QKeySequence("Ctrl+Z"));

    sceneMenu = menuBar->addMenu( "&Scene" );
    sceneMenu->addAction( "Create Game Object", mainWindow, SLOT(CommandCreateGameObject()), QKeySequence("Ctrl+N"));

    componentMenu = menuBar->addMenu( "&Component" );
    componentMenu->addAction( "Add Camera", mainWindow, SLOT(CommandCreateCameraComponent()));
}

void WindowMenu::GameObjectSelected( std::list< ae3d::GameObject* > gameObjects )
{
    componentMenu->setEnabled( !gameObjects.empty() );
}
