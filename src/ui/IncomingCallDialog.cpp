#include "IncomingCallDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

IncomingCallDialog::IncomingCallDialog(const QString& callerNumber, const QString& callerName, QWidget* parent)
    : QDialog(parent)
    , m_callerNumber(callerNumber)
    , m_callerName(callerName.isEmpty() ? callerNumber : callerName)
{
    setupUi();
}

void IncomingCallDialog::setupUi()
{
    setWindowTitle("Incoming Call");
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setFixedSize(300, 240);
    setStyleSheet(R"(
        QDialog {
            background-color: #FFFFFF;
            border-radius: 12px;
            font-family: "SF Pro Text", "Helvetica Neue", sans-serif;
        }
    )");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 24, 20, 20);
    layout->setSpacing(10);

    // Call icon avatar
    auto* avatar = new QLabel("📞", this);
    avatar->setFixedSize(56, 56);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        "background: #ECFDF5; color: #10B981; font-size: 26px; border: 2px solid #A7F3D0; border-radius: 28px;"
    );

    auto* avatarLayout = new QHBoxLayout();
    avatarLayout->addStretch();
    avatarLayout->addWidget(avatar);
    avatarLayout->addStretch();
    layout->addLayout(avatarLayout);

    // Caller Name & Number
    auto* nameLabel = new QLabel(m_callerName, this);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet("font-size: 17px; font-weight: 800; color: #0F172A; margin-top: 4px;");

    auto* typeLabel = new QLabel("Incoming Voice Call...", this);
    typeLabel->setAlignment(Qt::AlignCenter);
    typeLabel->setStyleSheet("font-size: 11px; font-weight: 500; color: #64748B;");

    layout->addWidget(nameLabel);
    layout->addWidget(typeLabel);
    layout->addStretch();

    // Action buttons (Answer / Decline)
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    auto* declineBtn = new QPushButton("✕ Decline", this);
    declineBtn->setFixedHeight(42);
    declineBtn->setCursor(Qt::PointingHandCursor);
    declineBtn->setStyleSheet(
        "QPushButton { background: #FEE2E2; color: #DC2626; border: 1px solid #FECACA; border-radius: 8px; font-size: 13px; font-weight: 700; }"
        "QPushButton:hover { background: #EF4444; color: #FFFFFF; border-color: #EF4444; }"
    );
    connect(declineBtn, &QPushButton::clicked, this, &IncomingCallDialog::onDecline);

    auto* answerBtn = new QPushButton("📞 Answer", this);
    answerBtn->setFixedHeight(42);
    answerBtn->setCursor(Qt::PointingHandCursor);
    answerBtn->setStyleSheet(
        "QPushButton { background: #10B981; color: #FFFFFF; border: none; border-radius: 8px; font-size: 13px; font-weight: 700; }"
        "QPushButton:hover { background: #059669; }"
    );
    connect(answerBtn, &QPushButton::clicked, this, &IncomingCallDialog::onAnswer);

    btnLayout->addWidget(declineBtn);
    btnLayout->addWidget(answerBtn);
    layout->addLayout(btnLayout);
}

void IncomingCallDialog::onAnswer()
{
    m_answered = true;
    accept();
}

void IncomingCallDialog::onDecline()
{
    m_answered = false;
    reject();
}
