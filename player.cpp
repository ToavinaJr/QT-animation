#include "player.h"
#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <QRect>
#include <QDebug>
#include <QWidget>
#include <cmath>
#include <QPointF>

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
    // setCurrentFrame(m_standingFrame); // Géré dans  si au sol
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
// bool Player::isOnGround() const {
//     if (!parentWidget()) return false; // Ne peut pas être au sol sans parent

//     // Crée un petit rectangle juste SOUS le joueur pour tester le sol/plateforme
//     int checkY = pos().y() + m_frameHeight; // 1 pixel sous les pieds
//     QRect groundCheckRect = getCollisionRect(QPoint(pos().x(), checkY - COLLISION_MARGIN_BOTTOM));
//     groundCheckRect.setHeight(COLLISION_MARGIN_BOTTOM + 1); // Le rect de collision plus 1 pixel en bas

//     // Vérifie la collision avec les obstacles
//     QWidget* dummy = nullptr;
//     if (checkCollision(groundCheckRect, dummy)) {
//         return true; // Collision avec un obstacle en dessous
//     }

//     // Vérifie si on est au "sol" de la fenêtre parente (si pas d'obstacles)
//     if (pos().y() + m_frameHeight >= parentWidget()->height()) {
//         return true;
//     }

//     return false;
// }

// ... (includes et début de classe) ...

// Vérifie si le joueur repose sur une surface solide (version plus stricte)
bool Player::isOnGround() const {
    if (!parentWidget()) return false;

    // --- VERSION CORRIGÉE (Points sous la boîte de collision) ---
    int currentX = pos().x();
    int currentY = pos().y();

    // Coordonnées X des bords de la boîte de collision
    int collisionX_Left = currentX + COLLISION_MARGIN_LEFT;
    int collisionX_Right = currentX + m_frameWidth - COLLISION_MARGIN_RIGHT -1; // -1 pour être dans le pixel

    // Coordonnée Y juste sous le bas de la boîte de collision
    int checkY = currentY + m_frameHeight - COLLISION_MARGIN_BOTTOM + 1; // +1 pixel en dessous

    // Points à vérifier
    QPoint checkPointLeft(collisionX_Left, checkY);
    QPoint checkPointRight(collisionX_Right, checkY);

    // Fonction lambda pour vérifier un point contre les obstacles et le sol de la fenêtre
    auto isPointSolid = [&](const QPoint& point) {
        // Vérifie le sol de la fenêtre
        if (point.y() >= parentWidget()->height()) {
            return true;
        }
        // Vérifie les obstacles
        for (const QWidget* obstacle : m_obstacles) {
            if (obstacle && obstacle->isVisible() && obstacle->geometry().contains(point)) {
                return true;
            }
        }
        return false;
    };

    // Le joueur est au sol si l'un OU l'autre des points est sur une surface solide
    if (isPointSolid(checkPointLeft) || isPointSolid(checkPointRight)) {
        return true;
    }
    // --- FIN VERSION CORRIGÉE ---

    return false;
}


// --- REFACTORISATION de updateState pour séparer X et Y ---
void Player::updateState() {
    if (!parentWidget()) return;

    // --- Variables initiales ---
    int currentX = x();
    int currentY = y();
    int finalX = currentX; // Position X finale après résolution H
    int finalY = currentY; // Position Y finale après résolution V
    int parentWidth = parentWidget()->width();
    int parentHeight = parentWidget()->height();

    // --- Phase 1: Mouvement Horizontal et Collision ---
    int deltaX = 0;
    if (m_currentDirection == Direction::Left) {
        deltaX = -m_speed;
    } else if (m_currentDirection == Direction::Right) {
        deltaX = m_speed;
    }

    if (deltaX != 0) {
        int tryX = currentX + deltaX;

        // Vérification des bords de la fenêtre (horizontal)
        if (tryX < 0) tryX = 0;
        if (tryX + m_frameWidth > parentWidth) tryX = parentWidth - m_frameWidth;

        // Vérifier collision horizontale avec les obstacles
        QRect horizontalCheckRect = getCollisionRect(QPoint(tryX, currentY)); // Utilise Y actuel
        QWidget* hObstacle = nullptr;
        if (checkCollision(horizontalCheckRect, hObstacle)) {
            resolveHorizontalCollision(hObstacle, tryX); // Ajuste tryX si collision
        }
        finalX = tryX; // X final est le résultat après collision H
    }

    // --- Phase 2: Gravité et Mouvement Vertical ---
    // Appliquer la gravité seulement si en l'air ou si on vient de quitter le sol
    if (!m_isJumpingOrFalling && !isOnGround()) {
        m_isJumpingOrFalling = true; // Commencer à tomber si on quitte une plateforme
        // m_velocityY = 0; // Chute commence sans vitesse verticale initiale
    }

    if (m_isJumpingOrFalling) {
        m_velocityY += GRAVITY;
        if (m_velocityY > MAX_FALL_SPEED) m_velocityY = MAX_FALL_SPEED;
    }
    // Note : Ne pas mettre m_velocityY = 0 ici si isOnGround, car on peut être
    // sur le sol mais vouloir initier un saut au même tick. C'est géré dans
    // resolveVerticalCollision lors de l'atterrissage.


    // --- Phase 3: Collision Verticale ---
    int deltaY = static_cast<int>(std::round(m_velocityY));

    if (deltaY != 0 || m_isJumpingOrFalling) // Vérifier même si deltaY est 0 si on est censé tomber
    {
        int tryY = currentY + deltaY;

        // Vérification des bords de la fenêtre (vertical)
        // Sol
        if (tryY + m_frameHeight - COLLISION_MARGIN_BOTTOM >= parentHeight) {
            tryY = parentHeight - (m_frameHeight - COLLISION_MARGIN_BOTTOM);
            if (m_velocityY >= 0) { // Atterrissage sur le sol fenêtre
                m_velocityY = 0;
                m_isJumpingOrFalling = false;
                //qDebug() << "Landed on window floor";
            }
        }
        // Plafond
        if (tryY + COLLISION_MARGIN_TOP < 0) {
            tryY = 0 - COLLISION_MARGIN_TOP;
            if(m_velocityY < 0) { // Cogner plafond fenêtre
                m_velocityY = 0;
            }
        }

        // Vérifier collision verticale avec les obstacles (en utilisant X final)
        QRect verticalCheckRect = getCollisionRect(QPoint(finalX, tryY));
        QWidget* vObstacle = nullptr;
        if (checkCollision(verticalCheckRect, vObstacle)) {
            // La fonction resolve met à jour m_velocityY et m_isJumpingOrFalling si nécessaire
            resolveVerticalCollision(vObstacle, tryY);
        }
        finalY = tryY; // Y final est le résultat après collision V

        // Double-check: si après résolution V, on est toujours en l'air mais isOnGround() dit vrai, forcer état sol
        // Utile si la résolution nous place exactement sur le sol dans le même tick.
        if (!m_isJumpingOrFalling && m_velocityY == 0 && !isOnGround()) {
            // Cela ne devrait pas arriver souvent avec la nouvelle logique isOnGround/resolveV,
            // mais sécurité si on glisse très légèrement d'une plateforme
            m_isJumpingOrFalling = true;
            //qDebug() << "Forcing fall state after V resolve check";
        } else if (m_isJumpingOrFalling && m_velocityY == 0 && isOnGround()){
            // Si on a atterri (vitesse nulle) et qu'on est bien au sol -> confirmer état sol
            m_isJumpingOrFalling = false;
            //qDebug() << "Confirmed ground state after V resolve check";
        }


    } else {
        // Si deltaY est 0 et pas en train de tomber, Y ne change pas
        finalY = currentY;
        // Assurer que la vitesse est nulle si on est confirmé au sol
        if (isOnGround()) {
            m_velocityY = 0;
            m_isJumpingOrFalling = false;
        }
    }


    // --- Phase 4: Mettre à jour l'animation ---
    if (m_isJumpingOrFalling) {
        // Mettre une frame de saut/chute si disponible
        // m_currentFrame = FRAME_JUMP; // Remplacer par l'index de la frame de saut
        // Sinon, on peut garder la frame debout ou de marche si on bouge H
        if (m_currentDirection == Direction::None) {
            m_currentFrame = m_standingFrame; // Frame debout si immobile en l'air
        } else {
            // Garder l'animation de marche si on bouge horizontalement en l'air
            int firstAnimationFrame = (m_standingFrame == 0) ? 1 : 0;
            int numAnimatedFrames = (m_standingFrame == 0) ? m_totalFrames - 1 : m_totalFrames;
            if (numAnimatedFrames > 0) {
                int currentAnimatedFrameIndex = (m_currentFrame - firstAnimationFrame + 1) % numAnimatedFrames;
                m_currentFrame = firstAnimationFrame + currentAnimatedFrameIndex;
            }
        }
    } else if (m_currentDirection != Direction::None) { // Au sol et en mouvement H
        int firstAnimationFrame = (m_standingFrame == 0) ? 1 : 0;
        int numAnimatedFrames = (m_standingFrame == 0) ? m_totalFrames - 1 : m_totalFrames;
        if (numAnimatedFrames > 0) {
            int currentAnimatedFrameIndex = (m_currentFrame - firstAnimationFrame + 1) % numAnimatedFrames;
            m_currentFrame = firstAnimationFrame + currentAnimatedFrameIndex;
        }
    } else { // Au sol et immobile
        m_currentFrame = m_standingFrame;
    }


    // --- Phase 5: Appliquer le déplacement ---
    if (x() != finalX || y() != finalY) {
        move(finalX, finalY);
    }

    // --- Phase 6: Redessiner ---
    update(); // Toujours utile pour l'animation
}

// Assurez-vous que resolveVerticalCollision met bien à jour m_isJumpingOrFalling = false lors de l'atterrissage
void Player::resolveVerticalCollision(QWidget* obstacle, int& nextY) {
    QRect obstacleRect = obstacle->geometry();
    int playerCollisionBottom = nextY + m_frameHeight - COLLISION_MARGIN_BOTTOM;
    int playerCollisionTop = nextY + COLLISION_MARGIN_TOP;

    // --- CORRECTION ICI ---
    // int currentY = m_posY; // Incorrect
    int currentY = y();      // Correct: utilise la fonction y() du QWidget
    // --- FIN CORRECTION ---

    if (m_velocityY >= 0 && playerCollisionBottom >= obstacleRect.top() && currentY + m_frameHeight - COLLISION_MARGIN_BOTTOM <= obstacleRect.top() +1) {
        // ^^^ Ajout condition : Vérifie si le bas du joueur était AU-DESSUS de l'obstacle au tick précédent (ou presque)
        //     pour confirmer que c'est bien un atterrissage par le dessus et non un contact latéral en tombant.

        nextY = obstacleRect.top() - (m_frameHeight - COLLISION_MARGIN_BOTTOM);
        // -- Confirmation de l'atterrissage --
        m_velocityY = 0;
        m_isJumpingOrFalling = false; // Important : Mettre à false ici !
        //qDebug() << "Landed on obstacle. New Y:" << nextY;

    } else if (m_velocityY < 0 && playerCollisionTop <= obstacleRect.bottom() && currentY + COLLISION_MARGIN_TOP >= obstacleRect.bottom() -1) {
        // ^^^ Ajout condition : Vérifie si le haut du joueur était EN DESSOUS de l'obstacle au tick précédent.

        nextY = obstacleRect.bottom() - COLLISION_MARGIN_TOP;
        //qDebug() << "Hitting head on obstacle. New Y:" << nextY;
        m_velocityY = 0; // Arrête la montée
        // m_isJumpingOrFalling reste true (on va retomber)
    }
}


// resolveHorizontalCollision reste identique
void Player::resolveHorizontalCollision(QWidget* obstacle, int& nextX) {
    QRect obstacleRect = obstacle->geometry();
    int playerCollisionRight = nextX + m_frameWidth - COLLISION_MARGIN_RIGHT;
    int playerCollisionLeft = nextX + COLLISION_MARGIN_LEFT;

    if (m_currentDirection == Direction::Right && playerCollisionRight >= obstacleRect.left()) {
        nextX = obstacleRect.left() - (m_frameWidth - COLLISION_MARGIN_RIGHT);
    } else if (m_currentDirection == Direction::Left && playerCollisionLeft <= obstacleRect.right()) {
        nextX = obstacleRect.right() - COLLISION_MARGIN_LEFT;
    }
}

// jump(), startMoving(), stopMoving(), getCurrentDirection(), setObstacles(),
// getCollisionRect(), checkCollision(), setCurrentFrame(), paintEvent()
// restent comme précédemment.

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

// void Player::resolveVerticalCollision(QWidget* obstacle, int& nextY) {
//     QRect obstacleRect = obstacle->geometry();
//     int playerCollisionBottom = nextY + m_frameHeight - COLLISION_MARGIN_BOTTOM;
//     int playerCollisionTop = nextY + COLLISION_MARGIN_TOP;

//     if (m_velocityY > 0 && playerCollisionBottom > obstacleRect.top()) { // Chute sur l'obstacle
//         //qDebug() << "Landing on obstacle at" << obstacleRect.top();
//         nextY = obstacleRect.top() - m_frameHeight; // Place juste au-dessus
//         m_velocityY = 0;
//         m_isJumpingOrFalling = false;
//     } else if (m_velocityY < 0 && playerCollisionTop < obstacleRect.bottom()) { // Cogner la tête en dessous
//         //qDebug() << "Hitting head on obstacle at" << obstacleRect.bottom();
//         nextY = obstacleRect.bottom(); // Place juste en dessous
//         m_velocityY = 0; // Arrête la montée
//         // m_isJumpingOrFalling reste true, car on va retomber
//     }
// }

// void Player::resolveHorizontalCollision(QWidget* obstacle, int& nextX) {
//     QRect obstacleRect = obstacle->geometry();
//     int playerCollisionRight = nextX + m_frameWidth - COLLISION_MARGIN_RIGHT;
//     int playerCollisionLeft = nextX + COLLISION_MARGIN_LEFT;

//     if (m_currentDirection == Direction::Right && playerCollisionRight > obstacleRect.left()) {
//         // Collision par la droite
//         nextX = obstacleRect.left() - m_frameWidth + COLLISION_MARGIN_RIGHT; // Place juste à gauche
//     } else if (m_currentDirection == Direction::Left && playerCollisionLeft < obstacleRect.right()) {
//         // Collision par la gauche
//         nextX = obstacleRect.right() - COLLISION_MARGIN_LEFT; // Place juste à droite
//     }
//     // Optionnel: arrêter l'animation
//     // m_currentFrame = m_standingFrame;
// }




// void Player::updateState() {
//     if (!parentWidget()) return; // Sécurité

//     // 1. Appliquer la gravité et déterminer si on est en l'air
//     applyGravityAndVerticalMovement();

//     // 2. Calculer la position X suivante basée sur l'input
//     int currentX = x();
//     int nextX = currentX;
//     int parentWidth = parentWidget()->width();
//     if (m_currentDirection == Direction::Left) {
//         nextX -= m_speed;
//     } else if (m_currentDirection == Direction::Right) {
//         nextX += m_speed;
//     }
//     // Collision avec les bords de la fenêtre (horizontal)
//     if (nextX < 0) nextX = 0;
//     if (nextX + m_frameWidth > parentWidth) nextX = parentWidth - m_frameWidth;


//     // 3. Calculer la position Y suivante basée sur la vélocité verticale
//     int currentY = y();
//     // Utiliser std::round pour convertir la vélocité en déplacement entier
//     int nextY = currentY + static_cast<int>(std::round(m_velocityY));
//     int parentHeight = parentWidget()->height();
//     // Collision avec le "sol" de la fenêtre parente
//     if (nextY + m_frameHeight > parentHeight) {
//         nextY = parentHeight - m_frameHeight;
//         if (m_velocityY > 0) { // Atterrissage sur le sol de la fenêtre
//             m_velocityY = 0;
//             m_isJumpingOrFalling = false;
//         }
//     }
//     // Empêcher de sortir par le haut (moins probable mais sécurité)
//     if (nextY < 0) {
//         nextY = 0;
//         if(m_velocityY < 0) m_velocityY = 0; // Arrêter la montée si on touche le plafond
//     }


//     // 4. Vérification et résolution des collisions avec les obstacles
//     QRect futureCollisionRect = getCollisionRect(QPoint(nextX, nextY));
//     QWidget* collidedObstacle = nullptr;

//     // Boucle pour résoudre les collisions potentiellement complexes (optionnel, simple check suffit pour commencer)
//     // Pour l'instant, on vérifie une fois
//     if (checkCollision(futureCollisionRect, collidedObstacle)) {
//         // Collision détectée ! Déterminer si c'est principalement vertical ou horizontal
//         // Approche simple : vérifier séparément les collisions H et V

//         // a) Vérifier collision verticale seule
//         QRect verticalCheckRect = getCollisionRect(QPoint(currentX, nextY)); // X actuel, Y futur
//         QWidget* verticalObstacle = nullptr;
//         if (checkCollision(verticalCheckRect, verticalObstacle) && verticalObstacle == collidedObstacle) {
//             resolveVerticalCollision(verticalObstacle, nextY);
//             // Recalculer le rect de collision après résolution verticale pour le check H
//             futureCollisionRect = getCollisionRect(QPoint(nextX, nextY));
//             collidedObstacle = nullptr; // Réinitialiser pour le check H
//             checkCollision(futureCollisionRect, collidedObstacle); // Re-vérifier H avec le Y ajusté
//         }

//         // b) Vérifier collision horizontale (potentiellement avec le Y ajusté)
//         if (collidedObstacle) { // S'il y a *encore* collision (ou c'était H initialement)
//             QRect horizontalCheckRect = getCollisionRect(QPoint(nextX, currentY)); // X futur, Y actuel (ou ajusté si collision V a eu lieu)
//             QWidget* horizontalObstacle = nullptr;
//             // On re-vérifie avec le Y actuel/ajusté pour être sûr que c'est bien H
//             if (checkCollision(horizontalCheckRect, horizontalObstacle) && horizontalObstacle == collidedObstacle) {
//                 resolveHorizontalCollision(horizontalObstacle, nextX);
//             } else if (m_isJumpingOrFalling){
//                 // Cas spécial: atterrissage en diagonale sur un coin?
//                 // Si après résolution V, il n'y a plus collision H, on peut ignorer H.
//                 // Si on est en l'air et qu'on touche un mur, on veut quand même résoudre H.
//                 resolveHorizontalCollision(collidedObstacle, nextX);
//             }
//         }
//     }


//     // 5. Mettre à jour la frame d'animation
//     if (m_isJumpingOrFalling) {
//         // Pourrait avoir une frame de saut/chute spécifique ici
//         // m_currentFrame = FRAME_JUMP;
//         // Pour l'instant, on garde la dernière frame ou la frame debout
//         if (m_currentDirection == Direction::None) {
//             m_currentFrame = m_standingFrame;
//         } // Sinon on garde l'animation de marche en l'air
//     } else if (m_currentDirection != Direction::None) { // Au sol et en mouvement
//         int firstAnimationFrame = (m_standingFrame == 0) ? 1 : 0;
//         int numAnimatedFrames = (m_standingFrame == 0) ? m_totalFrames - 1 : m_totalFrames;
//         if (numAnimatedFrames > 0) {
//             int currentAnimatedFrameIndex = (m_currentFrame - firstAnimationFrame + 1) % numAnimatedFrames;
//             m_currentFrame = firstAnimationFrame + currentAnimatedFrameIndex;
//         }
//     } else { // Au sol et immobile
//         m_currentFrame = m_standingFrame;
//     }

//     // 6. Déplacer le widget à la position finale calculée
//     if (x() != nextX || y() != nextY) {
//         move(nextX, nextY);
//     }

//     // 7. Demander un redessin
//     update();
// }


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
