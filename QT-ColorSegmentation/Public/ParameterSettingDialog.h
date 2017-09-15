/**
 * @file       ParameterSettingDialog.h
 * @version    1.0
 * @date       2017年07月22日
 * @author     C_Are
 * @copyright  Leju
 *
 * @brief      参数设置界面，ParameterSettingDialog类的h文件
 */

#ifndef PARAMETERSETTINGDIALOG_H
#define PARAMETERSETTINGDIALOG_H

#include "precompiled.h"

namespace Ui {
class ParameterSettingDialog;
}

class ParameterSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterSettingDialog(QWidget *parent = 0);
    ~ParameterSettingDialog();

    enum Type {
        Resolution,
        ImageQuality,
        CenterRatio,
        TurnRatio,
        ArriveRatio,
        AccessRatio,
        DelayCount,
        QuickCount,
        SlowCount,
        ObstacleTurnCount,
        GobackTurnCount
    };

    void setResolution(int w, int h);
    void setValue(Type type, const QVariant &val);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

signals:
    void sendData(const QByteArray &msg);
    void debugViewChanged(double centerRatio, double turnRatio);

private:
    Ui::ParameterSettingDialog *ui;

    void init();

    QString m_resolution;
    int m_imageQuality;
    double m_centerRatio;
    double m_turnRatio;
    double m_arriveRatio;
    double m_accessRatio;
    int m_delayCount;
    int m_quickCount;
    int m_slowCount;
    int m_obstacleTurnCount;
    int m_gobackTurnCount;
};

#endif // PARAMETERSETTINGDIALOG_H
