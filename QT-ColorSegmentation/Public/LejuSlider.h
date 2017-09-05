#ifndef LEJUSLIDER_H
#define LEJUSLIDER_H

#include "precompiled.h"

class LejuSlider : public QSlider
{
    Q_OBJECT
public:
    explicit LejuSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

    bool isBlocked;

    void SetValue(int val);

signals:
    void sliderValueChanged(int val);

private slots:
    void onValueChanged(int val);

protected:
    virtual void mouseReleaseEvent(QMouseEvent *e);

};

class LejuPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit LejuPushButton(QString name,QWidget *parent = 0);
    explicit LejuPushButton(QWidget *parent = 0);
    ~LejuPushButton();

    void setPressedAndHoldEnable(bool pState);

signals:
    void pressedAndHold();

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private slots:
    void onTimeout();

private:
    QTimer *m_timer;
    bool m_bHoldEnabled;
};

class SliderGroupBox : public QWidget
{
    Q_OBJECT
public:
    explicit SliderGroupBox(QWidget *parent = 0);
    ~SliderGroupBox();

    void setType(const QString &pType);
    QString getName();
    void setName(const QString &name);
    int value();
    void setValue(int val, bool flag = false);
    void setRange(int pMin,int pMax);
    void hideSlider();
    bool isContainsMouse();

signals:
    void sliderValueChanged(int val);
    void sliderPressed();
    void sliderMoved(int val);
    void sliderReleased();
    void buttonClicked();
    void buttonPressedAndHold();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void onSliderValueChanged(int val);
    void onMinusBtnClicked();
    void onPlusBtnCliked();
    void onMinusBtnPressedAndHold();
    void onPlusBtnPressedAndHold();

private:
    QString m_type;
    QLabel *nameLabel;
    LejuSlider *slider;
    QLabel *valueLabel;
    LejuPushButton *plusBtn;
    LejuPushButton *minusBtn;
    QHBoxLayout *btnLayout;
    QHBoxLayout *mainLayout;
    bool isEntered;
    bool isBlock;
};

#endif // LEJUSLIDER_H
