#include "SipBridge.h"

SipBridge::SipBridge(SipEngine& engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine)
{
    m_engine.setListener(this);
}

SipBridge::~SipBridge()
{
    m_engine.setListener(nullptr);
}

void SipBridge::onRegistrationStateChanged(SipRegistrationState state, int code, const std::string& reason)
{
    emit registrationChanged(static_cast<int>(state), code, QString::fromStdString(reason));
}

void SipBridge::onCallStateChanged(const SipCallInfo& info)
{
    emit callStateChanged(
        info.callId,
        static_cast<int>(info.state),
        QString::fromStdString(info.remoteUri),
        info.duration,
        QString::fromStdString(info.lastReason),
        info.lastStatusCode
    );
}

void SipBridge::onIncomingCall(const SipCallInfo& info)
{
    emit incomingCall(info.callId, QString::fromStdString(info.remoteUri));
}

void SipBridge::onCallMediaState(int callId)
{
    emit mediaStateChanged(callId);
}
