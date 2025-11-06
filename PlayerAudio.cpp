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

    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

        // metadata
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

        // duration
        double lengthSecs = 0.0;
        if (reader->sampleRate > 0.0)
            lengthSecs = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;

        currentLength = lengthSecs;

        int mins = static_cast<int>(lengthSecs / 60.0);
        int secs = static_cast<int>(std::fmod(lengthSecs, 60.0));
        info.durationString = juce::String::formatted("%02d:%02d", mins, secs);

        // reset position
        transportSource.setPosition(0.0);
    }
    else
    {
        DBG("PlayerAudio::loadFile - Could not open file: " << file.getFullPathName());
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
            transportSource.setPosition(reader->lengthInSamples / reader->sampleRate);
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
    resamplingSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    resamplingSource->getNextAudioBlock(bufferToFill);
}

void PlayerAudio::releaseResources()
{
    resamplingSource->releaseResources();
    transportSource.releaseResources();
}

void PlayerAudio::setGain(float gain) { transportSource.setGain(gain); }
float PlayerAudio::getGain() const { return transportSource.getGain(); }

double PlayerAudio::getCurrentPosition() const { return transportSource.getCurrentPosition(); }
double PlayerAudio::getLengthInSeconds() const { return currentLength; }

void PlayerAudio::setPositionSafe(double pos)
{
    if (pos < 0.0) pos = 0.0;
    double length = getLengthInSeconds();
    if (pos > length) pos = length;
    transportSource.setPosition(pos);
}

void PlayerAudio::skipForward(double seconds)
{
    if (seconds > 0.0)
        setPositionSafe(transportSource.getCurrentPosition() + seconds);
}

void PlayerAudio::skipBackward(double seconds)
{
    if (seconds > 0.0)
        setPositionSafe(transportSource.getCurrentPosition() - seconds);
}

void PlayerAudio::setSpeed(double speed)
{
    if (speed > 0.0)
    {
        currentSpeed = speed;
        resamplingSource->setResamplingRatio(speed);
    }
}
