#ifndef PLAYER_H
#define PLAYER_H

#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QWidget>
#include "constants.h"

class Player : public QWidget
{
public:
    explicit Player(QWidget* parent = nullptr);
    void nextFrame();
    void paintEvent(QPaintEvent* );

private:
    int m_currentFrame;
    int m_frameWidth;
    int m_frameHeight;
    int m_totalesFrames;
    QPixmap m_spritesheet;
};

#endif // PLAYER_H
