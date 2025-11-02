#pragma once
#include <JuceHeader.h>

struct AudioFileInfo
{
    juce::String title;
    juce::String artist;
    juce::String album;
    juce::String durationString;
};

class PlayerAudio : public juce::AudioSource
{
public:
    PlayerAudio();
    ~PlayerAudio() override;

    AudioFileInfo loadFile(const juce::File& file);
    void play();
    void pause();
    void stop();
    void restart();
    void goToStart();
    void goToEnd();
    void toggleMute();
    void setLooping(bool shouldLoop);

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void setGain(float gain);
    float getGain() const;

    double getCurrentPosition() const;
    double getLengthInSeconds() const;

    void skipForward(double seconds);
    void skipBackward(double seconds);
    void setPositionSafe(double posInSeconds);

    void setSpeed(double speed);
    double getSpeed() const { return currentSpeed; }

    bool isMuted() const { return muted; }

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;

    bool muted = false;
    bool looping = false;
    float previousVolume = 1.0f;
    double currentLength = 0.0;
    double currentSpeed = 1.0;
};
