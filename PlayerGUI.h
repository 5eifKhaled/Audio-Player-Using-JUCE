#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::Timer
{
public:
    PlayerGUI();
    ~PlayerGUI() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void timerCallback() override;

    PlayerAudio& getPlayerAudio() { return playerAudio; }

private:
    PlayerAudio playerAudio;

    //buttons
    juce::TextButton loadButton{ "Load" };
    juce::TextButton playButton{ "Play" };
    juce::TextButton pauseButton{ "Pause" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton restartButton{ "Restart" };
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopButton{ "Loop" };
    juce::TextButton startButton{ "|<" };
    juce::TextButton endButton{ ">|" };
    juce::TextButton back10Button{ "<<10s" };
    juce::TextButton fwd10Button{ "10s>>" };

    //sliders
    juce::Slider volumeSlider;
    juce::Slider progressSlider;
    juce::Slider speedSlider;
    juce::Label speedLabel;

    //metadata
    juce::Label titleLabel;
    juce::Label artistLabel;
    juce::Label durationLabel;
    juce::Label albumLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;

    bool isLooping = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};
