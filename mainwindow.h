#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList> // Ajout pour QList
#include "player.h" // Pour Player::Direction

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QKeyEvent;
class QWidget; // Déclaration anticipée pour la liste d'obstacles

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void setupObstacles(); // Méthode pour créer les obstacles

    Ui::MainWindow *ui;
    Player* m_player;
    QList<QWidget*> m_obstaclesList; // Liste pour stocker les obstacles
};
#endif // MAINWINDOW_H
