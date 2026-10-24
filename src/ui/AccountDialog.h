#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <vector>
#include <unordered_map>
#include "../core/SipTypes.h"

class SipEngine;

class AccountDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AccountDialog(SipEngine* engine = nullptr, QWidget *parent = nullptr);

    SipAccountConfig getAccountConfig() const;

    // Multi-account management
    static std::vector<SipAccountConfig> loadAllAccounts();
    static void saveAllAccounts(const std::vector<SipAccountConfig>& accounts);
    static SipAccountConfig getActiveAccount();
    static bool setActiveAccount(const std::string& accountId);

    // Legacy single-account compatibility
    static SipAccountConfig loadConfig();
    static void saveConfig(const SipAccountConfig& cfg);

private slots:
    void onAccountSelected(int index);
    void onNewAccountClicked();
    void onDeleteAccountClicked();
    void onToggleActiveAccount(bool checked);
    void onSaveClicked();
    void onTogglePasswordDisplay();

private:
    void setupUi();
    void loadAccountsFromDisk();
    void refreshAccountSelector(const std::string& selectId = "");
    void displayAccount(const SipAccountConfig& cfg);
    void updatePasswordDisplayState();
    SipAccountConfig readFormIntoConfig() const;

    SipEngine* m_engine = nullptr;

    std::vector<SipAccountConfig> m_accounts;
    std::unordered_map<std::string, std::string> m_diskPasswords;
    int m_currentIndex = -1;
    bool m_passwordDisplayed = false;
    bool m_isUpdatingUI = false;

    // Multi-account selector bar
    QComboBox* m_accountCombo = nullptr;
    QPushButton* m_newAccountBtn = nullptr;
    QPushButton* m_deleteAccountBtn = nullptr;
    QCheckBox* m_activeAccountCheck = nullptr;

    // Form fields
    QLineEdit* m_accountNameEdit = nullptr;
    QLineEdit* m_serverEdit = nullptr;
    QLineEdit* m_proxyEdit = nullptr;
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_domainEdit = nullptr;
    QLineEdit* m_loginEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QLabel* m_displayPassLink = nullptr;
    QLineEdit* m_displayNameEdit = nullptr;
    QLineEdit* m_voicemailEdit = nullptr;
    QLineEdit* m_dialPrefixEdit = nullptr;
    QLineEdit* m_dialPlanEdit = nullptr;

    QCheckBox* m_hideCallerIdCheck = nullptr;
    QComboBox* m_encryptionCombo = nullptr;
    QComboBox* m_transportCombo = nullptr;
    QComboBox* m_publicAddressCombo = nullptr;

    QSpinBox* m_registerRefreshSpin = nullptr;
    QSpinBox* m_keepAliveSpin = nullptr;

    QCheckBox* m_publishPresenceCheck = nullptr;
    QCheckBox* m_allowIpRewriteCheck = nullptr;
    QCheckBox* m_iceCheck = nullptr;
    QCheckBox* m_disableSessionTimersCheck = nullptr;
};
