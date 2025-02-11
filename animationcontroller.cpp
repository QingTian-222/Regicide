#include "animationcontroller.h"

AnimationController::AnimationController(QObject *parent)
    : QObject{parent}
{

}
void AnimationController::addAnimation(QPropertyAnimation* animation) {
    animations.append(animation);
    QObject::connect(animation,&QPropertyAnimation::finished,[=](){

        if (currentIndex < animations.size() - 1) {
            currentIndex++;
            auto tg=animations[currentIndex];
            tg->setStartValue(tg->targetObject()->property(tg->propertyName()));
            tg->start();
        } else {
            qDebug() << "All animations completed.";
            emit finished();
        }
    });
}
void AnimationController::startAnimations() {
    if (!animations.isEmpty()) {
        currentIndex = 0;
        animations.first()->start();
    } else {
        qDebug() << "No animations to start.";
    }
}

void AnimationController::clear()
{
    for(auto i:animations)
        delete i;
    animations.clear();
}
