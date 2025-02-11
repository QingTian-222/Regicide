#include "tanimation.h"

TAnimation::TAnimation(QObject *target, const QByteArray &propertyName, QObject *parent)
    : QPropertyAnimation(target, propertyName, parent)
{

}
void TAnimation::updateCurrentValue(const QVariant &value)
{
    if (!have_started) {
        have_started = true;
        emit started();
    }
    QPropertyAnimation::updateCurrentValue(value);
}
