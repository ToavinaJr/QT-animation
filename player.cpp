#include "player.h"
#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <QRect> // Inclusion pour QRect
#include <QDebug>
#include <QWidget>

Player::Player(QWidget* parent)
    : QWidget(parent),
    m_currentFrame(MARIO_STANDING_FRAME),
    m_frameWidth(MARIO_WIDTH),
    m_frameHeight(MARIO_HEIGHT),
    m_totalFrames(MARIO_FRAMES),
    m_standingFrame(MARIO_STANDING_FRAME),
    m_animationTimer(new QTimer(this)),
    m_currentDirection(Direction::None),
    m_facingDirection(Direction::Right),
    m_speed(MARIO_SPEED)
// m_obstacles est initialisé par défaut (liste vide)
{
    if (!m_spritesheet.load(":/images/mario.png")) {
        qWarning() << "ERREUR: Impossible de charger le spritesheet ':/images/mario.png'. Vérifiez le fichier de ressources (qrc).";
    }

    setFixedSize(m_frameWidth, m_frameHeight);
    connect(m_animationTimer, &QTimer::timeout, this, &Player::updateState);
}

void Player::setObstacles(const QList<QWidget*>& obstacles) {
    m_obstacles = obstacles;
}

void Player::startMoving(Direction direction) {
    if (direction == Direction::None) return;
    m_currentDirection = direction;
    m_facingDirection = direction;
    if (!m_animationTimer->isActive()) {
        m_animationTimer->start(100);
    }
}

void Player::stopMoving() {
    m_currentDirection = Direction::None;
    m_animationTimer->stop();
    setCurrentFrame(m_standingFrame);
}

Player::Direction Player::getCurrentDirection() const {
    return m_currentDirection;
}

// Fonction helper pour vérifier la collision avec les obstacles
bool Player::checkCollision(const QRect& futureRect) const
{
    for (const QWidget* obstacle : m_obstacles) {
        if (obstacle && obstacle->isVisible() && futureRect.intersects(obstacle->geometry())) {
            // qDebug() << "Collision détectée avec un obstacle !";
            return true; // Collision trouvée
        }
    }
    return false; // Pas de collision
}


void Player::updateState() {
    // 1. Mettre à jour la frame d'animation
    int firstAnimationFrame = (m_standingFrame == 0) ? 1 : 0;
    int numAnimatedFrames = (m_standingFrame == 0) ? m_totalFrames - 1 : m_totalFrames;

    if (m_currentDirection != Direction::None && numAnimatedFrames > 0) {
        int currentAnimatedFrameIndex = (m_currentFrame - firstAnimationFrame + 1) % numAnimatedFrames;
        m_currentFrame = firstAnimationFrame + currentAnimatedFrameIndex;
    } else {
        m_currentFrame = m_standingFrame;
    }

    // 2. Calculer la prochaine position et vérifier les collisions AVANT de bouger
    if (m_currentDirection != Direction::None && parentWidget()) {
        int currentX = x();
        int currentY = y();
        int nextX = currentX;
        int parentWidth = parentWidget()->width();

        // Calcul de la position X future brute
        if (m_currentDirection == Direction::Left) {
            nextX -= m_speed;
            if (nextX < 0) nextX = 0;
        } else if (m_currentDirection == Direction::Right) {
            nextX += m_speed;
            if (nextX + m_frameWidth > parentWidth) {
                nextX = parentWidth - m_frameWidth;
            }
        }

        // --- MODIFICATION : Calcul du rectangle de COLLISION ---
        // Ce rectangle est plus petit que le widget, basé sur les marges
        int collisionX = nextX + COLLISION_MARGIN_LEFT;
        int collisionY = currentY + COLLISION_MARGIN_TOP; // Y ne change pas, mais on applique la marge
        int collisionWidth = m_frameWidth - COLLISION_MARGIN_LEFT - COLLISION_MARGIN_RIGHT;
        int collisionHeight = m_frameHeight - COLLISION_MARGIN_TOP - COLLISION_MARGIN_BOTTOM;

        // S'assurer que la largeur/hauteur ne sont pas négatives si les marges sont trop grandes
        if (collisionWidth < 1) collisionWidth = 1;
        if (collisionHeight < 1) collisionHeight = 1;

        QRect futureCollisionRect(collisionX, collisionY, collisionWidth, collisionHeight);
        // --- FIN MODIFICATION ---

        // Vérifier la collision en utilisant le rectangle de collision réduit
        if (!checkCollision(futureCollisionRect)) { // Utilise futureCollisionRect !
            // Pas de collision détectée, on peut bouger
            if (x() != nextX) {
                move(nextX, currentY); // On déplace toujours le WIDGET complet
            }
        } else {
            // Collision détectée avec un obstacle, ne pas bouger horizontalement
            if (m_currentDirection != Direction::None) {
                // Optionnel : Geler l'animation sur la frame immobile
                m_currentFrame = m_standingFrame;
            }
        }
    }

    // 3. Demander un redessin
    update();
}


void Player::setCurrentFrame(int frame) {
    int targetFrame = frame % m_totalFrames;
    if (m_currentFrame != targetFrame) {
        m_currentFrame = targetFrame;
        update();
    }
}

void Player::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);

    int frameX = m_currentFrame * m_frameWidth;
    QRect sourceRect(frameX, 0, m_frameWidth, m_frameHeight);
    QRect targetRect(0, 0, m_frameWidth, m_frameHeight);

    bool flipHorizontally = (m_facingDirection == Direction::Left);

    if (flipHorizontally) {
        painter.save();
        painter.translate(m_frameWidth / 2.0, 0);
        painter.scale(-1, 1);
        painter.translate(-m_frameWidth / 2.0, 0);
    }

    painter.drawPixmap(targetRect, m_spritesheet, sourceRect);

    if (flipHorizontally) {
        painter.restore();
    }
}
