#ifndef PAINTERLABEL_H
#define PAINTERLABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

class PaintLabel : public QLabel
{
    Q_OBJECT
public:
    explicit PaintLabel(QWidget *parent = 0);

    bool isPressed() const
    {
        return isLeftButtonPressed;
    }

    void drawRect(const QRect &rect);
    void cleanRect();

signals:
    void selectFinished(const QRect&);

protected:
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    QPoint mousePoint[2];
    QRect m_rect;
    QRect mark_rect;
    bool isLeftButtonPressed;
    int pen_width;
    bool isClean;
    bool isMoved;
};

class TestLabel : public QLabel
{
    Q_OBJECT
public:
    explicit TestLabel(QWidget *parent = 0);

    void drawArea(double v1, double v2);

protected:
    virtual void paintEvent(QPaintEvent *e);

private:
    double m_ratio1;
    double m_ratio2;
};

#endif // PAINTERLABEL_H
