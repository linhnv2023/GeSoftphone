#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QSlider>
#include <QRadioButton>
#include "../core/SipTypes.h"

class SipEngine;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(SipEngine* engine = nullptr, QWidget *parent = nullptr);

private slots:
    void onSaveClicked();

private:
    void setupUi();
    void loadValues();

    SipEngine* m_engine = nullptr;

    // Left column
    QCheckBox* m_singleCallCheck = nullptr;
    QLineEdit* m_ringtoneEdit = nullptr;
    QSlider* m_ringVolumeSlider = nullptr;
    QComboBox* m_ringDeviceCombo = nullptr;
    QComboBox* m_speakerCombo = nullptr;
    QComboBox* m_micCombo = nullptr;
    QCheckBox* m_micAmpCheck = nullptr;
    QCheckBox* m_softwareLevelCheck = nullptr;
    QListWidget* m_availableCodecsList = nullptr;
    QListWidget* m_enabledCodecsList = nullptr;
    QCheckBox* m_vadCheck = nullptr;
    QCheckBox* m_ecCheck = nullptr;
    QCheckBox* m_forceCodecCheck = nullptr;
    QComboBox* m_cameraCombo = nullptr;
    QComboBox* m_videoCodecCombo = nullptr;
    QCheckBox* m_h264Check = nullptr;
    QCheckBox* m_h263Check = nullptr;
    QCheckBox* m_vp8Check = nullptr;
    QSpinBox* m_bitrateSpin = nullptr;
    QSpinBox* m_sourcePortSpin = nullptr;
    QCheckBox* m_rportCheck = nullptr;
    QSpinBox* m_rtpPortMinSpin = nullptr;
    QSpinBox* m_rtpPortMaxSpin = nullptr;
    QLineEdit* m_nameserverEdit = nullptr;
    QCheckBox* m_dnsSrvCheck = nullptr;
    QCheckBox* m_stunCheck = nullptr;
    QLineEdit* m_stunEdit = nullptr;

    // Right column
    QCheckBox* m_recordCheck = nullptr;
    QLineEdit* m_recordDirEdit = nullptr;
    QRadioButton* m_mp3Radio = nullptr;
    QRadioButton* m_wavRadio = nullptr;
    QComboBox* m_dtmfCombo = nullptr;
    QComboBox* m_autoAnswerCombo = nullptr;
    QSpinBox* m_autoAnswerSecSpin = nullptr;
    QComboBox* m_denyIncomingCombo = nullptr;
    QLineEdit* m_directoryEdit = nullptr;
    QComboBox* m_defaultActionCombo = nullptr;
    QCheckBox* m_mediaButtonsCheck = nullptr;
    QCheckBox* m_soundEventsCheck = nullptr;
    QCheckBox* m_bringToFrontCheck = nullptr;
    QCheckBox* m_randomPopupCheck = nullptr;
    QCheckBox* m_callWaitingCheck = nullptr;
    QCheckBox* m_logFileCheck = nullptr;
    QCheckBox* m_localAccountCheck = nullptr;
    QCheckBox* m_crashReportCheck = nullptr;
    QComboBox* m_updatesCombo = nullptr;
};
