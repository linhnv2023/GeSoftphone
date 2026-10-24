#pragma once

#include <QObject>
#include "SipEngine.h"

class SipBridge : public QObject, public ISipEngineListener
{
    Q_OBJECT
public:
    explicit SipBridge(SipEngine& engine, QObject* parent = nullptr);
    ~SipBridge() override;

    // ISipEngineListener implementation
    void onRegistrationStateChanged(SipRegistrationState state, int code, const std::string& reason) override;
    void onCallStateChanged(const SipCallInfo& info) override;
    void onIncomingCall(const SipCallInfo& info) override;
    void onCallMediaState(int callId) override;

signals:
    void registrationChanged(int state, int code, const QString& reason);
    void callStateChanged(int callId, int state, const QString& remoteUri, int duration, const QString& reason, int statusCode = 0);
    void incomingCall(int callId, const QString& remoteUri);
    void mediaStateChanged(int callId);

private:
    SipEngine& m_engine;
};
