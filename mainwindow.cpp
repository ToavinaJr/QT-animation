#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "player.h" // Implémentation de Player est nécessaire ici
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_player(nullptr)
{
    ui->setupUi(this);

    m_player = new Player(this); // 'this' est le parent

    // Positionnement initial du joueur
    int initialX = (width() - m_player->width()) / 2;
    int initialY = height() - m_player->height() - 20; // Un peu au-dessus du bas
    if (initialX < 0) initialX = 0; // Sécurité si fenêtre trop petite au départ
    if (initialY < 0) initialY = 0;
    m_player->move(initialX, initialY);

    // Important pour capturer les touches
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    setMinimumSize(400, 200); // Taille minimale fenêtre
    // Note: m_player n'est PAS setCentralWidget pour permettre move()
}

MainWindow::~MainWindow()
{
    // m_player est géré par Qt via la relation parent/enfant
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }

    switch (event->key()) {
    case Qt::Key_Left:
        m_player->startMoving(Player::Direction::Left);
        event->accept();
        break;
    case Qt::Key_Right:
        m_player->startMoving(Player::Direction::Right);
        event->accept();
        break;
    default:
        QMainWindow::keyPressEvent(event); // Passer aux autres gestionnaires
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }

    // Vérifie si la touche relâchée correspond à la direction ACTUELLE du mouvement
    Player::Direction currentMoveDirection = m_player->getCurrentDirection();

    switch (event->key()) {
    case Qt::Key_Left:
        if (currentMoveDirection == Player::Direction::Left) {
            m_player->stopMoving();
        }
        event->accept();
        break;
    case Qt::Key_Right:
        if (currentMoveDirection == Player::Direction::Right) {
            m_player->stopMoving();
        }
        event->accept();
        break;
    default:
        QMainWindow::keyReleaseEvent(event); // Passer aux autres gestionnaires
    }
}
