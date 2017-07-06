#include "painterlabel.h"

PaintLabel::PaintLabel(QWidget *parent) : QLabel(parent)
  ,isLeftButtonPressed(false),pen_width(2)
{
    setScaledContents(true);
    isClean = true;
}

void PaintLabel::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);
    if (isLeftButtonPressed && isMoved)
    {
        QPainter p(this);
        p.setPen(QPen(Qt::green,pen_width));
        m_rect = QRect(mousePoint[0] ,mousePoint[1]);
        p.drawRect(m_rect);
    }

    if (!isClean)
    {
        QPainter p(this);
        p.setPen(QPen(Qt::red, pen_width));
        p.drawRect(mark_rect);
    }
}

void PaintLabel::drawRect(const QRect &rect)
{
    mark_rect = rect;
    isClean = false;
    this->repaint();
}

void PaintLabel::cleanRect()
{
    isClean = true;
    this->repaint();
}


void PaintLabel::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        isLeftButtonPressed = true;
        mousePoint[0] = e->pos();
    }
    QLabel::mousePressEvent(e);
}

void PaintLabel::mouseMoveEvent(QMouseEvent *e)
{
    if (isLeftButtonPressed)
    {
        isMoved = true;
        mousePoint[1] = e->pos();
        this->repaint();
    }
    QLabel::mouseMoveEvent(e);
}

void PaintLabel::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && isLeftButtonPressed)
    {
        bool isOver = false;
        if (e->x() < 0)
        {
            mousePoint[1].setX(0);
            isOver = true;
        }
        else if (e->x() > this->width())
        {
            mousePoint[1].setX(this->width()-pen_width);
            isOver = true;
        }
        if (e->y() < 0)
        {
            mousePoint[1].setY(0);
            isOver = true;
        }
        else if (e->y() > this->height())
        {
            mousePoint[1].setY(this->height()-pen_width);
            isOver = true;
        }

        if (isOver)
        {
            this->repaint();
        }

        emit selectFinished(m_rect);
        isLeftButtonPressed = false;
        isMoved = false;
    }
    QLabel::mouseReleaseEvent(e);
}

TestLabel::TestLabel(QWidget *parent): QLabel(parent)
{
    m_ratio1 = 0.42;
    m_ratio2 = 0.8;
}

void TestLabel::drawArea(double v1, double v2)
{
    m_ratio1 = v1;
    m_ratio2 = v2;
    this->repaint();
}

void TestLabel::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter p(this);
    p.setPen(QPen(Qt::green,1));
    int w1 = qRound(this->width()*m_ratio1);
    int w2 = qRound(this->width()*(1-m_ratio1));
    int w3 = qRound(this->width()*m_ratio1*m_ratio2);
    int w4 = this->width()-w3;
    QLine line1(w1,0,w1,this->height());
    QLine line2(w2,0,w2,this->height());
    p.drawLine(line1);
    p.drawLine(line2);
    QFont font = p.font();
    font.setPixelSize(14);
    p.setFont(font);
    p.drawText(QRect(w1+10,this->height()*0.8, (w2-w1-20), this->height()*0.2-10), Qt::AlignCenter, tr("Center"));

    p.setPen(QPen(Qt::white,1,Qt::DotLine));
    QLine line3(w3,0,w3,this->height());
    QLine line4(w4,0,w4,this->height());
    p.drawLine(line3);
    p.drawLine(line4);
}
