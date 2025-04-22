#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "player.h"
#include <QKeyEvent>
#include <QWidget> // Inclusion pour créer les obstacles
#include <QPalette> // Pour colorer les obstacles facilement

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_player(nullptr)
{
    ui->setupUi(this);

    setMinimumSize(600, 300); // Augmenter un peu la taille pour les obstacles

    // 1. Créer le joueur d'abord pour connaître sa taille/position initiale
    m_player = new Player(this);
    int playerInitialY = height() - m_player->height() - 20;
    int playerInitialX = 50; // Commencer à gauche
    m_player->move(playerInitialX, playerInitialY);

    // 2. Créer les obstacles
    setupObstacles();

    // 3. Informer le joueur des obstacles existants
    m_player->setObstacles(m_obstaclesList);

    // Rendre le joueur visible (les obstacles sont rendus visibles dans setupObstacles)
    m_player->show(); // Important si non défini comme centralWidget

    // Focus pour les touches
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

MainWindow::~MainWindow()
{
    // Les obstacles et le joueur sont enfants de MainWindow,
    // Qt gère leur suppression.
    delete ui;
}

void MainWindow::setupObstacles()
{
    // Assurez-vous que la liste est vide si cette méthode est appelée plusieurs fois
    // qDeleteAll(m_obstaclesList); // Supprime les anciens widgets si nécessaire
    // m_obstaclesList.clear();

    // Obstacle 1 : Un mur simple
    QWidget* wall = new QWidget(this);
    wall->setFixedSize(30, 150);
    // Positionner par rapport au bas de la fenêtre et à la position Y du joueur
    wall->move(300, height() - wall->height() - 20);
    // wall->setStyleSheet("background-color: brown; border: 1px solid black;"); // Style simple
    QPalette pal = wall->palette();
    pal.setColor(QPalette::Window, Qt::darkGray); // Couleur de fond
    wall->setAutoFillBackground(true);
    wall->setPalette(pal);
    wall->show(); // Rendre l'obstacle visible
    m_obstaclesList.append(wall); // Ajouter à la liste

    // Obstacle 2 : Une plateforme plus basse
    QWidget* platform = new QWidget(this);
    platform->setFixedSize(100, 20);
    platform->move(450, height() - platform->height() - 60); // Un peu plus haut
    // platform->setStyleSheet("background-color: green;");
    pal.setColor(QPalette::Window, Qt::darkGreen);
    platform->setAutoFillBackground(true);
    platform->setPalette(pal);
    platform->show();
    m_obstaclesList.append(platform);
}


// keyPressEvent et keyReleaseEvent restent identiques
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }
    // S'assurer que m_player existe avant de l'utiliser
    if (!m_player) return;

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
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }
    // S'assurer que m_player existe avant de l'utiliser
    if (!m_player) return;

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
        QMainWindow::keyReleaseEvent(event);
    }
}
