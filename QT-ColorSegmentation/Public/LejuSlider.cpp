#include "LejuSlider.h"

LejuSlider::LejuSlider(QWidget *parent) : QSlider(parent)
{
    isBlocked = false;
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
}

void LejuSlider::SetValue(int val)
{
    isBlocked = true;
    setValue(val);
    isBlocked = false;
}

void LejuSlider::onValueChanged(int val)
{
    if (!isBlocked)
        emit sliderValueChanged(val);
}

void LejuSlider::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        int p = (int)((double)(e->x()/(double)this->width()*(double)(this->maximum() - this->minimum())));
        p += this->minimum();
        this->setValue(p);
//        emit sliderReleased();
    }
    QSlider::mouseReleaseEvent(e);
}
