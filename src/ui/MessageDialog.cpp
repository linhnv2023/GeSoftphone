#include "MessageDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QFrame>

MessageDialog::MessageDialog(const QString& recipient, QWidget* parent)
    : QDialog(parent)
    , m_recipient(recipient)
{
    setupUi(recipient);
}

void MessageDialog::setupUi(const QString& recipient)
{
    setWindowTitle(tr("Message - %1").arg(recipient));
    setModal(false);
    resize(380, 420);

    setStyleSheet(
        "QDialog {"
        "   background-color: #F8FAFC;"
        "   font-family: \"SF Pro Text\", \"Helvetica Neue\", sans-serif;"
        "}"
        "QLabel {"
        "   color: #334155;"
        "}"
        "QTextEdit {"
        "   background-color: #FFFFFF;"
        "   border: 1.2px solid #CBD5E1;"
        "   border-radius: 6px;"
        "   font-size: 12px;"
        "   padding: 8px;"
        "   color: #0F172A;"
        "}"
        "QLineEdit {"
        "   background-color: #FFFFFF;"
        "   border: 1.2px solid #CBD5E1;"
        "   border-radius: 6px;"
        "   padding: 6px 10px;"
        "   font-size: 12.5px;"
        "   color: #0F172A;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #3B82F6;"
        "}"
        "QPushButton#sendBtn {"
        "   background-color: #10B981;"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 6px;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   padding: 6px 14px;"
        "}"
        "QPushButton#sendBtn:hover {"
        "   background-color: #059669;"
        "}"
        "QPushButton#sendBtn:pressed {"
        "   background-color: #047857;"
        "}"
    );

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // Header
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    auto* avatar = new QLabel(this);
    avatar->setFixedSize(36, 36);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setText("💬");
    avatar->setStyleSheet("background-color: #ECFDF5; border: 1.5px solid #A7F3D0; border-radius: 18px; font-size: 16px;");
    headerLayout->addWidget(avatar);

    auto* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(1);
    auto* recipLabel = new QLabel(recipient, this);
    recipLabel->setStyleSheet("font-size: 14px; font-weight: 700; color: #0F172A;");
    auto* statusLabel = new QLabel(tr("SIP Instant Message Session"), this);
    statusLabel->setStyleSheet("font-size: 11px; color: #10B981; font-weight: 600;");
    titleLayout->addWidget(recipLabel);
    titleLayout->addWidget(statusLabel);
    headerLayout->addLayout(titleLayout, 1);
    mainLayout->addLayout(headerLayout);

    // Divider
    auto* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #E2E8F0; max-height: 1px; border: none;");
    mainLayout->addWidget(line);

    // Message History Area
    m_historyArea = new QTextEdit(this);
    m_historyArea->setReadOnly(true);
    m_historyArea->append("<span style='color: #94A3B8; font-size: 11px;'><i>" +
                          tr("Conversation started at %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss")) +
                          "</i></span><br>");
    mainLayout->addWidget(m_historyArea, 1);

    // Composer Row
    auto* composerLayout = new QHBoxLayout();
    composerLayout->setSpacing(8);

    m_msgInput = new QLineEdit(this);
    m_msgInput->setPlaceholderText(tr("Type message and press Enter..."));
    connect(m_msgInput, &QLineEdit::returnPressed, this, &MessageDialog::onSendClicked);
    composerLayout->addWidget(m_msgInput, 1);

    m_sendBtn = new QPushButton(tr("Send"), this);
    m_sendBtn->setObjectName("sendBtn");
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    connect(m_sendBtn, &QPushButton::clicked, this, &MessageDialog::onSendClicked);
    composerLayout->addWidget(m_sendBtn);

    mainLayout->addLayout(composerLayout);
    m_msgInput->setFocus();
}

void MessageDialog::onSendClicked()
{
    QString txt = m_msgInput->text().trimmed();
    if (txt.isEmpty()) return;

    QString timeStr = QDateTime::currentDateTime().toString("hh:mm");
    QString bubble = QString(
        "<div style='margin-bottom: 6px; text-align: right;'>"
        "  <span style='background-color: #EFF6FF; border: 1px solid #BFDBFE; border-radius: 8px; padding: 4px 8px; display: inline-block;'>"
        "    <b style='color: #1D4ED8;'>You:</b> <span style='color: #0F172A;'>%1</span>"
        "  </span>"
        "  <div style='font-size: 9px; color: #94A3B8; margin-top: 2px;'>%2</div>"
        "</div>"
    ).arg(txt.toHtmlEscaped(), timeStr);

    m_historyArea->append(bubble);
    m_msgInput->clear();
}
