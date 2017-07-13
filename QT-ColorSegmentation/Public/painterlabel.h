/**
 * @file       painterlabel.h
 * @version    1.0
 * @date       2017年07月08日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      显示标记矩形，PaintLabel类和TestLabel类的h文件
 * @details    PaintLabel类可以根据鼠标拖动任意画一个矩形,
 *             TestLabel类是一个用来显示服务端设置的区域位置，只是一个用来调试的类
 */

#ifndef PAINTERLABEL_H
#define PAINTERLABEL_H

#include "precompiled.h"

/**
 * @class     PaintLabel
 * @brief     根据鼠标拖动描绘矩形
 * @details   继承QLabel类,通过重写事件，实现在控件内描绘矩形
 */

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
    QPoint mousePoint[2];  /**< 记录鼠标按下和移动的点的位置 */
    QRect m_rect;          /**< 鼠标拖动描绘的矩形 */
    QRect mark_rect;       /**< 外部传入的矩形 */
    bool isLeftButtonPressed;  /**< 鼠标左键是否按下的标志 */
    int pen_width;   /**< 画笔大小 */
    bool isClean;    /**< false则根据mark_rect描绘一个矩形，true则清除mark_rect */
    bool isMoved;    /**< 鼠标是否正在移动的标志 */
};

/**
 * @class     TestLabel
 * @brief     根据指定位置画线
 * @details   继承QLabel类,通过重写事件，实现在控件内描绘直线
 */

class TestLabel : public QLabel
{
    Q_OBJECT
public:
    explicit TestLabel(QWidget *parent = 0);

    void drawArea(double v1, double v2);

protected:
    virtual void paintEvent(QPaintEvent *e);

private:
    double m_ratio1;    /**< 中间区域位置的比例 */
    double m_ratio2;    /**< 是执行大幅度还是执行小幅度动作的区域位置划分 */
};

#endif // PAINTERLABEL_H
