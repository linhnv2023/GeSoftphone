#include "AccountDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QSettings>
#include <QUuid>

AccountDialog::AccountDialog(SipEngine* engine, QWidget *parent)
    : QDialog(parent)
    , m_engine(engine)
{
    setupUi();
    loadAccountsFromDisk();
}

void AccountDialog::setupUi()
{
    setWindowTitle("SIP Accounts Manager");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(380, 680);
    setStyleSheet(R"(
        QDialog {
            background-color: #FFFFFF;
            font-family: "SF Pro Text", "Helvetica Neue", sans-serif;
            font-size: 12px;
            color: #0F172A;
        }
        QLabel {
            color: #334155;
            font-size: 11.5px;
            font-weight: 500;
        }
        QLineEdit, QComboBox, QSpinBox {
            background-color: #F8FAFC;
            border: 1.2px solid #CBD5E1;
            border-radius: 6px;
            padding: 4px 8px;
            min-height: 22px;
            font-size: 12px;
            color: #0F172A;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus {
            background-color: #FFFFFF;
            border-color: #3B82F6;
        }
        QCheckBox {
            font-size: 11.5px;
            color: #334155;
            spacing: 6px;
        }
        QPushButton {
            background-color: #F1F5F9;
            color: #475569;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            padding: 5px 14px;
            min-height: 24px;
            font-size: 11.5px;
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
        QLabel.helpMark {
            color: #3B82F6;
            font-size: 11px;
            font-weight: bold;
        }
        QLabel.linkText {
            color: #2563EB;
            text-decoration: underline;
            font-size: 11px;
        }
    )");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 12, 16, 12);
    mainLayout->setSpacing(8);

    // ========================================================================
    // Top Multi-Account Selector Bar
    // ========================================================================
    auto* topAccountCard = new QFrame(this);
    topAccountCard->setStyleSheet("QFrame { background: #F8FAFC; border: 1.2px solid #CBD5E1; border-radius: 8px; padding: 4px; }");
    auto* topCardLayout = new QVBoxLayout(topAccountCard);
    topCardLayout->setContentsMargins(8, 6, 8, 6);
    topCardLayout->setSpacing(6);

    auto* accRow1 = new QHBoxLayout();
    accRow1->setSpacing(6);
    auto* accLabel = new QLabel("Account:", this);
    accLabel->setStyleSheet("font-weight: 700; color: #0F172A; font-size: 11.5px;");
    accRow1->addWidget(accLabel);

    m_accountCombo = new QComboBox(this);
    m_accountCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(m_accountCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AccountDialog::onAccountSelected);
    accRow1->addWidget(m_accountCombo);

    m_newAccountBtn = new QPushButton("+ New", this);
    m_newAccountBtn->setStyleSheet("QPushButton { background: #EFF6FF; color: #2563EB; border: 1px solid #BFDBFE; font-weight: 700; padding: 3px 8px; } QPushButton:hover { background: #DBEAFE; }");
    connect(m_newAccountBtn, &QPushButton::clicked, this, &AccountDialog::onNewAccountClicked);
    accRow1->addWidget(m_newAccountBtn);

    m_deleteAccountBtn = new QPushButton("✕", this);
    m_deleteAccountBtn->setToolTip("Delete this account");
    m_deleteAccountBtn->setStyleSheet("QPushButton { background: #FEF2F2; color: #DC2626; border: 1px solid #FECACA; font-weight: 700; padding: 3px 8px; } QPushButton:hover { background: #FEE2E2; }");
    connect(m_deleteAccountBtn, &QPushButton::clicked, this, &AccountDialog::onDeleteAccountClicked);
    accRow1->addWidget(m_deleteAccountBtn);
    topCardLayout->addLayout(accRow1);

    auto* accRow2 = new QHBoxLayout();
    m_activeAccountCheck = new QCheckBox("Active Account", this);
    m_activeAccountCheck->setStyleSheet("QCheckBox { font-weight: 600; color: #0F172A; }");
    connect(m_activeAccountCheck, &QCheckBox::toggled, this, &AccountDialog::onToggleActiveAccount);
    accRow2->addWidget(m_activeAccountCheck);
    accRow2->addStretch();

    m_activateNowBtn = new QPushButton("⚡ Activate (Use this Account)", this);
    m_activateNowBtn->setToolTip("Click to instantly make this account active and connect");
    connect(m_activateNowBtn, &QPushButton::clicked, this, &AccountDialog::onActivateNowClicked);
    accRow2->addWidget(m_activateNowBtn);
    topCardLayout->addLayout(accRow2);

    mainLayout->addWidget(topAccountCard);

    // ========================================================================
    // Account Form Fields
    // ========================================================================
    auto* form = new QGridLayout();
    form->setVerticalSpacing(5);
    form->setHorizontalSpacing(8);

    int row = 0;
    auto addField = [&](const QString& label, QWidget* widget, bool hasHelp = true) {
        auto* lbl = new QLabel(label, this);
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->addWidget(lbl, row, 0);
        form->addWidget(widget, row, 1);
        if (hasHelp) {
            auto* helpLbl = new QLabel("?", this);
            helpLbl->setStyleSheet("color: #0066CC; font-weight: bold; padding-left: 2px;");
            form->addWidget(helpLbl, row, 2);
        }
        row++;
    };

    m_accountNameEdit = new QLineEdit(this);
    addField("Account Name", m_accountNameEdit, false);
    connect(m_accountNameEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (m_isUpdatingUI || m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_accounts.size())) return;
        QString title = text.trimmed().isEmpty() ? m_usernameEdit->text().trimmed() : text.trimmed();
        if (title.isEmpty()) title = QString("Account %1").arg(m_currentIndex + 1);
        if (m_activeAccountCheck->isChecked()) title += "  (Active)";
        m_isUpdatingUI = true;
        m_accountCombo->setItemText(m_currentIndex, title);
        m_isUpdatingUI = false;
    });

    m_serverEdit = new QLineEdit(this);
    addField("SIP Server *", m_serverEdit, true);

    m_proxyEdit = new QLineEdit(this);
    addField("SIP Proxy", m_proxyEdit, true);

    m_usernameEdit = new QLineEdit(this);
    addField("Username *", m_usernameEdit, true);
    connect(m_usernameEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (m_isUpdatingUI || m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_accounts.size())) return;
        if (m_accountNameEdit->text().trimmed().isEmpty()) {
            QString title = text.trimmed().isEmpty() ? QString("Account %1").arg(m_currentIndex + 1) : text.trimmed();
            if (m_activeAccountCheck->isChecked()) title += "  (Active)";
            m_isUpdatingUI = true;
            m_accountCombo->setItemText(m_currentIndex, title);
            m_isUpdatingUI = false;
        }
    });

    m_domainEdit = new QLineEdit(this);
    addField("Domain *", m_domainEdit, true);

    m_loginEdit = new QLineEdit(this);
    addField("Login", m_loginEdit, true);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    connect(m_passwordEdit, &QLineEdit::textEdited, this, [this](const QString&) {
        updatePasswordDisplayState();
    });
    addField("Password", m_passwordEdit, true);

    // "display password" link (only active before saving)
    m_displayPassLink = new QLabel(this);
    m_displayPassLink->setTextFormat(Qt::RichText);
    m_displayPassLink->setStyleSheet("font-size: 11px;");
    connect(m_displayPassLink, &QLabel::linkActivated, this, &AccountDialog::onTogglePasswordDisplay);
    form->addWidget(m_displayPassLink, row++, 1);

    m_displayNameEdit = new QLineEdit(this);
    addField("Display Name", m_displayNameEdit, true);

    m_voicemailEdit = new QLineEdit(this);
    addField("Voicemail Number", m_voicemailEdit, true);

    m_dialPrefixEdit = new QLineEdit(this);
    addField("Dialing Prefix", m_dialPrefixEdit, true);

    m_dialPlanEdit = new QLineEdit(this);
    addField("Dial Plan", m_dialPlanEdit, true);

    // Hide Caller ID checkbox
    m_hideCallerIdCheck = new QCheckBox("Hide Caller ID", this);
    form->addWidget(m_hideCallerIdCheck, row++, 1);

    // Media Encryption
    m_encryptionCombo = new QComboBox(this);
    m_encryptionCombo->addItems({"Disabled", "Optional", "Mandatory"});
    addField("Media Encryption", m_encryptionCombo, true);

    // Transport
    m_transportCombo = new QComboBox(this);
    m_transportCombo->addItems({"Auto (UDP & TCP)", "UDP", "TCP", "TLS"});
    addField("Transport", m_transportCombo, true);

    // Public Address
    m_publicAddressCombo = new QComboBox(this);
    m_publicAddressCombo->addItems({"Auto", "STUN"});
    addField("Public Address", m_publicAddressCombo, true);

    // Register Refresh & Keep-Alive
    auto* refreshLayout = new QHBoxLayout();
    m_registerRefreshSpin = new QSpinBox(this);
    m_registerRefreshSpin->setRange(60, 3600);
    m_registerRefreshSpin->setValue(300);
    m_registerRefreshSpin->setFixedWidth(65);

    auto* keepAliveLabel = new QLabel("Keep-Alive", this);
    m_keepAliveSpin = new QSpinBox(this);
    m_keepAliveSpin->setRange(5, 300);
    m_keepAliveSpin->setValue(15);
    m_keepAliveSpin->setFixedWidth(55);

    refreshLayout->addWidget(m_registerRefreshSpin);
    refreshLayout->addSpacing(10);
    refreshLayout->addWidget(keepAliveLabel);
    refreshLayout->addWidget(m_keepAliveSpin);
    refreshLayout->addStretch();

    auto* regLabel = new QLabel("Register Refresh", this);
    regLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->addWidget(regLabel, row, 0);
    form->addLayout(refreshLayout, row, 1);
    row++;

    // Advanced Checkboxes
    auto addCheck = [&](const QString& label, QCheckBox*& ptr) {
        ptr = new QCheckBox(label, this);
        form->addWidget(ptr, row, 1);
        auto* help = new QLabel("?", this);
        help->setStyleSheet("color: #0066CC; font-weight: bold; padding-left: 2px;");
        form->addWidget(help, row, 2);
        row++;
    };

    addCheck("Publish Presence", m_publishPresenceCheck);
    addCheck("Allow IP Rewrite", m_allowIpRewriteCheck);
    addCheck("ICE", m_iceCheck);
    addCheck("Disable Session Timers", m_disableSessionTimersCheck);

    mainLayout->addLayout(form);
    mainLayout->addStretch();

    // Bottom Action Buttons
    auto* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setFixedWidth(75);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto* saveBtn = new QPushButton("Save & Apply", this);
    saveBtn->setObjectName("saveBtn");
    saveBtn->setDefault(true);
    saveBtn->setFixedWidth(100);
    connect(saveBtn, &QPushButton::clicked, this, &AccountDialog::onSaveClicked);

    bottomLayout->addWidget(cancelBtn);
    bottomLayout->addWidget(saveBtn);
    mainLayout->addLayout(bottomLayout);
}

void AccountDialog::loadAccountsFromDisk()
{
    m_accounts = loadAllAccounts();
    if (m_accounts.empty()) {
        SipAccountConfig def;
        def.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        def.label = "Account 1";
        def.isActive = true;
        def.isSaved = false;
        m_accounts.push_back(def);
    }
    m_diskPasswords.clear();
    for (const auto& a : m_accounts) {
        if (a.isSaved && !a.password.empty()) {
            m_diskPasswords[a.id] = a.password;
        }
    }
    refreshAccountSelector();
}

void AccountDialog::refreshAccountSelector(const std::string& selectId)
{
    m_isUpdatingUI = true;
    m_accountCombo->clear();

    int targetIdx = 0;
    for (size_t i = 0; i < m_accounts.size(); ++i) {
        const auto& acc = m_accounts[i];
        QString title = QString::fromStdString(acc.label.empty() ? acc.username : acc.label);
        if (title.isEmpty()) title = QString("Account %1").arg(i + 1);
        if (acc.isActive) {
            title += "  (Active)";
        }
        m_accountCombo->addItem(title, QString::fromStdString(acc.id));

        if (!selectId.empty() && acc.id == selectId) {
            targetIdx = static_cast<int>(i);
        } else if (selectId.empty() && acc.isActive) {
            targetIdx = static_cast<int>(i);
        }
    }

    if (targetIdx >= static_cast<int>(m_accounts.size())) {
        targetIdx = m_accounts.empty() ? -1 : 0;
    }

    m_currentIndex = targetIdx;
    if (targetIdx >= 0) {
        m_accountCombo->setCurrentIndex(targetIdx);
        displayAccount(m_accounts[targetIdx]);
    }
    m_isUpdatingUI = false;
}

void AccountDialog::displayAccount(const SipAccountConfig& cfg)
{
    m_accountNameEdit->setText(QString::fromStdString(cfg.label));
    m_serverEdit->setText(QString::fromStdString(cfg.server));
    m_proxyEdit->setText(QString::fromStdString(cfg.proxy));
    m_usernameEdit->setText(QString::fromStdString(cfg.username));
    m_domainEdit->setText(QString::fromStdString(cfg.domain.empty() ? cfg.server : cfg.domain));
    m_loginEdit->setText(QString::fromStdString(cfg.authID));
    m_passwordEdit->setText(QString::fromStdString(cfg.password));
    m_passwordDisplayed = false;
    updatePasswordDisplayState();
    m_displayNameEdit->setText(QString::fromStdString(cfg.displayName));
    m_voicemailEdit->setText(QString::fromStdString(cfg.voicemail));
    m_dialPrefixEdit->setText(QString::fromStdString(cfg.dialPrefix));
    m_dialPlanEdit->setText(QString::fromStdString(cfg.dialPlan));

    m_hideCallerIdCheck->setChecked(cfg.hideCallerId);
    m_encryptionCombo->setCurrentIndex(cfg.encryption);

    int transportIdx = 0;
    if (cfg.transport == "TCP") transportIdx = 2;
    else if (cfg.transport == "TLS") transportIdx = 3;
    else if (cfg.transport == "UDP") transportIdx = 1;
    m_transportCombo->setCurrentIndex(transportIdx);

    m_publicAddressCombo->setCurrentIndex(cfg.publicAddress);
    m_registerRefreshSpin->setValue(cfg.registerRefresh > 0 ? cfg.registerRefresh : 300);
    m_keepAliveSpin->setValue(cfg.keepAlive > 0 ? cfg.keepAlive : 15);

    m_publishPresenceCheck->setChecked(cfg.publish);
    m_allowIpRewriteCheck->setChecked(cfg.allowIpRewrite);
    m_iceCheck->setChecked(cfg.ice);
    m_disableSessionTimersCheck->setChecked(cfg.disableSessionTimers);

    m_activeAccountCheck->setChecked(cfg.isActive);
    updateActiveButtonState();
}

void AccountDialog::updatePasswordDisplayState()
{
    bool isSavedPassword = false;
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_accounts.size())) {
        const std::string& accId = m_accounts[m_currentIndex].id;
        auto it = m_diskPasswords.find(accId);
        if (it != m_diskPasswords.end() && !it->second.empty()) {
            if (m_passwordEdit->text() == QString::fromStdString(it->second)) {
                isSavedPassword = true;
            }
        }
    }

    if (isSavedPassword) {
        m_passwordDisplayed = false;
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_displayPassLink->setText("<span style='color: #64748B; font-size: 11px; font-weight: 500;'>🔒 Password saved & protected</span>");
        m_displayPassLink->setCursor(Qt::ArrowCursor);
        m_displayPassLink->setToolTip("For security, saved passwords cannot be displayed.");
    } else {
        m_displayPassLink->setCursor(Qt::PointingHandCursor);
        m_displayPassLink->setToolTip("");
        if (m_passwordDisplayed) {
            m_passwordEdit->setEchoMode(QLineEdit::Normal);
            m_displayPassLink->setText("<a href='#' style='color: #2563EB; font-size: 11px; text-decoration: underline;'>Hide password</a>");
        } else {
            m_passwordEdit->setEchoMode(QLineEdit::Password);
            m_displayPassLink->setText("<a href='#' style='color: #2563EB; font-size: 11px; text-decoration: underline;'>Display password</a>");
        }
    }
}

void AccountDialog::updateActiveButtonState()
{
    if (!m_activateNowBtn) return;
    bool isActive = m_activeAccountCheck ? m_activeAccountCheck->isChecked() : false;
    if (isActive) {
        m_activateNowBtn->setText("⏸ Deactivate (Go Offline)");
        m_activateNowBtn->setStyleSheet(
            "QPushButton { background: #FEF2F2; color: #DC2626; border: 1.2px solid #FECACA; "
            "font-weight: 700; border-radius: 6px; padding: 4px 14px; font-size: 11px; } "
            "QPushButton:hover { background: #FEE2E2; border-color: #F87171; } "
            "QPushButton:pressed { background: #FECACA; }"
        );
        m_activateNowBtn->setEnabled(true);
        m_activateNowBtn->setCursor(Qt::PointingHandCursor);
        m_activateNowBtn->setToolTip("Click to disconnect and deactivate this account (go offline)");
    } else {
        m_activateNowBtn->setText("⚡ Activate (Use this Account)");
        m_activateNowBtn->setStyleSheet(
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2563EB, stop:1 #3B82F6); "
            "color: #FFFFFF; border: none; font-weight: 700; border-radius: 6px; padding: 4px 14px; font-size: 11px; } "
            "QPushButton:hover { background: #1D4ED8; } "
            "QPushButton:pressed { background: #1E40AF; }"
        );
        m_activateNowBtn->setEnabled(true);
        m_activateNowBtn->setCursor(Qt::PointingHandCursor);
        m_activateNowBtn->setToolTip("Click to instantly make this account active and connect");
    }
}

SipAccountConfig AccountDialog::readFormIntoConfig() const
{
    SipAccountConfig cfg;
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_accounts.size())) {
        cfg = m_accounts[m_currentIndex];
    } else {
        cfg.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    }

    cfg.label = m_accountNameEdit->text().trimmed().toStdString();
    cfg.server = m_serverEdit->text().trimmed().toStdString();
    cfg.proxy = m_proxyEdit->text().trimmed().toStdString();
    cfg.username = m_usernameEdit->text().trimmed().toStdString();
    cfg.domain = m_domainEdit->text().trimmed().toStdString();
    cfg.authID = m_loginEdit->text().trimmed().isEmpty() ? cfg.username : m_loginEdit->text().trimmed().toStdString();
    cfg.password = m_passwordEdit->text().toStdString();
    cfg.displayName = m_displayNameEdit->text().trimmed().toStdString();
    cfg.voicemail = m_voicemailEdit->text().trimmed().toStdString();
    cfg.dialPrefix = m_dialPrefixEdit->text().trimmed().toStdString();
    cfg.dialPlan = m_dialPlanEdit->text().trimmed().toStdString();

    QString transport = m_transportCombo->currentText();
    if (transport.contains("UDP")) cfg.transport = "UDP";
    else if (transport == "TCP") cfg.transport = "TCP";
    else if (transport == "TLS") cfg.transport = "TLS";
    else cfg.transport = "UDP";

    cfg.encryption = m_encryptionCombo->currentIndex();
    cfg.publicAddress = m_publicAddressCombo->currentIndex();
    cfg.registerRefresh = m_registerRefreshSpin->value();
    cfg.keepAlive = m_keepAliveSpin->value();
    cfg.hideCallerId = m_hideCallerIdCheck->isChecked();
    cfg.allowIpRewrite = m_allowIpRewriteCheck->isChecked();
    cfg.ice = m_iceCheck->isChecked();
    cfg.disableSessionTimers = m_disableSessionTimersCheck->isChecked();
    cfg.publish = m_publishPresenceCheck->isChecked();
    cfg.isActive = m_activeAccountCheck->isChecked();

    if (cfg.label.empty()) {
        cfg.label = cfg.username.empty() ? "Account" : cfg.username;
    }

    return cfg;
}

void AccountDialog::onAccountSelected(int index)
{
    if (m_isUpdatingUI || index < 0 || index >= static_cast<int>(m_accounts.size())) return;

    // Save current active changes before switching
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_accounts.size())) {
        m_accounts[m_currentIndex] = readFormIntoConfig();
    }

    m_currentIndex = index;
    displayAccount(m_accounts[index]);
}

void AccountDialog::onNewAccountClicked()
{
    // Save current form into m_accounts[m_currentIndex]
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_accounts.size())) {
        m_accounts[m_currentIndex] = readFormIntoConfig();
    }

    SipAccountConfig newAcc;
    newAcc.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    newAcc.label = QString("Account %1").arg(m_accounts.size() + 1).toStdString();
    newAcc.isActive = false; // Don't deactivate current active account unless requested
    newAcc.transport = "UDP";
    newAcc.registerRefresh = 300;
    newAcc.keepAlive = 15;
    newAcc.allowIpRewrite = true;
    m_accounts.push_back(newAcc);

    refreshAccountSelector(newAcc.id);
    m_accountNameEdit->setFocus();
    m_accountNameEdit->selectAll();
}

void AccountDialog::onDeleteAccountClicked()
{
    if (m_accounts.empty() || m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_accounts.size())) return;

    QString accName = QString::fromStdString(m_accounts[m_currentIndex].label.empty() ? m_accounts[m_currentIndex].username : m_accounts[m_currentIndex].label);
    if (accName.isEmpty()) accName = QString("Account %1").arg(m_currentIndex + 1);

    auto reply = QMessageBox::question(this, "Delete Account",
        QString("Are you sure you want to delete \"%1\"?").arg(accName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        std::string deletedId = m_accounts[m_currentIndex].id;
        m_diskPasswords.erase(deletedId);
        bool wasActive = m_accounts[m_currentIndex].isActive;
        m_accounts.erase(m_accounts.begin() + m_currentIndex);

        if (m_accounts.empty()) {
            SipAccountConfig def;
            def.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
            def.label = "Account 1";
            def.isActive = true;
            def.isSaved = false;
            m_accounts.push_back(def);
        } else if (wasActive) {
            m_accounts.front().isActive = true;
        }

        saveAllAccounts(m_accounts);
        refreshAccountSelector();
    }
}

void AccountDialog::onToggleActiveAccount(bool checked)
{
    if (m_isUpdatingUI || m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_accounts.size())) return;

    if (checked) {
        for (size_t i = 0; i < m_accounts.size(); ++i) {
            m_accounts[i].isActive = (static_cast<int>(i) == m_currentIndex);
        }
    } else {
        m_accounts[m_currentIndex].isActive = false;
    }

    // Refresh combo labels
    m_isUpdatingUI = true;
    for (size_t i = 0; i < m_accounts.size(); ++i) {
        QString title = QString::fromStdString(m_accounts[i].label.empty() ? m_accounts[i].username : m_accounts[i].label);
        if (title.isEmpty()) title = QString("Account %1").arg(i + 1);
        if (m_accounts[i].isActive) title += "  (Active)";
        m_accountCombo->setItemText(static_cast<int>(i), title);
    }
    m_isUpdatingUI = false;
    updateActiveButtonState();
}

void AccountDialog::onActivateNowClicked()
{
    if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_accounts.size())) return;

    // Save current form into m_accounts[m_currentIndex]
    m_accounts[m_currentIndex] = readFormIntoConfig();

    // 1. If currently active -> Deactivate / Disconnect (Go Offline)
    if (m_accounts[m_currentIndex].isActive) {
        m_accounts[m_currentIndex].isActive = false;
        m_accounts[m_currentIndex].isSaved = true;
        if (!m_accounts[m_currentIndex].password.empty()) {
            m_diskPasswords[m_accounts[m_currentIndex].id] = m_accounts[m_currentIndex].password;
        }
        saveAllAccounts(m_accounts);
        accept(); // Triggers autoRegister in MainWindow which will unregister SIP and go Offline
        return;
    }

    // 2. If not active -> Activate this account
    // Validate current account
    if (m_accounts[m_currentIndex].username.empty()) {
        QMessageBox::warning(this, "Validation Error", "Username is required.");
        m_usernameEdit->setFocus();
        return;
    }
    if (m_accounts[m_currentIndex].server.empty() && m_accounts[m_currentIndex].domain.empty()) {
        QMessageBox::warning(this, "Validation Error", "SIP Server or Domain is required.");
        m_serverEdit->setFocus();
        return;
    }

    QString srv = QString::fromStdString(m_accounts[m_currentIndex].server);
    QString dom = QString::fromStdString(m_accounts[m_currentIndex].domain);
    QString prx = QString::fromStdString(m_accounts[m_currentIndex].proxy);
    if (srv.contains('@') || dom.contains('@') || prx.contains('@')) {
        QMessageBox::warning(this, "Validation Error",
            "SIP Server, Domain, and Proxy must NOT contain the '@' symbol.\n\n"
            "If you entered an address with '@' (e.g. 'hmpbx@hocmai.vn:25060'), please change it to use a dot '.' (e.g. 'hmpbx.hocmai.vn:25060').");
        if (srv.contains('@')) m_serverEdit->setFocus();
        else if (dom.contains('@')) m_domainEdit->setFocus();
        else m_proxyEdit->setFocus();
        return;
    }

    // Set this account as active, deactivate all others
    for (size_t i = 0; i < m_accounts.size(); ++i) {
        m_accounts[i].isActive = (static_cast<int>(i) == m_currentIndex);
        m_accounts[i].isSaved = true;
        if (!m_accounts[i].password.empty()) {
            m_diskPasswords[m_accounts[i].id] = m_accounts[i].password;
        }
    }

    // Save all accounts to disk immediately
    saveAllAccounts(m_accounts);

    // Close dialog with Accepted to trigger instant registration in MainWindow!
    accept();
}

void AccountDialog::onTogglePasswordDisplay()
{
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_accounts.size())) {
        const std::string& accId = m_accounts[m_currentIndex].id;
        auto it = m_diskPasswords.find(accId);
        if (it != m_diskPasswords.end() && !it->second.empty()) {
            if (m_passwordEdit->text() == QString::fromStdString(it->second)) {
                // Saved password cannot be displayed
                m_passwordDisplayed = false;
                updatePasswordDisplayState();
                return;
            }
        }
    }

    m_passwordDisplayed = !m_passwordDisplayed;
    updatePasswordDisplayState();
}

void AccountDialog::onSaveClicked()
{
    // Save current form into m_accounts[m_currentIndex]
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_accounts.size())) {
        m_accounts[m_currentIndex] = readFormIntoConfig();
    }

    // Validate the currently selected account
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_accounts.size())) {
        if (m_accounts[m_currentIndex].username.empty()) {
            QMessageBox::warning(this, "Validation Error", "Username is required.");
            m_usernameEdit->setFocus();
            return;
        }
        if (m_accounts[m_currentIndex].server.empty() && m_accounts[m_currentIndex].domain.empty()) {
            QMessageBox::warning(this, "Validation Error", "SIP Server or Domain is required.");
            m_serverEdit->setFocus();
            return;
        }

        QString srv = QString::fromStdString(m_accounts[m_currentIndex].server);
        QString dom = QString::fromStdString(m_accounts[m_currentIndex].domain);
        QString prx = QString::fromStdString(m_accounts[m_currentIndex].proxy);
        if (srv.contains('@') || dom.contains('@') || prx.contains('@')) {
            QMessageBox::warning(this, "Validation Error",
                "SIP Server, Domain, and Proxy must NOT contain the '@' symbol.\n\n"
                "If you entered an address with '@' (e.g. 'hmpbx@hocmai.vn:25060'), please change it to use a dot '.' (e.g. 'hmpbx.hocmai.vn:25060').");
            if (srv.contains('@')) m_serverEdit->setFocus();
            else if (dom.contains('@')) m_domainEdit->setFocus();
            else m_proxyEdit->setFocus();
            return;
        }
    }

    // Clean up completely blank accounts that were never configured
    std::vector<SipAccountConfig> cleanedAccounts;
    for (size_t i = 0; i < m_accounts.size(); ++i) {
        const auto& a = m_accounts[i];
        if (!a.username.empty() || !a.server.empty() || static_cast<int>(i) == m_currentIndex) {
            cleanedAccounts.push_back(a);
        }
    }
    if (cleanedAccounts.empty()) {
        cleanedAccounts = m_accounts;
    }

    // Mark all accounts saved and preserve user's active/inactive choice
    for (auto& acc : cleanedAccounts) {
        acc.isSaved = true;
        if (!acc.password.empty()) {
            m_diskPasswords[acc.id] = acc.password;
        }
    }

    saveAllAccounts(cleanedAccounts);
    accept();
}

SipAccountConfig AccountDialog::getAccountConfig() const
{
    return getActiveAccount();
}

std::vector<SipAccountConfig> AccountDialog::loadAllAccounts()
{
    std::vector<SipAccountConfig> list;
    QSettings s;
    int size = s.value("accounts/size", 0).toInt();
    QString activeId = s.value("accounts/active_id", "").toString();

    if (size <= 0) {
        // Migration from legacy single account config
        QString server = s.value("sip/server", "").toString();
        QString username = s.value("sip/username", "").toString();
        if (!server.isEmpty() || !username.isEmpty()) {
            SipAccountConfig legacy;
            legacy.id = "acc_legacy_1";
            legacy.label = s.value("sip/account_name", "Account 1").toString().toStdString();
            legacy.isActive = true;
            legacy.server = server.toStdString();
            legacy.domain = s.value("sip/domain", "").toString().toStdString();
            legacy.proxy = s.value("sip/proxy", "").toString().toStdString();
            legacy.username = username.toStdString();
            legacy.authID = s.value("sip/auth_id", "").toString().toStdString();
            legacy.password = s.value("sip/password", "").toString().toStdString();
            legacy.displayName = s.value("sip/display_name", "").toString().toStdString();
            legacy.transport = s.value("sip/transport", "UDP").toString().toStdString();
            legacy.registerRefresh = s.value("sip/register_refresh", 300).toInt();
            legacy.keepAlive = s.value("sip/keep_alive", 15).toInt();
            legacy.voicemail = s.value("sip/voicemail", "").toString().toStdString();
            legacy.dialPrefix = s.value("sip/dial_prefix", "").toString().toStdString();
            legacy.dialPlan = s.value("sip/dial_plan", "").toString().toStdString();
            legacy.hideCallerId = s.value("sip/hide_caller_id", false).toBool();
            legacy.encryption = s.value("sip/encryption", 0).toInt();
            legacy.publicAddress = s.value("sip/public_address", 0).toInt();
            legacy.allowIpRewrite = s.value("sip/allow_ip_rewrite", true).toBool();
            legacy.ice = s.value("sip/ice", false).toBool();
            legacy.disableSessionTimers = s.value("sip/disable_session_timers", false).toBool();
            legacy.isSaved = true;
            list.push_back(legacy);
            saveAllAccounts(list);
            return list;
        }
        return list;
    }

    for (int i = 0; i < size; ++i) {
        QString prefix = QString("accounts/%1/").arg(i);
        SipAccountConfig cfg;
        cfg.id = s.value(prefix + "id", QString("acc_%1").arg(i)).toString().toStdString();
        cfg.label = s.value(prefix + "label", QString("Account %1").arg(i + 1)).toString().toStdString();
        cfg.server = s.value(prefix + "server", "").toString().toStdString();
        cfg.domain = s.value(prefix + "domain", "").toString().toStdString();
        cfg.proxy = s.value(prefix + "proxy", "").toString().toStdString();
        cfg.username = s.value(prefix + "username", "").toString().toStdString();
        cfg.authID = s.value(prefix + "auth_id", "").toString().toStdString();
        cfg.password = s.value(prefix + "password", "").toString().toStdString();
        cfg.displayName = s.value(prefix + "display_name", "").toString().toStdString();
        cfg.transport = s.value(prefix + "transport", "UDP").toString().toStdString();
        cfg.registerRefresh = s.value(prefix + "register_refresh", 300).toInt();
        cfg.keepAlive = s.value(prefix + "keep_alive", 15).toInt();
        cfg.voicemail = s.value(prefix + "voicemail", "").toString().toStdString();
        cfg.dialPrefix = s.value(prefix + "dial_prefix", "").toString().toStdString();
        cfg.dialPlan = s.value(prefix + "dial_plan", "").toString().toStdString();
        cfg.hideCallerId = s.value(prefix + "hide_caller_id", false).toBool();
        cfg.encryption = s.value(prefix + "encryption", 0).toInt();
        cfg.publicAddress = s.value(prefix + "public_address", 0).toInt();
        cfg.allowIpRewrite = s.value(prefix + "allow_ip_rewrite", true).toBool();
        cfg.ice = s.value(prefix + "ice", false).toBool();
        cfg.disableSessionTimers = s.value(prefix + "disable_session_timers", false).toBool();
        cfg.isSaved = true;

        cfg.isActive = (!activeId.isEmpty() && cfg.id == activeId.toStdString()) || (activeId.isEmpty() && i == 0);
        list.push_back(cfg);
    }

    return list;
}

void AccountDialog::saveAllAccounts(const std::vector<SipAccountConfig>& accounts)
{
    QSettings s;
    s.remove("accounts");
    s.setValue("accounts/size", static_cast<int>(accounts.size()));
    std::string activeId;

    for (size_t i = 0; i < accounts.size(); ++i) {
        const auto& cfg = accounts[i];
        if (cfg.isActive || (activeId.empty() && i == accounts.size() - 1)) {
            activeId = cfg.id;
        }
        QString prefix = QString("accounts/%1/").arg(i);
        s.setValue(prefix + "id", QString::fromStdString(cfg.id));
        s.setValue(prefix + "label", QString::fromStdString(cfg.label));
        s.setValue(prefix + "server", QString::fromStdString(cfg.server));
        s.setValue(prefix + "domain", QString::fromStdString(cfg.domain));
        s.setValue(prefix + "proxy", QString::fromStdString(cfg.proxy));
        s.setValue(prefix + "username", QString::fromStdString(cfg.username));
        s.setValue(prefix + "auth_id", QString::fromStdString(cfg.authID));
        s.setValue(prefix + "password", QString::fromStdString(cfg.password));
        s.setValue(prefix + "display_name", QString::fromStdString(cfg.displayName));
        s.setValue(prefix + "transport", QString::fromStdString(cfg.transport));
        s.setValue(prefix + "register_refresh", cfg.registerRefresh);
        s.setValue(prefix + "keep_alive", cfg.keepAlive);
        s.setValue(prefix + "voicemail", QString::fromStdString(cfg.voicemail));
        s.setValue(prefix + "dial_prefix", QString::fromStdString(cfg.dialPrefix));
        s.setValue(prefix + "dial_plan", QString::fromStdString(cfg.dialPlan));
        s.setValue(prefix + "hide_caller_id", cfg.hideCallerId);
        s.setValue(prefix + "encryption", cfg.encryption);
        s.setValue(prefix + "public_address", cfg.publicAddress);
        s.setValue(prefix + "allow_ip_rewrite", cfg.allowIpRewrite);
        s.setValue(prefix + "ice", cfg.ice);
        s.setValue(prefix + "disable_session_timers", cfg.disableSessionTimers);
    }
    s.setValue("accounts/active_id", QString::fromStdString(activeId));

    // Also sync the active account to legacy sip/* keys for backward compatibility
    for (const auto& a : accounts) {
        if (a.isActive) {
            s.setValue("sip/server", QString::fromStdString(a.server));
            s.setValue("sip/username", QString::fromStdString(a.username));
            s.setValue("sip/password", QString::fromStdString(a.password));
            s.setValue("sip/domain", QString::fromStdString(a.domain));
            s.setValue("sip/proxy", QString::fromStdString(a.proxy));
            s.setValue("sip/auth_id", QString::fromStdString(a.authID));
            s.setValue("sip/display_name", QString::fromStdString(a.displayName));
            s.setValue("sip/transport", QString::fromStdString(a.transport));
            s.setValue("sip/account_name", QString::fromStdString(a.label));
            break;
        }
    }
}

SipAccountConfig AccountDialog::getActiveAccount()
{
    auto accounts = loadAllAccounts();
    for (const auto& a : accounts) {
        if (a.isActive) return a;
    }
    // No account is marked active -> return empty config (Offline)
    return SipAccountConfig();
}

bool AccountDialog::setActiveAccount(const std::string& accountId)
{
    auto accounts = loadAllAccounts();
    bool found = false;
    for (auto& a : accounts) {
        if (!accountId.empty() && a.id == accountId) {
            a.isActive = true;
            found = true;
        } else {
            a.isActive = false;
        }
    }
    // If accountId is empty, it means deactivate all (Go Offline)
    if (found || accountId.empty()) {
        saveAllAccounts(accounts);
    }
    return found;
}

SipAccountConfig AccountDialog::loadConfig()
{
    QSettings s;
    SipAccountConfig cfg;
    cfg.isSaved = true;
    cfg.server = s.value("sip/server", "").toString().toStdString();
    cfg.domain = s.value("sip/domain", "").toString().toStdString();
    cfg.proxy = s.value("sip/proxy", "").toString().toStdString();
    cfg.username = s.value("sip/username", "").toString().toStdString();
    cfg.authID = s.value("sip/auth_id", "").toString().toStdString();
    cfg.password = s.value("sip/password", "").toString().toStdString();
    cfg.displayName = s.value("sip/display_name", "").toString().toStdString();
    cfg.transport = s.value("sip/transport", "UDP").toString().toStdString();
    cfg.registerRefresh = s.value("sip/register_refresh", 300).toInt();
    cfg.keepAlive = s.value("sip/keep_alive", 15).toInt();
    return cfg;
}

void AccountDialog::saveConfig(const SipAccountConfig& cfg)
{
    std::vector<SipAccountConfig> list = loadAllAccounts();
    bool updated = false;
    for (auto& a : list) {
        if (a.isActive || a.id == cfg.id) {
            a = cfg;
            a.isActive = true;
            updated = true;
            break;
        }
    }
    if (!updated) {
        list.push_back(cfg);
    }
    saveAllAccounts(list);
}
