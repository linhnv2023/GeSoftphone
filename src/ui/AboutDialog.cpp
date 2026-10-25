#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
}

void AboutDialog::setupUi()
{
    setWindowTitle(tr("About GeSoftphone"));
    setModal(true);
    setFixedSize(400, 415);

    setStyleSheet(
        "QDialog {"
        "   background-color: #F8FAFC;"
        "   font-family: \"SF Pro Text\", \"Helvetica Neue\", sans-serif;"
        "}"
        "QLabel {"
        "   color: #334155;"
        "}"
        "QPushButton#closeBtn {"
        "   background-color: #3B82F6;"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 6px;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   padding: 7px 24px;"
        "}"
        "QPushButton#closeBtn:hover {"
        "   background-color: #2563EB;"
        "}"
    );

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 20);
    mainLayout->setSpacing(16);

    // App Branding Row
    auto* brandLayout = new QHBoxLayout();
    brandLayout->setSpacing(16);

    auto* logoLabel = new QLabel(this);
    logoLabel->setPixmap(QIcon(":/icons/res/gesoftphone.png").pixmap(52, 52));
    logoLabel->setFixedSize(52, 52);
    brandLayout->addWidget(logoLabel);

    auto* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);

    auto* nameLabel = new QLabel("GeSoftphone", this);
    nameLabel->setStyleSheet("font-size: 20px; font-weight: 800; color: #0F172A;");

    auto* verLabel = new QLabel("Version 3.22.16 (macOS Native)", this);
    verLabel->setStyleSheet("font-size: 11.5px; font-weight: 600; color: #3B82F6;");

    auto* devLabel = new QLabel("Developed by linhnv2", this);
    devLabel->setStyleSheet("font-size: 11.5px; font-weight: 600; color: #059669;");

    titleLayout->addWidget(nameLabel);
    titleLayout->addWidget(verLabel);
    titleLayout->addWidget(devLabel);
    brandLayout->addLayout(titleLayout, 1);
    mainLayout->addLayout(brandLayout);

    // Tech Specs Pill Badges
    auto* badgeLayout = new QHBoxLayout();
    badgeLayout->setSpacing(6);

    auto makeBadge = [this](const QString& text) {
        auto* b = new QLabel(text, this);
        b->setStyleSheet(
            "background-color: #EFF6FF; color: #1D4ED8; "
            "border: 1px solid #BFDBFE; border-radius: 4px; "
            "padding: 3px 6px; font-size: 10px; font-weight: 700;"
        );
        return b;
    };

    badgeLayout->addWidget(makeBadge("PJSIP 2.14.1"));
    badgeLayout->addWidget(makeBadge("Opus HD 48kHz"));
    badgeLayout->addWidget(makeBadge("Qt 6.8"));
    badgeLayout->addWidget(makeBadge("SQLite 3"));
    badgeLayout->addStretch();
    mainLayout->addLayout(badgeLayout);

    // Details Card
    auto* card = new QFrame(this);
    card->setStyleSheet("background-color: #FFFFFF; border: 1.2px solid #CBD5E1; border-radius: 6px; padding: 12px;");
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(8);

    auto* descLabel = new QLabel(
        tr("Cross-platform SIP VoIP softphone for macOS and Linux.\n\n"
           "Built with C++20 and Qt 6, using PJSIP 2.14.1 for SIP signaling and RTP media streaming. Features audio calling with Opus, G.711, and G.722 codecs, multi-account management, and local SQLite data storage."),
        this
    );
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 11.5px; line-height: 1.4; color: #475569; border: none;");
    cardLayout->addWidget(descLabel);

    mainLayout->addWidget(card);

    // Copyright
    auto* copyLabel = new QLabel("© 2026 GeSoftphone. Developed by linhnv2. All rights reserved.", this);
    copyLabel->setStyleSheet("font-size: 10.5px; color: #94A3B8;");
    copyLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(copyLabel);

    // Button
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    auto* closeBtn = new QPushButton(tr("Close"), this);
    closeBtn->setObjectName("closeBtn");
    closeBtn->setCursor(Qt::PointingHandCursor);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);
}
