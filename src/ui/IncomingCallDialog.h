#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>

class IncomingCallDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IncomingCallDialog(const QString& callerNumber, const QString& callerName, QWidget* parent = nullptr);

    bool wasAnswered() const { return m_answered; }

private slots:
    void onAnswer();
    void onDecline();

private:
    void setupUi();

    QString m_callerNumber;
    QString m_callerName;
    bool m_answered = false;
};
