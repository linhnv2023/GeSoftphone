#include "SipEngine.h"
#include <pjsua-lib/pjsua.h>
#include <iostream>
#include <mutex>

static ISipEngineListener* g_listener = nullptr;
static std::mutex g_listenerMutex;

static void on_pjsip_log(int level, const char *data, int len)
{
    PJ_UNUSED_ARG(level);
    static FILE* f = nullptr;
    static bool attempted = false;
    if (!f && !attempted) {
        attempted = true;
        const char* home = getenv("HOME");
        if (home) {
            std::string logPath = std::string(home) + "/Library/Application Support/GE/GeSoftphone/pjsip.log";
            f = fopen(logPath.c_str(), "a");
        }
    }
    if (f) {
        fwrite(data, 1, len, f);
        fflush(f);
    }
}

// Static PJSIP callbacks
static void on_reg_state2(pjsua_acc_id acc_id, pjsua_reg_info* info)
{
    std::lock_guard<std::mutex> lock(g_listenerMutex);
    if (!g_listener) return;

    SipRegistrationState state = REG_STATE_UNREGISTERED;
    int code = 0;
    std::string reason;

    if (info && info->cbparam) {
        code = info->cbparam->code;
        if (info->cbparam->reason.slen > 0) {
            reason = std::string(info->cbparam->reason.ptr, info->cbparam->reason.slen);
        }
        if (info->cbparam->code / 100 == 2) {
            state = REG_STATE_REGISTERED;
        } else if (info->cbparam->code >= 400) {
            state = REG_STATE_FAILED;
        } else {
            state = REG_STATE_PROGRESS;
        }
    }

    if (g_listener) {
        try {
            g_listener->onRegistrationStateChanged(state, code, reason);
        } catch (...) {}
    }
}

static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data* rdata)
{
    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    std::lock_guard<std::mutex> lock(g_listenerMutex);
    if (!g_listener) return;

    pjsua_call_info ci;
    if (pjsua_call_get_info(call_id, &ci) == PJ_SUCCESS) {
        SipCallInfo info;
        info.callId = call_id;
        info.state = CALL_STATE_INCOMING;
        info.isIncoming = true;
        if (ci.remote_info.slen > 0) {
            info.remoteUri = std::string(ci.remote_info.ptr, ci.remote_info.slen);
        }
        try {
            g_listener->onIncomingCall(info);
        } catch (...) {}
    }
}

static void on_call_state(pjsua_call_id call_id, pjsip_event* e)
{
    PJ_UNUSED_ARG(e);

    std::lock_guard<std::mutex> lock(g_listenerMutex);
    if (!g_listener) return;

    pjsua_call_info ci;
    if (pjsua_call_get_info(call_id, &ci) == PJ_SUCCESS) {
        SipCallInfo info;
        info.callId = call_id;
        info.lastStatusCode = ci.last_status;
        if (ci.last_status_text.slen > 0) {
            info.lastReason = std::string(ci.last_status_text.ptr, ci.last_status_text.slen);
        }
        if (ci.remote_info.slen > 0) {
            info.remoteUri = std::string(ci.remote_info.ptr, ci.remote_info.slen);
        }
        info.duration = ci.connect_duration.sec;

        switch (ci.state) {
            case PJSIP_INV_STATE_NULL:
                info.state = CALL_STATE_NULL;
                break;
            case PJSIP_INV_STATE_CALLING:
                info.state = CALL_STATE_CALLING;
                break;
            case PJSIP_INV_STATE_INCOMING:
                info.state = CALL_STATE_INCOMING;
                break;
            case PJSIP_INV_STATE_EARLY:
                info.state = CALL_STATE_EARLY;
                break;
            case PJSIP_INV_STATE_CONNECTING:
                info.state = CALL_STATE_CONNECTING;
                break;
            case PJSIP_INV_STATE_CONFIRMED:
                info.state = CALL_STATE_CONFIRMED;
                break;
            case PJSIP_INV_STATE_DISCONNECTED:
                info.state = CALL_STATE_DISCONNECTED;
                break;
        }

        try {
            g_listener->onCallStateChanged(info);
        } catch (...) {}
    }
}

static void on_call_media_state(pjsua_call_id call_id)
{
    pjsua_call_info ci;
    if (pjsua_call_get_info(call_id, &ci) == PJ_SUCCESS) {
        if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE && ci.conf_slot != PJSUA_INVALID_ID) {
            // Connect the call audio stream to sound devices
            pjsua_conf_connect(ci.conf_slot, 0);
            pjsua_conf_connect(0, ci.conf_slot);
        }
    }

    std::lock_guard<std::mutex> lock(g_listenerMutex);
    if (g_listener) {
        try {
            g_listener->onCallMediaState(call_id);
        } catch (...) {}
    }
}

// PIMPL implementation
struct SipEngine::Impl {
    bool running = false;
    bool muted = false;
    pjsua_acc_id accId = PJSUA_INVALID_ID;
    pjsua_transport_id transportId = PJSUA_INVALID_ID;
    SipAccountConfig currentConfig;
};

SipEngine::SipEngine()
    : m_impl(std::make_unique<Impl>())
{
}

SipEngine::~SipEngine()
{
    destroy();
}

bool SipEngine::init()
{
    if (m_impl->running) return true;

    pj_status_t status = pjsua_create();
    if (status != PJ_SUCCESS) return false;

    pjsua_config ua_cfg;
    pjsua_config_default(&ua_cfg);
    ua_cfg.cb.on_reg_state2 = &on_reg_state2;
    ua_cfg.cb.on_incoming_call = &on_incoming_call;
    ua_cfg.cb.on_call_state = &on_call_state;
    ua_cfg.cb.on_call_media_state = &on_call_media_state;
    ua_cfg.user_agent = pj_str(const_cast<char*>("GE-Softphone/1.0"));

    // Configure STUN Server for NAT traversal
    static char stunServer[] = "stun.l.google.com:19302";
    ua_cfg.stun_srv[0] = pj_str(stunServer);
    ua_cfg.stun_srv_cnt = 1;

    pjsua_logging_config log_cfg;
    pjsua_logging_config_default(&log_cfg);
    log_cfg.level = 4;
    log_cfg.console_level = 4;
    log_cfg.msg_logging = PJ_TRUE;
    log_cfg.cb = &on_pjsip_log;

    pjsua_media_config media_cfg;
    pjsua_media_config_default(&media_cfg);
    media_cfg.clock_rate = 48000; // 48kHz Fullband Studio Quality
    media_cfg.channel_count = 1;
    media_cfg.ec_tail_len = 200; // 200ms Acoustic Echo Cancellation (AEC)
    media_cfg.no_vad = PJ_FALSE; // Voice Activity Detection (VAD)

    // 1. Adaptive Jitter Buffer Optimization (Prevents audio dropouts and robot voice on Wi-Fi/LTE)
    media_cfg.jb_init = 60;      // 60ms initial buffer
    media_cfg.jb_min_pre = 40;   // 40ms minimum delay
    media_cfg.jb_max_pre = 240;  // 240ms max prefetch buffer
    media_cfg.jb_max = 350;      // 350ms absolute max size

    // 2. Audio Device Latency tuning (Prevents audio underruns/clicks on CoreAudio & ALSA/Pulse)
    media_cfg.snd_rec_latency = 100;
    media_cfg.snd_play_latency = 120;
    media_cfg.snd_auto_close_time = 1;

    status = pjsua_init(&ua_cfg, &log_cfg, &media_cfg);
    if (status != PJ_SUCCESS) {
        pjsua_destroy();
        return false;
    }

    pjsua_transport_config trans_cfg;
    pjsua_transport_config_default(&trans_cfg);
    trans_cfg.port = 5060;
    trans_cfg.qos_type = PJ_QOS_TYPE_CONTROL; // DSCP CS3 / AF31 for SIP Signaling

    status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &trans_cfg, &m_impl->transportId);
    if (status != PJ_SUCCESS) {
        // Try random port
        trans_cfg.port = 0;
        pjsua_transport_create(PJSIP_TRANSPORT_UDP, &trans_cfg, &m_impl->transportId);
    }

    // Enable TCP transport alongside UDP for firewall/NAT bypass
    pjsua_transport_id tcp_id = PJSUA_INVALID_ID;
    trans_cfg.port = 0;
    pjsua_transport_create(PJSIP_TRANSPORT_TCP, &trans_cfg, &tcp_id);

    status = pjsua_start();
    if (status != PJ_SUCCESS) {
        pjsua_destroy();
        return false;
    }

    // Disable automatic RFC 3261 switch from UDP to TCP for requests > 1300 bytes.
    // Asterisk PBX at port 25060 listens strictly on UDP; switching to TCP causes
    // the authenticated INVITE to fail with connection timeout!
    pjsip_cfg()->endpt.disable_tcp_switch = PJ_TRUE;

    // 4. Codec Priority & In-band Forward Error Correction (FEC)
    // First, disable ALL codecs so SDP stays compact and never exceeds UDP MTU (1500 bytes).
    // Excessive codecs (Speex, GSM, iLBC, AMR) cause SIP INVITE packets to exceed 1500 bytes,
    // resulting in IP fragmentation that routers and carrier NATs silently drop!
    pjsua_codec_info codec_info[64];
    unsigned codec_count = 64;
    if (pjsua_enum_codecs(codec_info, &codec_count) == PJ_SUCCESS) {
        for (unsigned i = 0; i < codec_count; ++i) {
            pjsua_codec_set_priority(&codec_info[i].codec_id, PJMEDIA_CODEC_PRIO_DISABLED);
        }
    }

    // Prioritize HD Opus
    pj_str_t codec_opus = pj_str(const_cast<char*>("opus/48000/2"));
    pjsua_codec_set_priority(&codec_opus, 255);

    pjmedia_codec_param param;
    if (pjsua_codec_get_param(&codec_opus, &param) == PJ_SUCCESS) {
        bool hasFec = false;
        for (int i = 0; i < param.setting.dec_fmtp.cnt; ++i) {
            if (pj_strcmp2(&param.setting.dec_fmtp.param[i].name, "useinbandfec") == 0) {
                param.setting.dec_fmtp.param[i].val = pj_str(const_cast<char*>("1"));
                hasFec = true;
                break;
            }
        }
        if (!hasFec && param.setting.dec_fmtp.cnt < PJ_ARRAY_SIZE(param.setting.dec_fmtp.param)) {
            param.setting.dec_fmtp.param[param.setting.dec_fmtp.cnt].name = pj_str(const_cast<char*>("useinbandfec"));
            param.setting.dec_fmtp.param[param.setting.dec_fmtp.cnt].val = pj_str(const_cast<char*>("1"));
            param.setting.dec_fmtp.cnt++;
        }
        param.info.avg_bps = 32000; // 32kbps HD voice
        pjsua_codec_set_param(&codec_opus, &param);
    }

    // Standard G.711a (PCMA - Vietnam & European PSTN standard)
    pj_str_t codec_pcma = pj_str(const_cast<char*>("PCMA/8000/1"));
    pjsua_codec_set_priority(&codec_pcma, 220);

    // Standard G.711u (PCMU - Asterisk standard)
    pj_str_t codec_pcmu = pj_str(const_cast<char*>("PCMU/8000/1"));
    pjsua_codec_set_priority(&codec_pcmu, 210);

    // Wideband G.722
    pj_str_t codec_g722 = pj_str(const_cast<char*>("G722/16000/1"));
    pjsua_codec_set_priority(&codec_g722, 200);

    m_impl->running = true;
    return true;
}

void SipEngine::destroy()
{
    if (m_impl->running) {
        unregisterAccount();
        pjsua_destroy();
        m_impl->running = false;
    }
}

bool SipEngine::isRunning() const
{
    return m_impl->running;
}

bool SipEngine::registerAccount(const SipAccountConfig& config)
{
    if (!m_impl->running) return false;

    unregisterAccount();
    m_impl->currentConfig = config;

    pjsua_acc_config acc_cfg;
    pjsua_acc_config_default(&acc_cfg);

    std::string domain = !config.domain.empty() ? config.domain : config.server;
    std::string server = !config.server.empty() ? config.server : config.domain;

    std::string idUri;
    if (!config.displayName.empty()) {
        idUri = "\"" + config.displayName + "\" <sip:" + config.username + "@" + domain + ">";
    } else {
        idUri = "<sip:" + config.username + "@" + domain + ">";
    }

    std::string regUri = "sip:" + server;
    if (config.transport == "TCP") {
        regUri += ";transport=tcp";
    } else if (config.transport == "TLS") {
        regUri += ";transport=tls";
    }

    acc_cfg.id = pj_str(const_cast<char*>(idUri.c_str()));
    acc_cfg.reg_uri = pj_str(const_cast<char*>(regUri.c_str()));
    acc_cfg.reg_timeout = (config.registerRefresh > 0) ? config.registerRefresh : 300;

    std::string proxyUri;
    if (!config.proxy.empty()) {
        proxyUri = config.proxy;
        if (proxyUri.find("sip:") != 0 && proxyUri.find("sips:") != 0) {
            proxyUri = "sip:" + proxyUri;
        }
        // RFC 3261 Loose Routing parameter is mandatory for modern SIP proxies!
        // Without ;lr, PJSIP applies RFC 2543 strict routing which replaces the
        // dialed number with the proxy address, preventing the PBX from receiving the call!
        if (proxyUri.find(";lr") == std::string::npos) {
            proxyUri += ";lr";
        }
        if (config.transport == "TCP" && proxyUri.find("transport=") == std::string::npos) {
            proxyUri += ";transport=tcp";
        } else if (config.transport == "TLS" && proxyUri.find("transport=") == std::string::npos) {
            proxyUri += ";transport=tls";
        }
        acc_cfg.proxy_cnt = 1;
        acc_cfg.proxy[0] = pj_str(const_cast<char*>(proxyUri.c_str()));
    }

    // NAT Traversal & Keep-Alive Enhancement
    acc_cfg.allow_contact_rewrite = 2; // PJSUA_CONTACT_REWRITE_ALWAYS_UPDATE
    acc_cfg.contact_rewrite_method = PJSUA_CONTACT_REWRITE_ALWAYS_UPDATE | PJSUA_CONTACT_REWRITE_UNREGISTER;
    acc_cfg.allow_via_rewrite = PJ_TRUE;
    acc_cfg.allow_sdp_nat_rewrite = PJ_TRUE;
    acc_cfg.ka_interval = (config.keepAlive > 0) ? config.keepAlive : 15;
    acc_cfg.ka_data = pj_str(const_cast<char*>("\r\n\r\n"));

    // Media Encryption (SRTP)
    if (config.encryption == 1) {
        acc_cfg.use_srtp = PJMEDIA_SRTP_OPTIONAL;
        acc_cfg.srtp_secure_signaling = 0;
    } else if (config.encryption == 2) {
        acc_cfg.use_srtp = PJMEDIA_SRTP_MANDATORY;
        acc_cfg.srtp_secure_signaling = 0;
    } else {
        acc_cfg.use_srtp = PJMEDIA_SRTP_DISABLED;
    }

    // ICE (Interactive Connectivity Establishment)
    acc_cfg.ice_cfg.enable_ice = config.ice ? PJ_TRUE : PJ_FALSE;

    // Session Timers
    if (config.disableSessionTimers) {
        acc_cfg.timer_setting.min_se = 0;
        acc_cfg.timer_setting.sess_expires = 0;
    }

    if (config.transport == "UDP" && m_impl->transportId != PJSUA_INVALID_ID) {
        acc_cfg.transport_id = m_impl->transportId;
    }

    // Network resilience
    acc_cfg.reg_retry_interval = 30;
    acc_cfg.reg_first_retry_interval = 5;

    acc_cfg.cred_count = 1;
    acc_cfg.cred_info[0].realm = pj_str(const_cast<char*>("*"));
    acc_cfg.cred_info[0].scheme = pj_str(const_cast<char*>("digest"));
    acc_cfg.cred_info[0].username = pj_str(const_cast<char*>(
        config.authID.empty() ? config.username.c_str() : config.authID.c_str()
    ));
    acc_cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    acc_cfg.cred_info[0].data = pj_str(const_cast<char*>(config.password.c_str()));

    pj_status_t status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &m_impl->accId);
    return (status == PJ_SUCCESS);
}

void SipEngine::unregisterAccount()
{
    if (m_impl->accId != PJSUA_INVALID_ID) {
        pjsua_acc_del(m_impl->accId);
        m_impl->accId = PJSUA_INVALID_ID;
    }
    m_impl->currentConfig = SipAccountConfig();
}

int SipEngine::makeCall(const std::string& destination, bool hasVideo)
{
    if (!m_impl->running || m_impl->accId == PJSUA_INVALID_ID) return -1;

    std::string target = destination;
    while (!target.empty() && isspace(static_cast<unsigned char>(target.front()))) target.erase(0, 1);
    while (!target.empty() && isspace(static_cast<unsigned char>(target.back()))) target.pop_back();
    if (target.empty()) return -1;

    std::string fullUri;
    if (target.find("sip:") == 0 || target.find("sips:") == 0) {
        fullUri = target;
    } else {
        fullUri = "sip:" + target;
    }

    // Append registered domain/server if target has no '@' host
    if (fullUri.find('@') == std::string::npos) {
        std::string host = !m_impl->currentConfig.domain.empty() ?
                           m_impl->currentConfig.domain : m_impl->currentConfig.server;
        if (host.empty()) {
            pjsua_acc_info acc_info;
            if (pjsua_acc_get_info(m_impl->accId, &acc_info) == PJ_SUCCESS && acc_info.acc_uri.slen > 0) {
                std::string accUri(acc_info.acc_uri.ptr, acc_info.acc_uri.slen);
                auto atPos = accUri.find('@');
                if (atPos != std::string::npos) {
                    host = accUri.substr(atPos + 1);
                    auto endPos = host.find_first_of("> ;");
                    if (endPos != std::string::npos) host = host.substr(0, endPos);
                }
            }
        }
        if (!host.empty()) {
            fullUri += "@" + host;
        }
    }

    if (m_impl->currentConfig.transport == "TCP" && fullUri.find("transport=") == std::string::npos) {
        fullUri += ";transport=tcp";
    } else if (m_impl->currentConfig.transport == "TLS" && fullUri.find("transport=") == std::string::npos) {
        fullUri += ";transport=tls";
    }

    std::string dialUri = (fullUri.front() == '<') ? fullUri : ("<" + fullUri + ">");
    pj_str_t pj_dst = pj_str(const_cast<char*>(dialUri.c_str()));
    pjsua_call_id call_id = PJSUA_INVALID_ID;

    pjsua_call_setting opt;
    pjsua_call_setting_default(&opt);
    opt.vid_cnt = hasVideo ? 1 : 0;

    PJ_LOG(3, ("SipEngine", "Initiating call: dialUri='%s', accId=%d", dialUri.c_str(), m_impl->accId));

    pj_status_t status = pjsua_call_make_call(m_impl->accId, &pj_dst, &opt, NULL, NULL, &call_id);
    if (status != PJ_SUCCESS) {
        char errBuf[PJ_ERR_MSG_SIZE];
        pj_strerror(status, errBuf, sizeof(errBuf));
        PJ_LOG(1, ("SipEngine", "pjsua_call_make_call to '%s' failed: %s (status=%d)", dialUri.c_str(), errBuf, status));
        return -1;
    }
    return call_id;
}

bool SipEngine::answerCall(int callId)
{
    pj_status_t status = pjsua_call_answer(callId, 200, NULL, NULL);
    return (status == PJ_SUCCESS);
}

bool SipEngine::hangupCall(int callId)
{
    pj_status_t status = pjsua_call_hangup(callId, 0, NULL, NULL);
    return (status == PJ_SUCCESS);
}

bool SipEngine::holdCall(int callId)
{
    pj_status_t status = pjsua_call_set_hold(callId, NULL);
    return (status == PJ_SUCCESS);
}

bool SipEngine::unholdCall(int callId)
{
    pj_status_t status = pjsua_call_reinvite(callId, PJ_TRUE, NULL);
    return (status == PJ_SUCCESS);
}

bool SipEngine::sendDtmf(int callId, const std::string& digits)
{
    pj_str_t pj_digits = pj_str(const_cast<char*>(digits.c_str()));
    pj_status_t status = pjsua_call_dial_dtmf(callId, &pj_digits);
    return (status == PJ_SUCCESS);
}

bool SipEngine::transferCall(int callId, const std::string& destination)
{
    if (!m_impl->running || callId < 0) return false;
    std::string target = destination;
    while (!target.empty() && isspace(static_cast<unsigned char>(target.front()))) target.erase(0, 1);
    while (!target.empty() && isspace(static_cast<unsigned char>(target.back()))) target.pop_back();

    std::string fullUri = target;
    if (fullUri.find("sip:") != 0 && fullUri.find("sips:") != 0) {
        fullUri = "sip:" + fullUri;
    }
    if (fullUri.find('@') == std::string::npos) {
        std::string host = !m_impl->currentConfig.domain.empty() ?
                           m_impl->currentConfig.domain : m_impl->currentConfig.server;
        if (!host.empty()) {
            fullUri += "@" + host;
        }
    }
    std::string xferUri = (fullUri.front() == '<') ? fullUri : ("<" + fullUri + ">");
    pj_str_t pj_dst = pj_str(const_cast<char*>(xferUri.c_str()));
    pj_status_t status = pjsua_call_xfer(callId, &pj_dst, NULL);
    return (status == PJ_SUCCESS);
}

void SipEngine::setListener(ISipEngineListener* listener)
{
    std::lock_guard<std::mutex> lock(g_listenerMutex);
    g_listener = listener;
}

bool SipEngine::muteMicrophone(bool mute)
{
    if (!m_impl->running) return false;
    m_impl->muted = mute;
    // slot 0 in conference bridge is the sound device
    if (mute) {
        pjsua_conf_adjust_rx_level(0, 0.0f);
    } else {
        pjsua_conf_adjust_rx_level(0, 1.0f);
    }
    return true;
}

bool SipEngine::isMicrophoneMuted() const
{
    return m_impl->muted;
}

bool SipEngine::setSpeakerVolume(float level)
{
    if (!m_impl->running) return false;
    // PJSIP adjusts tx level to slot 0 (playback device)
    pj_status_t status = pjsua_conf_adjust_tx_level(0, level);
    return (status == PJ_SUCCESS);
}

bool SipEngine::setMicVolume(float level)
{
    if (!m_impl->running) return false;
    if (m_impl->muted) return true; // Keep muted if mute state active
    pj_status_t status = pjsua_conf_adjust_rx_level(0, level);
    return (status == PJ_SUCCESS);
}

bool SipEngine::getCallQualityStats(int callId, float& packetLossPercent, unsigned& jitterMs, unsigned& rttMs)
{
    packetLossPercent = 0.0f;
    jitterMs = 0;
    rttMs = 0;

    if (!m_impl->running || callId < 0) return false;
    if (!pjsua_call_is_active(callId) || !pjsua_call_has_media(callId)) return false;

    pjsua_stream_stat stat;
    pj_status_t status = pjsua_call_get_stream_stat(callId, 0, &stat);
    if (status == PJ_SUCCESS) {
        jitterMs = stat.rtcp.rx.jitter.mean / 1000;
        rttMs = stat.rtcp.rtt.mean / 1000;
        unsigned long total = stat.rtcp.rx.pkt + stat.rtcp.rx.loss;
        if (total > 0) {
            packetLossPercent = ((float)stat.rtcp.rx.loss * 100.0f) / (float)total;
        }
        return true;
    }
    return false;
}

bool SipEngine::getAudioLevels(int callId, unsigned& rxLevel, unsigned& txLevel)
{
    rxLevel = 0;
    txLevel = 0;
    if (!m_impl->running || callId < 0) return false;
    if (!pjsua_call_is_active(callId) || !pjsua_call_has_media(callId)) return false;

    pjsua_call_info ci;
    if (pjsua_call_get_info(callId, &ci) == PJ_SUCCESS && ci.conf_slot != PJSUA_INVALID_ID) {
        pj_status_t status = pjsua_conf_get_signal_level(ci.conf_slot, &txLevel, &rxLevel);
        return (status == PJ_SUCCESS);
    }
    return false;
}

std::vector<AudioDeviceInfo> SipEngine::getAudioDevices()
{
    std::vector<AudioDeviceInfo> devices;
    if (!m_impl->running) return devices;

    unsigned count = 64;
    pjmedia_snd_dev_info info[64];
    if (pjsua_enum_snd_devs(info, &count) == PJ_SUCCESS) {
        for (unsigned i = 0; i < count; ++i) {
            AudioDeviceInfo dev;
            dev.id = static_cast<int>(i);
            dev.name = info[i].name;
            dev.inputChannels = info[i].input_count;
            dev.outputChannels = info[i].output_count;
            dev.defaultSampleRate = info[i].default_samples_per_sec;
            devices.push_back(dev);
        }
    }
    return devices;
}

bool SipEngine::setAudioDevices(int playbackDevId, int captureDevId)
{
    if (!m_impl->running) return false;
    pj_status_t status = pjsua_set_snd_dev(captureDevId, playbackDevId);
    return (status == PJ_SUCCESS);
}

bool SipEngine::handleIpChange()
{
    if (!m_impl->running) return false;
    pjsua_ip_change_param param;
    pjsua_ip_change_param_default(&param);
    pj_status_t status = pjsua_handle_ip_change(&param);
    return (status == PJ_SUCCESS);
}


