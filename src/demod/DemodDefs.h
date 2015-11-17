#pragma once

#include "ThreadQueue.h"
#include "CubicSDRDefs.h"
#include "liquid/liquid.h"

#include <atomic>
#include <mutex>

//#define DEMOD_TYPE_NULL 0
//#define DEMOD_TYPE_FM 1
//#define DEMOD_TYPE_AM 2
//#define DEMOD_TYPE_LSB 3
//#define DEMOD_TYPE_USB 4
//#define DEMOD_TYPE_DSB 5
//#define DEMOD_TYPE_ASK 6
//#define DEMOD_TYPE_APSK 7
//#define DEMOD_TYPE_BPSK 8
//#define DEMOD_TYPE_DPSK 9
//#define DEMOD_TYPE_PSK 10
//#define DEMOD_TYPE_OOK 11
//#define DEMOD_TYPE_ST 12
//#define DEMOD_TYPE_SQAM 13
//#define DEMOD_TYPE_QAM 14
//#define DEMOD_TYPE_QPSK 15
//#define DEMOD_TYPE_RAW 16

#include "IOThread.h"

class DemodulatorThread;
class DemodulatorThreadCommand {
public:
    enum DemodulatorThreadCommandEnum {
        DEMOD_THREAD_CMD_NULL,
        DEMOD_THREAD_CMD_SET_BANDWIDTH,
        DEMOD_THREAD_CMD_SET_FREQUENCY,
        DEMOD_THREAD_CMD_SET_AUDIO_RATE,
        DEMOD_THREAD_CMD_DEMOD_PREPROCESS_TERMINATED,
        DEMOD_THREAD_CMD_DEMOD_TERMINATED,
        DEMOD_THREAD_CMD_AUDIO_TERMINATED
    };

    DemodulatorThreadCommand() :
            cmd(DEMOD_THREAD_CMD_NULL), context(NULL), llong_value(0) {

    }

    DemodulatorThreadCommand(DemodulatorThreadCommandEnum cmd) :
            cmd(cmd), context(NULL), llong_value(0) {

    }

    DemodulatorThreadCommandEnum cmd;
    void *context;
    long long llong_value;
};

class DemodulatorThreadControlCommand {
public:
    enum DemodulatorThreadControlCommandEnum {
        DEMOD_THREAD_CMD_CTL_NULL, DEMOD_THREAD_CMD_CTL_SQUELCH_ON, DEMOD_THREAD_CMD_CTL_SQUELCH_OFF, DEMOD_THREAD_CMD_CTL_TYPE
    };

    DemodulatorThreadControlCommand() :
            cmd(DEMOD_THREAD_CMD_CTL_NULL), demodType("") {
    }

    DemodulatorThreadControlCommandEnum cmd;
    std::string demodType;
};

class DemodulatorThreadIQData: public ReferenceCounter {
public:
    long long frequency;
    long long sampleRate;
    std::vector<liquid_float_complex> data;
    std::mutex busy_rw;

    DemodulatorThreadIQData() :
            frequency(0), sampleRate(0) {

    }

    DemodulatorThreadIQData & operator=(const DemodulatorThreadIQData &other) {
        frequency = other.frequency;
        sampleRate = other.sampleRate;
        data.assign(other.data.begin(), other.data.end());
        return *this;
    }

    ~DemodulatorThreadIQData() {

    }
};


class DemodulatorThreadPostIQData: public ReferenceCounter {
public:
    std::vector<liquid_float_complex> data;
    long long sampleRate;

    DemodulatorThreadPostIQData() :
            sampleRate(0) {

    }

    ~DemodulatorThreadPostIQData() {
        std::lock_guard < std::mutex > lock(m_mutex);
    }
};


class DemodulatorThreadAudioData: public ReferenceCounter {
public:
    long long frequency;
    unsigned int sampleRate;
    unsigned char channels;

    std::vector<float> *data;

    DemodulatorThreadAudioData() :
            frequency(0), sampleRate(0), channels(0), data(NULL) {

    }

    DemodulatorThreadAudioData(long long frequency, unsigned int sampleRate, std::vector<float> *data) :
            frequency(frequency), sampleRate(sampleRate), channels(1), data(data) {

    }

    ~DemodulatorThreadAudioData() {

    }
};

typedef ThreadQueue<DemodulatorThreadIQData *> DemodulatorThreadInputQueue;
typedef ThreadQueue<DemodulatorThreadPostIQData *> DemodulatorThreadPostInputQueue;
typedef ThreadQueue<DemodulatorThreadCommand> DemodulatorThreadCommandQueue;
typedef ThreadQueue<DemodulatorThreadControlCommand> DemodulatorThreadControlCommandQueue;

class DemodulatorThreadParameters {
public:
    long long frequency;
    long long sampleRate;
    unsigned int bandwidth; // set equal to disable second stage re-sampling?
    unsigned int audioSampleRate;

    std::string demodType;

    DemodulatorThreadParameters() :
            frequency(0), sampleRate(DEFAULT_SAMPLE_RATE), bandwidth(200000), audioSampleRate(0),
            demodType("FM") {

    }

    ~DemodulatorThreadParameters() {

    }
};
