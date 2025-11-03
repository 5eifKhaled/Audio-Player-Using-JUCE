#include "PlayerGUI.h"

PlayerGUI::PlayerGUI()
{
    // buttons
    addAndMakeVisible(loadButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(pauseButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(restartButton);
    addAndMakeVisible(muteButton);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(startButton);
    addAndMakeVisible(endButton);
    addAndMakeVisible(back10Button);
    addAndMakeVisible(fwd10Button);

    for (auto* btn : { &loadButton, &playButton, &pauseButton, &stopButton, &restartButton,
                       &muteButton, &loopButton, &startButton, &endButton, &back10Button, &fwd10Button })
        btn->addListener(this);

    // sliders
    addAndMakeVisible(volumeSlider);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.addListener(this);

    addAndMakeVisible(progressSlider);
    progressSlider.setRange(0.0, 1.0);
    progressSlider.addListener(this);

    // speed
    addAndMakeVisible(speedSlider);
    speedSlider.setRange(0.5, 2.0, 0.01);
    speedSlider.setValue(1.0);
    speedSlider.addListener(this);

    addAndMakeVisible(speedLabel);
    speedLabel.setText("Speed: 1.0x", juce::dontSendNotification);
    speedLabel.setJustificationType(juce::Justification::centred);

    // metadata
    for (auto* label : { &titleLabel, &artistLabel, &albumLabel, &durationLabel })
    {
        addAndMakeVisible(label);
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setFont(juce::Font(14.0f, juce::Font::bold));
    }

    // Playlist
    addAndMakeVisible(playlistBox);
    playlistBox.setModel(this);
    playlistBox.setColour(juce::ListBox::backgroundColourId, juce::Colours::black);
    playlistBox.setColour(juce::ListBox::textColourId, juce::Colours::white);
    playlistBox.setRowHeight(24);

    startTimerHz(20);
}

PlayerGUI::~PlayerGUI() {}

void PlayerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);
}

void PlayerGUI::resized()
{
    auto area = getLocalBounds();

    auto playlistArea = area.removeFromRight(getWidth() / 3);
    playlistBox.setBounds(playlistArea.reduced(10));

    auto playerArea = area.reduced(10);

    auto topRow = playerArea.removeFromTop(100);
    int buttonWidth = 80;
    int buttonHeight = 30;
    int spacing = 10;

    loadButton.setBounds(spacing, 20, buttonWidth, buttonHeight);
    playButton.setBounds(loadButton.getRight() + spacing, 20, buttonWidth, buttonHeight);
    pauseButton.setBounds(playButton.getRight() + spacing, 20, buttonWidth, buttonHeight);
    stopButton.setBounds(pauseButton.getRight() + spacing, 20, buttonWidth, buttonHeight);
    restartButton.setBounds(stopButton.getRight() + spacing, 20, buttonWidth, buttonHeight);

    volumeSlider.setBounds(restartButton.getRight() + 2 * spacing, 25, 150, 20);
    progressSlider.setBounds(20, 70, getWidth() * 2 / 3 - 40, 20);
    speedSlider.setBounds(20, 100, getWidth() * 2 / 3 - 40, 20);
    speedLabel.setBounds(20, 125, getWidth() * 2 / 3 - 40, 20);

    titleLabel.setBounds(20, 160, getWidth() * 2 / 3 - 40, 20);
    artistLabel.setBounds(20, 180, getWidth() * 2 / 3 - 40, 20);
    albumLabel.setBounds(20, 200, getWidth() * 2 / 3 - 40, 20);
    durationLabel.setBounds(20, 220, getWidth() * 2 / 3 - 40, 20);

    // controls
    auto bottomArea = playerArea.removeFromBottom(70);
    int buttonCount = 6;
    int totalWidth = buttonCount * buttonWidth + (buttonCount - 1) * spacing;
    int startX = (bottomArea.getWidth() - totalWidth) / 2;
    int y = bottomArea.getY() + 10;

    startButton.setBounds(startX, y, buttonWidth, buttonHeight);
    back10Button.setBounds(startButton.getRight() + spacing, y, buttonWidth, buttonHeight);
    fwd10Button.setBounds(back10Button.getRight() + spacing, y, buttonWidth, buttonHeight);
    endButton.setBounds(fwd10Button.getRight() + spacing, y, buttonWidth, buttonHeight);
    muteButton.setBounds(endButton.getRight() + spacing, y, buttonWidth, buttonHeight);
    loopButton.setBounds(muteButton.getRight() + spacing, y, buttonWidth, buttonHeight);
}


void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
        playerAudio.setGain((float)slider->getValue());

    else if (slider == &progressSlider)
    {
        double length = playerAudio.getLengthInSeconds();
        if (length > 0.0)
            playerAudio.setPositionSafe(length * slider->getValue());
    }

    else if (slider == &speedSlider)
    {
        double newSpeed = speedSlider.getValue();
        playerAudio.setSpeed(newSpeed);
        speedLabel.setText("Speed: " + juce::String(newSpeed, 2) + "x", juce::dontSendNotification);
    }
}

void PlayerGUI::timerCallback()
{
    if (playerAudio.getLengthInSeconds() > 0.0)
    {
        double position = playerAudio.getCurrentPosition();
        double length = playerAudio.getLengthInSeconds();
        progressSlider.setValue(position / length, juce::dontSendNotification);
    }
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select audio files...",
            juce::File{},
            "*.wav;*.mp3;*.flac");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles |
            juce::FileBrowserComponent::canSelectMultipleItems,
            [this](const juce::FileChooser& fc)
            {
                auto results = fc.getResults();
                if (results.isEmpty()) return;

                for (auto& file : results)
                    playlistFiles.push_back(file);

                playlistBox.updateContent();
                playlistBox.repaint();

                if (!playlistFiles.empty())
                {
                    auto info = playerAudio.loadFile(playlistFiles.front());
                    titleLabel.setText("Title: " + info.title, juce::dontSendNotification);
                    artistLabel.setText("Artist: " + info.artist, juce::dontSendNotification);
                    durationLabel.setText("Duration: " + info.durationString, juce::dontSendNotification);
                }
            });
    }
    else if (button == &muteButton)
    {
        playerAudio.toggleMute();
        muteButton.setButtonText(playerAudio.isMuted() ? "Unmute" : "Mute");
    }
    else if (button == &loopButton)
    {
        isLooping = !isLooping;
        playerAudio.setLooping(isLooping);
        loopButton.setButtonText(isLooping ? "Loop On" : "Loop Off");
    }
    else if (button == &playButton) playerAudio.play();
    else if (button == &pauseButton) playerAudio.pause();
    else if (button == &stopButton) playerAudio.stop();
    else if (button == &restartButton) playerAudio.restart();
    else if (button == &startButton) playerAudio.goToStart();
    else if (button == &endButton) playerAudio.goToEnd();
    else if (button == &back10Button) playerAudio.skipBackward(10.0);
    else if (button == &fwd10Button) playerAudio.skipForward(10.0);
}

//playlist
int PlayerGUI::getNumRows()
{
    return (int)playlistFiles.size();
}

void PlayerGUI::paintListBoxItem(int rowNumber, juce::Graphics& g,
    int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= (int)playlistFiles.size()) return;

    if (rowIsSelected)
        g.fillAll(juce::Colours::darkorange);
    else
        g.fillAll(juce::Colours::darkgrey.darker(0.7f));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawText(playlistFiles[rowNumber].getFileNameWithoutExtension(),
        10, 0, width - 20, height,
        juce::Justification::centredLeft);
}


void PlayerGUI::selectedRowsChanged(int lastRowSelected)
{
    if (lastRowSelected >= 0 && lastRowSelected < (int)playlistFiles.size())
    {
        auto file = playlistFiles[lastRowSelected];
        auto info = playerAudio.loadFile(file);

        titleLabel.setText("Title: " + info.title, juce::dontSendNotification);
        artistLabel.setText("Artist: " + info.artist, juce::dontSendNotification);
        durationLabel.setText("Duration: " + info.durationString, juce::dontSendNotification);

        playerAudio.play();
    }
}
