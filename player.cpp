#include "player.h"
#include <QPainter>
#include <QTimer>
#include <QRect>
#include <QDebug>

Player::Player(QWidget* parent)
    : QWidget(parent),
    m_currentFrame(0)
{

    if (!m_spritesheet.load(":/images/mario.png")) {
        qWarning("ERREUR: Impossible de charger le spritesheet ':/images/mario.png'");

    }

    m_frameWidth = MARIO_WIDTH;
    m_frameHeight = MARIO_HEIGHT;
    m_totalesFrames = 21;

    this->setFixedSize(m_frameWidth, m_frameHeight);

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Player::nextFrame);
    timer->start(100);
}

void Player::nextFrame() {
    m_currentFrame = (m_currentFrame + 1) % m_totalesFrames;
    update();
}

void Player::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    int frameX = m_currentFrame * m_frameWidth;

    QRect sourceRect(frameX, 0, m_frameWidth, m_frameHeight);
    QRect targetRect(0, 0, m_frameWidth, m_frameHeight);

    painter.drawPixmap(targetRect, m_spritesheet, sourceRect);
}
