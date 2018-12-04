#include "LejuSlider.h"

static int m_nWight = 45;
static int m_nHeight = 25;

LejuSlider::LejuSlider(Qt::Orientation orientation, QWidget *parent) : QSlider(parent)
{
    setOrientation(orientation);
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
/*******************************************************************************************************************************/

LejuPushButton::LejuPushButton(QString name, QWidget *parent) : QPushButton(parent)
{
    setText(name);
    setStyleSheet("QPushButton{background-color: qlineargradient("
                  "x1: 0, y1: 0, x2: 0, y2: 0.5, x3: 0, y3: 1,stop: 0 #525252, stop: 1 #3B3B3B);"
                  "border: 1px solid #525252; border-radius: 5px;"
                  "color: #FFFFFF; font-size: 15px;border-style: outset; outline: 0px;}"
                  "QPushButton:pressed{border-style: inset; background-color: qlineargradient("
                  "x1: 0, y1: 0, x2: 0, y2: 0.5, x3: 0, y3: 1,stop: 0 #818181, stop: 1 #6F6F6F);"
                  "border: 1px solid #525252; border-radius: 5px;"
                  "color: #CCCCCC; font-size: 15px;outline: 0px;}"
                  "QPushButton:checked{background-color: qlineargradient("
                  "x1: 0, y1: 0, x2: 0, y2: 0.5, x3: 0, y3: 1,stop: 0 #00CCFF, stop: 1 #1AA0FF);"
                  "border: 1px solid #525252; border-radius: 5px;"
                  "color: #FFFFFF; font-size: 15px;border-style: outset;outline: 0px;}");
    setMinimumSize(m_nHeight,m_nHeight);

    m_bHoldEnabled = false;
    m_timer = new QTimer(this);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
}

LejuPushButton::LejuPushButton(QWidget *parent) : QPushButton(parent)
{
    setStyleSheet("QPushButton{background-color: qlineargradient("
                  "x1: 0, y1: 0, x2: 0, y2: 0.5, x3: 0, y3: 1,stop: 0 #525252, stop: 1 #3B3B3B);"
                  "border: 1px solid #525252; border-radius: 5px;"
                  "color: #FFFFFF; font-size: 15px;border-style: outset;outline: 0px;}"
                  "QPushButton:pressed{border-style: inset; background-color: qlineargradient("
                  "x1: 0, y1: 0, x2: 0, y2: 0.5, x3: 0, y3: 1,stop: 0 #818181, stop: 1 #6F6F6F);"
                  "border: 1px solid #525252; border-radius: 5px;"
                  "color: #CCCCCC; font-size: 15px;outline: 0px;}"
                  "QPushButton:checked{background-color: qlineargradient("
                  "x1: 0, y1: 0, x2: 0, y2: 0.5, x3: 0, y3: 1,stop: 0 #00CCFF, stop: 1 #1AA0FF);"
                  "border: 1px solid #525252; border-radius: 5px;"
                  "color: #FFFFFF; font-size: 15px;border-style: outset;outline: 0px;}");
    setMinimumSize(m_nHeight,m_nHeight);

    m_bHoldEnabled = false;
    m_timer = new QTimer(this);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
}

void LejuPushButton::onTimeout()
{
    emit pressedAndHold();
    m_timer->setInterval(100);
}

void LejuPushButton::setPressedAndHoldEnable(bool pState)
{
    m_bHoldEnabled = pState;
}

void LejuPushButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_bHoldEnabled)
    {
        m_timer->start(800);
    }
    QPushButton::mousePressEvent(e);
}

void LejuPushButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_bHoldEnabled)
    {
        m_timer->stop();
    }
    QPushButton::mouseReleaseEvent(e);
}

LejuPushButton::~LejuPushButton()
{

}

/*******************************************************************************************************************************/

SliderGroupBox::SliderGroupBox(QWidget *parent) :
    QWidget(parent),m_type("Int"),isEntered(false),isBlock(false)
{
    nameLabel = new QLabel(this);
    nameLabel->setFixedSize(m_nWight,m_nHeight);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("QLabel {color : #E6E6E6; font: bold; font-size: 14px;}");
    QFont font( "Microsoft YaHei", 10, 75);
    nameLabel->setFont(font);

//    nameLabel->setStyleSheet("background-color: #00000000; color: #FFFFFF; font-size: 16px;");

    slider = new LejuSlider(Qt::Horizontal,this);
    slider->setStyleSheet("QSlider{background-color: #00000000;}"
                                 "QSlider::groove:horizontal{height: 2px; background-color: #999999; border: 0px solid #b3b3b3;border-radius: 1px;}"
                                 "QSlider::handle:horizontal{background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 0.5, x3: 0, y3: 1,stop: 0 #000000, stop: 1 #000000); "
                                 "height: 14px; width: 5px; "
                                 "margin: -6px 0px -6px 0px; border-radius: 2px; border: 1px solid #000000;}"
                                 "QSlider::add-page:horizontal{background-color: #FFFFFF; border-radius: 2px;}"
                                 "QSlider::sub-page:horizontal{background-color: #FFFFFF; border-radius: 2px;}");
    slider->setFixedWidth(m_nWight*2);
    slider->setFixedHeight(m_nHeight*.6);

    valueLabel = new QLabel("0",this);
    valueLabel->setFixedSize(m_nWight*.8,m_nHeight*.8);
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet("background-color: #FFFFFF; color: #797979; font-size: 12px; border-radius: 5px; border: 1px solid #666666;");

    plusBtn = new LejuPushButton("-",this);
    plusBtn->setPressedAndHoldEnable(true);
    plusBtn->setFixedSize(m_nHeight*.8,m_nHeight*.8);
    plusBtn->setStyleSheet("background-color: #444444;"
                           "color: #757575;"
                           "font: bold 16px;"
                           "margin: 0.5px;"
                           "text-align: center;"
                           "border-radius:3px;");


    minusBtn = new LejuPushButton("+",this);
    minusBtn->setPressedAndHoldEnable(true);
    minusBtn->setFixedSize(m_nHeight*.8,m_nHeight*.8);
    minusBtn->setStyleSheet("background-color: #444444;"
                            "color: #757575;"
                            "font: bold 16px;"
                            "margin: 0.5px;"
                            "text-align: center;"
                            "border-radius:3px;");

    btnLayout = new QHBoxLayout;
    btnLayout->addWidget(minusBtn);
    btnLayout->addWidget(valueLabel);
    btnLayout->addWidget(plusBtn);
    btnLayout->setMargin(0);
    btnLayout->setSpacing(0);


    // Layout combination

    mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(nameLabel);

    sliderValueLayout = new QVBoxLayout(this);
    sliderValueLayout->addWidget(slider);
    sliderValueLayout->addLayout(btnLayout);

    mainLayout->addLayout(sliderValueLayout);

//    mainLayout->addWidget(slider);
//    mainLayout->addLayout(btnLayout);

    mainLayout->setMargin(0);
    mainLayout->setSpacing(8);

//    setMinimumSize(80,m_nHeight);

    slider->installEventFilter(this);
    plusBtn->installEventFilter(this);
    minusBtn->installEventFilter(this);

    connect(slider,SIGNAL(valueChanged(int)),this,SLOT(onSliderValueChanged(int)));
    connect(slider,SIGNAL(sliderPressed()),this,SIGNAL(sliderPressed()));
    connect(slider,SIGNAL(sliderMoved(int)),this,SIGNAL(sliderMoved(int)));
    connect(slider,SIGNAL(sliderReleased()),this,SIGNAL(sliderReleased()));
    connect(minusBtn,SIGNAL(clicked(bool)),this,SLOT(onMinusBtnClicked()));
    connect(plusBtn,SIGNAL(clicked(bool)),this,SLOT(onPlusBtnCliked()));
    connect(minusBtn,SIGNAL(pressedAndHold()),this,SLOT(onMinusBtnPressedAndHold()));
    connect(plusBtn,SIGNAL(pressedAndHold()),this,SLOT(onPlusBtnPressedAndHold()));
}

void SliderGroupBox::hideSlider()
{
    slider->hide();
}

void SliderGroupBox::setType(const QString &pType)
{
    m_type = pType;
    if (pType == "float")
    {
        valueLabel->setText("0.00");
    }
}

QString SliderGroupBox::getName()
{
    return nameLabel->text();
}

void SliderGroupBox::setName(const QString &name)
{
    nameLabel->setText(name);
}

int SliderGroupBox::value()
{
    return slider->value();
}

void SliderGroupBox::setValue(int val, bool flag)
{
    if (flag)
    {
        isBlock = true;
        slider->setValue(val);
        isBlock = false;
    }
    else
    {
        slider->setValue(val);
    }
}

void SliderGroupBox::setRange(int pMin, int pMax)
{
    slider->setRange(pMin,pMax);
}

void SliderGroupBox::onSliderValueChanged(int val)
{
    if (m_type == "float")
    {
        valueLabel->setText(QString::number(val / 10.0, 'f', 2));
    }
    else
        valueLabel->setText(QString::number(val));

    if (!isBlock)
        emit sliderValueChanged(val);
}

void SliderGroupBox::onMinusBtnClicked()
{
    int pMinnum,pVal;
    pMinnum = slider->minimum();
    pVal = slider->value();
    pVal--;
    if (pVal >= pMinnum)
    {
        slider->setValue(pVal);
        emit buttonClicked();
    }
}

void SliderGroupBox::onPlusBtnCliked()
{
    int pMaxnum,pVal;
    pMaxnum = slider->maximum();
    pVal = slider->value();
    pVal++;
    if (pVal <= pMaxnum)
    {
        slider->setValue(pVal);
        emit buttonClicked();
    }
}

void SliderGroupBox::onMinusBtnPressedAndHold()
{
    int pMinnum,pVal;
    pMinnum = slider->minimum();
    pVal = slider->value();
    pVal--;
    if (pVal >= pMinnum)
    {
        slider->setValue(pVal);
        emit buttonPressedAndHold();
    }
}

void SliderGroupBox::onPlusBtnPressedAndHold()
{
    int pMaxnum,pVal;
    pMaxnum = slider->maximum();
    pVal = slider->value();
    pVal++;
    if (pVal <= pMaxnum)
    {
        slider->setValue(pVal);
        emit buttonPressedAndHold();
    }
}

bool SliderGroupBox::isContainsMouse()
{
    return isEntered;
}

bool SliderGroupBox::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == slider || watched == plusBtn || watched == minusBtn)
    {
        if (event->type() == QEvent::Enter)
        {
            isEntered = true;
            return true;
        }
        else if (event->type() == QEvent::Leave)
        {
            isEntered = false;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
        return QWidget::eventFilter(watched,event);
}

SliderGroupBox::~SliderGroupBox()
{

}
