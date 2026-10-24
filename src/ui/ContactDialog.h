#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include "domain/Contact.h"

class ContactDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ContactDialog(QWidget* parent = nullptr);
    explicit ContactDialog(const Contact& contact, QWidget* parent = nullptr);

    Contact getContact() const;

private slots:
    void onSaveClicked();

private:
    void setupUi(bool isEdit);

    QLineEdit* m_nameInput{nullptr};
    QLineEdit* m_numberInput{nullptr};
    QLineEdit* m_emailInput{nullptr};
    QLabel* m_errorLabel{nullptr};

    std::string m_contactId;
    bool m_isEditMode{false};
};
