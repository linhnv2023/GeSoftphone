#include "SettingsDialog.h"
#include "../core/SipEngine.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QSettings>

SettingsDialog::SettingsDialog(SipEngine* engine, QWidget *parent)
    : QDialog(parent)
    , m_engine(engine)
{
    setupUi();
    loadValues();
}

void SettingsDialog::setupUi()
{
    setWindowTitle("Settings");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(680, 520);
    setStyleSheet(R"(
        QDialog {
            background-color: #FFFFFF;
            font-family: "SF Pro Text", "Helvetica Neue", sans-serif;
            font-size: 11.5px;
            color: #0F172A;
        }
        QLabel {
            color: #334155;
            font-size: 11px;
            font-weight: 500;
        }
        QLineEdit, QComboBox, QSpinBox, QListWidget {
            background-color: #F8FAFC;
            border: 1.2px solid #CBD5E1;
            border-radius: 5px;
            padding: 2px 6px;
            font-size: 11px;
            color: #0F172A;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QListWidget:focus {
            background-color: #FFFFFF;
            border-color: #3B82F6;
        }
        QListWidget {
            border: 1.2px solid #CBD5E1;
            border-radius: 6px;
            background: #FFFFFF;
        }
        QListWidget::item {
            padding: 3px 6px;
            border-radius: 4px;
        }
        QListWidget::item:selected {
            background-color: #EFF6FF;
            color: #1E40AF;
        }
        QCheckBox, QRadioButton {
            font-size: 11px;
            color: #334155;
            spacing: 5px;
        }
        QPushButton {
            background-color: #F1F5F9;
            color: #475569;
            border: 1px solid #CBD5E1;
            border-radius: 5px;
            padding: 4px 10px;
            font-size: 11px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #E2E8F0;
            color: #0F172A;
        }
        QPushButton#saveBtn {
            background-color: #3B82F6;
            color: #FFFFFF;
            border: none;
            font-weight: 700;
        }
        QPushButton#saveBtn:hover {
            background-color: #2563EB;
        }
        QLabel.helpText {
            color: #3B82F6;
            font-weight: bold;
        }
    )");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 10, 14, 10);
    mainLayout->setSpacing(6);

    // Top: Single Call Mode
    auto* topRow = new QHBoxLayout();
    auto* h1 = new QLabel("?", this); h1->setStyleSheet("color: #0066CC; font-weight: bold;");
    m_singleCallCheck = new QCheckBox("Single Call Mode", this);
    topRow->addWidget(h1);
    topRow->addWidget(m_singleCallCheck);
    topRow->addStretch();
    mainLayout->addLayout(topRow);

    // Two Columns Split
    auto* colsLayout = new QHBoxLayout();
    colsLayout->setSpacing(24);

    // ========================================================================
    // LEFT COLUMN
    // ========================================================================
    auto* leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(4);

    // Ringtone
    auto* ringRow = new QHBoxLayout();
    ringRow->addWidget(new QLabel("Ringtone", this));
    m_ringtoneEdit = new QLineEdit(this);
    auto* browseRing = new QPushButton("...", this);
    browseRing->setFixedWidth(24);
    auto* clearRing = new QPushButton("X", this);
    clearRing->setFixedWidth(24);
    ringRow->addWidget(m_ringtoneEdit);
    ringRow->addWidget(browseRing);
    ringRow->addWidget(clearRing);
    leftLayout->addLayout(ringRow);

    m_ringVolumeSlider = new QSlider(Qt::Horizontal, this);
    m_ringVolumeSlider->setRange(0, 100);
    m_ringVolumeSlider->setValue(80);
    leftLayout->addWidget(m_ringVolumeSlider);

    // Audio Devices
    auto addDevRow = [this, &leftLayout](const QString& label, QComboBox*& combo) {
        auto* row = new QHBoxLayout();
        auto* lbl = new QLabel(label, this);
        lbl->setFixedWidth(75);
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        combo = new QComboBox(this);
        combo->addItem("Default");
        row->addWidget(lbl);
        row->addWidget(combo);
        leftLayout->addLayout(row);
    };

    addDevRow("Ring Device", m_ringDeviceCombo);
    addDevRow("Speaker", m_speakerCombo);
    addDevRow("Microphone", m_micCombo);

    m_micAmpCheck = new QCheckBox("Microphone Amplification", this);
    m_softwareLevelCheck = new QCheckBox("Software Level Adjustment", this);
    leftLayout->addWidget(m_micAmpCheck);
    leftLayout->addWidget(m_softwareLevelCheck);

    // Codecs Selection (Available vs Enabled)
    auto* codecBox = new QHBoxLayout();
    auto* availBox = new QVBoxLayout();
    availBox->addWidget(new QLabel("Available Codecs", this));
    m_availableCodecsList = new QListWidget(this);
    m_availableCodecsList->addItems({"Opus 24 kHz", "G.722 16 kHz", "G.722.1 16 kHz", "G.729 8 kHz", "GSM 8 kHz"});
    m_availableCodecsList->setFixedSize(110, 80);
    availBox->addWidget(m_availableCodecsList);

    auto* arrowsBox = new QVBoxLayout();
    auto* toEnabledBtn = new QPushButton("►", this); toEnabledBtn->setFixedWidth(24);
    auto* toAvailBtn = new QPushButton("◄", this); toAvailBtn->setFixedWidth(24);
    arrowsBox->addStretch();
    arrowsBox->addWidget(toEnabledBtn);
    arrowsBox->addWidget(toAvailBtn);
    arrowsBox->addStretch();

    auto* enabledBox = new QVBoxLayout();
    enabledBox->addWidget(new QLabel("Enabled Codecs", this));
    m_enabledCodecsList = new QListWidget(this);
    m_enabledCodecsList->addItems({"Opus 48 kHz", "G.711 A-law", "G.711 u-law"});
    m_enabledCodecsList->setFixedSize(110, 80);
    enabledBox->addWidget(m_enabledCodecsList);

    codecBox->addLayout(availBox);
    codecBox->addLayout(arrowsBox);
    codecBox->addLayout(enabledBox);
    leftLayout->addLayout(codecBox);

    // VAD / EC / Force Codec
    auto* vadEcRow = new QHBoxLayout();
    m_vadCheck = new QCheckBox("VAD", this);
    m_ecCheck = new QCheckBox("EC", this);
    m_ecCheck->setChecked(true);
    m_forceCodecCheck = new QCheckBox("Force Codec for Incoming", this);
    vadEcRow->addWidget(m_vadCheck);
    vadEcRow->addWidget(m_ecCheck);
    vadEcRow->addWidget(m_forceCodecCheck);
    leftLayout->addLayout(vadEcRow);

    // Video Codecs & Camera
    auto* camRow = new QHBoxLayout();
    camRow->addWidget(new QLabel("Camera", this));
    m_cameraCombo = new QComboBox(this);
    m_cameraCombo->addItem("Default");
    camRow->addWidget(m_cameraCombo);
    leftLayout->addLayout(camRow);

    auto* vidCodecsRow = new QHBoxLayout();
    m_h264Check = new QCheckBox("H.264", this); m_h264Check->setChecked(true);
    m_h263Check = new QCheckBox("H.263", this); m_h263Check->setChecked(true);
    m_vp8Check = new QCheckBox("VP8", this); m_vp8Check->setChecked(true);
    vidCodecsRow->addWidget(m_h264Check);
    vidCodecsRow->addWidget(m_h263Check);
    vidCodecsRow->addWidget(m_vp8Check);
    vidCodecsRow->addWidget(new QLabel("Bitrate", this));
    m_bitrateSpin = new QSpinBox(this);
    m_bitrateSpin->setRange(64, 4096);
    m_bitrateSpin->setValue(256);
    vidCodecsRow->addWidget(m_bitrateSpin);
    leftLayout->addLayout(vidCodecsRow);

    // Ports & STUN
    auto* portsRow = new QHBoxLayout();
    portsRow->addWidget(new QLabel("Source Port", this));
    m_sourcePortSpin = new QSpinBox(this);
    m_sourcePortSpin->setRange(0, 65535);
    m_sourcePortSpin->setValue(0);
    portsRow->addWidget(m_sourcePortSpin);
    m_rportCheck = new QCheckBox("rport", this);
    m_rportCheck->setChecked(true);
    portsRow->addWidget(m_rportCheck);
    portsRow->addWidget(new QLabel("RTP", this));
    m_rtpPortMinSpin = new QSpinBox(this); m_rtpPortMinSpin->setRange(0, 65535); m_rtpPortMinSpin->setValue(0);
    m_rtpPortMaxSpin = new QSpinBox(this); m_rtpPortMaxSpin->setRange(0, 65535); m_rtpPortMaxSpin->setValue(0);
    portsRow->addWidget(m_rtpPortMinSpin);
    portsRow->addWidget(new QLabel("-", this));
    portsRow->addWidget(m_rtpPortMaxSpin);
    leftLayout->addLayout(portsRow);

    auto* stunRow = new QHBoxLayout();
    m_stunCheck = new QCheckBox("STUN Server", this);
    m_stunEdit = new QLineEdit(this);
    stunRow->addWidget(m_stunCheck);
    stunRow->addWidget(m_stunEdit);
    leftLayout->addLayout(stunRow);

    colsLayout->addLayout(leftLayout, 1);

    // ========================================================================
    // RIGHT COLUMN
    // ========================================================================
    auto* rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(4);

    // Call Recording
    auto* recRow = new QHBoxLayout();
    m_recordCheck = new QCheckBox("Call Recording", this);
    m_recordDirEdit = new QLineEdit("Recordings", this);
    auto* browseRec = new QPushButton("...", this); browseRec->setFixedWidth(24);
    recRow->addWidget(m_recordCheck);
    recRow->addWidget(m_recordDirEdit);
    recRow->addWidget(browseRec);
    rightLayout->addLayout(recRow);

    auto* recFmtRow = new QHBoxLayout();
    recFmtRow->addSpacing(20);
    m_mp3Radio = new QRadioButton("MP3", this); m_mp3Radio->setChecked(true);
    m_wavRadio = new QRadioButton("WAV", this);
    recFmtRow->addWidget(m_mp3Radio);
    recFmtRow->addWidget(m_wavRadio);
    recFmtRow->addStretch();
    rightLayout->addLayout(recFmtRow);

    // DTMF Method
    auto addComboRow = [this, &rightLayout](const QString& label, QComboBox*& combo, const QStringList& items) {
        auto* row = new QHBoxLayout();
        auto* lbl = new QLabel(label, this);
        lbl->setFixedWidth(100);
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        combo = new QComboBox(this);
        combo->addItems(items);
        row->addWidget(lbl);
        row->addWidget(combo);
        rightLayout->addLayout(row);
    };

    addComboRow("DTMF Method", m_dtmfCombo, {"Auto", "In-band", "RFC 2833", "SIP INFO"});

    // Auto Answer
    auto* ansRow = new QHBoxLayout();
    auto* ansLbl = new QLabel("Auto Answer", this);
    ansLbl->setFixedWidth(100);
    ansLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_autoAnswerCombo = new QComboBox(this);
    m_autoAnswerCombo->addItems({"Control Button", "Always", "Disabled"});
    m_autoAnswerSecSpin = new QSpinBox(this);
    m_autoAnswerSecSpin->setRange(0, 60);
    m_autoAnswerSecSpin->setValue(0);
    ansRow->addWidget(ansLbl);
    ansRow->addWidget(m_autoAnswerCombo);
    ansRow->addWidget(m_autoAnswerSecSpin);
    ansRow->addWidget(new QLabel("sec", this));
    rightLayout->addLayout(ansRow);

    addComboRow("Deny Incoming", m_denyIncomingCombo, {"Control Button", "Busy Here", "Decline"});

    auto* dirRow = new QHBoxLayout();
    auto* dirLbl = new QLabel("Directory of Users", this);
    dirLbl->setFixedWidth(100);
    dirLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_directoryEdit = new QLineEdit(this);
    dirRow->addWidget(dirLbl);
    dirRow->addWidget(m_directoryEdit);
    rightLayout->addLayout(dirRow);

    addComboRow("Default List Action", m_defaultActionCombo, {"Default", "Call", "Message"});

    // Feature Checkboxes
    auto addCb = [this, &rightLayout](const QString& label, QCheckBox*& ptr, bool checked = false) {
        ptr = new QCheckBox(label, this);
        ptr->setChecked(checked);
        rightLayout->addWidget(ptr);
    };

    addCb("Handle Media Buttons", m_mediaButtonsCheck, false);
    addCb("Sound Events", m_soundEventsCheck, true);
    addCb("Bring to Front on Incoming Call", m_bringToFrontCheck, true);
    addCb("Random Popup Position", m_randomPopupCheck, false);
    addCb("Call Waiting", m_callWaitingCheck, true);
    addCb("Enable Log File", m_logFileCheck, false);
    addCb("Enable Local Account", m_localAccountCheck, false);
    addCb("Send Crash Report", m_crashReportCheck, true);

    // Check for Updates
    auto* updRow = new QHBoxLayout();
    updRow->addWidget(new QLabel("Check for Updates", this));
    m_updatesCombo = new QComboBox(this);
    m_updatesCombo->addItems({"Daily", "Weekly", "Monthly", "Never"});
    m_updatesCombo->setCurrentIndex(1);
    updRow->addWidget(m_updatesCombo);
    rightLayout->addLayout(updRow);

    colsLayout->addLayout(rightLayout, 1);

    mainLayout->addLayout(colsLayout);
    mainLayout->addStretch();

    // Bottom Action Buttons
    auto* bottomRow = new QHBoxLayout();
    auto* saveBtn = new QPushButton("Save", this);
    saveBtn->setObjectName("saveBtn");
    saveBtn->setDefault(true);
    saveBtn->setFixedWidth(75);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setFixedWidth(75);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    bottomRow->addStretch();
    bottomRow->addWidget(saveBtn);
    bottomRow->addSpacing(6);
    bottomRow->addWidget(cancelBtn);
    mainLayout->addLayout(bottomRow);

    // Populate Hardware Devices if SipEngine available
    if (m_engine) {
        auto devs = m_engine->getAudioDevices();
        for (const auto& d : devs) {
            if (d.outputChannels > 0) m_speakerCombo->addItem(QString::fromStdString(d.name), d.id);
            if (d.inputChannels > 0) m_micCombo->addItem(QString::fromStdString(d.name), d.id);
        }
    }
}

void SettingsDialog::loadValues()
{
    QSettings s;
    m_singleCallCheck->setChecked(s.value("settings/single_call_mode", false).toBool());
    m_ringtoneEdit->setText(s.value("settings/ringtone", "").toString());
    m_ringVolumeSlider->setValue(s.value("settings/ring_volume", 80).toInt());
    m_vadCheck->setChecked(s.value("settings/vad", false).toBool());
    m_ecCheck->setChecked(s.value("settings/ec", true).toBool());
    m_forceCodecCheck->setChecked(s.value("settings/force_codec", false).toBool());
    m_stunCheck->setChecked(s.value("settings/stun_enabled", false).toBool());
    m_stunEdit->setText(s.value("settings/stun_server", "").toString());
    m_soundEventsCheck->setChecked(s.value("settings/sound_events", true).toBool());
    m_bringToFrontCheck->setChecked(s.value("settings/bring_to_front", true).toBool());
    m_callWaitingCheck->setChecked(s.value("settings/call_waiting", true).toBool());
}

void SettingsDialog::onSaveClicked()
{
    QSettings s;
    s.setValue("settings/single_call_mode", m_singleCallCheck->isChecked());
    s.setValue("settings/ringtone", m_ringtoneEdit->text());
    s.setValue("settings/ring_volume", m_ringVolumeSlider->value());
    s.setValue("settings/vad", m_vadCheck->isChecked());
    s.setValue("settings/ec", m_ecCheck->isChecked());
    s.setValue("settings/force_codec", m_forceCodecCheck->isChecked());
    s.setValue("settings/stun_enabled", m_stunCheck->isChecked());
    s.setValue("settings/stun_server", m_stunEdit->text());
    s.setValue("settings/sound_events", m_soundEventsCheck->isChecked());
    s.setValue("settings/bring_to_front", m_bringToFrontCheck->isChecked());
    s.setValue("settings/call_waiting", m_callWaitingCheck->isChecked());

    if (m_engine) {
        int spkId = m_speakerCombo->currentData().toInt();
        int micId = m_micCombo->currentData().toInt();
        m_engine->setAudioDevices(spkId, micId);
    }

    accept();
}
