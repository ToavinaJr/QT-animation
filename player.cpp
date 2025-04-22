#include "player.h"
#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <QRect>
#include <QDebug>
#include <QWidget> // Pour parentWidget()

Player::Player(QWidget* parent)
    : QWidget(parent),
    m_currentFrame(MARIO_STANDING_FRAME),
    m_frameWidth(MARIO_WIDTH),
    m_frameHeight(MARIO_HEIGHT),
    m_totalFrames(MARIO_FRAMES),
    m_standingFrame(MARIO_STANDING_FRAME),
    m_animationTimer(new QTimer(this)),
    m_currentDirection(Direction::None),
    m_facingDirection(Direction::Right), // Suppose que la frame 0 regarde à droite
    m_speed(MARIO_SPEED)
{
    if (!m_spritesheet.load(":/images/mario.png")) {
        qWarning() << "ERREUR: Impossible de charger le spritesheet ':/images/mario.png'. Vérifiez le fichier de ressources (qrc).";
    }

    setFixedSize(m_frameWidth, m_frameHeight);
    connect(m_animationTimer, &QTimer::timeout, this, &Player::updateState);
}

void Player::startMoving(Direction direction) {
    if (direction == Direction::None) return;

    m_currentDirection = direction;
    m_facingDirection = direction; // Met à jour la direction du regard

    if (!m_animationTimer->isActive()) {
        m_animationTimer->start(100);
    }
}

void Player::stopMoving() {
    m_currentDirection = Direction::None; // Arrête le mouvement physique
    m_animationTimer->stop();
    // m_facingDirection n'est PAS modifié ici
    setCurrentFrame(m_standingFrame);
}

Player::Direction Player::getCurrentDirection() const {
    return m_currentDirection;
}

void Player::updateState() {
    // 1. Mettre à jour la frame d'animation (seulement si en mouvement)
    int firstAnimationFrame = (m_standingFrame == 0) ? 1 : 0;
    int numAnimatedFrames = (m_standingFrame == 0) ? m_totalFrames - 1 : m_totalFrames;

    if (m_currentDirection != Direction::None && numAnimatedFrames > 0) {
        int currentAnimatedFrameIndex = (m_currentFrame - firstAnimationFrame + 1) % numAnimatedFrames;
        m_currentFrame = firstAnimationFrame + currentAnimatedFrameIndex;
    } else {
        m_currentFrame = m_standingFrame; // Assure la frame immobile si non en mouvement
    }

    // 2. Mettre à jour la position (si en mouvement)
    if (m_currentDirection != Direction::None && parentWidget()) {
        int newX = x();
        int parentWidth = parentWidget()->width();

        if (m_currentDirection == Direction::Left) {
            newX -= m_speed;
            if (newX < 0) newX = 0;
        } else if (m_currentDirection == Direction::Right) {
            newX += m_speed;
            if (newX + width() > parentWidth) newX = parentWidth - width();
        }
        move(newX, y());
    }

    // 3. Demander un redessin
    update();
}

void Player::setCurrentFrame(int frame) {
    // Vérifie si la frame change pour éviter des update() inutiles
    if (m_currentFrame != (frame % m_totalFrames)) {
        m_currentFrame = frame % m_totalFrames;
        update();
    } else if (m_currentFrame != m_standingFrame && m_currentDirection == Direction::None) {
        // Force la frame immobile si on s'arrête et qu'on n'y est pas déjà
        m_currentFrame = m_standingFrame;
        update();
    }
}

void Player::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);

    int frameX = m_currentFrame * m_frameWidth;
    QRect sourceRect(frameX, 0, m_frameWidth, m_frameHeight);
    QRect targetRect(0, 0, m_frameWidth, m_frameHeight);

    // Utilise m_facingDirection pour décider de l'inversion
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
