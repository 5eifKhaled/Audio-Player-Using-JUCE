#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::Timer,
    public juce::ListBoxModel
{
public:
    PlayerGUI();
    ~PlayerGUI() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void timerCallback() override;

    // playlist
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g,
        int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;

    PlayerAudio& getPlayerAudio() { return playerAudio; }

private:
    PlayerAudio playerAudio;

    juce::TextButton loadButton{ "Load" };
    juce::TextButton playButton{ "Play" };
    juce::TextButton pauseButton{ "Pause" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton restartButton{ "Restart" };
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopButton{ "Loop" };
    juce::TextButton startButton{ "|<" };
    juce::TextButton endButton{ ">|" };
    juce::TextButton back10Button{ "<10s" };
    juce::TextButton fwd10Button{ "10s>" };

    juce::Slider volumeSlider;
    juce::Slider progressSlider;
    juce::Slider speedSlider;
    juce::Label speedLabel;

    juce::Label titleLabel;
    juce::Label artistLabel;
    juce::Label albumLabel;
    juce::Label durationLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::ListBox playlistBox;
    std::vector<juce::File> playlistFiles;

    bool isLooping = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};
