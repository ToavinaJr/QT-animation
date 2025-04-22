#ifndef PLAYER_H
#define PLAYER_H

#include <QPixmap>
#include <QWidget>
#include <QList> // Ajout pour QList
#include "constants.h"

class QPaintEvent;
class QTimer;
// QPoint n'a pas besoin d'être forward-déclaré si on inclut <QPoint> dans le .cpp,
// mais on peut le faire pour être propre. class QPoint;

class Player : public QWidget
{
    Q_OBJECT

public:
    enum class Direction { None, Left, Right };

    explicit Player(QWidget* parent = nullptr);

    // --- Interface Publique ---
    void startMoving(Direction direction);
    void stopMoving();
    void jump(); // Ajouté
    Direction getCurrentDirection() const;
    void setObstacles(const QList<QWidget*>& obstacles);

protected:
    // --- Événements Surchargés ---
    void paintEvent(QPaintEvent* event) override;

private slots:
    // --- Slots Privés ---
    void updateState(); // Gère toute la logique de mise à jour

private:
    // --- Méthodes Helper Privées ---
    void setCurrentFrame(int frame);
    // Signature corrigée/ajoutée
    QRect getCollisionRect(const QPoint& futureTopLeft) const;
    // Signature corrigée pour retourner l'obstacle
    bool checkCollision(const QRect& futureCollisionRect, QWidget*& collidedObstacle) const;
    // Ajouté
    void applyGravityAndVerticalMovement();
    // Ajouté
    void resolveVerticalCollision(QWidget* obstacle, int& nextY);
    // Ajouté
    void resolveHorizontalCollision(QWidget* obstacle, int& nextX);
    // Ajouté
    bool isOnGround() const;

    // --- Variables Membres ---
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
    QList<QWidget*> m_obstacles;

    // --- Membres Ajoutés pour Saut/Gravité ---
    bool m_isJumpingOrFalling;
    double m_velocityY;
};

#endif // PLAYER_H
