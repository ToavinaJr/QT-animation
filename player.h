#ifndef PLAYER_H
#define PLAYER_H

#include <QPixmap>
#include <QWidget>
#include <QList> // Ajout pour QList
#include "constants.h"

class QPaintEvent;
class QTimer;
// class QWidget; // Déjà inclus via <QWidget>

class Player : public QWidget
{
    Q_OBJECT

public:
    enum class Direction { None, Left, Right };

    explicit Player(QWidget* parent = nullptr);

    void startMoving(Direction direction);
    void stopMoving();
    Direction getCurrentDirection() const;

    // Méthode pour recevoir la liste des obstacles
    void setObstacles(const QList<QWidget*>& obstacles);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void updateState();

private:
    void setCurrentFrame(int frame);
    bool checkCollision(const QRect& futureRect) const; // Helper pour la collision

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

    QList<QWidget*> m_obstacles; // Liste des obstacles connus par le joueur
};

#endif // PLAYER_H
