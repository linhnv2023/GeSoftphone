#pragma once

#include "SipTypes.h"
#include <memory>
#include <string>
#include <vector>


class ISipEngineListener
{
public:
    virtual ~ISipEngineListener() = default;
    virtual void onRegistrationStateChanged(SipRegistrationState state, int code, const std::string& reason) = 0;
    virtual void onCallStateChanged(const SipCallInfo& info) = 0;
    virtual void onIncomingCall(const SipCallInfo& info) = 0;
    virtual void onCallMediaState(int callId) = 0;
};

class SipEngine
{
public:
    SipEngine();
    ~SipEngine();

    bool init();
    void destroy();
    bool isRunning() const;

    bool registerAccount(const SipAccountConfig& config);
    void unregisterAccount();

    int makeCall(const std::string& destination, bool hasVideo = false);
    bool answerCall(int callId);
    bool hangupCall(int callId);
    bool holdCall(int callId);
    bool unholdCall(int callId);
    bool sendDtmf(int callId, const std::string& digits);
    bool transferCall(int callId, const std::string& destination);

    bool muteMicrophone(bool mute);
    bool isMicrophoneMuted() const;
    bool setSpeakerVolume(float level);
    bool setMicVolume(float level);

    // Call Quality & Hardware Management
    bool getCallQualityStats(int callId, float& packetLossPercent, unsigned& jitterMs, unsigned& rttMs);
    bool getAudioLevels(int callId, unsigned& rxLevel, unsigned& txLevel);
    std::vector<AudioDeviceInfo> getAudioDevices();
    bool setAudioDevices(int playbackDevId, int captureDevId);
    bool handleIpChange();

    void setListener(ISipEngineListener* listener);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

