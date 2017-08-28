#ifndef LEJUSLIDER_H
#define LEJUSLIDER_H

#include "precompiled.h"

class LejuSlider : public QSlider
{
    Q_OBJECT
public:
    explicit LejuSlider(QWidget *parent = nullptr);

    bool isBlocked;

    void SetValue(int val);

signals:
    void sliderValueChanged(int val);

private slots:
    void onValueChanged(int val);

protected:
    virtual void mouseReleaseEvent(QMouseEvent *e);

};

#endif // LEJUSLIDER_H
