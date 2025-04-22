#ifndef PLAYER_H
#define PLAYER_H

#include <QPixmap>
#include <QWidget>
#include <QList>
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
    void jump(); // Nouvelle méthode pour initier le saut
    Direction getCurrentDirection() const;
    void setObstacles(const QList<QWidget*>& obstacles);

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void updateState();

private:
    // Helpers
    void setCurrentFrame(int frame);
    QRect getCollisionRect(const QPoint& futurePos) const; // Calcule le rect de collision à une position donnée
    bool checkCollision(const QRect& futureCollisionRect, QWidget*& collidedObstacle) const; // Modifiée pour retourner l'obstacle
    void applyGravityAndVerticalMovement(); // Nouvelle méthode pour gérer la physique verticale
    void resolveVerticalCollision(QWidget* obstacle, int& nextY); // Gère l'atterrissage/cogner
    void resolveHorizontalCollision(QWidget* obstacle, int& nextX); // Gère collision mur
    bool isOnGround() const; // Vérifie si le joueur est sur une surface solide

    // Membres existants
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

    // --- AJOUT : Membres pour le saut/gravité ---
    bool m_isJumpingOrFalling; // True si en l'air
    double m_velocityY;        // Vitesse verticale actuelle
};

#endif // PLAYER_H
