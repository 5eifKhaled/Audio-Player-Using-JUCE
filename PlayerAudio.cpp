#include "PlayerAudio.h"

PlayerAudio::PlayerAudio()
{
    formatManager.registerBasicFormats();
    resamplingSource = std::make_unique<juce::ResamplingAudioSource>(&transportSource, false, 2);
}

PlayerAudio::~PlayerAudio()
{
    resamplingSource->releaseResources();
    transportSource.releaseResources();
}

AudioFileInfo PlayerAudio::loadFile(const juce::File& file)
{
    AudioFileInfo info;
    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        readerSource.reset(new juce::AudioFormatReaderSource(reader, true));

        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

        auto& meta = reader->metadataValues;
        auto getMeta = [&](const juce::String& a, const juce::String& b)
            {
                juce::String v = meta[a];
                if (v.isEmpty()) v = meta[b];
                return v;
            };

        info.title = getMeta("title", "TITLE");
        info.artist = getMeta("artist", "ARTIST");
        info.album = getMeta("album", "ALBUM");

        if (info.title.isEmpty())  info.title = file.getFileNameWithoutExtension();
        if (info.artist.isEmpty()) info.artist = "Unknown Artist";
        if (info.album.isEmpty())  info.album = "Unknown Album";

        double lengthSecs = 0.0;
        if (reader->sampleRate > 0.0)
            lengthSecs = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;

        int mins = static_cast<int>(lengthSecs / 60.0);
        int secs = static_cast<int>(std::fmod(lengthSecs, 60.0));
        info.durationString = juce::String::formatted("%02d:%02d", mins, secs);

        transportSource.setPosition(0.0);
        currentLength = lengthSecs;
    }
    return info;
}

void PlayerAudio::play() { transportSource.start(); }
void PlayerAudio::pause() { transportSource.stop(); }

void PlayerAudio::stop()
{
    transportSource.stop();
    transportSource.setPosition(0.0);
}

void PlayerAudio::restart()
{
    transportSource.setPosition(0.0);
    transportSource.start();
}

void PlayerAudio::goToStart() { transportSource.setPosition(0.0); }

void PlayerAudio::goToEnd()
{
    if (readerSource)
    {
        auto* reader = readerSource->getAudioFormatReader();
        if (reader && reader->sampleRate > 0.0)
            transportSource.setPosition(static_cast<double>(reader->lengthInSamples) / reader->sampleRate);
    }
}

void PlayerAudio::toggleMute()
{
    if (muted)
        transportSource.setGain(previousVolume);
    else
    {
        previousVolume = transportSource.getGain();
        transportSource.setGain(0.0f);
    }
    muted = !muted;
}

void PlayerAudio::setLooping(bool shouldLoop)
{
    if (readerSource)
        readerSource->setLooping(shouldLoop);
    looping = shouldLoop;
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    if (resamplingSource)
        resamplingSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (resamplingSource)
        resamplingSource->getNextAudioBlock(bufferToFill);
    else
        bufferToFill.clearActiveBufferRegion();
}

void PlayerAudio::releaseResources()
{
    if (resamplingSource)
        resamplingSource->releaseResources();
    transportSource.releaseResources();
}

void PlayerAudio::setGain(float gain)
{
    transportSource.setGain(gain);
}

float PlayerAudio::getGain() const
{
    return transportSource.getGain();
}

double PlayerAudio::getCurrentPosition() const
{
    return transportSource.getCurrentPosition();
}

double PlayerAudio::getLengthInSeconds() const
{
    return currentLength;
}

void PlayerAudio::setPositionSafe(double posInSeconds)
{
    if (posInSeconds < 0.0) posInSeconds = 0.0;
    double length = getLengthInSeconds();
    if (length > 0.0 && posInSeconds > length) posInSeconds = length;
    transportSource.setPosition(posInSeconds);
}

void PlayerAudio::skipForward(double seconds)
{
    if (seconds <= 0.0) return;
    double pos = transportSource.getCurrentPosition();
    setPositionSafe(pos + seconds);
}

void PlayerAudio::skipBackward(double seconds)
{
    if (seconds <= 0.0) return;
    double pos = transportSource.getCurrentPosition();
    setPositionSafe(pos - seconds);
}

void PlayerAudio::setSpeed(double speed)
{
    if (speed <= 0.0) return;
    currentSpeed = speed;
    if (resamplingSource)
        resamplingSource->setResamplingRatio(speed);
}
