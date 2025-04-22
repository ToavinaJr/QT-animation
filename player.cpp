#include "player.h"
#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <QRect>
#include <QDebug>
#include <QWidget>
#include <cmath> // Pour std::round, std::min, std::max

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
    m_speed(MARIO_SPEED),
    // Initialisation des membres de saut/gravité
    m_isJumpingOrFalling(false), // Commence au sol
    m_velocityY(0.0)
{
    if (!m_spritesheet.load(":/images/mario.png")) {
        qWarning() << "ERREUR: Impossible de charger le spritesheet ':/images/mario.png'. Vérifiez le fichier de ressources (qrc).";
    }

    setFixedSize(m_frameWidth, m_frameHeight);
    connect(m_animationTimer, &QTimer::timeout, this, &Player::updateState);

    // Démarre le timer immédiatement car il gère maintenant aussi la gravité
    m_animationTimer->start(16); // Environ 60 FPS (ajuster si besoin)
}

void Player::setObstacles(const QList<QWidget*>& obstacles) {
    m_obstacles = obstacles;
    // Vérifier si on commence en l'air (au cas où le spawn est au-dessus du sol/obstacles)
    if (!isOnGround()) {
        m_isJumpingOrFalling = true;
    }
}

void Player::startMoving(Direction direction) {
    if (direction == Direction::None) return;
    m_currentDirection = direction;
    m_facingDirection = direction;
    // Le timer tourne déjà pour la gravité
}

void Player::stopMoving() {
    m_currentDirection = Direction::None;
    // Ne pas arrêter le timer, la gravité doit continuer d'agir
    // setCurrentFrame(m_standingFrame); // Géré dans updateState si au sol
}

void Player::jump() {
    // Ne saute que si le joueur est sur le sol
    if (!m_isJumpingOrFalling && isOnGround()) {
        m_velocityY = JUMP_STRENGTH;
        m_isJumpingOrFalling = true;
        //qDebug() << "Jump! Initial velocity:" << m_velocityY;
    }
}

Player::Direction Player::getCurrentDirection() const {
    return m_currentDirection;
}

// --- Fonctions Helper ---

// Calcule le rectangle de collision pour une position donnée du *coin supérieur gauche* du widget
QRect Player::getCollisionRect(const QPoint& futureTopLeft) const {
    int collisionX = futureTopLeft.x() + COLLISION_MARGIN_LEFT;
    int collisionY = futureTopLeft.y() + COLLISION_MARGIN_TOP;
    int collisionWidth = m_frameWidth - COLLISION_MARGIN_LEFT - COLLISION_MARGIN_RIGHT;
    int collisionHeight = m_frameHeight - COLLISION_MARGIN_TOP - COLLISION_MARGIN_BOTTOM;
    if (collisionWidth < 1) collisionWidth = 1;
    if (collisionHeight < 1) collisionHeight = 1;
    return QRect(collisionX, collisionY, collisionWidth, collisionHeight);
}

// Vérifie la collision et retourne l'obstacle touché (ou nullptr)
bool Player::checkCollision(const QRect& futureCollisionRect, QWidget*& collidedObstacle) const {
    collidedObstacle = nullptr; // Initialise
    for (QWidget* obstacle : m_obstacles) {
        if (obstacle && obstacle->isVisible() && futureCollisionRect.intersects(obstacle->geometry())) {
            collidedObstacle = obstacle;
            return true; // Collision trouvée
        }
    }
    return false; // Pas de collision
}

// Vérifie si le joueur repose sur une surface solide
bool Player::isOnGround() const {
    if (!parentWidget()) return false; // Ne peut pas être au sol sans parent

    // Crée un petit rectangle juste SOUS le joueur pour tester le sol/plateforme
    int checkY = pos().y() + m_frameHeight; // 1 pixel sous les pieds
    QRect groundCheckRect = getCollisionRect(QPoint(pos().x(), checkY - COLLISION_MARGIN_BOTTOM));
    groundCheckRect.setHeight(COLLISION_MARGIN_BOTTOM + 1); // Le rect de collision plus 1 pixel en bas

    // Vérifie la collision avec les obstacles
    QWidget* dummy = nullptr;
    if (checkCollision(groundCheckRect, dummy)) {
        return true; // Collision avec un obstacle en dessous
    }

    // Vérifie si on est au "sol" de la fenêtre parente (si pas d'obstacles)
    if (pos().y() + m_frameHeight >= parentWidget()->height()) {
        return true;
    }

    return false;
}


// --- Logique Principale de Mise à Jour ---

void Player::applyGravityAndVerticalMovement() {
    if (m_isJumpingOrFalling) {
        // Appliquer la gravité
        m_velocityY += GRAVITY;
        // Limiter la vitesse de chute
        if (m_velocityY > MAX_FALL_SPEED) {
            m_velocityY = MAX_FALL_SPEED;
        }
    } else {
        // Si on n'est pas en train de sauter/tomber, vérifier si on devrait tomber (marcher hors d'une plateforme)
        if (!isOnGround()) {
            //qDebug() << "Walking off ledge";
            m_isJumpingOrFalling = true;
            // m_velocityY = 0; // Commence à tomber sans vitesse initiale (la gravité s'applique au prochain tick)
        } else {
            m_velocityY = 0; // Assure que la vitesse est nulle quand au sol
        }
    }
}

void Player::resolveVerticalCollision(QWidget* obstacle, int& nextY) {
    QRect obstacleRect = obstacle->geometry();
    int playerCollisionBottom = nextY + m_frameHeight - COLLISION_MARGIN_BOTTOM;
    int playerCollisionTop = nextY + COLLISION_MARGIN_TOP;

    if (m_velocityY > 0 && playerCollisionBottom > obstacleRect.top()) { // Chute sur l'obstacle
        //qDebug() << "Landing on obstacle at" << obstacleRect.top();
        nextY = obstacleRect.top() - m_frameHeight; // Place juste au-dessus
        m_velocityY = 0;
        m_isJumpingOrFalling = false;
    } else if (m_velocityY < 0 && playerCollisionTop < obstacleRect.bottom()) { // Cogner la tête en dessous
        //qDebug() << "Hitting head on obstacle at" << obstacleRect.bottom();
        nextY = obstacleRect.bottom(); // Place juste en dessous
        m_velocityY = 0; // Arrête la montée
        // m_isJumpingOrFalling reste true, car on va retomber
    }
}

void Player::resolveHorizontalCollision(QWidget* obstacle, int& nextX) {
    QRect obstacleRect = obstacle->geometry();
    int playerCollisionRight = nextX + m_frameWidth - COLLISION_MARGIN_RIGHT;
    int playerCollisionLeft = nextX + COLLISION_MARGIN_LEFT;

    if (m_currentDirection == Direction::Right && playerCollisionRight > obstacleRect.left()) {
        // Collision par la droite
        nextX = obstacleRect.left() - m_frameWidth + COLLISION_MARGIN_RIGHT; // Place juste à gauche
    } else if (m_currentDirection == Direction::Left && playerCollisionLeft < obstacleRect.right()) {
        // Collision par la gauche
        nextX = obstacleRect.right() - COLLISION_MARGIN_LEFT; // Place juste à droite
    }
    // Optionnel: arrêter l'animation
    // m_currentFrame = m_standingFrame;
}


void Player::updateState() {
    if (!parentWidget()) return; // Sécurité

    // 1. Appliquer la gravité et déterminer si on est en l'air
    applyGravityAndVerticalMovement();

    // 2. Calculer la position X suivante basée sur l'input
    int currentX = x();
    int nextX = currentX;
    int parentWidth = parentWidget()->width();
    if (m_currentDirection == Direction::Left) {
        nextX -= m_speed;
    } else if (m_currentDirection == Direction::Right) {
        nextX += m_speed;
    }
    // Collision avec les bords de la fenêtre (horizontal)
    if (nextX < 0) nextX = 0;
    if (nextX + m_frameWidth > parentWidth) nextX = parentWidth - m_frameWidth;


    // 3. Calculer la position Y suivante basée sur la vélocité verticale
    int currentY = y();
    // Utiliser std::round pour convertir la vélocité en déplacement entier
    int nextY = currentY + static_cast<int>(std::round(m_velocityY));
    int parentHeight = parentWidget()->height();
    // Collision avec le "sol" de la fenêtre parente
    if (nextY + m_frameHeight > parentHeight) {
        nextY = parentHeight - m_frameHeight;
        if (m_velocityY > 0) { // Atterrissage sur le sol de la fenêtre
            m_velocityY = 0;
            m_isJumpingOrFalling = false;
        }
    }
    // Empêcher de sortir par le haut (moins probable mais sécurité)
    if (nextY < 0) {
        nextY = 0;
        if(m_velocityY < 0) m_velocityY = 0; // Arrêter la montée si on touche le plafond
    }


    // 4. Vérification et résolution des collisions avec les obstacles
    QRect futureCollisionRect = getCollisionRect(QPoint(nextX, nextY));
    QWidget* collidedObstacle = nullptr;

    // Boucle pour résoudre les collisions potentiellement complexes (optionnel, simple check suffit pour commencer)
    // Pour l'instant, on vérifie une fois
    if (checkCollision(futureCollisionRect, collidedObstacle)) {
        // Collision détectée ! Déterminer si c'est principalement vertical ou horizontal
        // Approche simple : vérifier séparément les collisions H et V

        // a) Vérifier collision verticale seule
        QRect verticalCheckRect = getCollisionRect(QPoint(currentX, nextY)); // X actuel, Y futur
        QWidget* verticalObstacle = nullptr;
        if (checkCollision(verticalCheckRect, verticalObstacle) && verticalObstacle == collidedObstacle) {
            resolveVerticalCollision(verticalObstacle, nextY);
            // Recalculer le rect de collision après résolution verticale pour le check H
            futureCollisionRect = getCollisionRect(QPoint(nextX, nextY));
            collidedObstacle = nullptr; // Réinitialiser pour le check H
            checkCollision(futureCollisionRect, collidedObstacle); // Re-vérifier H avec le Y ajusté
        }

        // b) Vérifier collision horizontale (potentiellement avec le Y ajusté)
        if (collidedObstacle) { // S'il y a *encore* collision (ou c'était H initialement)
            QRect horizontalCheckRect = getCollisionRect(QPoint(nextX, currentY)); // X futur, Y actuel (ou ajusté si collision V a eu lieu)
            QWidget* horizontalObstacle = nullptr;
            // On re-vérifie avec le Y actuel/ajusté pour être sûr que c'est bien H
            if (checkCollision(horizontalCheckRect, horizontalObstacle) && horizontalObstacle == collidedObstacle) {
                resolveHorizontalCollision(horizontalObstacle, nextX);
            } else if (m_isJumpingOrFalling){
                // Cas spécial: atterrissage en diagonale sur un coin?
                // Si après résolution V, il n'y a plus collision H, on peut ignorer H.
                // Si on est en l'air et qu'on touche un mur, on veut quand même résoudre H.
                resolveHorizontalCollision(collidedObstacle, nextX);
            }
        }
    }


    // 5. Mettre à jour la frame d'animation
    if (m_isJumpingOrFalling) {
        // Pourrait avoir une frame de saut/chute spécifique ici
        // m_currentFrame = FRAME_JUMP;
        // Pour l'instant, on garde la dernière frame ou la frame debout
        if (m_currentDirection == Direction::None) {
            m_currentFrame = m_standingFrame;
        } // Sinon on garde l'animation de marche en l'air
    } else if (m_currentDirection != Direction::None) { // Au sol et en mouvement
        int firstAnimationFrame = (m_standingFrame == 0) ? 1 : 0;
        int numAnimatedFrames = (m_standingFrame == 0) ? m_totalFrames - 1 : m_totalFrames;
        if (numAnimatedFrames > 0) {
            int currentAnimatedFrameIndex = (m_currentFrame - firstAnimationFrame + 1) % numAnimatedFrames;
            m_currentFrame = firstAnimationFrame + currentAnimatedFrameIndex;
        }
    } else { // Au sol et immobile
        m_currentFrame = m_standingFrame;
    }

    // 6. Déplacer le widget à la position finale calculée
    if (x() != nextX || y() != nextY) {
        move(nextX, nextY);
    }

    // 7. Demander un redessin
    update();
}


// --- Reste du code (setCurrentFrame, paintEvent) ---

void Player::setCurrentFrame(int frame) {
    int targetFrame = frame % m_totalFrames;
    if (m_currentFrame != targetFrame) {
        m_currentFrame = targetFrame;
        update(); // Demande redessin seulement si la frame change
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

    /* // Décommenter pour débugger la boîte de collision
    painter.setPen(Qt::red);
    QRect debugCollisionRect = getCollisionRect(pos());
    // Convertir les coordonnées globales en locales pour le dessin DANS le widget
    debugCollisionRect.moveTo(COLLISION_MARGIN_LEFT, COLLISION_MARGIN_TOP);
    painter.drawRect(debugCollisionRect.adjusted(0,0,-1,-1));
    */
}
