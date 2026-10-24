#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>

class TransferDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransferDialog(const QString& activeCaller, QWidget* parent = nullptr);

    QString getTargetExtension() const;
    bool isAttendedTransfer() const;

private slots:
    void onTransferClicked();

private:
    void setupUi(const QString& activeCaller);

    QLineEdit* m_targetInput{nullptr};
    QRadioButton* m_blindRadio{nullptr};
    QRadioButton* m_attendedRadio{nullptr};
    QLabel* m_errorLabel{nullptr};
};
