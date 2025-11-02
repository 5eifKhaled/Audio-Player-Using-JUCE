#include "PlayerGUI.h"

PlayerGUI::PlayerGUI()
{
    //buttons
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

    //sliders
    addAndMakeVisible(volumeSlider);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.addListener(this);
    playerAudio.setGain((float)volumeSlider.getValue());

    addAndMakeVisible(progressSlider);
    progressSlider.setRange(0.0, 1.0);
    progressSlider.addListener(this);

    addAndMakeVisible(speedSlider);
    speedSlider.setRange(0.5, 2.0, 0.01);
    speedSlider.setValue(1.0);
    speedSlider.addListener(this);

    addAndMakeVisible(speedLabel);
    speedLabel.setText("Speed: 1.00x", juce::dontSendNotification);
    speedLabel.setJustificationType(juce::Justification::centred);

    //metadata (i think it has a little problem)
    for (auto* label : { &titleLabel, &artistLabel, &albumLabel, &durationLabel })
    {
        addAndMakeVisible(label);
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setFont(juce::Font(13.0f, juce::Font::bold));
        label->setJustificationType(juce::Justification::centredLeft);
    }

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
    auto topArea = area.removeFromTop(140);
    auto bottomArea = area.removeFromBottom(80);

    int buttonWidth = 80;
    int buttonHeight = 30;
    int spacing = 10;

    loadButton.setBounds(spacing, 10, buttonWidth, buttonHeight);
    playButton.setBounds(loadButton.getRight() + spacing, 10, buttonWidth, buttonHeight);
    pauseButton.setBounds(playButton.getRight() + spacing, 10, buttonWidth, buttonHeight);
    stopButton.setBounds(pauseButton.getRight() + spacing, 10, buttonWidth, buttonHeight);
    restartButton.setBounds(stopButton.getRight() + spacing, 10, buttonWidth, buttonHeight);

    volumeSlider.setBounds(restartButton.getRight() + 2 * spacing, 15, 150, 20);
    progressSlider.setBounds(20, 50, getWidth() - 40, 20);

    speedSlider.setBounds(20, 80, getWidth() - 40, 20);
    speedLabel.setBounds(20, 105, getWidth() - 40, 18);

    int metaY = 130;
    titleLabel.setBounds(20, metaY, getWidth() - 40, 18); metaY += 20;
    artistLabel.setBounds(20, metaY, getWidth() - 40, 18); metaY += 20;
    albumLabel.setBounds(20, metaY, getWidth() - 40, 18); metaY += 20;
    durationLabel.setBounds(20, metaY, getWidth() - 40, 18);

    int buttonCount = 6;
    int totalWidth = buttonCount * buttonWidth + (buttonCount - 1) * spacing;
    int startX = (getWidth() - totalWidth) / 2;
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
    {
        playerAudio.setGain((float)slider->getValue());
    }
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
    double length = playerAudio.getLengthInSeconds();
    if (length > 0.0)
    {
        double position = playerAudio.getCurrentPosition();
        progressSlider.setValue(position / length, juce::dontSendNotification);
    }
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select an audio file...",
            juce::File{},
            "*.wav;*.mp3;*.flac");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                juce::File file = fc.getResult();
                if (file.existsAsFile())
                {
                    AudioFileInfo info = playerAudio.loadFile(file);

                    titleLabel.setText("Title: " + info.title, juce::dontSendNotification);
                    artistLabel.setText("Artist: " + info.artist, juce::dontSendNotification);
                    albumLabel.setText("Album: " + info.album, juce::dontSendNotification);
                    durationLabel.setText("Duration: " + info.durationString, juce::dontSendNotification);

                    playerAudio.setGain((float)volumeSlider.getValue());
                    playerAudio.setSpeed(speedSlider.getValue());
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
