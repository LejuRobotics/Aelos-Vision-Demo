/**
 * @file       painterlabel.cpp
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      显示标记矩形，PaintLabel类和TestLabel类的cpp文件
 * @details
 */

#include "painterlabel.h"

/**
 * @brief     PaintLabel类的构造函数
 * @param     parent 父对象
 */

PaintLabel::PaintLabel(QWidget *parent) : QLabel(parent)
  ,isLeftButtonPressed(false),pen_width(2)
{
    setScaledContents(true);
    isClean = true;
    m_singled = true;
}

/**
 * @brief     重写paintEvent事件
 * @details   通过记录的点的位置，描绘矩形
 */

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
        if (m_singled)
        {
            p.setPen(QPen(m_markColor, pen_width));
            p.drawRect(mark_rect);
        }
        else
        {
            if (m_mark_color_list.size() != m_mark_rect_vector.size())
                return;

            for (int i=0; i< m_mark_rect_vector.size(); ++i)
            {
                p.setPen(QPen(m_mark_color_list[i], pen_width));
                p.drawRect(m_mark_rect_vector[i]);
            }
        }
    }
}

/**
 * @brief    根据传入的参数描绘一个矩形
 * @param    rect 矩形
 */

void PaintLabel::drawRect(const QRect &_rect)
{
    mark_rect = _rect;
    isClean = false;
    m_singled = true;
    this->repaint();
}

void PaintLabel::drawRects(const QVector<QRect> &vector)
{
    m_mark_rect_vector = vector;
    isClean = false;
    m_singled = false;
    this->repaint();
}

/**
 * @brief    清除之前描绘的矩形
 */

void PaintLabel::cleanRect()
{
    isClean = true;
    this->repaint();
}

void PaintLabel::setMarkColor(const QString &pColor)
{
    QStringList rgb = pColor.split(",");
    if (rgb.length() != 3)
    {
        return;
    }
    int r = rgb[0].toInt();
    int g = rgb[1].toInt();
    int b = rgb[2].toInt();
    m_markColor.setRgb(r,g,b);
}

void PaintLabel::addMarkColor(const QString &pColor)
{
    setMarkColor(pColor);
    m_mark_color_list.append(m_markColor);
}

void PaintLabel::removeMarkColorAt(int index)
{
    isClean = true;
    m_mark_color_list.removeAt(index);
    isClean = false;
}

/**
 * @brief     重写mousePressEvent事件
 * @details   记录第一次鼠标点击的位置
 */

void PaintLabel::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        isLeftButtonPressed = true;
        mousePoint[0] = e->pos();
    }
    QLabel::mousePressEvent(e);
}

/**
 * @brief     重写mouseMoveEvent事件
 * @details   根据鼠标拖动描绘矩形
 */

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

/**
 * @brief     重写mouseReleaseEvent事件
 * @details   描绘的矩形大小不超过该控件大小，并发送信号selectFinished()
 */

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

/**
 * @brief     TestLabel类的构造函数
 * @param     parent 父对象
 */

TestLabel::TestLabel(QWidget *parent): QLabel(parent)
{
    m_ratio1 = 0.42;
    m_ratio2 = 0.8;
}

/**
 * @brief     根据传入的参数重新绘线
 * @param     v1 中间区域的位置划分
 * @param     v2 机器人是执行大幅度转动动作还是小幅度转动动作的区域划分
 */

void TestLabel::drawArea(double v1, double v2)
{
    m_ratio1 = v1;
    m_ratio2 = v2;
    this->repaint();
}

/**
 * @brief     重写paintEvent事件
 * @details   根据v1,v2的值重新描绘直线，划分区域
 */

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
