/**
 * @file       ParameterSettingDialog.cpp
 * @version    1.0
 * @date       2017年07月22日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      参数设置界面，ParameterSettingDialog类的cpp文件
 */

#include "ParameterSettingDialog.h"
#include "ui_ParameterSettingDialog.h"

ParameterSettingDialog::ParameterSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParameterSettingDialog)
{
    ui->setupUi(this);

    ui->comboBox->addItem("160x120");
    ui->comboBox->addItem("320x240");
    ui->comboBox->addItem("640x480");
    ui->comboBox->addItem("1280x960");

    init();
}

ParameterSettingDialog::~ParameterSettingDialog()
{
    delete ui;
}

void ParameterSettingDialog::setResolution(int w, int h)
{
    QString currentVal = QString("%1x%2").arg(w).arg(h);
    for (int i=0; i<ui->comboBox->count(); ++i)
    {
        if (currentVal == ui->comboBox->itemText(i))
        {
            ui->comboBox->setCurrentIndex(i);
            return;
        }
    }

    ui->comboBox->addItem(currentVal);
    ui->comboBox->setCurrentIndex(ui->comboBox->count()-1);
}

void ParameterSettingDialog::setValue(ParameterSettingDialog::Type type, const QVariant &val)
{
    switch (type)
    {
    case Resolution:
    {
        QString currentVal = val.toString();
        for (int i=0; i<ui->comboBox->count(); ++i)
        {
            if (currentVal == ui->comboBox->itemText(i))
            {
                ui->comboBox->setCurrentIndex(i);
                return;
            }
        }
        ui->comboBox->addItem(currentVal);
        ui->comboBox->setCurrentIndex(ui->comboBox->count()-1);
    }
        break;
    case ImageQuality:
        ui->quality_spinbox->setValue(val.toInt());
        break;
    case CenterRatio:
        ui->center_spinBox->setValue(val.toDouble());
        break;
    case TurnRatio:
        ui->turn_spinbox->setValue(val.toDouble());
        break;
    case ArriveRatio:
        ui->arrive_spinbox->setValue(val.toDouble());
        break;
    case AccessRatio:
        ui->access_spinbox->setValue(val.toDouble());
        break;
    case DelayCount:
        ui->delay_edit->setText(val.toString());
        break;
    case QuickCount:
        ui->quick_edit->setText(val.toString());
        break;
    case SlowCount:
        ui->slow_edit->setText(val.toString());
        break;
    case ObstacleTurnCount:
        ui->turnCount_edit->setText(val.toString());
        break;
    case GobackTurnCount:
        ui->turnRoundCount_edit->setText(val.toString());
        break;
    default:
        break;
    }
}

void ParameterSettingDialog::init()
{
    m_resolution = ui->comboBox->currentText();
    m_imageQuality = ui->quality_spinbox->value();
    m_centerRatio = ui->center_spinBox->value();
    m_turnRatio = ui->turn_spinbox->value();
    m_arriveRatio = ui->arrive_spinbox->value();
    m_accessRatio = ui->access_spinbox->value();
    m_delayCount = ui->delay_edit->text().toInt();
    m_quickCount = ui->quick_edit->text().toInt();
    m_slowCount = ui->slow_edit->text().toInt();
    m_obstacleTurnCount = ui->turnCount_edit->text().toInt();
    m_gobackTurnCount = ui->turnRoundCount_edit->text().toInt();
}

void ParameterSettingDialog::on_buttonBox_accepted()
{
    this->hide();
    init();
    QString msg;
    msg = QString("start set param\r\n"
                  "Image.Quality=%1\r\n"
                  "Center.Ratio=%2\r\n"
                  "Turn.Ratio=%3\r\n"
                  "Arrive.Ratio=%4\r\n"
                  "Access.Ratio=%5\r\n"
                  "Delay.Count=%6\r\n"
                  "Quick.Count=%7\r\n"
                  "Slow.Count=%8\r\n"
                  "Camera.Resolution=%9\r\n"
                  "Obstacle.Turn.Count=%10\r\n"
                  "Goback.Turn.Count=%11\r\n")
            .arg(m_imageQuality).arg(m_centerRatio).arg(m_turnRatio)
            .arg(m_arriveRatio).arg(m_accessRatio).arg(m_delayCount)
            .arg(m_quickCount).arg(m_slowCount).arg(m_resolution)
            .arg(m_obstacleTurnCount).arg(m_gobackTurnCount);

    emit sendData(msg.toUtf8());
    emit debugViewChanged(m_centerRatio,m_turnRatio);
}

void ParameterSettingDialog::on_buttonBox_rejected()
{
    this->hide();
    ui->quality_spinbox->setValue(m_imageQuality);
    ui->center_spinBox->setValue(m_centerRatio);
    ui->turn_spinbox->setValue(m_turnRatio);
    ui->arrive_spinbox->setValue(m_arriveRatio);
    ui->access_spinbox->setValue(m_accessRatio);
    ui->delay_edit->setText(QString::number(m_delayCount));
    ui->quick_edit->setText(QString::number(m_quickCount));
    ui->slow_edit->setText(QString::number(m_slowCount));
    ui->turnCount_edit->setText(QString::number(m_obstacleTurnCount));
    ui->turnRoundCount_edit->setText(QString::number(m_gobackTurnCount));
}

