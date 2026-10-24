#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QSystemTrayIcon>
#include <QSlider>
#include <QProgressBar>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include "../core/SipEngine.h"
#include "../core/SipBridge.h"
#include "../core/RingtoneManager.h"
#include "../data/Database.h"
#include "../data/CallsRepository.h"
#include "../data/ContactsRepository.h"

// Modern Elegant Keypad Button
class KeypadButton : public QPushButton
{
    Q_OBJECT
public:
    KeypadButton(const QString& mainText, const QString& subText = "", QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_main;
    QString m_sub;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onCallClicked();
    void onHangupClicked();
    void onHoldClicked();
    void onTransferClicked();
    void onVideoCallClicked();
    void onMessageClicked();
    void onVoicemailClicked();
    void onAutoConnectToggled(bool checked);
    void onAutoAnswerToggled(bool checked);
    void onRecordToggled(bool checked);
    void onConferenceClicked();
    void onMuteClicked();
    void onMicMuteClicked();
    void onSpeakerMuteClicked();
    void onSpeakerVolumeChanged(int value);
    void onMicVolumeChanged(int value);
    void onKeyClicked(const QString& key);
    void onBackspaceClicked();
    void onRedialClicked();
    void onClearClicked();
    void onHistoryDropdownClicked();
    void onNumberChanged(const QString& text);

    void onRegistrationChanged(int state, int code, const QString& reason);
    void onCallStateChanged(int callId, int state, const QString& remoteUri, int duration, const QString& reason, int statusCode = 0);
    void onIncomingCall(int callId, const QString& remoteUri);
    void onQualityTimerTick();
    void onCallTimerTick();

    // Menus & Dialogs
    void onOpenAccountDialog();
    void onOpenSettingsDialog();
    void onAlwaysOnTopToggled(bool checked);
    void onAboutClicked();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void switchActiveAccount(const QString& accountId);
    void onAccountBadgeClicked();

    // Logs Tab
    void refreshCallsHistory();
    void onCallsItemDoubleClicked(int row, int col);
    void onCallsContextMenu(const QPoint& pos);
    void onFilterLogs(const QString& text);
    void onHistoryFilterChanged(int index);
    void onDeleteCall();
    void onCopyCallNumber();
    void onClearAllLogs();
    void onShowCallDetails();

    // Contacts Tab
    void refreshContacts();
    void onContactsItemDoubleClicked(int row, int col);
    void onContactsContextMenu(const QPoint& pos);
    void onFilterContacts(const QString& text);
    void onAddContact();
    void onEditContact();
    void onDeleteContact();
    void onCopyContactNumber();

private:
    void setupUi();
    void setupTray();
    void applyModernTheme();
    void autoRegister();
    void updateCallBtnState();
    void updateCompleter();

    SipEngine m_sipEngine;
    SipBridge m_sipBridge;
    RingtoneManager m_ringtone;

    std::unique_ptr<Database> m_db;
    std::unique_ptr<CallsRepository> m_callsRepo;
    std::unique_ptr<ContactsRepository> m_contactsRepo;

    int m_activeCallId = -1;
    int m_currentCallRecordId = -1;
    bool m_isHold = false;
    bool m_isMicMuted = false;
    bool m_isSpkMuted = false;
    int m_callDurationSec = 0;
    QString m_lastDialed;
    QTimer* m_qualityTimer = nullptr;
    QTimer* m_callTimer = nullptr;

    // UI elements
    QTabWidget* m_mainTabs = nullptr;

    // Phone Tab
    QLineEdit* m_numberInput = nullptr;
    QPushButton* m_historyBtn = nullptr;
    QPushButton* m_backspaceBtn = nullptr;
    QPushButton* m_callBtn = nullptr;
    QPushButton* m_hangupBtn = nullptr;
    QPushButton* m_holdBtn = nullptr;
    QPushButton* m_transferBtn = nullptr;
    QPushButton* m_muteBtn = nullptr;
    QPushButton* m_videoBtn = nullptr;
    QPushButton* m_msgBtn = nullptr;

    // Feature Toggles Row (AC, AA, CONF, REC, VM)
    QPushButton* m_acBtn = nullptr;
    QPushButton* m_aaBtn = nullptr;
    QPushButton* m_confBtn = nullptr;
    QPushButton* m_recBtn = nullptr;
    QPushButton* m_vmBtn = nullptr;

    // Active Call Banner
    QWidget* m_callActiveBanner = nullptr;
    QLabel* m_callRemoteLabel = nullptr;
    QLabel* m_callTimerLabel = nullptr;

    // Audio & VU Meters
    QPushButton* m_spkBtn = nullptr;
    QSlider* m_spkSlider = nullptr;
    QProgressBar* m_speakerLevelBar = nullptr;
    QPushButton* m_micBtn = nullptr;
    QSlider* m_micSlider = nullptr;
    QProgressBar* m_micLevelBar = nullptr;
    QLabel* m_qualityBadge = nullptr;

    // Bottom Status Bar
    QLabel* m_statusIcon = nullptr;
    QLabel* m_statusDot = nullptr;
    QLabel* m_statusText = nullptr;
    QPushButton* m_accountBadge = nullptr;
    QMenu* m_mainMenu = nullptr;
    QMenu* m_accountsMenu = nullptr;

    void updateAccountBadge();
    void updateMainMenuAccounts();

    // Logs Tab
    QTableWidget* m_logsTable = nullptr;
    QLineEdit* m_logsSearch = nullptr;
    QComboBox* m_logsFilterCombo = nullptr;

    // Contacts Tab
    QTableWidget* m_contactsTable = nullptr;
    QLineEdit* m_contactsSearch = nullptr;

    QSystemTrayIcon* m_trayIcon = nullptr;
};
