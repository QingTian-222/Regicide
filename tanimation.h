#ifndef TANIMATION_H
#define TANIMATION_H

#include <qpropertyanimation.h>

class TAnimation : public QPropertyAnimation
{
    Q_OBJECT

public:
    TAnimation(QObject *target, const QByteArray &propertyName, QObject *parent = nullptr);

signals:
    void started();

protected:
    void updateCurrentValue(const QVariant &value) override;

private:
    bool have_started = false;
};

#endif // TANIMATION_H
