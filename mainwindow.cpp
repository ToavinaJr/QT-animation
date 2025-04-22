#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "player.h"
#include <QKeyEvent>
#include <QWidget>
#include <QPalette>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_player(nullptr)
{
    ui->setupUi(this);
    setMinimumSize(600, 300);

    m_player = new Player(this);
    // Position initiale un peu plus haute pour tester la chute initiale
    int playerInitialY = height() - m_player->height() - 150;
    int playerInitialX = 50;
    m_player->move(playerInitialX, playerInitialY);

    setupObstacles();
    m_player->setObstacles(m_obstaclesList); // Doit être appelé APRES la création des obstacles
    m_player->show();

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupObstacles() {
    // Obstacle 1: Mur
    QWidget* wall = new QWidget(this);
    wall->setFixedSize(30, 100);
    wall->move(300, height() - wall->height() - 20); // Au sol
    QPalette palW = wall->palette();
    palW.setColor(QPalette::Window, Qt::darkGray);
    wall->setAutoFillBackground(true); wall->setPalette(palW); wall->show();
    m_obstaclesList.append(wall);

    // Obstacle 2: Plateforme
    QWidget* platform = new QWidget(this);
    platform->setFixedSize(100, 20);
    platform->move(450, height() - platform->height() - 80); // En hauteur
    QPalette palP = platform->palette();
    palP.setColor(QPalette::Window, Qt::darkGreen);
    platform->setAutoFillBackground(true); platform->setPalette(palP); platform->show();
    m_obstaclesList.append(platform);

    // Obstacle 3: Sol (facultatif, le bas de la fenêtre sert de sol)
    // QWidget* ground = new QWidget(this);
    // ground->setFixedSize(width(), 20);
    // ground->move(0, height() - ground->height());
    // QPalette palG = ground->palette();
    // palG.setColor(QPalette::Window, Qt::yellow); // Couleur différente pour le sol
    // ground->setAutoFillBackground(true); ground->setPalette(palG); ground->show();
    // m_obstaclesList.append(ground); // Ajouter si on veut un sol physique
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat() || !m_player) {
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
    // --- AJOUT : Touche pour sauter ---
    case Qt::Key_Space: // Ou Qt::Key_Up si vous préférez
        m_player->jump();
        event->accept();
        break;
    // --- FIN AJOUT ---
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat() || !m_player) {
        event->ignore();
        return;
    }

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
    // Le relâchement de la touche de saut n'a généralement pas d'effet ici
    case Qt::Key_Space: // Ou Qt::Key_Up
        event->accept(); // Accepter l'événement même si on ne fait rien
        break;
    default:
        QMainWindow::keyReleaseEvent(event);
    }
}
