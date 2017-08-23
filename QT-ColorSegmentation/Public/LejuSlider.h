#ifndef LEJUSLIDER_H
#define LEJUSLIDER_H

#include "precompiled.h"

class LejuSlider : public QSlider
{
    Q_OBJECT
public:
    explicit LejuSlider(QWidget *parent = nullptr);

protected:
    virtual void mouseReleaseEvent(QMouseEvent *e);
};

#endif // LEJUSLIDER_H
