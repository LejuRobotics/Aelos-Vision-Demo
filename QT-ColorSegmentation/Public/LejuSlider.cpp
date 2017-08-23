#include "LejuSlider.h"

LejuSlider::LejuSlider(QWidget *parent) : QSlider(parent)
{

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
