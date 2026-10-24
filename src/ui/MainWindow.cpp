#include "MainWindow.h"
#include "AccountDialog.h"
#include "SettingsDialog.h"
#include "IncomingCallDialog.h"
#include "ContactDialog.h"
#include "TransferDialog.h"
#include "AboutDialog.h"
#include "MessageDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QApplication>
#include <QSettings>
#include <QCloseEvent>
#include <QToolButton>
#include <QInputDialog>
#include <QClipboard>
#include <QPainter>
#include <QPainterPath>
#include <QCompleter>
#include <QKeyEvent>
#include <QSet>
#include <QUuid>

// ============================================================================
// Modern Keypad Button (Clean, Refined, Premium Tactile)
// ============================================================================
KeypadButton::KeypadButton(const QString& mainText, const QString& subText, QWidget* parent)
    : QPushButton(parent)
    , m_main(mainText)
    , m_sub(subText)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::NoFocus);
    setMinimumHeight(34);
    setMaximumHeight(44);
    setCursor(Qt::PointingHandCursor);
}

void KeypadButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF r = rect();
    r.adjust(0.5, 0.5, -0.5, -0.5);

    QColor bgColor("#FFFFFF");
    QColor borderColor("#CBD5E1");
    if (isDown()) {
        bgColor = QColor("#E2E8F0");
        borderColor = QColor("#94A3B8");
    } else if (underMouse()) {
        bgColor = QColor("#F8FAFC");
        borderColor = QColor("#94A3B8");
    }

    QPainterPath path;
    path.addRoundedRect(r, 6.0, 6.0);
    p.fillPath(path, bgColor);
    p.setPen(QPen(borderColor, 1.0));
    p.drawPath(path);

    bool isSymbol = (m_main == "*" || m_main == "#");
    bool isRedial = (m_main == "R");
    bool isClear = (m_main == "C");

    QFont mainFont = font();
    mainFont.setBold(true);
    mainFont.setPixelSize(17);
    p.setFont(mainFont);

    if (isRedial) p.setPen(QColor("#2563EB"));
    else if (isClear) p.setPen(QColor("#DC2626"));
    else if (isSymbol) p.setPen(QColor("#475569"));
    else p.setPen(QColor("#0F172A"));

    if (m_sub.isEmpty()) {
        p.drawText(r, Qt::AlignCenter, m_main);
    } else {
        QFontMetrics fm(mainFont);
        int mainW = fm.horizontalAdvance(m_main);

        QFont subFont = mainFont;
        subFont.setBold(true);
        subFont.setPixelSize(8);
        QFontMetrics sfm(subFont);
        int subW = sfm.horizontalAdvance(m_sub);

        int totalW = mainW + subW + 4;
        qreal startX = r.center().x() - (totalW / 2.0);
        qreal baselineY = r.center().y() + (fm.ascent() / 2.0) - 1.0;

        p.drawText(QPointF(startX, baselineY), m_main);

        p.setFont(subFont);
        p.setPen(QColor("#94A3B8"));
        qreal subBaselineY = baselineY - (fm.ascent() - sfm.ascent()) / 2.0;
        p.drawText(QPointF(startX + mainW + 4.0, subBaselineY), m_sub);
    }
}

// ============================================================================
// MainWindow Implementation (Modern & Professional)
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_sipBridge(m_sipEngine)
{
    setupUi();
    setupTray();
    applyModernTheme();

    // Setup SQLite DB in AppData
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDir);
    QString dbPath = appDataDir + "/gesoftphone.db";

    try {
        m_db = std::make_unique<Database>(dbPath.toStdString());
        m_db->InitCalls();
        m_db->InitContacts();
        m_callsRepo = std::make_unique<CallsRepository>(m_db->Get());
        m_contactsRepo = std::make_unique<ContactsRepository>(m_db->Get());
        refreshCallsHistory();
        refreshContacts();
        updateCompleter();
    } catch (...) {}

    // Connect SIP Bridge Signals
    connect(&m_sipBridge, &SipBridge::registrationChanged, this, &MainWindow::onRegistrationChanged);
    connect(&m_sipBridge, &SipBridge::callStateChanged, this, &MainWindow::onCallStateChanged);
    connect(&m_sipBridge, &SipBridge::incomingCall, this, &MainWindow::onIncomingCall);

    // Quality timer
    m_qualityTimer = new QTimer(this);
    connect(m_qualityTimer, &QTimer::timeout, this, &MainWindow::onQualityTimerTick);

    // Call duration timer
    m_callTimer = new QTimer(this);
    connect(m_callTimer, &QTimer::timeout, this, &MainWindow::onCallTimerTick);

    // Initialize SIP Engine
    if (m_sipEngine.init()) {
        QSettings audioSettings;
        int spkDevId = audioSettings.value("audio/speaker_device", -1).toInt();
        int micDevId = audioSettings.value("audio/mic_device", -1).toInt();
        if (spkDevId != -1 || micDevId != -1) {
            m_sipEngine.setAudioDevices(spkDevId, micDevId);
        }
        int savedSpkVol = audioSettings.value("audio/spk_volume", 85).toInt();
        int savedMicVol = audioSettings.value("audio/mic_volume", 85).toInt();
        m_sipEngine.setSpeakerVolume(static_cast<float>(savedSpkVol) / 100.0f);
        m_sipEngine.setMicVolume(static_cast<float>(savedMicVol) / 100.0f);

        autoRegister();
    } else {
        m_statusDot->setStyleSheet("color: #EF4444; font-size: 13px;");
        m_statusText->setText("Engine Error");
    }
}

MainWindow::~MainWindow()
{
    if (m_qualityTimer) m_qualityTimer->stop();
    if (m_callTimer) m_callTimer->stop();
    m_sipEngine.destroy();
}

void MainWindow::applyModernTheme()
{
    setStyleSheet(R"(
        QMainWindow {
            background-color: #F8FAFC;
            font-family: "SF Pro Text", "Helvetica Neue", sans-serif;
            font-size: 12px;
        }
        /* Modern Softphone Tab Bar */
        QTabWidget::pane {
            border: 1px solid #CBD5E1;
            background: #FFFFFF;
            border-radius: 6px;
            top: -1px;
        }
        QTabBar::tab {
            background: #F1F5F9;
            border: 1px solid #CBD5E1;
            border-bottom: none;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            padding: 5px 14px;
            font-size: 11.5px;
            font-weight: 600;
            color: #64748B;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background: #FFFFFF;
            color: #0F172A;
            font-weight: 700;
            border-bottom: 2px solid #FFFFFF;
        }
        QTabBar::tab:hover:!selected {
            color: #1E293B;
            background: #E2E8F0;
        }
        /* Sliders */
        QSlider::groove:horizontal {
            height: 4px;
            background: #E2E8F0;
            border-radius: 2px;
        }
        QSlider::sub-page:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3B82F6, stop:1 #2563EB);
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: #FFFFFF;
            border: 1.5px solid #2563EB;
            width: 12px;
            margin-top: -4px;
            margin-bottom: -4px;
            border-radius: 6px;
        }
        QSlider::handle:horizontal:hover {
            background: #EFF6FF;
            border-color: #1D4ED8;
        }
        /* Hardware VU Meters */
        QProgressBar {
            background: #E2E8F0;
            border: 1px solid #CBD5E1;
            border-radius: 2px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #10B981, stop:0.75 #34D399, stop:0.9 #F59E0B, stop:1.0 #EF4444);
            border-radius: 1px;
        }
        /* Tables */
        QTableWidget {
            border: none;
            background: #FFFFFF;
            gridline-color: #F1F5F9;
            font-size: 11px;
            color: #1E293B;
            selection-background-color: #EFF6FF;
            selection-color: #1E40AF;
        }
        QHeaderView::section {
            background-color: #F8FAFC;
            border: none;
            border-bottom: 1px solid #E2E8F0;
            padding: 5px 8px;
            font-size: 10px;
            font-weight: 700;
            color: #64748B;
            text-transform: uppercase;
        }
        QTableWidget::item {
            padding: 4px;
        }
        /* Search inputs */
        QLineEdit#searchInput {
            background: #F1F5F9;
            border: 1px solid #E2E8F0;
            border-radius: 12px;
            padding: 4px 10px;
            font-size: 11px;
            color: #0F172A;
        }
        QLineEdit#searchInput:focus {
            background: #FFFFFF;
            border-color: #3B82F6;
        }
        QComboBox#filterCombo {
            background: #F1F5F9;
            border: 1px solid #E2E8F0;
            border-radius: 6px;
            padding: 2px 6px;
            font-size: 11px;
            color: #475569;
        }
        /* Sleek Modern Tooltips */
        QToolTip {
            background-color: #0F172A;
            color: #F8FAFC;
            border: 1px solid #334155;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 11px;
            font-weight: 500;
        }
        /* Minimalist Scrollbars (macOS Style) */
        QScrollBar:vertical {
            border: none;
            background: #F8FAFC;
            width: 6px;
            margin: 0px;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background: #CBD5E1;
            min-height: 24px;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical:hover {
            background: #94A3B8;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar:horizontal {
            border: none;
            background: #F8FAFC;
            height: 6px;
            margin: 0px;
            border-radius: 3px;
        }
        QScrollBar::handle:horizontal {
            background: #CBD5E1;
            min-width: 24px;
            border-radius: 3px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #94A3B8;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
    )");
}

void MainWindow::setupUi()
{
    setWindowIcon(QIcon(":/icons/res/gesoftphone.png"));
    setWindowTitle("GeSoftphone");
    setMinimumSize(305, 530);
    resize(320, 560);

    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(6, 6, 6, 4);
    rootLayout->setSpacing(4);

    // ========================================================================
    // MAIN TABS
    // ========================================================================
    m_mainTabs = new QTabWidget(this);

    // Corner Menu Button (▼)
    auto* menuBtn = new QToolButton(this);
    menuBtn->setText("▼");
    menuBtn->setToolTip("Menu");
    menuBtn->setPopupMode(QToolButton::InstantPopup);
    menuBtn->setCursor(Qt::PointingHandCursor);
    menuBtn->setStyleSheet(
        "QToolButton { border: 1px solid #CBD5E1; background: #F8FAFC; border-radius: 4px; "
        "padding: 3px 8px; font-size: 10px; color: #334155; margin: 2px 2px 2px 4px; }"
        "QToolButton:hover { background: #E2E8F0; color: #0F172A; border-color: #94A3B8; }"
        "QToolButton::menu-indicator { image: none; }"
    );

    m_mainMenu = new QMenu(menuBtn);
    m_accountsMenu = m_mainMenu->addMenu(QIcon(":/icons/res/contact.ico"), "Accounts");
    updateMainMenuAccounts();

    auto* manageAccountsAct = m_mainMenu->addAction(QIcon(":/icons/res/contact.ico"), "Manage Accounts...");
    auto* settingsAct = m_mainMenu->addAction("Settings...");
    m_mainMenu->addSeparator();
    auto* alwaysOnTopAct = m_mainMenu->addAction("Always on Top");
    alwaysOnTopAct->setCheckable(true);
    QSettings initSettings;
    bool savedTop = initSettings.value("always_on_top", false).toBool();
    alwaysOnTopAct->setChecked(savedTop);
    if (savedTop) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    }
    m_mainMenu->addSeparator();
    auto* aboutAct = m_mainMenu->addAction("About...");
    m_mainMenu->addSeparator();
    auto* exitAct = m_mainMenu->addAction("Quit");

    connect(manageAccountsAct, &QAction::triggered, this, &MainWindow::onOpenAccountDialog);
    connect(settingsAct, &QAction::triggered, this, &MainWindow::onOpenSettingsDialog);
    connect(alwaysOnTopAct, &QAction::toggled, this, &MainWindow::onAlwaysOnTopToggled);
    connect(aboutAct, &QAction::triggered, this, &MainWindow::onAboutClicked);
    connect(exitAct, &QAction::triggered, qApp, &QApplication::quit);

    menuBtn->setMenu(m_mainMenu);
    m_mainTabs->setCornerWidget(menuBtn, Qt::TopRightCorner);

    // ========================================================================
    // TAB 1: PHONE (DIALPAD & CONTROLS)
    // ========================================================================
    auto* phonePage = new QWidget();
    auto* phoneLayout = new QVBoxLayout(phonePage);
    phoneLayout->setContentsMargins(6, 6, 6, 6);
    phoneLayout->setSpacing(6);

    // Active Call Banner (Appears dynamically during calls)
    m_callActiveBanner = new QFrame(this);
    m_callActiveBanner->setStyleSheet("QFrame { background: #F0FDF4; border: 1.5px solid #BBF7D0; border-radius: 6px; padding: 4px; }");
    auto* bannerLayout = new QVBoxLayout(m_callActiveBanner);
    bannerLayout->setContentsMargins(6, 4, 6, 4);
    bannerLayout->setSpacing(4);

    auto* bannerTop = new QHBoxLayout();
    m_callRemoteLabel = new QLabel("Active Call", this);
    m_callRemoteLabel->setStyleSheet("font-size: 12.5px; font-weight: 700; color: #166534;");
    m_callTimerLabel = new QLabel("00:00", this);
    m_callTimerLabel->setStyleSheet("font-family: 'SF Mono', Menlo, Monaco, monospace; font-size: 13px; font-weight: 800; color: #15803D;");
    bannerTop->addWidget(m_callRemoteLabel);
    bannerTop->addStretch();
    bannerTop->addWidget(m_callTimerLabel);
    bannerLayout->addLayout(bannerTop);

    // In-Call Controls Row (Hold, Mute, Transfer, Hangup)
    auto* inCallBtnsRow = new QHBoxLayout();
    inCallBtnsRow->setSpacing(4);

    m_holdBtn = new QPushButton("⏸ Hold", this);
    m_holdBtn->setFixedHeight(28);
    m_holdBtn->setCursor(Qt::PointingHandCursor);
    m_holdBtn->setStyleSheet("QPushButton { background: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 4px; font-size: 10.5px; font-weight: 600; color: #334155; }"
                             "QPushButton:hover { background: #E2E8F0; }");
    connect(m_holdBtn, &QPushButton::clicked, this, &MainWindow::onHoldClicked);

    m_muteBtn = new QPushButton("🎙️ Mute", this);
    m_muteBtn->setFixedHeight(28);
    m_muteBtn->setCursor(Qt::PointingHandCursor);
    m_muteBtn->setStyleSheet("QPushButton { background: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 4px; font-size: 10.5px; font-weight: 600; color: #334155; }"
                             "QPushButton:hover { background: #E2E8F0; }");
    connect(m_muteBtn, &QPushButton::clicked, this, &MainWindow::onMuteClicked);

    m_transferBtn = new QPushButton("↪ Transfer", this);
    m_transferBtn->setFixedHeight(28);
    m_transferBtn->setCursor(Qt::PointingHandCursor);
    m_transferBtn->setStyleSheet("QPushButton { background: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 4px; font-size: 10.5px; font-weight: 600; color: #334155; }"
                                 "QPushButton:hover { background: #E2E8F0; }");
    connect(m_transferBtn, &QPushButton::clicked, this, &MainWindow::onTransferClicked);

    m_hangupBtn = new QPushButton("✕ Hang Up", this);
    m_hangupBtn->setFixedHeight(28);
    m_hangupBtn->setCursor(Qt::PointingHandCursor);
    m_hangupBtn->setStyleSheet("QPushButton { background: #EF4444; color: #FFFFFF; border: none; border-radius: 4px; font-size: 11px; font-weight: 700; }"
                               "QPushButton:hover { background: #DC2626; }");
    connect(m_hangupBtn, &QPushButton::clicked, this, &MainWindow::onHangupClicked);

    inCallBtnsRow->addWidget(m_holdBtn);
    inCallBtnsRow->addWidget(m_muteBtn);
    inCallBtnsRow->addWidget(m_transferBtn);
    inCallBtnsRow->addWidget(m_hangupBtn);
    bannerLayout->addLayout(inCallBtnsRow);

    m_callActiveBanner->setVisible(false);
    phoneLayout->addWidget(m_callActiveBanner);

    // Number Input Box with Backspace & History Dropdown
    auto* inputCard = new QFrame(this);
    inputCard->setStyleSheet("QFrame { background: #FFFFFF; border: 1.5px solid #CBD5E1; border-radius: 6px; }");
    auto* inputCardLayout = new QHBoxLayout(inputCard);
    inputCardLayout->setContentsMargins(8, 2, 4, 2);
    inputCardLayout->setSpacing(2);

    m_numberInput = new QLineEdit(this);
    m_numberInput->setPlaceholderText("Enter number or SIP URI...");
    m_numberInput->setStyleSheet("QLineEdit { border: none; background: transparent; font-size: 16px; font-weight: 600; color: #0F172A; }");
    connect(m_numberInput, &QLineEdit::textChanged, this, &MainWindow::onNumberChanged);
    connect(m_numberInput, &QLineEdit::returnPressed, this, &MainWindow::onCallClicked);

    m_historyBtn = new QPushButton("▾", this);
    m_historyBtn->setFixedSize(22, 26);
    m_historyBtn->setToolTip("Recent Dialed Numbers");
    m_historyBtn->setCursor(Qt::PointingHandCursor);
    m_historyBtn->setStyleSheet("QPushButton { border: none; background: transparent; font-size: 12px; color: #94A3B8; border-radius: 3px; }"
                                "QPushButton:hover { background: #F1F5F9; color: #0F172A; }");
    connect(m_historyBtn, &QPushButton::clicked, this, &MainWindow::onHistoryDropdownClicked);

    m_backspaceBtn = new QPushButton("⌫", this);
    m_backspaceBtn->setFixedSize(26, 26);
    m_backspaceBtn->setCursor(Qt::PointingHandCursor);
    m_backspaceBtn->setToolTip("Backspace");
    m_backspaceBtn->setStyleSheet("QPushButton { border: none; background: transparent; font-size: 14px; color: #94A3B8; border-radius: 3px; }"
                                  "QPushButton:hover { background: #F1F5F9; color: #EF4444; }");
    connect(m_backspaceBtn, &QPushButton::clicked, this, &MainWindow::onBackspaceClicked);

    inputCardLayout->addWidget(m_numberInput, 1);
    inputCardLayout->addWidget(m_historyBtn);
    inputCardLayout->addWidget(m_backspaceBtn);
    phoneLayout->addWidget(inputCard);

    // Keypad Grid (5 rows x 3 columns)
    auto* keypadGrid = new QGridLayout();
    keypadGrid->setSpacing(4);
    keypadGrid->setContentsMargins(0, 1, 0, 1);

    struct KeyDef { QString main; QString sub; };
    const KeyDef kRows[5][3] = {
        { {"1", ""},      {"2", "ABC"},   {"3", "DEF"} },
        { {"4", "GHI"},    {"5", "JKL"},   {"6", "MNO"} },
        { {"7", "PQRS"},   {"8", "TUV"},   {"9", "WXYZ"} },
        { {"*", ""},      {"0", ""},      {"#", ""} },
        { {"R", ""},      {"+", ""},      {"C", ""} }
    };

    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 3; ++c) {
            QString main = kRows[r][c].main;
            QString sub = kRows[r][c].sub;
            auto* kBtn = new KeypadButton(main, sub, this);
            if (main == "R") {
                kBtn->setToolTip("Redial");
                connect(kBtn, &QPushButton::clicked, this, &MainWindow::onRedialClicked);
            } else if (main == "C") {
                kBtn->setToolTip("Clear");
                connect(kBtn, &QPushButton::clicked, this, &MainWindow::onClearClicked);
            } else if (main == "+") {
                kBtn->setToolTip("Insert +");
                connect(kBtn, &QPushButton::clicked, this, [this]() {
                    m_numberInput->setText(m_numberInput->text() + "+");
                });
            } else {
                connect(kBtn, &QPushButton::clicked, this, [this, main]() { onKeyClicked(main); });
            }
            keypadGrid->addWidget(kBtn, r, c);
        }
    }
    phoneLayout->addLayout(keypadGrid);

    // Action Row below Keypad: [ 📹 Video ] [ Call ] [ 💬 Message ]
    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(5);

    m_videoBtn = new QPushButton(this);
    m_videoBtn->setIcon(QIcon(":/icons/res/button_video.ico"));
    m_videoBtn->setIconSize(QSize(20, 20));
    m_videoBtn->setToolTip("Video Call");
    m_videoBtn->setFixedSize(42, 36);
    m_videoBtn->setCursor(Qt::PointingHandCursor);
    m_videoBtn->setStyleSheet(
        "QPushButton { background: #FFFFFF; border: 1.2px solid #CBD5E1; border-radius: 6px; }"
        "QPushButton:hover { background: #F1F5F9; border-color: #94A3B8; }"
    );
    connect(m_videoBtn, &QPushButton::clicked, this, &MainWindow::onVideoCallClicked);

    m_callBtn = new QPushButton("Call", this);
    m_callBtn->setFixedHeight(36);
    m_callBtn->setCursor(Qt::PointingHandCursor);
    m_callBtn->setStyleSheet(
        "QPushButton { background: #10B981; color: #FFFFFF; font-size: 14px; font-weight: 700; border: none; border-radius: 6px; }"
        "QPushButton:hover { background: #059669; }"
        "QPushButton:pressed { background: #047857; }"
        "QPushButton:disabled { background: #E2E8F0; color: #94A3B8; }"
    );
    connect(m_callBtn, &QPushButton::clicked, this, &MainWindow::onCallClicked);

    m_msgBtn = new QPushButton(this);
    m_msgBtn->setIcon(QIcon(":/icons/res/button_message.ico"));
    m_msgBtn->setIconSize(QSize(20, 20));
    m_msgBtn->setToolTip("Instant Message");
    m_msgBtn->setFixedSize(42, 36);
    m_msgBtn->setCursor(Qt::PointingHandCursor);
    m_msgBtn->setStyleSheet(
        "QPushButton { background: #FFFFFF; border: 1.2px solid #CBD5E1; border-radius: 6px; }"
        "QPushButton:hover { background: #F1F5F9; border-color: #94A3B8; }"
    );
    connect(m_msgBtn, &QPushButton::clicked, this, &MainWindow::onMessageClicked);

    actionRow->addWidget(m_videoBtn);
    actionRow->addWidget(m_callBtn, 1);
    actionRow->addWidget(m_msgBtn);
    phoneLayout->addLayout(actionRow);

    // Audio & Levels Card (Speaker & Mic with mute icon buttons)
    auto* audioCard = new QFrame(this);
    audioCard->setStyleSheet("QFrame { background: #F8FAFC; border: 1px solid #CBD5E1; border-radius: 6px; }");
    auto* audioCardLayout = new QVBoxLayout(audioCard);
    audioCardLayout->setContentsMargins(6, 4, 6, 4);
    audioCardLayout->setSpacing(3);

    // Speaker row
    auto* spkRow = new QHBoxLayout();
    spkRow->setSpacing(6);
    m_spkBtn = new QPushButton(this);
    m_spkBtn->setIcon(QIcon(":/icons/res/button_speaker.ico"));
    m_spkBtn->setIconSize(QSize(16, 16));
    m_spkBtn->setFixedSize(26, 22);
    m_spkBtn->setToolTip("Mute Speaker");
    m_spkBtn->setCursor(Qt::PointingHandCursor);
    m_spkBtn->setStyleSheet("QPushButton { border: 1px solid #CBD5E1; background: #FFFFFF; border-radius: 4px; } QPushButton:hover { background: #E2E8F0; }");
    connect(m_spkBtn, &QPushButton::clicked, this, &MainWindow::onSpeakerMuteClicked);

    QSettings audioSettings;
    int savedSpkVol = audioSettings.value("audio/spk_volume", 85).toInt();
    int savedMicVol = audioSettings.value("audio/mic_volume", 85).toInt();

    m_spkSlider = new QSlider(Qt::Horizontal, this);
    m_spkSlider->setRange(0, 100);
    m_spkSlider->setValue(savedSpkVol);
    connect(m_spkSlider, &QSlider::valueChanged, this, &MainWindow::onSpeakerVolumeChanged);

    const QString ledMeterStyle =
        "QProgressBar { background: #E2E8F0; border: 1px solid #CBD5E1; border-radius: 2px; } "
        "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0.00 #10B981, stop:0.75 #34D399, stop:0.90 #F59E0B, stop:1.00 #EF4444); "
        "border-radius: 1px; }";

    m_speakerLevelBar = new QProgressBar(this);
    m_speakerLevelBar->setRange(0, 255);
    m_speakerLevelBar->setValue(0);
    m_speakerLevelBar->setTextVisible(false);
    m_speakerLevelBar->setFixedSize(36, 6);
    m_speakerLevelBar->setStyleSheet(ledMeterStyle);

    spkRow->addWidget(m_spkBtn);
    spkRow->addWidget(m_spkSlider);
    spkRow->addWidget(m_speakerLevelBar);
    audioCardLayout->addLayout(spkRow);

    // Mic row
    auto* micRow = new QHBoxLayout();
    micRow->setSpacing(6);
    m_micBtn = new QPushButton(this);
    m_micBtn->setIcon(QIcon(":/icons/res/button_mutein.ico"));
    m_micBtn->setIconSize(QSize(16, 16));
    m_micBtn->setFixedSize(26, 22);
    m_micBtn->setToolTip("Mute Microphone");
    m_micBtn->setCursor(Qt::PointingHandCursor);
    m_micBtn->setStyleSheet("QPushButton { border: 1px solid #CBD5E1; background: #FFFFFF; border-radius: 4px; } QPushButton:hover { background: #E2E8F0; }");
    connect(m_micBtn, &QPushButton::clicked, this, &MainWindow::onMicMuteClicked);

    m_micSlider = new QSlider(Qt::Horizontal, this);
    m_micSlider->setRange(0, 100);
    m_micSlider->setValue(savedMicVol);
    connect(m_micSlider, &QSlider::valueChanged, this, &MainWindow::onMicVolumeChanged);

    m_micLevelBar = new QProgressBar(this);
    m_micLevelBar->setRange(0, 255);
    m_micLevelBar->setValue(0);
    m_micLevelBar->setTextVisible(false);
    m_micLevelBar->setFixedSize(36, 6);
    m_micLevelBar->setStyleSheet(ledMeterStyle);

    micRow->addWidget(m_micBtn);
    micRow->addWidget(m_micSlider);
    micRow->addWidget(m_micLevelBar);
    audioCardLayout->addLayout(micRow);

    phoneLayout->addWidget(audioCard);

    // Feature Toggles Row (AC, AA, CONF, REC, VM)
    auto* featRow = new QHBoxLayout();
    featRow->setSpacing(5);

    auto createFeatBtn = [this](const QString& text, const QString& tooltip, bool checkable = false) {
        auto* b = new QPushButton(text, this);
        b->setFixedHeight(24);
        b->setCheckable(checkable);
        b->setToolTip(tooltip);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            "QPushButton { background: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 4px; font-size: 10px; font-weight: 700; color: #475569; padding: 2px 6px; }"
            "QPushButton:hover { background: #F1F5F9; border-color: #94A3B8; color: #0F172A; }"
            "QPushButton:checked { background: #EFF6FF; border-color: #3B82F6; color: #1E40AF; }"
        );
        return b;
    };

    m_acBtn = createFeatBtn("AC", "Auto Connect", true);
    connect(m_acBtn, &QPushButton::toggled, this, &MainWindow::onAutoConnectToggled);

    m_aaBtn = createFeatBtn("AA", "Auto Answer", true);
    connect(m_aaBtn, &QPushButton::toggled, this, &MainWindow::onAutoAnswerToggled);

    m_confBtn = createFeatBtn("CONF", "Conference Call", false);
    connect(m_confBtn, &QPushButton::clicked, this, &MainWindow::onConferenceClicked);

    m_recBtn = createFeatBtn("REC", "Record Call", true);
    connect(m_recBtn, &QPushButton::toggled, this, &MainWindow::onRecordToggled);

    m_vmBtn = createFeatBtn("", "Voicemail", false);
    m_vmBtn->setIcon(QIcon(":/icons/res/voicemail.bmp"));
    m_vmBtn->setIconSize(QSize(18, 14));
    connect(m_vmBtn, &QPushButton::clicked, this, &MainWindow::onVoicemailClicked);

    featRow->addWidget(m_acBtn);
    featRow->addWidget(m_aaBtn);
    featRow->addWidget(m_confBtn);
    featRow->addWidget(m_recBtn);
    featRow->addWidget(m_vmBtn);
    phoneLayout->addLayout(featRow);

    // Call Quality Telemetry
    m_qualityBadge = new QLabel("Ready  •  Opus HD 48 kHz", this);
    m_qualityBadge->setStyleSheet("font-size: 9.5px; font-weight: 500; color: #94A3B8;");
    m_qualityBadge->setAlignment(Qt::AlignCenter);
    phoneLayout->addWidget(m_qualityBadge);

    m_mainTabs->addTab(phonePage, "Phone");

    // ========================================================================
    // TAB 2: CALL LOGS (Calls History)
    // ========================================================================
    auto* logsPage = new QWidget();
    auto* logsLayout = new QVBoxLayout(logsPage);
    logsLayout->setContentsMargins(4, 4, 4, 4);
    logsLayout->setSpacing(4);

    m_logsTable = new QTableWidget(this);
    m_logsTable->setColumnCount(5);
    m_logsTable->setHorizontalHeaderLabels({"Name", "Number", "Time", "Duration", "Status"});
    m_logsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_logsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_logsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_logsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_logsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_logsTable->verticalHeader()->setVisible(false);
    m_logsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_logsTable->setShowGrid(false);
    m_logsTable->setAlternatingRowColors(true);
    m_logsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_logsTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onCallsItemDoubleClicked);
    connect(m_logsTable, &QTableWidget::customContextMenuRequested, this, &MainWindow::onCallsContextMenu);
    logsLayout->addWidget(m_logsTable);

    // Bottom Search & Filter Bar
    auto* logsFooter = new QHBoxLayout();
    logsFooter->setSpacing(4);

    auto* logsSearchIcon = new QLabel(this);
    logsSearchIcon->setPixmap(QIcon(":/icons/res/button_search.ico").pixmap(16, 16));

    m_logsSearch = new QLineEdit(this);
    m_logsSearch->setObjectName("searchInput");
    m_logsSearch->setPlaceholderText("Search history...");
    m_logsSearch->setClearButtonEnabled(true);
    connect(m_logsSearch, &QLineEdit::textChanged, this, &MainWindow::onFilterLogs);

    m_logsFilterCombo = new QComboBox(this);
    m_logsFilterCombo->setObjectName("filterCombo");
    m_logsFilterCombo->addItems({"All", "Incoming", "Outgoing", "Missed"});
    connect(m_logsFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onHistoryFilterChanged);

    logsFooter->addWidget(logsSearchIcon);
    logsFooter->addWidget(m_logsSearch, 1);
    logsFooter->addWidget(m_logsFilterCombo);
    logsLayout->addLayout(logsFooter);

    m_mainTabs->addTab(logsPage, "Logs");

    // ========================================================================
    // TAB 3: CONTACTS
    // ========================================================================
    auto* contactsPage = new QWidget();
    auto* contactsLayout = new QVBoxLayout(contactsPage);
    contactsLayout->setContentsMargins(4, 4, 4, 4);
    contactsLayout->setSpacing(4);

    m_contactsTable = new QTableWidget(this);
    m_contactsTable->setColumnCount(2);
    m_contactsTable->setHorizontalHeaderLabels({"Name", "Number"});
    m_contactsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_contactsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_contactsTable->verticalHeader()->setVisible(false);
    m_contactsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_contactsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_contactsTable->setShowGrid(false);
    m_contactsTable->setAlternatingRowColors(true);
    m_contactsTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_contactsTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onContactsItemDoubleClicked);
    connect(m_contactsTable, &QTableWidget::customContextMenuRequested, this, &MainWindow::onContactsContextMenu);
    contactsLayout->addWidget(m_contactsTable);

    // Bottom Search & Add Bar
    auto* contactsFooter = new QHBoxLayout();
    contactsFooter->setSpacing(4);

    auto* contactsSearchIcon = new QLabel(this);
    contactsSearchIcon->setPixmap(QIcon(":/icons/res/button_search.ico").pixmap(16, 16));

    m_contactsSearch = new QLineEdit(this);
    m_contactsSearch->setObjectName("searchInput");
    m_contactsSearch->setPlaceholderText("Search contacts...");
    m_contactsSearch->setClearButtonEnabled(true);
    connect(m_contactsSearch, &QLineEdit::textChanged, this, &MainWindow::onFilterContacts);

    auto* addContactBtn = new QPushButton("+ Add", this);
    addContactBtn->setCursor(Qt::PointingHandCursor);
    addContactBtn->setStyleSheet("QPushButton { background: #3B82F6; color: white; border: none; border-radius: 4px; font-size: 11px; font-weight: 700; padding: 3px 8px; }"
                                 "QPushButton:hover { background: #2563EB; }");
    connect(addContactBtn, &QPushButton::clicked, this, &MainWindow::onAddContact);

    contactsFooter->addWidget(contactsSearchIcon);
    contactsFooter->addWidget(m_contactsSearch, 1);
    contactsFooter->addWidget(addContactBtn);
    contactsLayout->addLayout(contactsFooter);

    m_mainTabs->addTab(contactsPage, "Contacts");

    rootLayout->addWidget(m_mainTabs);

    // ========================================================================
    // STATUS BAR
    // ========================================================================
    auto* statusBarFrame = new QFrame(this);
    statusBarFrame->setStyleSheet("QFrame { background: #F8FAFC; border-top: 1px solid #CBD5E1; }");
    auto* statusBarLayout = new QHBoxLayout(statusBarFrame);
    statusBarLayout->setContentsMargins(6, 3, 6, 3);
    statusBarLayout->setSpacing(5);

    m_statusIcon = new QLabel(this);
    m_statusIcon->setPixmap(QIcon(":/icons/res/status_offline.ico").pixmap(14, 14));

    m_statusDot = new QLabel("●", this);
    m_statusDot->setVisible(false);

    m_statusText = new QLabel("Offline", this);
    m_statusText->setStyleSheet("font-size: 11px; font-weight: 600; color: #334155;");

    m_accountBadge = new QPushButton("not configured ▾", this);
    m_accountBadge->setObjectName("accountBadge");
    m_accountBadge->setCursor(Qt::PointingHandCursor);
    m_accountBadge->setToolTip("Click to switch active account");
    m_accountBadge->setStyleSheet(
        "QPushButton#accountBadge { font-size: 11px; font-weight: 600; color: #1E40AF; "
        "background: #EFF6FF; border: 1px solid #BFDBFE; border-radius: 4px; padding: 2px 8px; } "
        "QPushButton#accountBadge:hover { background: #DBEAFE; border-color: #93C5FD; }"
    );
    connect(m_accountBadge, &QPushButton::clicked, this, &MainWindow::onAccountBadgeClicked);

    statusBarLayout->addWidget(m_statusIcon);
    statusBarLayout->addWidget(m_statusDot);
    statusBarLayout->addWidget(m_statusText);
    statusBarLayout->addStretch();
    statusBarLayout->addWidget(m_accountBadge);

    rootLayout->addWidget(statusBarFrame);
}

void MainWindow::updateCallBtnState()
{
    if (m_activeCallId != -1) {
        m_callBtn->setVisible(false);
        m_hangupBtn->parentWidget()->setVisible(true);
        m_callActiveBanner->setVisible(true);
    } else {
        m_callBtn->setVisible(true);
        m_hangupBtn->parentWidget()->setVisible(false);
        m_callActiveBanner->setVisible(false);
        QString num = m_numberInput->text().trimmed();
        m_callBtn->setEnabled(!num.isEmpty());
    }
}

void MainWindow::onNumberChanged(const QString& text)
{
    Q_UNUSED(text);
    updateCallBtnState();
}

void MainWindow::setupTray()
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/icons/res/gesoftphone.png"));
    m_trayIcon->setToolTip("GeSoftphone");

    auto* trayMenu = new QMenu(this);
    auto* showAction = new QAction("Show GeSoftphone", this);
    connect(showAction, &QAction::triggered, this, &MainWindow::showNormal);

    auto* settingsAction = new QAction("Settings...", this);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onOpenSettingsDialog);

    auto* quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    trayMenu->addAction(showAction);
    trayMenu->addAction(settingsAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    m_trayIcon->setContextMenu(trayMenu);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);
    m_trayIcon->show();
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (isVisible()) hide();
        else { showNormal(); activateWindow(); }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    bool runInTray = settings.value("run_in_tray", false).toBool();
    if (runInTray) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::autoRegister()
{
    SipAccountConfig cfg = AccountDialog::getActiveAccount();
    if (!cfg.username.empty() && !cfg.server.empty()) {
        if (m_statusIcon) m_statusIcon->setPixmap(QIcon(":/icons/res/status_yellow.ico").pixmap(14, 14));
        m_statusText->setText("Connecting...");
        updateAccountBadge();
        m_sipEngine.registerAccount(cfg);
    } else {
        if (m_statusIcon) m_statusIcon->setPixmap(QIcon(":/icons/res/status_offline.ico").pixmap(14, 14));
        m_statusText->setText("Offline");
        if (m_accountBadge) m_accountBadge->setText("not configured ▾");
    }
    updateMainMenuAccounts();
}

void MainWindow::onKeyClicked(const QString& key)
{
    m_numberInput->setText(m_numberInput->text() + key);
    if (m_activeCallId != -1) {
        m_sipEngine.sendDtmf(m_activeCallId, key.toStdString());
    }
}

void MainWindow::onBackspaceClicked()
{
    QString txt = m_numberInput->text();
    if (!txt.isEmpty()) {
        txt.chop(1);
        m_numberInput->setText(txt);
    }
}

void MainWindow::onRedialClicked()
{
    if (!m_lastDialed.isEmpty()) {
        m_numberInput->setText(m_lastDialed);
        onCallClicked();
    }
}

void MainWindow::onClearClicked()
{
    m_numberInput->clear();
}

void MainWindow::onHistoryDropdownClicked()
{
    if (!m_callsRepo) return;
    auto calls = m_callsRepo->GetAll(30);
    if (calls.empty()) return;

    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 6px; padding: 4px; font-size: 11.5px; }"
        "QMenu::item { padding: 4px 18px 4px 8px; border-radius: 4px; color: #0F172A; }"
        "QMenu::item:selected { background: #EFF6FF; color: #1E40AF; }"
    );

    QSet<QString> seen;
    for (const auto& c : calls) {
        QString num = QString::fromStdString(c->number).trimmed();
        if (!seen.contains(num) && !num.isEmpty()) {
            seen.insert(num);
            QString title = QString::fromStdString(c->name.empty() ? c->number : c->name);
            if (title != num) title = QString("%1  •  %2").arg(title, num);
            auto* act = menu.addAction(title);
            connect(act, &QAction::triggered, this, [this, num]() {
                m_numberInput->setText(num);
                m_numberInput->setFocus();
            });
        }
        if (seen.size() >= 12) break;
    }

    if (!menu.actions().isEmpty()) {
        menu.exec(m_historyBtn->mapToGlobal(QPoint(0, m_historyBtn->height() + 2)));
    }
}

void MainWindow::updateCompleter()
{
    QStringList candidates;
    if (m_callsRepo) {
        auto calls = m_callsRepo->GetAll(100);
        for (const auto& c : calls) {
            if (!c->number.empty()) candidates << QString::fromStdString(c->number);
            if (!c->name.empty() && c->name != c->number) candidates << QString::fromStdString(c->name);
        }
    }
    if (m_contactsRepo) {
        auto contacts = m_contactsRepo->GetAll();
        for (const auto& c : contacts) {
            if (!c->number.empty()) candidates << QString::fromStdString(c->number);
            if (!c->name.empty()) candidates << QString::fromStdString(c->name);
        }
    }
    candidates.removeDuplicates();
    auto* completer = new QCompleter(candidates, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    m_numberInput->setCompleter(completer);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        if (!m_numberInput->hasFocus()) {
            onKeyClicked(QString::number(key - Qt::Key_0));
            return;
        }
    } else if (key == Qt::Key_Asterisk) {
        if (!m_numberInput->hasFocus()) {
            onKeyClicked("*");
            return;
        }
    } else if (key == Qt::Key_NumberSign) {
        if (!m_numberInput->hasFocus()) {
            onKeyClicked("#");
            return;
        }
    } else if (key == Qt::Key_Plus) {
        if (!m_numberInput->hasFocus()) {
            onKeyClicked("+");
            return;
        }
    } else if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        onCallClicked();
        return;
    } else if (key == Qt::Key_Escape) {
        if (m_activeCallId != -1) {
            onHangupClicked();
        } else {
            onClearClicked();
        }
        return;
    } else if (key == Qt::Key_Backspace) {
        if (!m_numberInput->hasFocus()) {
            onBackspaceClicked();
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::onCallClicked()
{
    if (m_activeCallId != -1) {
        onHangupClicked();
        return;
    }

    QString number = m_numberInput->text().trimmed();
    if (number.isEmpty()) return;

    int callId = m_sipEngine.makeCall(number.toStdString(), false);
    if (callId != -1) {
        m_activeCallId = callId;
        m_lastDialed = number;
        m_isHold = false;
        m_callDurationSec = 0;
        m_holdBtn->setText("⏸ Hold");

        m_callRemoteLabel->setText(QString("Calling: %1").arg(number));
        m_callTimerLabel->setText("00:00");
        m_callTimer->start(1000);

        updateCallBtnState();

        m_statusDot->setStyleSheet("color: #3B82F6; font-size: 14px;");
        m_statusText->setText(QString("Calling %1...").arg(number));
        m_qualityTimer->start(400);

        if (m_callsRepo) {
            try {
                Call c;
                c.callId = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
                c.number = number.toStdString();
                c.name = number.toStdString();
                c.type = CALL_TYPE_OUTGOING;
                c.time = QDateTime::currentSecsSinceEpoch();
                c.duration = 0;
                c.info = "Calling...";
                m_currentCallRecordId = m_callsRepo->Add(c);
                refreshCallsHistory();
            } catch (...) {}
        }
    } else {
        QMessageBox::warning(this, tr("Call Failed"), tr("Cannot initiate call to \"%1\". Please verify the destination number and server connection.").arg(number));
    }
}

void MainWindow::onVideoCallClicked()
{
    QString number = m_numberInput->text().trimmed();
    if (number.isEmpty()) return;
    int callId = m_sipEngine.makeCall(number.toStdString(), true);
    if (callId != -1) {
        m_activeCallId = callId;
        m_lastDialed = number;
        m_callDurationSec = 0;
        m_callTimer->start(1000);
        updateCallBtnState();
    }
}

void MainWindow::onHangupClicked()
{
    if (m_activeCallId != -1) {
        m_sipEngine.hangupCall(m_activeCallId);
        m_activeCallId = -1;
    }
    m_qualityTimer->stop();
    m_callTimer->stop();
    m_speakerLevelBar->setValue(0);
    m_micLevelBar->setValue(0);
    m_qualityBadge->setText("Call Ended");
    updateCallBtnState();

    m_statusDot->setStyleSheet("color: #10B981; font-size: 14px;");
    m_statusText->setText("Online • Ready");
}

void MainWindow::onHoldClicked()
{
    if (m_activeCallId == -1) return;
    m_isHold = !m_isHold;
    if (m_isHold) {
        m_sipEngine.holdCall(m_activeCallId);
        m_holdBtn->setText("▶ Resume");
        m_holdBtn->setStyleSheet("QPushButton { background: #FEF3C7; border: 1px solid #F59E0B; border-radius: 6px; font-size: 11px; font-weight: 700; color: #B45309; }");
        m_statusText->setText("Call on Hold");
    } else {
        m_sipEngine.unholdCall(m_activeCallId);
        m_holdBtn->setText("⏸ Hold");
        m_holdBtn->setStyleSheet("QPushButton { background: #F1F5F9; border: 1px solid #CBD5E1; border-radius: 6px; font-size: 11px; font-weight: 600; color: #334155; }");
        m_statusText->setText("Call Active");
    }
}

void MainWindow::onTransferClicked()
{
    if (m_activeCallId == -1) return;
    QString activeCaller = m_callRemoteLabel ? m_callRemoteLabel->text() : "";
    TransferDialog dlg(activeCaller, this);
    if (dlg.exec() == QDialog::Accepted) {
        QString target = dlg.getTargetExtension();
        if (!target.isEmpty()) {
            m_sipEngine.transferCall(m_activeCallId, target.toStdString());
            m_statusText->setText(QString("Transferring to %1...").arg(target));
        }
    }
}

void MainWindow::onMuteClicked()
{
    m_isMicMuted = !m_isMicMuted;
    m_sipEngine.muteMicrophone(m_isMicMuted);
    if (m_isMicMuted) {
        m_muteBtn->setText("🔇 Muted");
        m_muteBtn->setStyleSheet("QPushButton { background: #FEE2E2; border: 1px solid #EF4444; border-radius: 6px; font-size: 11px; font-weight: 700; color: #B91C1C; }");
    } else {
        m_muteBtn->setText("🎙️ Mute");
        m_muteBtn->setStyleSheet("QPushButton { background: #F1F5F9; border: 1px solid #CBD5E1; border-radius: 6px; font-size: 11px; font-weight: 600; color: #334155; }");
    }
}

void MainWindow::onSpeakerMuteClicked()
{
    m_isSpkMuted = !m_isSpkMuted;
    if (m_isSpkMuted) {
        m_spkBtn->setIcon(QIcon(":/icons/res/button_mutedout.ico"));
        m_spkBtn->setToolTip("Unmute Speaker");
        m_spkSlider->setEnabled(false);
    } else {
        m_spkBtn->setIcon(QIcon(":/icons/res/button_speaker.ico"));
        m_spkBtn->setToolTip("Mute Speaker");
        m_spkSlider->setEnabled(true);
    }
}

void MainWindow::onMicMuteClicked()
{
    m_isMicMuted = !m_isMicMuted;
    m_sipEngine.muteMicrophone(m_isMicMuted);
    if (m_isMicMuted) {
        m_micBtn->setIcon(QIcon(":/icons/res/button_mutedin.ico"));
        m_micBtn->setToolTip("Unmute Microphone");
        m_micSlider->setEnabled(false);
    } else {
        m_micBtn->setIcon(QIcon(":/icons/res/button_mutein.ico"));
        m_micBtn->setToolTip("Mute Microphone");
        m_micSlider->setEnabled(true);
    }
}

void MainWindow::onSpeakerVolumeChanged(int value)
{
    float level = static_cast<float>(value) / 100.0f;
    m_sipEngine.setSpeakerVolume(level);
    QSettings s;
    s.setValue("audio/spk_volume", value);
}

void MainWindow::onMicVolumeChanged(int value)
{
    float level = static_cast<float>(value) / 100.0f;
    m_sipEngine.setMicVolume(level);
    QSettings s;
    s.setValue("audio/mic_volume", value);
}

void MainWindow::onRegistrationChanged(int state, int code, const QString& reason)
{
    if (state == REG_STATE_REGISTERED) {
        if (m_statusIcon) m_statusIcon->setPixmap(QIcon(":/icons/res/status_online.ico").pixmap(14, 14));
        m_statusText->setText("Online");
        m_statusText->setToolTip("SIP Account Registered & Active");
    } else if (state == REG_STATE_PROGRESS) {
        if (m_statusIcon) m_statusIcon->setPixmap(QIcon(":/icons/res/status_yellow.ico").pixmap(14, 14));
        m_statusText->setText("Connecting...");
        m_statusText->setToolTip("Connecting to SIP Server...");
    } else {
        if (m_statusIcon) m_statusIcon->setPixmap(QIcon(":/icons/res/status_offline.ico").pixmap(14, 14));
        if (code > 0) {
            m_statusText->setText(QString("Offline (%1)").arg(code));
            m_statusText->setToolTip(QString("Registration Failed (%1: %2)").arg(code).arg(reason));
        } else {
            m_statusText->setText("Offline");
            m_statusText->setToolTip("SIP Account Offline");
        }
    }
}

void MainWindow::onCallStateChanged(int callId, int state, const QString& remoteUri, int duration, const QString& reason, int statusCode)
{
    if (state == CALL_STATE_CALLING) {
        m_statusDot->setStyleSheet("color: #3B82F6; font-size: 14px;");
        m_statusText->setText(QString("Calling %1...").arg(remoteUri));
        m_callRemoteLabel->setText(QString("Calling: %1").arg(remoteUri));
    } else if (state == CALL_STATE_EARLY) {
        // 180 Ringing / 183 Session Progress from PBX
        m_statusDot->setStyleSheet("color: #F59E0B; font-size: 14px;");
        m_statusText->setText(QString("Ringing %1...").arg(remoteUri));
        m_callRemoteLabel->setText(QString("Ringing: %1").arg(remoteUri));
    } else if (state == CALL_STATE_CONNECTING) {
        m_statusDot->setStyleSheet("color: #3B82F6; font-size: 14px;");
        m_statusText->setText("Connecting...");
        m_callRemoteLabel->setText(QString("Connecting: %1").arg(remoteUri));
    } else if (state == CALL_STATE_CONFIRMED) {
        m_callDurationSec = duration;
        m_callRemoteLabel->setText(QString("Talking: %1").arg(remoteUri));
        m_statusDot->setStyleSheet("color: #10B981; font-size: 14px;");
        if (m_statusIcon) m_statusIcon->setPixmap(QIcon(":/icons/res/status_online.ico").pixmap(14, 14));
        m_statusText->setText(QString("In Call with %1").arg(remoteUri));
        updateCallBtnState();
        if (!m_qualityTimer->isActive()) m_qualityTimer->start(400);
        if (!m_callTimer->isActive()) m_callTimer->start(1000);

        if (m_callsRepo && m_currentCallRecordId > 0) {
            try {
                auto callRecord = m_callsRepo->GetById(m_currentCallRecordId);
                if (callRecord) {
                    callRecord->info = "200 OK";
                    m_callsRepo->Update(*callRecord);
                }
            } catch (...) {}
        }
    } else if (state == CALL_STATE_DISCONNECTED) {
        m_ringtone.stopIncomingRing();
        m_ringtone.playHangupTone();
        if (m_activeCallId == callId || m_activeCallId == -1) {
            m_activeCallId = -1;
            m_qualityTimer->stop();
            m_callTimer->stop();
            m_speakerLevelBar->setValue(0);
            m_micLevelBar->setValue(0);
            updateCallBtnState();
            if (m_statusIcon) m_statusIcon->setPixmap(QIcon(":/icons/res/status_online.ico").pixmap(14, 14));

            QString reasonText = reason.trimmed();
            QString statusMsg;
            if (statusCode > 0) {
                statusMsg = reasonText.isEmpty() ? QString("SIP %1").arg(statusCode) : QString("%1 %2").arg(statusCode).arg(reasonText);
            } else if (!reasonText.isEmpty()) {
                statusMsg = reasonText;
            } else {
                statusMsg = "Call Ended";
            }

            m_statusDot->setStyleSheet("color: #10B981; font-size: 14px;");
            m_statusText->setText(QString("Ended: %1").arg(statusMsg));
            m_qualityBadge->setText(QString("Ended: %1").arg(statusMsg));
            if (m_callRemoteLabel) {
                m_callRemoteLabel->setText(QString("Call Ended • %1").arg(statusMsg));
            }

            if (m_callsRepo && m_currentCallRecordId > 0) {
                try {
                    auto callRecord = m_callsRepo->GetById(m_currentCallRecordId);
                    if (callRecord) {
                        callRecord->duration = m_callDurationSec > 0 ? m_callDurationSec : duration;
                        callRecord->info = (callRecord->duration > 0 ? "200 OK" : statusMsg).toStdString();
                        m_callsRepo->Update(*callRecord);
                        refreshCallsHistory();
                    }
                } catch (...) {}
                m_currentCallRecordId = -1;
            }
        }
    }
}

void MainWindow::onCallTimerTick()
{
    m_callDurationSec++;
    int h = m_callDurationSec / 3600;
    int m = (m_callDurationSec % 3600) / 60;
    int s = m_callDurationSec % 60;
    if (h > 0) {
        m_callTimerLabel->setText(QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
    } else {
        m_callTimerLabel->setText(QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
    }
}

void MainWindow::onQualityTimerTick()
{
    if (m_activeCallId == -1) {
        m_qualityTimer->stop();
        return;
    }

    float loss = 0.0f;
    unsigned jitter = 0;
    unsigned rtt = 0;
    if (m_sipEngine.getCallQualityStats(m_activeCallId, loss, jitter, rtt)) {
        m_qualityBadge->setText(QString("📶 Opus HD • Loss: %1% | Jitter: %2ms | RTT: %3ms")
            .arg(loss, 0, 'f', 1).arg(jitter).arg(rtt));
    }

    unsigned rx = 0;
    unsigned tx = 0;
    if (m_sipEngine.getAudioLevels(m_activeCallId, rx, tx)) {
        if (!m_isSpkMuted) m_speakerLevelBar->setValue(rx);
        if (!m_isMicMuted) m_micLevelBar->setValue(tx);
    }
}

void MainWindow::onIncomingCall(int callId, const QString& remoteUri)
{
    if (m_trayIcon) {
        m_trayIcon->showMessage("Incoming Call", QString("Call from %1").arg(remoteUri), QSystemTrayIcon::Information, 5000);
    }
    showNormal();
    activateWindow();

    m_ringtone.startIncomingRing();

    IncomingCallDialog dlg(remoteUri, remoteUri, this);
    int res = dlg.exec();
    m_ringtone.stopIncomingRing();

    if (res == QDialog::Accepted && dlg.wasAnswered()) {
        m_activeCallId = callId;
        m_callDurationSec = 0;
        m_sipEngine.answerCall(callId);
        m_callRemoteLabel->setText(QString("Talking: %1").arg(remoteUri));
        m_callTimer->start(1000);
        updateCallBtnState();
        m_statusText->setText("Connected");
        m_qualityTimer->start(400);

        if (m_callsRepo) {
            try {
                Call c;
                c.callId = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
                c.number = remoteUri.toStdString();
                c.name = remoteUri.toStdString();
                c.type = CALL_TYPE_INCOMING;
                c.time = QDateTime::currentSecsSinceEpoch();
                c.info = "Talking...";
                m_currentCallRecordId = m_callsRepo->Add(c);
                refreshCallsHistory();
            } catch (...) {}
        }
    } else {
        m_ringtone.playHangupTone();
        m_sipEngine.hangupCall(callId);
        if (m_callsRepo) {
            try {
                Call c;
                c.callId = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
                c.number = remoteUri.toStdString();
                c.name = remoteUri.toStdString();
                c.type = CALL_TYPE_MISSED;
                c.time = QDateTime::currentSecsSinceEpoch();
                c.info = "Missed";
                m_callsRepo->Add(c);
                refreshCallsHistory();
            } catch (...) {}
        }
    }
}

void MainWindow::onMessageClicked()
{
    QString number = m_numberInput->text().trimmed();
    if (number.isEmpty()) {
        if (m_mainTabs->currentIndex() == 2 && m_contactsTable->currentRow() >= 0) {
            auto* it = m_contactsTable->item(m_contactsTable->currentRow(), 1);
            if (it) number = it->text();
        } else if (m_mainTabs->currentIndex() == 1 && m_logsTable->currentRow() >= 0) {
            auto* it = m_logsTable->item(m_logsTable->currentRow(), 1);
            if (it) number = it->text();
        }
    }
    if (number.isEmpty()) {
        bool ok = false;
        number = QInputDialog::getText(this, tr("Instant Message"), tr("Enter recipient extension or SIP URI:"), QLineEdit::Normal, "", &ok);
        if (!ok || number.trimmed().isEmpty()) return;
        number = number.trimmed();
    }
    auto* dlg = new MessageDialog(number, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void MainWindow::onVoicemailClicked()
{
    QSettings s;
    QString vm = s.value("sip/voicemail", "").toString();
    if (vm.isEmpty()) vm = "*97";
    m_numberInput->setText(vm);
    onCallClicked();
}

void MainWindow::onAutoConnectToggled(bool checked)
{
    QSettings s;
    s.setValue("sip/auto_connect", checked);
}

void MainWindow::onAutoAnswerToggled(bool checked)
{
    QSettings s;
    s.setValue("sip/auto_answer", checked);
}

void MainWindow::onRecordToggled(bool checked)
{
    if (checked) {
        m_recBtn->setStyleSheet("QPushButton { background: #FEE2E2; border: 1.5px solid #EF4444; border-radius: 4px; font-size: 10px; font-weight: 800; color: #DC2626; }");
    } else {
        m_recBtn->setStyleSheet("QPushButton { background: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 4px; font-size: 10px; font-weight: 700; color: #475569; }");
    }
}

void MainWindow::onConferenceClicked()
{
    QMessageBox msg(this);
    msg.setWindowTitle(tr("3-Way Conference"));
    msg.setIcon(QMessageBox::Information);
    msg.setText(tr("<h3>3-Way Voice Conference</h3>"
                   "<p>To establish a multi-party conference:</p>"
                   "<ol>"
                   "<li>Initiate or accept the first call.</li>"
                   "<li>Click <b>Hold</b> (⏸) to place participant on hold.</li>"
                   "<li>Dial the second participant and connect.</li>"
                   "<li>Click <b>CONF</b> to bridge all participants into a single high-definition conference room.</li>"
                   "</ol>"));
    msg.setStyleSheet("QMessageBox { background: #F8FAFC; font-family: \"SF Pro Text\", sans-serif; } QPushButton { background: #3B82F6; color: white; border-radius: 5px; padding: 5px 15px; font-weight: 600; }");
    msg.exec();
}

void MainWindow::onAboutClicked()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::onOpenAccountDialog()
{
    AccountDialog dlg(&m_sipEngine, this);
    if (dlg.exec() == QDialog::Accepted) {
        autoRegister();
    }
}

void MainWindow::switchActiveAccount(const QString& accountId)
{
    if (accountId.isEmpty()) return;
    if (m_activeCallId != -1) {
        QMessageBox::warning(this, "Active Call", "Please end the active call before switching accounts.");
        return;
    }
    AccountDialog::setActiveAccount(accountId.toStdString());
    autoRegister();
}

void MainWindow::updateAccountBadge()
{
    if (!m_accountBadge) return;
    SipAccountConfig cfg = AccountDialog::getActiveAccount();
    if (!cfg.username.empty()) {
        QString label = QString::fromStdString(cfg.label.empty() ? cfg.username : cfg.label);
        m_accountBadge->setText(QString("%1 ▾").arg(label));
    } else {
        m_accountBadge->setText("not configured ▾");
    }
}

void MainWindow::onAccountBadgeClicked()
{
    auto accounts = AccountDialog::loadAllAccounts();
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 6px; padding: 4px; font-size: 11.5px; }"
        "QMenu::item { padding: 4px 18px 4px 8px; border-radius: 4px; color: #0F172A; }"
        "QMenu::item:selected { background: #EFF6FF; color: #1E40AF; }"
    );

    for (const auto& acc : accounts) {
        QString title = QString::fromStdString(acc.label.empty() ? acc.username : acc.label);
        if (title.isEmpty()) title = "Account";
        if (acc.isActive) {
            title = "✓  " + title;
        } else {
            title = "    " + title;
        }
        auto* act = menu.addAction(title);
        QString accId = QString::fromStdString(acc.id);
        connect(act, &QAction::triggered, this, [this, accId]() {
            switchActiveAccount(accId);
        });
    }

    menu.addSeparator();
    auto* manageAct = menu.addAction(QIcon(":/icons/res/contact.ico"), "Manage Accounts...");
    connect(manageAct, &QAction::triggered, this, &MainWindow::onOpenAccountDialog);

    if (m_accountBadge) {
        QPoint p = m_accountBadge->mapToGlobal(QPoint(0, -menu.sizeHint().height() - 4));
        menu.exec(p);
    }
}

void MainWindow::updateMainMenuAccounts()
{
    if (!m_accountsMenu) return;
    m_accountsMenu->clear();

    auto accounts = AccountDialog::loadAllAccounts();
    for (const auto& acc : accounts) {
        QString title = QString::fromStdString(acc.label.empty() ? acc.username : acc.label);
        if (title.isEmpty()) title = "Account";
        auto* act = m_accountsMenu->addAction(title);
        act->setCheckable(true);
        act->setChecked(acc.isActive);
        QString accId = QString::fromStdString(acc.id);
        connect(act, &QAction::triggered, this, [this, accId]() {
            switchActiveAccount(accId);
        });
    }

    m_accountsMenu->addSeparator();
    auto* manageAct = m_accountsMenu->addAction("Manage Accounts...");
    connect(manageAct, &QAction::triggered, this, &MainWindow::onOpenAccountDialog);
}

void MainWindow::onOpenSettingsDialog()
{
    SettingsDialog dlg(&m_sipEngine, this);
    dlg.exec();
}

void MainWindow::onAlwaysOnTopToggled(bool checked)
{
    QSettings s;
    s.setValue("always_on_top", checked);
    Qt::WindowFlags flags = windowFlags();
    if (checked) flags |= Qt::WindowStaysOnTopHint;
    else flags &= ~Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    show();
}

// ============================================================================
// CALL LOGS TAB
// ============================================================================
void MainWindow::refreshCallsHistory()
{
    if (!m_callsRepo) return;
    m_logsTable->setRowCount(0);
    auto calls = m_callsRepo->GetAll(150);

    for (const auto& c : calls) {
        int r = m_logsTable->rowCount();
        m_logsTable->insertRow(r);

        QIcon typeIcon;
        if (c->type == CALL_TYPE_OUTGOING) {
            typeIcon = QIcon(":/icons/res/call_out.ico");
        } else if (c->type == CALL_TYPE_INCOMING) {
            typeIcon = QIcon(":/icons/res/call_in.ico");
        } else {
            typeIcon = QIcon(":/icons/res/call_miss.ico");
        }

        QString displayName = QString::fromStdString(c->name.empty() ? c->number : c->name);
        auto* nameItem = new QTableWidgetItem(typeIcon, displayName);
        nameItem->setFont(QFont(font().family(), 11, QFont::DemiBold));
        nameItem->setData(Qt::UserRole, c->type);

        auto* numItem = new QTableWidgetItem(QString::fromStdString(c->number));
        numItem->setFont(QFont(font().family(), 11));
        numItem->setData(Qt::UserRole, c->id);

        QDateTime dt = QDateTime::fromSecsSinceEpoch(c->time);
        auto* timeItem = new QTableWidgetItem(dt.toString("hh:mm:ss AP"));
        timeItem->setForeground(QColor("#64748B"));

        QString durStr;
        if (c->duration > 0) {
            int h = c->duration / 3600;
            int m = (c->duration % 3600) / 60;
            int s = c->duration % 60;
            if (h > 0) {
                durStr = QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
            } else {
                durStr = QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));
            }
        }
        auto* durItem = new QTableWidgetItem(durStr.isEmpty() ? "-" : durStr);
        durItem->setForeground(QColor("#64748B"));
        durItem->setTextAlignment(Qt::AlignCenter);

        // Column 4: Status / SIP Code (Color-coded for instant visual diagnosis)
        QString rawInfo = QString::fromStdString(c->info).trimmed();
        QString statusText;
        QColor statusColor("#64748B"); // Default Slate
        QFont statusFont(font().family(), 10, QFont::Bold);

        if (c->duration > 0 || rawInfo.contains("200") || rawInfo.contains("OK") || rawInfo.contains("Answered", Qt::CaseInsensitive)) {
            statusText = "200 OK";
            statusColor = QColor("#059669"); // Emerald Green
        } else if (rawInfo.contains("486") || rawInfo.contains("Busy", Qt::CaseInsensitive)) {
            statusText = "486 Busy";
            statusColor = QColor("#D97706"); // Amber
        } else if (rawInfo.contains("404") || rawInfo.contains("Not Found", Qt::CaseInsensitive)) {
            statusText = "404 Not Found";
            statusColor = QColor("#EF4444"); // Crimson
        } else if (rawInfo.contains("487") || rawInfo.contains("Terminated", Qt::CaseInsensitive) || rawInfo.contains("Cancel", Qt::CaseInsensitive)) {
            statusText = "487 Cancelled";
            statusColor = QColor("#64748B"); // Slate
        } else if (rawInfo.contains("408") || rawInfo.contains("Timeout", Qt::CaseInsensitive)) {
            statusText = "408 Timeout";
            statusColor = QColor("#EA580C"); // Orange
        } else if (rawInfo.contains("401") || rawInfo.contains("407") || rawInfo.contains("Unauthorized", Qt::CaseInsensitive)) {
            statusText = "401 Auth Fail";
            statusColor = QColor("#DC2626"); // Red
        } else if (rawInfo.contains("503") || rawInfo.contains("Unavailable", Qt::CaseInsensitive)) {
            statusText = "503 Unavailable";
            statusColor = QColor("#7C3AED"); // Purple
        } else if (rawInfo.contains("603") || rawInfo.contains("Decline", Qt::CaseInsensitive)) {
            statusText = "603 Decline";
            statusColor = QColor("#DC2626");
        } else if (c->type == CALL_TYPE_MISSED) {
            statusText = "Missed";
            statusColor = QColor("#EF4444");
        } else if (!rawInfo.isEmpty()) {
            statusText = rawInfo;
        } else {
            statusText = "-";
        }

        auto* statusItem = new QTableWidgetItem(statusText);
        statusItem->setFont(statusFont);
        statusItem->setForeground(statusColor);
        statusItem->setTextAlignment(Qt::AlignCenter);

        // Tooltip for quick inspection
        QString tooltip = QString("Name: %1\nNumber: %2\nTime: %3\nDuration: %4\nStatus / Code: %5")
            .arg(displayName)
            .arg(QString::fromStdString(c->number))
            .arg(dt.toString("yyyy-MM-dd hh:mm:ss AP"))
            .arg(durStr.isEmpty() ? "0s (Unanswered)" : durStr)
            .arg(rawInfo.isEmpty() ? statusText : rawInfo);

        nameItem->setToolTip(tooltip);
        numItem->setToolTip(tooltip);
        timeItem->setToolTip(tooltip);
        durItem->setToolTip(tooltip);
        statusItem->setToolTip(tooltip);

        m_logsTable->setItem(r, 0, nameItem);
        m_logsTable->setItem(r, 1, numItem);
        m_logsTable->setItem(r, 2, timeItem);
        m_logsTable->setItem(r, 3, durItem);
        m_logsTable->setItem(r, 4, statusItem);
    }
    updateCompleter();
}

void MainWindow::onCallsItemDoubleClicked(int row, int col)
{
    Q_UNUSED(col);
    auto* item = m_logsTable->item(row, 1);
    if (item) {
        m_numberInput->setText(item->text());
        m_mainTabs->setCurrentIndex(0);
        onCallClicked();
    }
}

void MainWindow::onCallsContextMenu(const QPoint& pos)
{
    int row = m_logsTable->rowAt(pos.y());
    if (row < 0) return;

    QMenu menu(this);
    auto* callAct = menu.addAction(QIcon(":/icons/res/call_out.ico"), "Call");
    auto* vidAct = menu.addAction(QIcon(":/icons/res/button_video.ico"), "Video Call");
    auto* msgAct = menu.addAction(QIcon(":/icons/res/button_message.ico"), "Message");
    menu.addSeparator();
    auto* detailsAct = menu.addAction(QIcon(":/icons/res/button_log.ico"), "Call Details...");
    menu.addSeparator();
    auto* copyAct = menu.addAction("Copy Number");
    auto* delAct = menu.addAction("Delete Record");

    connect(callAct, &QAction::triggered, this, [this, row]() { onCallsItemDoubleClicked(row, 1); });
    connect(vidAct, &QAction::triggered, this, [this, row]() {
        auto* it = m_logsTable->item(row, 1);
        if (it) { m_numberInput->setText(it->text()); onVideoCallClicked(); }
    });
    connect(msgAct, &QAction::triggered, this, [this, row]() {
        auto* it = m_logsTable->item(row, 1);
        if (it) { m_numberInput->setText(it->text()); onMessageClicked(); }
    });
    connect(detailsAct, &QAction::triggered, this, &MainWindow::onShowCallDetails);
    connect(copyAct, &QAction::triggered, this, &MainWindow::onCopyCallNumber);
    connect(delAct, &QAction::triggered, this, &MainWindow::onDeleteCall);

    menu.exec(m_logsTable->viewport()->mapToGlobal(pos));
}

void MainWindow::onShowCallDetails()
{
    int row = m_logsTable->currentRow();
    if (row < 0) return;

    auto* nameIt = m_logsTable->item(row, 0);
    auto* numIt = m_logsTable->item(row, 1);
    auto* timeIt = m_logsTable->item(row, 2);
    auto* durIt = m_logsTable->item(row, 3);
    auto* statusIt = m_logsTable->item(row, 4);

    QString name = nameIt ? nameIt->text() : "Unknown";
    QString num = numIt ? numIt->text() : "";
    QString time = timeIt ? timeIt->text() : "";
    QString dur = durIt ? durIt->text() : "-";
    QString status = statusIt ? statusIt->text() : "-";

    QString colorHex = "#64748B";
    if (status.contains("200")) colorHex = "#059669";
    else if (status.contains("486")) colorHex = "#D97706";
    else if (status.contains("404") || status.contains("Missed", Qt::CaseInsensitive)) colorHex = "#EF4444";
    else if (status.contains("408") || status.contains("Timeout", Qt::CaseInsensitive)) colorHex = "#EA580C";
    else if (status.contains("503")) colorHex = "#7C3AED";

    QString html = QString(
        "<div style='font-family:-apple-system,BlinkMacSystemFont,\"SF Pro Text\",sans-serif; min-width:280px;'>"
        "<div style='font-size:15px; font-weight:700; margin-bottom:12px; color:#0F172A;'>Call Telemetry & Diagnostics</div>"
        "<table cellpadding='4' cellspacing='0' style='width:100%; font-size:12px; border-collapse:collapse;'>"
        "<tr><td style='color:#64748B; width:95px;'><b>Contact / Name:</b></td><td style='color:#0F172A; font-weight:600;'>%1</td></tr>"
        "<tr><td style='color:#64748B;'><b>Phone Number:</b></td><td style='color:#2563EB; font-weight:600;'>%2</td></tr>"
        "<tr><td style='color:#64748B;'><b>Timestamp:</b></td><td style='color:#0F172A;'>%3</td></tr>"
        "<tr><td style='color:#64748B;'><b>Duration:</b></td><td style='color:#0F172A; font-weight:600;'>%4</td></tr>"
        "<tr><td style='color:#64748B;'><b>SIP Status:</b></td><td><span style='background:%5; color:#FFFFFF; font-weight:700; padding:2px 8px; border-radius:4px;'>%6</span></td></tr>"
        "</table>"
        "</div>"
    ).arg(name.toHtmlEscaped())
     .arg(num.toHtmlEscaped())
     .arg(time.toHtmlEscaped())
     .arg(dur.toHtmlEscaped())
     .arg(colorHex)
     .arg(status.toHtmlEscaped());

    QMessageBox box(this);
    box.setWindowTitle("Call Details");
    box.setTextFormat(Qt::RichText);
    box.setText(html);
    box.setIcon(QMessageBox::Information);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void MainWindow::onFilterLogs(const QString& text)
{
    int filterMode = m_logsFilterCombo ? m_logsFilterCombo->currentIndex() : 0;
    for (int r = 0; r < m_logsTable->rowCount(); ++r) {
        auto* nameIt = m_logsTable->item(r, 0);
        auto* numIt = m_logsTable->item(r, 1);
        auto* statusIt = m_logsTable->item(r, 4);

        bool textMatch = (text.isEmpty() ||
                          (nameIt && nameIt->text().contains(text, Qt::CaseInsensitive)) ||
                          (numIt && numIt->text().contains(text, Qt::CaseInsensitive)) ||
                          (statusIt && statusIt->text().contains(text, Qt::CaseInsensitive)));
        
        bool typeMatch = true;
        if (filterMode > 0 && nameIt) {
            int rowType = nameIt->data(Qt::UserRole).toInt();
            if (filterMode == 1) { // Incoming
                typeMatch = (rowType == CALL_TYPE_INCOMING);
            } else if (filterMode == 2) { // Outgoing
                typeMatch = (rowType == CALL_TYPE_OUTGOING);
            } else if (filterMode == 3) { // Missed
                typeMatch = (rowType == CALL_TYPE_MISSED);
            }
        }

        m_logsTable->setRowHidden(r, !(textMatch && typeMatch));
    }
}

void MainWindow::onHistoryFilterChanged(int index)
{
    Q_UNUSED(index);
    onFilterLogs(m_logsSearch->text());
}

void MainWindow::onDeleteCall()
{
    int r = m_logsTable->currentRow();
    if (r >= 0) {
        auto* numIt = m_logsTable->item(r, 1);
        if (numIt && m_callsRepo) {
            int id = numIt->data(Qt::UserRole).toInt();
            if (id > 0) {
                try { m_callsRepo->Delete(id); } catch (...) {}
            }
        }
        m_logsTable->removeRow(r);
    }
}

void MainWindow::onClearAllLogs()
{
    if (!m_callsRepo) return;
    auto reply = QMessageBox::question(this, "Clear History", "Clear all call history?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_callsRepo->DeleteAll();
        refreshCallsHistory();
    }
}

void MainWindow::onCopyCallNumber()
{
    int r = m_logsTable->currentRow();
    if (r >= 0) {
        auto* it = m_logsTable->item(r, 1);
        if (it) QApplication::clipboard()->setText(it->text());
    }
}

// ============================================================================
// CONTACTS TAB
// ============================================================================
void MainWindow::refreshContacts()
{
    if (!m_contactsRepo) return;
    m_contactsTable->setRowCount(0);
    auto contacts = m_contactsRepo->GetAll();

    for (const auto& c : contacts) {
        int r = m_contactsTable->rowCount();
        m_contactsTable->insertRow(r);

        auto* nameItem = new QTableWidgetItem(QIcon(":/icons/res/status_online.ico"), QString::fromStdString(c->name));
        nameItem->setFont(QFont(font().family(), 11, QFont::DemiBold));

        auto* numItem = new QTableWidgetItem(QString::fromStdString(c->number));
        numItem->setForeground(QColor("#2563EB"));
        numItem->setFont(QFont(font().family(), 11));

        m_contactsTable->setItem(r, 0, nameItem);
        m_contactsTable->setItem(r, 1, numItem);
    }
    updateCompleter();
}

void MainWindow::onContactsItemDoubleClicked(int row, int col)
{
    Q_UNUSED(col);
    auto* item = m_contactsTable->item(row, 1);
    if (item) {
        m_numberInput->setText(item->text());
        m_mainTabs->setCurrentIndex(0);
        onCallClicked();
    }
}

void MainWindow::onContactsContextMenu(const QPoint& pos)
{
    int row = m_contactsTable->rowAt(pos.y());

    QMenu menu(this);
    auto* callAct = menu.addAction(QIcon(":/icons/res/call_out.ico"), "Call");
    auto* vidAct = menu.addAction(QIcon(":/icons/res/button_video.ico"), "Video Call");
    auto* msgAct = menu.addAction(QIcon(":/icons/res/button_message.ico"), "Message");
    menu.addSeparator();
    auto* addAct = menu.addAction("Add");
    auto* editAct = menu.addAction("Edit");
    auto* copyAct = menu.addAction("Copy");
    auto* delAct = menu.addAction("Delete");

    if (row >= 0) {
        connect(callAct, &QAction::triggered, this, [this, row]() { onContactsItemDoubleClicked(row, 1); });
        connect(vidAct, &QAction::triggered, this, [this, row]() {
            auto* it = m_contactsTable->item(row, 1);
            if (it) { m_numberInput->setText(it->text()); onVideoCallClicked(); }
        });
        connect(msgAct, &QAction::triggered, this, [this, row]() {
            auto* it = m_contactsTable->item(row, 1);
            if (it) { m_numberInput->setText(it->text()); onMessageClicked(); }
        });
        connect(copyAct, &QAction::triggered, this, &MainWindow::onCopyContactNumber);
        connect(editAct, &QAction::triggered, this, &MainWindow::onEditContact);
        connect(delAct, &QAction::triggered, this, &MainWindow::onDeleteContact);
    } else {
        callAct->setEnabled(false);
        vidAct->setEnabled(false);
        msgAct->setEnabled(false);
        editAct->setEnabled(false);
        copyAct->setEnabled(false);
        delAct->setEnabled(false);
    }
    connect(addAct, &QAction::triggered, this, &MainWindow::onAddContact);

    menu.exec(m_contactsTable->viewport()->mapToGlobal(pos));
}

void MainWindow::onFilterContacts(const QString& text)
{
    for (int r = 0; r < m_contactsTable->rowCount(); ++r) {
        bool match = false;
        for (int c = 0; c < 2; ++c) {
            auto* it = m_contactsTable->item(r, c);
            if (it && it->text().contains(text, Qt::CaseInsensitive)) {
                match = true; break;
            }
        }
        m_contactsTable->setRowHidden(r, !match);
    }
}

void MainWindow::onAddContact()
{
    ContactDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Contact c = dlg.getContact();
        if (m_contactsRepo) {
            m_contactsRepo->Add(c);
            refreshContacts();
        }
    }
}

void MainWindow::onEditContact()
{
    int r = m_contactsTable->currentRow();
    if (r < 0) return;
    auto* nameIt = m_contactsTable->item(r, 0);
    auto* numIt = m_contactsTable->item(r, 1);
    if (!nameIt || !numIt) return;

    Contact existing;
    existing.name = nameIt->text().toStdString();
    existing.number = numIt->text().toStdString();

    ContactDialog dlg(existing, this);
    if (dlg.exec() == QDialog::Accepted) {
        Contact updated = dlg.getContact();
        if (m_contactsRepo) {
            if (existing.number != updated.number) {
                auto all = m_contactsRepo->GetAll();
                for (const auto& c : all) {
                    if (c->number == existing.number && !c->id.empty()) {
                        try { m_contactsRepo->Delete(std::stoi(c->id)); } catch (...) {}
                        break;
                    }
                }
                m_contactsRepo->Add(updated);
            } else {
                m_contactsRepo->Update(updated);
            }
            refreshContacts();
        }
    }
}

void MainWindow::onDeleteContact()
{
    int r = m_contactsTable->currentRow();
    if (r >= 0) {
        auto* numIt = m_contactsTable->item(r, 1);
        if (numIt && m_contactsRepo) {
            auto all = m_contactsRepo->GetAll();
            for (const auto& c : all) {
                if (c->number == numIt->text().toStdString() && !c->id.empty()) {
                    try { m_contactsRepo->Delete(std::stoi(c->id)); } catch (...) {}
                    break;
                }
            }
        }
        m_contactsTable->removeRow(r);
        updateCompleter();
    }
}

void MainWindow::onCopyContactNumber()
{
    int r = m_contactsTable->currentRow();
    if (r >= 0) {
        auto* it = m_contactsTable->item(r, 1);
        if (it) QApplication::clipboard()->setText(it->text());
    }
}
