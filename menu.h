#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include "QLabel"

QT_BEGIN_NAMESPACE
namespace Ui { class menu; }
QT_END_NAMESPACE

class menu : public QWidget
{
    Q_OBJECT

public:
    menu(QWidget *parent = nullptr);
    ~menu();



//    bool eventFilter(QObject* object, QEvent* event);

    void setup();
    void dealCard(int cardNum, int Player);
    void init();
    void modifyMyHandCard();
    bool eventFilter(QObject *obj, QEvent *event);
    void click(int id);
    void move(int id);
    void leave(int id);
    void modifyPlayerCard(int id);
    void attack();
    void roundStart();
    void playerStart();
    void cure(int x);
    void modifyShowCard();
    void CaculateCard(int card_id, int val, int flag);
    void defend();
    void defendMode();
private slots:
    void on_confirmButton_clicked();


    void on_sortButton_clicked();

    void on_defendButton_clicked();

    void on_passButton_clicked();

private:
    Ui::menu *ui;
};
#endif // MENU_H
