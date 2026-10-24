#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>

class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageDialog(const QString& recipient, QWidget* parent = nullptr);

private slots:
    void onSendClicked();

private:
    void setupUi(const QString& recipient);

    QString m_recipient;
    QTextEdit* m_historyArea{nullptr};
    QLineEdit* m_msgInput{nullptr};
    QPushButton* m_sendBtn{nullptr};
};
