#pragma once

#include <QObject>
#include <QTimer>
#include <QProcess>

class RingtoneManager : public QObject
{
    Q_OBJECT
public:
    explicit RingtoneManager(QObject* parent = nullptr);
    ~RingtoneManager() override;

    void startIncomingRing();
    void stopIncomingRing();
    void playHangupTone();
    void playMessageTone();

private slots:
    void onRingTimerTick();

private:
    void playSystemRing();

    QTimer* m_ringTimer = nullptr;
    QProcess* m_currentProcess = nullptr;
    bool m_isRinging = false;
};
