#pragma once

#include <string>

enum SipRegistrationState {
    REG_STATE_UNREGISTERED,
    REG_STATE_PROGRESS,
    REG_STATE_REGISTERED,
    REG_STATE_FAILED
};

enum SipCallState {
    CALL_STATE_NULL,
    CALL_STATE_CALLING,
    CALL_STATE_INCOMING,
    CALL_STATE_EARLY,
    CALL_STATE_CONNECTING,
    CALL_STATE_CONFIRMED,
    CALL_STATE_DISCONNECTED
};

struct SipAccountConfig {
    std::string id;
    std::string label;
    bool isActive = false;
    bool isSaved = false;
    std::string server;
    std::string proxy;
    std::string username;
    std::string domain;
    int port = 5060;
    std::string authID;
    std::string password;
    std::string displayName;
    std::string transport = "UDP"; // "UDP", "TCP", "TLS"
    std::string stunServer = "stun.l.google.com:19302";
    bool enableStun = true;
    bool echoCancellation = true;
    bool vad = true;
    int registerRefresh = 300;
    int keepAlive = 15;
    bool publish = false;
    bool ice = false;
    bool srtp = false;
    
    // Per-account settings
    int encryption = 0;
    int publicAddress = 0;
    std::string voicemail;
    std::string dialPrefix;
    std::string dialPlan;
    bool hideCallerId = false;
    bool allowIpRewrite = true;
    bool disableSessionTimers = false;

    // Call Quality & Network Optimizations
    bool enableQoS = true;              // DSCP Voice Tagging (EF/CS3)
    bool enableAdaptiveJitterBuffer = true; // Dynamic Jitter Buffer
    int jbMin = 40;                     // Min delay 40ms
    int jbInit = 60;                    // Initial buffer 60ms
    int jbMax = 350;                    // Max buffer 350ms
    bool enableOpusFec = true;          // Opus Forward Error Correction
    int captureDevId = -1;              // Default mic
    int playbackDevId = -1;             // Default speaker
};

struct AudioDeviceInfo {
    int id = -1;
    std::string name;
    int inputChannels = 0;
    int outputChannels = 0;
    int defaultSampleRate = 16000;
};

struct SipCallInfo {
    int callId = -1;
    SipCallState state = CALL_STATE_NULL;
    std::string remoteUri;
    std::string remoteNumber;
    std::string remoteName;
    int duration = 0;
    int lastStatusCode = 0;
    std::string lastReason;
    bool hasVideo = false;
    bool isIncoming = false;

    // Real-time Call Quality Stats
    float packetLossPercent = 0.0f;
    unsigned jitterMs = 0;
    unsigned rttMs = 0;
    unsigned rxLevel = 0;
    unsigned txLevel = 0;
};

