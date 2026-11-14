#pragma once
#include "PluginProcessor.h"
#include <map>
#include <vector>

// Custom drawing: bitmap knobs, chrome toggle levers, LINK pills
class PanelLNF : public juce::LookAndFeel_V4
{
public:
    PanelLNF();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float a0, float a1, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
private:
    juce::Image knobImg;
};

class PA3609Editor : public juce::AudioProcessorEditor,
                     private juce::Timer,
                     private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit PA3609Editor (PA3609Processor&);
    ~PA3609Editor() override;
    void paint (juce::Graphics&) override;
    void resized() override {}

private:
    struct Knob
    {
        juce::Slider s { juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox };
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
        juce::String id, label;
        juce::StringArray ticks;
        juce::Point<int> c;
        bool isChoice = false;
    };
    struct Sw
    {
        juce::ToggleButton b;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> att;
        juce::String id, top, bot;
        juce::Point<int> c;
    };
    struct Sec { juce::Rectangle<int> r; juce::String name; };

    void addKnob (const juce::String& id, const juce::String& label, int cx, int cy);
    void addSw (const juce::String& id, const juce::String& top, const juce::String& bot, int cx, int cy);
    void addPill (const juce::String& id, int cx, int cy);
    void drawMeter (juce::Graphics&, juce::Rectangle<float>, float v, const juce::String& title);
    void drawLed (juce::Graphics&, juce::Point<float>, bool on, juce::Colour);
    void applyPreset (int index);
    juce::String readout (const Knob&) const;

    void timerCallback() override { repaint(); }
    void parameterChanged (const juce::String& id, float newValue) override;
    void applyLink (const juce::String& id, float newNorm);

    PanelLNF lnf;
    PA3609Processor& proc;
    juce::Image panelTex, screwImg;
    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" };
    juce::OwnedArray<Knob> knobs;
    juce::OwnedArray<Sw> sws;
    std::vector<Sec> secs;
    juce::StringArray linkIds;
    std::map<juce::String, float> prevNorm;
    bool ignore = false;
    float dispGr[2] { 0, 0 }, dispIn = -60, dispOut = -60;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PA3609Editor)
};
