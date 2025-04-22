#ifndef PLAYER_H
#define PLAYER_H

#include <QPixmap>
#include <QWidget>
#include "constants.h"

class QPaintEvent;
class QTimer;

class Player : public QWidget
{
    Q_OBJECT

public:
    enum class Direction { None, Left, Right };

    explicit Player(QWidget* parent = nullptr);

    void startMoving(Direction direction);
    void stopMoving();
    Direction getCurrentDirection() const; // Getter ajouté

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void updateState();

private:
    void setCurrentFrame(int frame);

    int m_currentFrame;
    int m_frameWidth;
    int m_frameHeight;
    int m_totalFrames;
    int m_standingFrame;
    QPixmap m_spritesheet;
    QTimer* m_animationTimer;
    Direction m_currentDirection;
    Direction m_facingDirection;
    int m_speed;
};

#endif // PLAYER_H
