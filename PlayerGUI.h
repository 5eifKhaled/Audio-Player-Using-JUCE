#pragma once

#include <JuceHeader.h>
#include "PlayerAudio.h"

// WaveformComponent is embedded here so you don't need another file.
class WaveformComponent : public juce::Component,
    private juce::ChangeListener
{
public:
    WaveformComponent()
        : thumbnailCache(5),
        thumbnail(512, formatManager, thumbnailCache)
    {
        formatManager.registerBasicFormats();
        thumbnail.addChangeListener(this);
    }

    ~WaveformComponent() override
    {
        thumbnail.removeChangeListener(this);
    }

    // Load a file into the thumbnail
    void setFile(const juce::File& f)
    {
        file = f;
        thumbnail.clear();
        if (file.existsAsFile())
            thumbnail.setSource(new juce::FileInputSource(file));
        repaint();
    }

    // position/length update from player
    void setPosition(double p) { currentPosition = p; repaint(); }
    void setLength(double l) { totalLength = l;    repaint(); }

    // A/B markers
    void setAB(double a, double b) { aMarker = a; bMarker = b; repaint(); }

    // callback used when user clicks/drag on waveform to seek
    std::function<void(double)> onPositionSelected;

    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced(6.0f);

        // background with simple shadow path
        juce::Path p;
        p.addRoundedRectangle(r, 8.0f);
        juce::DropShadow ds(juce::Colours::black.withAlpha(0.55f), 6, juce::Point<int>(0, 2));
        ds.drawForPath(g, p);

        g.setColour(juce::Colour::fromRGB(22, 24, 28));
        g.fillRoundedRectangle(r, 8.0f);

        if (thumbnail.getTotalLength() > 0.005)
        {
            auto drawR = getLocalBounds().reduced(12);
            juce::ColourGradient grad(juce::Colour::fromRGB(0, 195, 165),
                (float)drawR.getX(), (float)drawR.getY(),
                juce::Colour::fromRGB(0, 155, 255),
                (float)drawR.getRight(), (float)drawR.getBottom(),
                false);
            g.setGradientFill(grad);

            thumbnail.drawChannels(g, drawR, 0.0, thumbnail.getTotalLength(), 0.9f);

            double len = totalLength > 0.0 ? totalLength : thumbnail.getTotalLength();
            double pos = currentPosition;

            // played overlay
            if (len > 0.0)
            {
                float playedX = (float)((pos / len) * (float)getWidth());
                g.setColour(juce::Colours::black.withAlpha(0.18f));
                g.fillRect(0, drawR.getY(), (int)playedX, drawR.getHeight());
            }

            // A/B markers
            if (aMarker >= 0.0 && len > 0.0)
            {
                float ax = (float)((aMarker / len) * getWidth());
                g.setColour(juce::Colours::green.withAlpha(0.95f));
                g.drawVerticalLine((int)ax, drawR.getY(), drawR.getBottom());
            }
            if (bMarker >= 0.0 && len > 0.0)
            {
                float bx = (float)((bMarker / len) * getWidth());
                g.setColour(juce::Colours::red.withAlpha(0.95f));
                g.drawVerticalLine((int)bx, drawR.getY(), drawR.getBottom());
            }
            if (aMarker >= 0.0 && bMarker > aMarker && len > 0.0)
            {
                float ax = (float)((aMarker / len) * getWidth());
                float bx = (float)((bMarker / len) * getWidth());
                juce::Rectangle<float> region(ax, drawR.getY(), bx - ax, drawR.getHeight());
                g.setColour(juce::Colours::cyan.withAlpha(0.12f));
                g.fillRect(region);
            }

            // playhead
            if (len > 0.0)
            {
                float x = (float)((pos / len) * getWidth());
                g.setColour(juce::Colours::white);
                g.drawVerticalLine((int)x, drawR.getY() + 2.0f, drawR.getBottom() - 2.0f);
            }
        }
        else
        {
            g.setColour(juce::Colours::white.withAlpha(0.06f));
            g.drawFittedText("No waveform loaded", getLocalBounds(), juce::Justification::centred, 1);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override { seekFromMouse(e.position.x); }
    void mouseDrag(const juce::MouseEvent& e) override { seekFromMouse(e.position.x); }

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override { repaint(); }

    void seekFromMouse(float mouseX)
    {
        double len = totalLength > 0.0 ? totalLength : thumbnail.getTotalLength();
        if (len <= 0.0001) return;
        float x = juce::jlimit(0.0f, (float)getWidth(), mouseX);
        double pos = (double)(x / (float)getWidth()) * len;
        if (onPositionSelected) onPositionSelected(pos);
    }

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    juce::File file;

    double currentPosition = 0.0;
    double totalLength = 0.0;
    double aMarker = -1.0;
    double bMarker = -1.0;
};

// =================== PlayerGUI ===================
class PlayerGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::ListBoxModel,
    public juce::Timer
{
public:
    PlayerGUI();
    ~PlayerGUI() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Listeners
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void timerCallback() override;

    // playlist (ListBoxModel)
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;

    // expose audio if needed
    PlayerAudio& getPlayerAudio() noexcept { return playerAudio; }

private:
    PlayerAudio playerAudio;

    // Buttons
    juce::TextButton loadButton{ "Load" }, playButton{ "Play" }, pauseButton{ "Pause" },
        stopButton{ "Stop" }, restartButton{ "Restart" }, muteButton{ "Mute" },
        loopButton{ "Loop" }, startButton{ "|<" }, endButton{ ">|" },
        back10Button{ "<10s" }, fwd10Button{ "10s>" }, nextButton{ "Next>>" }, prevButton{ "<<Prev" };

    // A-B
    juce::TextButton setAButton{ "Set A" }, setBButton{ "Set B" }, loopABButton{ "Loop A-B" };

    // sliders + labels
    juce::Slider volumeSlider, progressSlider, speedSlider;
    juce::Label speedLabel, titleLabel, artistLabel, albumLabel, durationLabel;

    // playlist UI
    juce::ListBox playlistBox;
    juce::Array<juce::File> playlistFiles;
    juce::StringArray playlistNames;
    int currentIndex = -1;

    // waveform component (embedded)
    WaveformComponent waveform;

    std::unique_ptr<juce::FileChooser> fileChooser;

    // A-B state
    bool abLoopEnabled = false;
    double pointA = -1.0;
    double pointB = -1.0;

    bool isLooping = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};
