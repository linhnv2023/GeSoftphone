#include "RingtoneManager.h"
#include <QApplication>
#include <QFileInfo>

RingtoneManager::RingtoneManager(QObject* parent)
    : QObject(parent)
{
    m_ringTimer = new QTimer(this);
    m_ringTimer->setInterval(2200); // 2.2s repeat cadence
    connect(m_ringTimer, &QTimer::timeout, this, &RingtoneManager::onRingTimerTick);
}

RingtoneManager::~RingtoneManager()
{
    stopIncomingRing();
}

void RingtoneManager::startIncomingRing()
{
    if (m_isRinging) return;
    m_isRinging = true;
    playSystemRing();
    m_ringTimer->start();
}

void RingtoneManager::stopIncomingRing()
{
    if (!m_isRinging) return;
    m_isRinging = false;
    m_ringTimer->stop();

    if (m_currentProcess) {
        if (m_currentProcess->state() != QProcess::NotRunning) {
            m_currentProcess->kill();
            m_currentProcess->waitForFinished(50);
        }
        delete m_currentProcess;
        m_currentProcess = nullptr;
    }
}

void RingtoneManager::onRingTimerTick()
{
    if (m_isRinging) {
        playSystemRing();
    }
}

void RingtoneManager::playSystemRing()
{
#if defined(Q_OS_MACOS)
    // Clean audio playback via macOS afplay
    if (m_currentProcess) {
        if (m_currentProcess->state() != QProcess::NotRunning) {
            m_currentProcess->kill();
        }
        delete m_currentProcess;
        m_currentProcess = nullptr;
    }

    const QString soundPath = "/System/Library/Sounds/Ping.aiff";
    if (QFileInfo::exists(soundPath)) {
        m_currentProcess = new QProcess(this);
        m_currentProcess->start("/usr/bin/afplay", QStringList{soundPath});
    } else {
        QApplication::beep();
    }
#elif defined(Q_OS_LINUX)
    // Linux standard desktop sound via canberra-gtk-play or paplay
    if (m_currentProcess) {
        if (m_currentProcess->state() != QProcess::NotRunning) {
            m_currentProcess->kill();
        }
        delete m_currentProcess;
        m_currentProcess = nullptr;
    }
    m_currentProcess = new QProcess(this);
    m_currentProcess->start("sh", QStringList{"-c", "canberra-gtk-play -i phone-incoming-call 2>/dev/null || paplay /usr/share/sounds/freedesktop/stereo/phone-incoming-call.oga 2>/dev/null"});
#else
    QApplication::beep();
#endif
}

void RingtoneManager::playHangupTone()
{
#if defined(Q_OS_MACOS)
    QProcess::startDetached("/usr/bin/afplay", QStringList{"/System/Library/Sounds/Pop.aiff"});
#elif defined(Q_OS_LINUX)
    QProcess::startDetached("sh", QStringList{"-c", "canberra-gtk-play -i network-disconnected 2>/dev/null"});
#endif
}

void RingtoneManager::playMessageTone()
{
#if defined(Q_OS_MACOS)
    QProcess::startDetached("/usr/bin/afplay", QStringList{"/System/Library/Sounds/Tink.aiff"});
#elif defined(Q_OS_LINUX)
    QProcess::startDetached("sh", QStringList{"-c", "canberra-gtk-play -i message-new-instant 2>/dev/null"});
#endif
}
