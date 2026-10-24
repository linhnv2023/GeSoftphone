#include "TransferDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QFrame>

TransferDialog::TransferDialog(const QString& activeCaller, QWidget* parent)
    : QDialog(parent)
{
    setupUi(activeCaller);
}

void TransferDialog::setupUi(const QString& activeCaller)
{
    setWindowTitle(tr("Transfer Active Call"));
    setModal(true);
    setFixedSize(380, 290);

    setStyleSheet(
        "QDialog {"
        "   background-color: #F8FAFC;"
        "   font-family: \"SF Pro Text\", \"Helvetica Neue\", sans-serif;"
        "}"
        "QLabel {"
        "   color: #334155;"
        "   font-size: 11.5px;"
        "}"
        "QLineEdit {"
        "   background-color: #FFFFFF;"
        "   border: 1.2px solid #CBD5E1;"
        "   border-radius: 6px;"
        "   padding: 6px 10px;"
        "   font-size: 13px;"
        "   color: #0F172A;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #3B82F6;"
        "}"
        "QRadioButton {"
        "   font-size: 11.5px;"
        "   color: #334155;"
        "   font-weight: 500;"
        "}"
        "QPushButton#transferBtn {"
        "   background-color: #3B82F6;"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 6px;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   padding: 7px 18px;"
        "}"
        "QPushButton#transferBtn:hover {"
        "   background-color: #2563EB;"
        "}"
        "QPushButton#cancelBtn {"
        "   background-color: #F1F5F9;"
        "   color: #475569;"
        "   border: 1px solid #CBD5E1;"
        "   border-radius: 6px;"
        "   font-size: 12px;"
        "   font-weight: 600;"
        "   padding: 7px 14px;"
        "}"
        "QPushButton#cancelBtn:hover {"
        "   background-color: #E2E8F0;"
        "}"
    );

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 18, 20, 18);
    mainLayout->setSpacing(14);

    // Header with call badge
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    auto* iconLabel = new QLabel(this);
    iconLabel->setFixedSize(36, 36);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setText("↪️");
    iconLabel->setStyleSheet("background-color: #EFF6FF; border: 1.5px solid #BFDBFE; border-radius: 18px; font-size: 16px;");
    headerLayout->addWidget(iconLabel);

    auto* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    auto* titleLabel = new QLabel(tr("Call Transfer"), this);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: 700; color: #0F172A;");
    auto* callerLabel = new QLabel(tr("Active call with: %1").arg(activeCaller.isEmpty() ? tr("Unknown") : activeCaller), this);
    callerLabel->setStyleSheet("font-size: 11px; color: #64748B; font-weight: 500;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(callerLabel);
    headerLayout->addLayout(titleLayout, 1);
    mainLayout->addLayout(headerLayout);

    // Divider
    auto* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #E2E8F0; max-height: 1px; border: none;");
    mainLayout->addWidget(line);

    // Destination Input
    auto* inputLabel = new QLabel(tr("Destination Extension or SIP URI:"), this);
    inputLabel->setStyleSheet("font-weight: 600; color: #1E293B;");
    mainLayout->addWidget(inputLabel);

    m_targetInput = new QLineEdit(this);
    m_targetInput->setPlaceholderText(tr("e.g. 1002 or sip:sales@domain.com"));
    mainLayout->addWidget(m_targetInput);

    // Transfer Mode Selection
    auto* modeLayout = new QHBoxLayout();
    modeLayout->setSpacing(16);

    m_blindRadio = new QRadioButton(tr("Blind (Direct)"), this);
    m_blindRadio->setChecked(true);
    m_blindRadio->setToolTip(tr("Transfers call immediately without speaking to destination first"));

    m_attendedRadio = new QRadioButton(tr("Attended (Consult)"), this);
    m_attendedRadio->setToolTip(tr("Puts caller on hold and dials destination first"));

    modeLayout->addWidget(m_blindRadio);
    modeLayout->addWidget(m_attendedRadio);
    modeLayout->addStretch();
    mainLayout->addLayout(modeLayout);

    // Error label
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #EF4444; font-size: 11px; font-weight: 600;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    mainLayout->addStretch();

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    auto* cancelBtn = new QPushButton(tr("Cancel"), this);
    cancelBtn->setObjectName("cancelBtn");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    auto* transferBtn = new QPushButton(tr("Transfer Now"), this);
    transferBtn->setObjectName("transferBtn");
    transferBtn->setCursor(Qt::PointingHandCursor);
    connect(transferBtn, &QPushButton::clicked, this, &TransferDialog::onTransferClicked);
    btnLayout->addWidget(transferBtn);

    mainLayout->addLayout(btnLayout);
    m_targetInput->setFocus();
}

void TransferDialog::onTransferClicked()
{
    if (m_targetInput->text().trimmed().isEmpty()) {
        m_errorLabel->setText(tr("Please enter a destination extension."));
        m_errorLabel->show();
        m_targetInput->setFocus();
        return;
    }
    accept();
}

QString TransferDialog::getTargetExtension() const
{
    return m_targetInput->text().trimmed();
}

bool TransferDialog::isAttendedTransfer() const
{
    return m_attendedRadio->isChecked();
}
