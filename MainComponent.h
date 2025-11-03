#pragma once
#include <JuceHeader.h>
#include "PlayerGUI.h"

class MainComponent : public juce::AudioAppComponent
{
public:
    MainComponent()
    {
        addAndMakeVisible(playerGUI);
        setSize(600, 800);
        setAudioChannels(0, 2);
    }

    ~MainComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        playerGUI.getPlayerAudio().prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        playerGUI.getPlayerAudio().getNextAudioBlock(bufferToFill);
    }

    void releaseResources() override
    {
        playerGUI.getPlayerAudio().releaseResources();
    }

    void resized() override
    {
        playerGUI.setBounds(getLocalBounds());
    }

private:
    PlayerGUI playerGUI;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
