#ifndef ANIMATIONCONTROLLER_H
#define ANIMATIONCONTROLLER_H

#include <QObject>
#include <qlist.h>
#include <qpropertyanimation.h>

class AnimationController : public QObject
{
    Q_OBJECT

public:
    explicit AnimationController(QObject *parent = nullptr);
public slots:
    void addAnimation(QPropertyAnimation* animation);
    void startAnimations();
    void clear();
signals:
    void finished();
private:
    QList<QPropertyAnimation*> animations;
    int currentIndex = 0;
};

#endif // ANIMATIONCONTROLLER_H
