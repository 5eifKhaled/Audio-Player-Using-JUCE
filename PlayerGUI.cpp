#include "PlayerGUI.h"

PlayerGUI::PlayerGUI()
{
    // buttons and listeners
    auto buttons = { &loadButton, &playButton, &pauseButton, &stopButton, &restartButton,
                     &muteButton, &loopButton, &startButton, &endButton, &back10Button,
                     &fwd10Button, &nextButton, &prevButton, &setAButton, &setBButton, &loopABButton };

    for (auto* b : buttons)
    {
        addAndMakeVisible(b);
        b->addListener(this);
        b->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }

    // sliders
    addAndMakeVisible(volumeSlider);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.addListener(this);

    addAndMakeVisible(progressSlider);
    progressSlider.setRange(0.0, 1.0);
    progressSlider.addListener(this);

    addAndMakeVisible(speedSlider);
    speedSlider.setRange(0.5, 2.0, 0.01);
    speedSlider.setValue(1.0);
    speedSlider.addListener(this);

    addAndMakeVisible(speedLabel);
    speedLabel.setText("Speed: 1.0x", juce::dontSendNotification);
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

    // metadata labels (something aint right it only fetch metadata for .wav files)
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    artistLabel.setFont(juce::Font(13.0f));
    albumLabel.setFont(juce::Font(12.0f));
    durationLabel.setFont(juce::Font(12.0f));
    for (auto* lab : { &titleLabel, &artistLabel, &albumLabel, &durationLabel })
    {
        addAndMakeVisible(lab);
        lab->setColour(juce::Label::textColourId, juce::Colours::white);
    }

    // playlist box
    addAndMakeVisible(playlistBox);
    playlistBox.setModel(this);
    playlistBox.setRowHeight(26);
    playlistBox.setMultipleSelectionEnabled(false);

    // waveform
    addAndMakeVisible(waveform);

    waveform.onPositionSelected = [this](double sec)
        {
            playerAudio.setPositionSafe(sec);
        };

    setSize(1000, 520);
    startTimerHz(25);
}

PlayerGUI::~PlayerGUI()
{
}

void PlayerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(18, 20, 23));

    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.fillRect(0, 0, getWidth(), 44);

    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    //g.drawText("Modern Audio Player", 12, 0, getWidth(), 44, juce::Justification::centredLeft);
}

void PlayerGUI::resized()
{
    auto area = getLocalBounds().reduced(8);

    auto playlistArea = area.removeFromRight(getWidth() / 3);
    playlistBox.setBounds(playlistArea.reduced(8));

    auto left = area.reduced(8);

    int bw = 88, bh = 34, s = 8;
    int x = left.getX();

    loadButton.setBounds(x, left.getY(), 70, bh);
    playButton.setBounds(loadButton.getRight() + s, left.getY(), bw, bh);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(30, 150, 30));
    playButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(50, 200, 50));
    playButton.setColour(juce::TextButton::buttonDown, juce::Colour::fromRGB(20, 120, 20));
    playButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    pauseButton.setBounds(playButton.getRight() + s, left.getY(), bw, bh);
    stopButton.setBounds(pauseButton.getRight() + s, left.getY(), bw, bh);
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(180, 30, 30));
    stopButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(220, 50, 50));
    stopButton.setColour(juce::TextButton::buttonDown, juce::Colour::fromRGB(140, 20, 20));
    stopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    restartButton.setBounds(stopButton.getRight() + s, left.getY(), bw, bh);

    // wider volume slider
    int volWidth = juce::jlimit(160, left.getWidth() / 3, left.getWidth() / 2);
    volumeSlider.setBounds(restartButton.getRight() + s, left.getY() + 6, volWidth, 24);

    // waveform area
    int wfTop = left.getY() + bh + 16;
    int wfHeight = 220;
    waveform.setBounds(left.getX(), wfTop, left.getWidth(), wfHeight);

    // progress & speed
    progressSlider.setBounds(left.getX(), wfTop + wfHeight + 8, left.getWidth(), 18);
    speedSlider.setBounds(left.getX(), wfTop + wfHeight + 30, left.getWidth() - 120, 18);
    speedLabel.setBounds(speedSlider.getRight() + 8, wfTop + wfHeight + 28, 110, 22);

    // metadata
    titleLabel.setBounds(left.getX(), wfTop + wfHeight + 62, left.getWidth(), 22);
    artistLabel.setBounds(left.getX(), wfTop + wfHeight + 90, left.getWidth(), 18);
    albumLabel.setBounds(left.getX(), wfTop + wfHeight + 110, left.getWidth(), 18);
    durationLabel.setBounds(left.getX(), wfTop + wfHeight + 130, left.getWidth(), 18);

    // bottom controls
    int bottomY = getHeight() - 108;
    int count = 8;
    int totalW = count * bw + (count - 1) * s;
    int startX = left.getX() + (left.getWidth() - totalW) / 2;

    startButton.setBounds(startX, bottomY, bw, bh);
    back10Button.setBounds(startButton.getRight() + s, bottomY, bw, bh);
    fwd10Button.setBounds(back10Button.getRight() + s, bottomY, bw, bh);
    endButton.setBounds(fwd10Button.getRight() + s, bottomY, bw, bh);
    prevButton.setBounds(endButton.getRight() + s, bottomY, bw, bh);
    nextButton.setBounds(prevButton.getRight() + s, bottomY, bw, bh);
    muteButton.setBounds(nextButton.getRight() + s, bottomY, bw, bh);
    loopButton.setBounds(muteButton.getRight() + s, bottomY, bw, bh);

    // A-B row
    int abY = bottomY + bh + s;
    setAButton.setBounds(startX, abY, bw, bh);
    setBButton.setBounds(setAButton.getRight() + s, abY, bw, bh);
    loopABButton.setBounds(setBButton.getRight() + s, abY, bw, bh);
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
    {
        playerAudio.setGain((float)volumeSlider.getValue());
    }
    else if (slider == &progressSlider)
    {
        double len = playerAudio.getLengthInSeconds();
        if (len > 0.0)
            playerAudio.setPositionSafe(len * progressSlider.getValue());
    }
    else if (slider == &speedSlider)
    {
        playerAudio.setSpeed(speedSlider.getValue());
        speedLabel.setText("Speed: " + juce::String(speedSlider.getValue(), 2) + "x", juce::dontSendNotification);
    }
}

void PlayerGUI::timerCallback()
{
    double len = playerAudio.getLengthInSeconds();
    if (len > 0.0)
    {
        double pos = playerAudio.getCurrentPosition();
        progressSlider.setValue(len > 0.0 ? pos / len : 0.0, juce::dontSendNotification);

        // sync waveform
        waveform.setPosition(pos);
        waveform.setLength(len);
        waveform.setAB(pointA, pointB);

        // A-B loop enforce
        if (abLoopEnabled && pointA >= 0.0 && pointB > pointA && pos >= pointB)
            playerAudio.setPositionSafe(pointA);
    }
}

void PlayerGUI::buttonClicked(juce::Button* b)
{
    if (b == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>("Select audio files...", juce::File{}, "*.wav;*.mp3;*.flac");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectMultipleItems,
            [this](const juce::FileChooser& fc)
            {
                auto results = fc.getResults();
                if (results.isEmpty()) return;

                playlistFiles.clear();
                playlistNames.clear();

                for (auto& f : results)
                {
                    playlistFiles.add(f);
                    playlistNames.add(f.getFileName());
                }

                playlistBox.updateContent();

                // auto load first
                currentIndex = 0;
                auto info = playerAudio.loadFile(playlistFiles[currentIndex]);
                waveform.setFile(playlistFiles[currentIndex]);
                waveform.setLength(playerAudio.getLengthInSeconds());

                titleLabel.setText("Title: " + info.title, juce::dontSendNotification);
                artistLabel.setText("Artist: " + info.artist, juce::dontSendNotification);
                albumLabel.setText("Album: " + info.album, juce::dontSendNotification);
                durationLabel.setText("Duration: " + info.durationString, juce::dontSendNotification);

                playerAudio.play();
                playlistBox.selectRow(currentIndex);
            });
    }
    else if (b == &nextButton && currentIndex + 1 < playlistFiles.size())
    {
        currentIndex++;
        auto info = playerAudio.loadFile(playlistFiles[currentIndex]);
        waveform.setFile(playlistFiles[currentIndex]);
        playerAudio.play();
        playlistBox.selectRow(currentIndex);
        titleLabel.setText("Title: " + info.title, juce::dontSendNotification);
        artistLabel.setText("Artist: " + info.artist, juce::dontSendNotification);
    }
    else if (b == &prevButton && currentIndex > 0)
    {
        currentIndex--;
        auto info = playerAudio.loadFile(playlistFiles[currentIndex]);
        waveform.setFile(playlistFiles[currentIndex]);
        playerAudio.play();
        playlistBox.selectRow(currentIndex);
        titleLabel.setText("Title: " + info.title, juce::dontSendNotification);
        artistLabel.setText("Artist: " + info.artist, juce::dontSendNotification);
    }
    else if (b == &muteButton)
    {
        playerAudio.toggleMute();
        muteButton.setButtonText(playerAudio.isMuted() ? "Unmute" : "Mute");
    }
    else if (b == &loopButton)
    {
        isLooping = !isLooping;
        playerAudio.setLooping(isLooping);
        loopButton.setButtonText(isLooping ? "Loop On" : "Loop Off");
    }
    else if (b == &playButton) playerAudio.play();
    else if (b == &pauseButton) playerAudio.pause();
    else if (b == &stopButton) playerAudio.stop();
    else if (b == &restartButton) playerAudio.restart();
    else if (b == &startButton) playerAudio.goToStart();
    else if (b == &endButton) playerAudio.goToEnd();
    else if (b == &back10Button) playerAudio.skipBackward(10.0);
    else if (b == &fwd10Button) playerAudio.skipForward(10.0);

    else if (b == &setAButton)
    {
        pointA = playerAudio.getCurrentPosition();
        setAButton.setButtonText("A: " + juce::String(pointA, 2) + "s");
        waveform.setAB(pointA, pointB);
    }
    else if (b == &setBButton)
    {
        pointB = playerAudio.getCurrentPosition();
        setBButton.setButtonText("B: " + juce::String(pointB, 2) + "s");
        waveform.setAB(pointA, pointB);
    }
    else if (b == &loopABButton)
    {
        if (pointA >= 0.0 && pointB > pointA)
        {
            abLoopEnabled = !abLoopEnabled;
            loopABButton.setButtonText(abLoopEnabled ? "Loop A-B ON" : "Loop A-B OFF");
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                "Invalid A-B",
                "Set valid A and B points first (B > A).");
        }
    }
}

int PlayerGUI::getNumRows() { return playlistNames.size(); }

void PlayerGUI::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= playlistNames.size()) return;

    g.fillAll(rowIsSelected ? juce::Colour::fromRGB(48, 60, 90) : juce::Colour::fromRGB(30, 32, 36));
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText(playlistNames[rowNumber], 8, 0, width - 12, height, juce::Justification::centredLeft);
}

void PlayerGUI::selectedRowsChanged(int lastRowSelected)
{
    if (lastRowSelected >= 0 && lastRowSelected < playlistFiles.size())
    {
        currentIndex = lastRowSelected;
        auto info = playerAudio.loadFile(playlistFiles[currentIndex]);
        waveform.setFile(playlistFiles[currentIndex]);
        playerAudio.play();
        titleLabel.setText("Title: " + info.title, juce::dontSendNotification);
        artistLabel.setText("Artist: " + info.artist, juce::dontSendNotification);
        albumLabel.setText("Album: " + info.album, juce::dontSendNotification);
        durationLabel.setText("Duration: " + info.durationString, juce::dontSendNotification);
    }
}
