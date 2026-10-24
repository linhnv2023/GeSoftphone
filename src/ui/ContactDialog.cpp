#include "ContactDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>

ContactDialog::ContactDialog(QWidget* parent)
    : QDialog(parent)
    , m_isEditMode(false)
{
    setupUi(false);
}

ContactDialog::ContactDialog(const Contact& contact, QWidget* parent)
    : QDialog(parent)
    , m_isEditMode(true)
    , m_contactId(contact.id)
{
    setupUi(true);
    m_nameInput->setText(QString::fromStdString(contact.name));
    m_numberInput->setText(QString::fromStdString(contact.number));
    m_emailInput->setText(QString::fromStdString(contact.email));
}

void ContactDialog::setupUi(bool isEdit)
{
    setWindowTitle(isEdit ? tr("Edit Contact") : tr("Add Contact"));
    setModal(true);
    setFixedSize(380, 340);

    setStyleSheet(
        "QDialog {"
        "   background-color: #F8FAFC;"
        "   font-family: \"SF Pro Text\", \"Helvetica Neue\", sans-serif;"
        "}"
        "QLabel {"
        "   color: #334155;"
        "   font-size: 11.5px;"
        "   font-weight: 600;"
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
        "   background-color: #FFFFFF;"
        "}"
        "QPushButton#saveBtn {"
        "   background-color: #3B82F6;"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 6px;"
        "   font-size: 12px;"
        "   font-weight: 700;"
        "   padding: 7px 18px;"
        "}"
        "QPushButton#saveBtn:hover {"
        "   background-color: #2563EB;"
        "}"
        "QPushButton#saveBtn:pressed {"
        "   background-color: #1D4ED8;"
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

    // Header Card
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(12);

    auto* avatarLabel = new QLabel(this);
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setAlignment(Qt::AlignCenter);
    avatarLabel->setText(isEdit ? "✏️" : "👤");
    avatarLabel->setStyleSheet("background-color: #EFF6FF; border: 1.5px solid #BFDBFE; border-radius: 20px; font-size: 18px;");
    headerLayout->addWidget(avatarLabel);

    auto* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    auto* titleLabel = new QLabel(isEdit ? tr("Edit Contact Details") : tr("New Contact"), this);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: 700; color: #0F172A;");
    auto* subLabel = new QLabel(tr("Save contact name and SIP phone extension."), this);
    subLabel->setStyleSheet("font-size: 11px; font-weight: 400; color: #64748B;");
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subLabel);
    headerLayout->addLayout(titleLayout, 1);
    mainLayout->addLayout(headerLayout);

    // Divider
    auto* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #E2E8F0; max-height: 1px; border: none;");
    mainLayout->addWidget(line);

    // Form
    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(10);
    formLayout->setLabelAlignment(Qt::AlignLeft);

    m_nameInput = new QLineEdit(this);
    m_nameInput->setPlaceholderText(tr("e.g. Alice Smith"));
    formLayout->addRow(tr("Full Name:"), m_nameInput);

    m_numberInput = new QLineEdit(this);
    m_numberInput->setPlaceholderText(tr("e.g. 1001 or sip:alice@domain.com"));
    formLayout->addRow(tr("Phone / SIP URI:"), m_numberInput);

    m_emailInput = new QLineEdit(this);
    m_emailInput->setPlaceholderText(tr("e.g. alice@company.com (optional)"));
    formLayout->addRow(tr("Email Address:"), m_emailInput);

    mainLayout->addLayout(formLayout);

    // Error label
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #EF4444; font-size: 11px; font-weight: 600;");
    m_errorLabel->hide();
    mainLayout->addWidget(m_errorLabel);

    mainLayout->addStretch();

    // Action buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);
    btnLayout->addStretch();

    auto* cancelBtn = new QPushButton(tr("Cancel"), this);
    cancelBtn->setObjectName("cancelBtn");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    auto* saveBtn = new QPushButton(isEdit ? tr("Update") : tr("Save"), this);
    saveBtn->setObjectName("saveBtn");
    saveBtn->setCursor(Qt::PointingHandCursor);
    connect(saveBtn, &QPushButton::clicked, this, &ContactDialog::onSaveClicked);
    btnLayout->addWidget(saveBtn);

    mainLayout->addLayout(btnLayout);
}

void ContactDialog::onSaveClicked()
{
    QString name = m_nameInput->text().trimmed();
    QString num = m_numberInput->text().trimmed();

    if (name.isEmpty()) {
        m_errorLabel->setText(tr("Please provide a contact name."));
        m_errorLabel->show();
        m_nameInput->setFocus();
        return;
    }

    if (num.isEmpty()) {
        m_errorLabel->setText(tr("Please enter a phone number or SIP URI."));
        m_errorLabel->show();
        m_numberInput->setFocus();
        return;
    }

    accept();
}

Contact ContactDialog::getContact() const
{
    Contact c;
    c.id = m_contactId;
    c.name = m_nameInput->text().trimmed().toStdString();
    c.number = m_numberInput->text().trimmed().toStdString();
    c.email = m_emailInput->text().trimmed().toStdString();
    return c;
}
