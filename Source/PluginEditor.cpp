#include "PluginEditor.h"
#include "BinaryData.h"
using namespace juce;

namespace
{
    const Colour kTx (0xffe9e6dd), kAmber (0xffe8a24a);
    const int W = 1240, H = 548;

    struct Preset { const char* name; std::vector<std::pair<const char*, float>> v; };
    // keys l_* / c_* without a trailing 1/2 are applied to both channels
    const std::vector<Preset>& presets()
    {
        static const std::vector<Preset> p = {
            { "Init", {} },
            { "Drum bus",       { {"l_thr",11}, {"l_rec",1}, {"l_fast",1}, {"c_thr",-8}, {"c_rec",1}, {"c_gain",5}, {"c_ratio",2}, {"c_fast",0}, {"sch",80}, {"drive",60}, {"mix",80} } },
            { "Vocal (opto)",   { {"l_thr",13}, {"l_rec",2}, {"l_fast",0}, {"c_thr",-10}, {"c_rec",4}, {"c_gain",6}, {"c_ratio",1}, {"c_fast",0}, {"opto",1}, {"hf",30}, {"sch",100}, {"drive",40} } },
            { "Mix glue",       { {"l_thr",14}, {"l_rec",3}, {"l_fast",0}, {"c_thr",-4}, {"c_rec",4}, {"c_gain",3}, {"c_ratio",1}, {"c_fast",0}, {"sch",100}, {"drive",30} } },
            { "Parallel smash", { {"l_thr",9}, {"l_rec",0}, {"l_fast",1}, {"c_thr",-18}, {"c_rec",1}, {"c_gain",12}, {"c_ratio",4}, {"c_fast",1}, {"mix",35}, {"drive",70}, {"sch",120} } },
            { "M/S master",     { {"l_thr",14}, {"l_rec",3}, {"l_fast",0}, {"c_thr",-4}, {"c_rec",4}, {"c_gain",2}, {"c_ratio",1}, {"c_fast",0}, {"c_thr2",-6}, {"c_gain2",1}, {"ms",1}, {"link",0}, {"sch",60}, {"drive",25} } }
        };
        return p;
    }
}

//==============================================================================
PanelLNF::PanelLNF()
{
    knobImg = ImageCache::getFromMemory (BinaryData::knob_png, BinaryData::knob_pngSize);
}

void PanelLNF::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                 float a0, float a1, Slider&)
{
    auto r = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    g.drawImage (knobImg, r, RectanglePlacement::centred);

    const auto c = r.getCentre();
    const float capR = r.getWidth() * .30f;
    const float a = a0 + pos * (a1 - a0);
    Path p;
    p.addRoundedRectangle (-1.7f, -capR + 2.5f, 3.4f, capR * .62f, 1.6f);
    g.setColour (Colours::black.withAlpha (.45f));
    g.fillPath (p, AffineTransform::rotation (a).translated (c.x + 1.f, c.y + 1.5f));
    g.setColour (Colour (0xfff4f4f0));
    g.fillPath (p, AffineTransform::rotation (a).translated (c.x, c.y));
}

void PanelLNF::drawToggleButton (Graphics& g, ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat();
    const bool on = b.getToggleState();

    if (b.getComponentID() == "pill")
    {
        g.setColour (Colours::black);
        g.fillRoundedRectangle (r, r.getHeight() / 2);
        g.setColour (on ? kAmber : Colour (0xff222d38));
        g.fillRoundedRectangle (r.reduced (1.f), r.getHeight() / 2);
        g.setColour (on ? Colours::black : kTx);
        g.setFont (FontOptions (9.f, Font::bold));
        g.drawText ("LINK", b.getLocalBounds(), Justification::centred);
        return;
    }

    auto barrel = Rectangle<float> (20.f, 34.f).withCentre (r.getCentre());
    g.setColour (Colours::black.withAlpha (.55f));
    g.fillRoundedRectangle (barrel.translated (0.f, 2.f), 5.f);
    ColourGradient bg (Colour (0xff0c0c0c), barrel.getX(), 0.f, Colour (0xff0c0c0c), barrel.getRight(), 0.f, false);
    bg.addColour (.45, Colour (0xff5a5a5a));
    g.setGradientFill (bg);
    g.fillRoundedRectangle (barrel, 5.f);
    g.setColour (Colours::black);
    g.drawRoundedRectangle (barrel, 5.f, 1.f);

    const float ly = on ? barrel.getY() + 2.f : barrel.getBottom() - 17.f;
    auto lever = Rectangle<float> (14.f, 15.f).withCentre ({ barrel.getCentreX(), ly + 7.5f });
    g.setColour (Colours::black.withAlpha (.6f));
    g.fillRoundedRectangle (lever.translated (0.f, on ? 3.f : -2.f), 4.f);
    ColourGradient lg (Colour (0xff9c9c96), lever.getX(), 0.f, Colour (0xff8a8a84), lever.getRight(), 0.f, false);
    lg.addColour (.4, Colour (0xfffafaf5));
    g.setGradientFill (lg);
    g.fillRoundedRectangle (lever, 4.f);
}

//==============================================================================
PA3609Editor::PA3609Editor (PA3609Processor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);
    panelTex = ImageCache::getFromMemory (BinaryData::panel_png, BinaryData::panel_pngSize);
    screwImg = ImageCache::getFromMemory (BinaryData::screw_png, BinaryData::screw_pngSize);
    setSize (W, H);

    // sections ---------------------------------------------------------------
    const int y1 = 52, y2 = 224, sh = 164;
    secs.push_back ({ { 40, y1, 290, sh }, "LIMIT 1" });
    secs.push_back ({ { 40, y2, 290, sh }, "LIMIT 2" });
    secs.push_back ({ { 338, y1, 520, sh }, "COMPRESS 1" });
    secs.push_back ({ { 338, y2, 520, sh }, "COMPRESS 2" });
    secs.push_back ({ { 866, y1, 150, 336 }, "MODE" });
    secs.push_back ({ { 1024, y1, 176, 336 }, "GAIN REDUCTION" });
    secs.push_back ({ { 40, 396, 1160, 132 }, "OPTO LIMIT/COMP  -  HF FILTER  -  INPUT / SIDECHAIN / OUTPUT" });

    for (int ch = 0; ch < 2; ++ch)
    {
        const String s (ch + 1);
        const int cy = (ch == 0 ? y1 : y2) + 92;
        addKnob ("l_thr" + s, "threshold dBu", 95, cy);
        addKnob ("l_rec" + s, "recovery ms", 185, cy);
        addSw ("l_in" + s, "in", "out", 258, cy);
        addSw ("l_fast" + s, "fast", "slow", 302, cy);

        addKnob ("c_thr" + s, "threshold dBu", 398, cy);
        addKnob ("c_rec" + s, "recovery ms", 498, cy);
        addKnob ("c_gain" + s, "gain", 598, cy);
        addKnob ("c_ratio" + s, "ratio", 698, cy);
        addSw ("c_in" + s, "in", "out", 778, cy);
        addSw ("c_fast" + s, "fast", "slow", 823, cy);
    }
    addPill ("lk_lim", 185, 220);
    addPill ("lk_cmp", 598, 220);

    addSw ("bypass", "bypass", "in", 906, 130);
    addSw ("stereo", "stereo", "mono", 976, 130);
    addSw ("ms", "m/s", "l/r", 906, 252);
    addKnob ("link", "link %", 976, 252);

    const int by = 396 + 66;
    addSw ("opto", "opto", "diode", 100, by);
    addSw ("olim", "limit", "comp", 170, by);
    addKnob ("hf", "HF filter", 290, by);
    addKnob ("inG", "input", 440, by);
    addKnob ("sch", "sidechain HPF", 590, by);
    addKnob ("drive", "drive", 740, by);
    addKnob ("mix", "mix (parallel)", 890, by);
    addKnob ("outG", "output", 1040, by);

    // preset bar --------------------------------------------------------------
    StringArray names;
    for (auto& pr : presets()) names.add (pr.name);
    presetBox.addItemList (names, 1);
    presetBox.setSelectedId (1, dontSendNotification);
    presetBox.setBounds (440, 12, 230, 24);
    prevBtn.setBounds (408, 12, 26, 24);
    nextBtn.setBounds (676, 12, 26, 24);
    for (Component* c : { (Component*) &presetBox, (Component*) &prevBtn, (Component*) &nextBtn })
        addAndMakeVisible (c);
    presetBox.onChange = [this] { applyPreset (presetBox.getSelectedItemIndex()); };
    prevBtn.onClick = [this]
    {
        const int n = presetBox.getNumItems();
        presetBox.setSelectedItemIndex ((presetBox.getSelectedItemIndex() + n - 1) % n, sendNotification);
    };
    nextBtn.onClick = [this]
    {
        presetBox.setSelectedItemIndex ((presetBox.getSelectedItemIndex() + 1) % presetBox.getNumItems(), sendNotification);
    };

    // L/R link: register every per-channel parameter (ids like l_thr1 / c_gain2)
    for (auto* par : p.getParameters())
        if (auto* pid = dynamic_cast<AudioProcessorParameterWithID*> (par))
        {
            const auto id = pid->paramID;
            if ((id.startsWith ("l_") || id.startsWith ("c_")) && (id.endsWith ("1") || id.endsWith ("2")))
            {
                linkIds.add (id);
                prevNorm[id] = par->getValue();
                p.apvts.addParameterListener (id, this);
            }
        }
    startTimerHz (30);
}

PA3609Editor::~PA3609Editor()
{
    for (auto& id : linkIds) proc.apvts.removeParameterListener (id, this);
    setLookAndFeel (nullptr);
}

void PA3609Editor::applyPreset (int index)
{
    if (index < 0 || index >= (int) presets().size()) return;
    ignore = true;                                   // don't let L/R link interfere while loading
    auto setPlain = [this] (const String& id, float v)
    {
        if (auto* par = proc.apvts.getParameter (id))
        {
            par->beginChangeGesture();
            par->setValueNotifyingHost (par->convertTo0to1 (v));
            par->endChangeGesture();
        }
    };
    for (auto* par : proc.getParameters())
        if (auto* pid = dynamic_cast<RangedAudioParameter*> (par))
            if (pid->paramID != "bypass")
            {
                pid->beginChangeGesture();
                pid->setValueNotifyingHost (pid->getDefaultValue());
                pid->endChangeGesture();
            }

    for (auto& kv : presets()[(size_t) index].v)
    {
        const String id (kv.first);
        const bool perChannel = (id.startsWith ("l_") || id.startsWith ("c_")) && ! (id.endsWith ("1") || id.endsWith ("2"));
        if (perChannel) { setPlain (id + "1", kv.second); setPlain (id + "2", kv.second); }
        else            setPlain (id, kv.second);
    }
    ignore = false;
    for (auto& id : linkIds)
        prevNorm[id] = proc.apvts.getParameter (id)->getValue();
}

void PA3609Editor::addKnob (const String& id, const String& label, int cx, int cy)
{
    auto* k = knobs.add (new Knob());
    k->id = id; k->label = label; k->c = { cx, cy };
    auto* par = proc.apvts.getParameter (id);
    if (auto* ch = dynamic_cast<AudioParameterChoice*> (par)) { k->ticks = ch->choices; k->isChoice = true; }
    else
    {
        auto r = par->getNormalisableRange();
        for (int i = 0; i < 5; ++i) k->ticks.add (String (roundToInt (r.start + (float) i * (r.end - r.start) / 4.f)));
    }
    k->s.setRotaryParameters (MathConstants<float>::pi * 1.25f, MathConstants<float>::pi * 2.75f, true);
    k->s.setBounds (cx - 32, cy - 32, 64, 64);
    addAndMakeVisible (k->s);
    k->att = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k->s);
}

void PA3609Editor::addSw (const String& id, const String& top, const String& bot, int cx, int cy)
{
    auto* s = sws.add (new Sw());
    s->id = id; s->top = top; s->bot = bot; s->c = { cx, cy };
    s->b.setBounds (cx - 14, cy - 22, 28, 44);
    addAndMakeVisible (s->b);
    s->att = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id, s->b);
}

void PA3609Editor::addPill (const String& id, int cx, int cy)
{
    auto* s = sws.add (new Sw());
    s->id = id;
    s->b.setComponentID ("pill");
    s->b.setBounds (cx - 28, cy - 8, 56, 16);
    addAndMakeVisible (s->b);
    s->att = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id, s->b);
}

String PA3609Editor::readout (const Knob& k) const
{
    auto* par = proc.apvts.getParameter (k.id);
    if (k.isChoice) return par->getCurrentValueAsText();
    const float v = proc.apvts.getRawParameterValue (k.id)->load();
    String suffix;
    if (k.id.contains ("gain") || k.id.contains ("inG") || k.id.contains ("outG")) suffix = " dB";
    else if (k.id == "link" || k.id == "hf" || k.id == "drive" || k.id == "mix") suffix = "%";
    else if (k.id == "sch") suffix = " Hz";
    else if (k.id.contains ("thr")) suffix = " dBu";
    return String (v, 1) + suffix;
}

//==============================================================================
void PA3609Editor::drawLed (Graphics& g, Point<float> p, bool on, Colour c)
{
    if (on)
    {
        g.setGradientFill (ColourGradient (c.withAlpha (.75f), p.x, p.y, c.withAlpha (0.f), p.x + 11.f, p.y, true));
        g.fillEllipse (Rectangle<float> (22.f, 22.f).withCentre (p));
    }
    auto core = Rectangle<float> (8.f, 8.f).withCentre (p);
    g.setColour (Colours::black);
    g.fillEllipse (core.expanded (1.5f));
    g.setColour (on ? c.brighter (.5f) : Colour (0xff3a2a24));
    g.fillEllipse (core);
    g.setColour (Colours::white.withAlpha (on ? .55f : .15f));
    g.fillEllipse (Rectangle<float> (3.f, 2.f).withCentre ({ p.x - 1.2f, p.y - 1.8f }));
}

void PA3609Editor::drawMeter (Graphics& g, Rectangle<float> r, float v, const String& title)
{
    g.setColour (kTx);
    g.setFont (FontOptions (10.5f, Font::bold));
    g.drawText (title, r.toNearestInt().withHeight (14).translated (0, -18), Justification::centred);

    // housing below the glass
    auto hs = Rectangle<float> (r.getX() - 3.f, r.getBottom() + 1.f, r.getWidth() + 6.f, 40.f);
    g.setGradientFill (ColourGradient (Colour (0xff25282b), 0.f, hs.getY(), Colour (0xff0b0c0d), 0.f, hs.getBottom(), false));
    g.fillRoundedRectangle (hs, 3.f);
    g.setColour (Colour (0xff000000));
    g.fillEllipse (Rectangle<float> (13.f, 13.f).withCentre ({ hs.getCentreX(), hs.getCentreY() + 2.f }));
    g.setColour (Colour (0xff6b6d70));
    g.drawLine (hs.getCentreX() - 4.f, hs.getCentreY() + 6.f, hs.getCentreX() + 4.f, hs.getCentreY() - 2.f, 1.4f);

    // bezel + cream VU face
    g.setColour (Colour (0xff050505));
    g.fillRoundedRectangle (r.expanded (3.f), 4.f);
    g.setGradientFill (ColourGradient (Colour (0xfff2eedd), r.getX(), r.getY(), Colour (0xffd6d0b8), r.getRight(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 3.f);

    g.saveState();
    g.reduceClipRegion (r.toNearestInt());
    const float s = r.getWidth() / 200.f;
    const float px = r.getX() + 100.f * s, py = r.getY() + 150.f * s;
    g.setFont (FontOptions (9.f, Font::bold));
    for (int t = 0; t <= 20; t += 4)
    {
        const float a = degreesToRadians (-38.f + 76.f * (float) t / 20.f);
        const float sn = std::sin (a), cs = -std::cos (a);
        g.setColour (t >= 16 ? Colour (0xffb02a20) : Colour (0xff1a1a1a));
        g.drawLine (px + sn * 98.f * s, py + cs * 98.f * s, px + sn * 108.f * s, py + cs * 108.f * s, 1.6f);
        g.drawText (String (t), Rectangle<int> (14, 12).withCentre ({ (int) (px + sn * 86.f * s), (int) (py + cs * 86.f * s) }),
                    Justification::centred);
    }
    g.setColour (Colour (0xff1a1a1a));
    g.drawText ("dB", r.toNearestInt().withY ((int) (r.getY() + 70.f * s)).withHeight (12), Justification::centred);

    const float a = degreesToRadians (-38.f + 76.f * jlimit (0.f, 22.f, v) / 20.f);
    g.setColour (Colours::black.withAlpha (.25f));
    g.drawLine (px + 3.f, py + 2.f, px + std::sin (a) * 112.f * s + 3.f, py - std::cos (a) * 112.f * s + 2.f, 2.f);
    g.setColour (Colour (0xff121212));
    g.drawLine (px, py, px + std::sin (a) * 112.f * s, py - std::cos (a) * 112.f * s, 1.8f);

    // glass reflection
    g.setGradientFill (ColourGradient (Colours::white.withAlpha (.35f), r.getX(), r.getY(),
                                       Colours::white.withAlpha (.0f), r.getCentreX(), r.getBottom(), false));
    Path glare;
    glare.startNewSubPath (r.getX(), r.getY());
    glare.lineTo (r.getRight(), r.getY());
    glare.lineTo (r.getX(), r.getCentreY() + 8.f);
    glare.closeSubPath();
    g.fillPath (glare);
    g.setGradientFill (ColourGradient (Colours::transparentBlack, r.getCentreX(), r.getCentreY(),
                                       Colours::black.withAlpha (.35f), r.getX(), r.getY(), true));
    g.fillRect (r);
    g.restoreState();
}

void PA3609Editor::paint (Graphics& g)
{
    // faceplate: tiled brushed-navy texture + vignette
    g.setTiledImageFill (panelTex, 0, 0, 1.f);
    g.fillAll();
    g.setGradientFill (ColourGradient (Colours::transparentBlack, W * .5f, H * .5f,
                                       Colours::black.withAlpha (.5f), 0.f, 0.f, true));
    g.fillAll();
    g.setColour (Colours::white.withAlpha (.2f));
    g.fillRect (0, 0, W, 2);
    g.setColour (Colours::black.withAlpha (.6f));
    g.fillRect (0, H - 3, W, 3);

    // rack ears + screws
    for (int side = 0; side < 2; ++side)
    {
        const int ex = side == 0 ? 6 : W - 30;
        g.setColour (Colours::white.withAlpha (.07f));
        g.fillRoundedRectangle ((float) ex, 8.f, 24.f, (float) H - 16.f, 3.f);
        g.setColour (Colours::black.withAlpha (.35f));
        g.drawRoundedRectangle ((float) ex, 8.f, 24.f, (float) H - 16.f, 3.f, 1.f);
        for (int yy : { 60, H - 60 })
        {
            g.setColour (Colour (0xff070809));
            g.fillEllipse (Rectangle<float> (16.f, 11.f).withCentre ({ (float) ex + 12.f, (float) yy }));
            g.setColour (Colour (0xffb9bec2));
            g.drawEllipse (Rectangle<float> (18.f, 13.f).withCentre ({ (float) ex + 12.f, (float) yy }), 1.5f);
        }
        g.setImageResamplingQuality (Graphics::highResamplingQuality);
        g.drawImage (screwImg, Rectangle<float> ((float) ex + 1.f, (float) H / 2 - 11.f, 22.f, 22.f));
    }

    // header
    g.setColour (Colours::black.withAlpha (.5f));
    g.setFont (FontOptions (22.f, Font::bold));
    g.drawText ("PARAGON AUDIO", 41, 10, 240, 30, Justification::left);
    g.setColour (kAmber);
    g.drawText ("PARAGON AUDIO", 40, 8, 240, 30, Justification::left);
    g.setColour (kTx.withAlpha (.8f));
    g.setFont (FontOptions (11.f, Font::bold));
    g.drawText ("PA-3609", 275, 8, 100, 30, Justification::left);

    auto bar = [&] (float x, float y, float w, float db, const String& t)
    {
        g.setColour (Colour (0xff050607)); g.fillRoundedRectangle (x - 1.f, y - 1.f, w + 2.f, 12.f, 2.f);
        const float f = jlimit (0.f, 1.f, (db + 60.f) / 60.f) * w;
        g.setColour (db > -3 ? Colour (0xffe0473c) : db > -12 ? Colour (0xffe8c24a) : Colour (0xff3fbf6f));
        g.fillRect (x, y, f, 10.f);
        g.setColour (kTx); g.setFont (FontOptions (11.f, Font::bold));
        g.drawText (t, (int) x - 34, (int) y - 2, 30, 14, Justification::right);
    };
    auto fall = [] (float& d, float v) { d = jmax (20.f * std::log10 (v + 1e-9f), d - 1.f); return d; };
    bar (900.f, 18.f, 130.f, fall (dispIn,  proc.inPk.load()),  "in");
    bar (1070.f, 18.f, 130.f, fall (dispOut, proc.outPk.load()), "out");

    // sections: recessed, bevelled
    for (auto& s : secs)
    {
        auto r = s.r.toFloat();
        g.setColour (Colours::black.withAlpha (.22f));
        g.fillRoundedRectangle (r, 3.f);
        g.setColour (Colours::black.withAlpha (.55f));
        g.drawRoundedRectangle (r, 3.f, 1.5f);
        g.setColour (Colour (0xff9aa3ab).withAlpha (.65f));
        g.drawRoundedRectangle (r.reduced (1.f), 3.f, 1.f);
        g.setColour (kTx);
        g.setFont (FontOptions (12.f, Font::bold));
        g.drawText (s.name, s.r.withTrimmedTop (s.r.getHeight() - 24), Justification::centred);
    }

    // knob labels, ticks, readouts
    for (auto* k : knobs)
    {
        g.setColour (kTx);
        g.setFont (FontOptions (10.5f, Font::bold));
        g.drawText (k->label, k->c.x - 50, k->c.y - 66, 100, 13, Justification::centred);

        g.setFont (FontOptions (9.f, Font::bold));
        const int n = k->ticks.size();
        for (int i = 0; i < n; ++i)
        {
            const float t = n > 1 ? (float) i / (float) (n - 1) : 0.f;
            const float a = MathConstants<float>::pi * 1.25f + t * MathConstants<float>::pi * 1.5f;
            const int tx = k->c.x + (int) std::lround (std::sin (a) * 41.f);
            const int ty = k->c.y - (int) std::lround (std::cos (a) * 41.f);
            g.drawText (k->ticks[i], tx - 17, ty - 6, 34, 12, Justification::centred);
        }
        g.setFont (FontOptions (10.f, Font::plain));
        g.setColour (kTx.withAlpha (.8f));
        g.drawText (readout (*k), k->c.x - 45, k->c.y + 37, 90, 12, Justification::centred);
    }

    // switch labels + LEDs
    g.setFont (FontOptions (10.f, Font::bold));
    for (auto* s : sws)
    {
        if (s->b.getComponentID() == "pill") continue;
        g.setColour (kTx);
        g.drawText (s->top, s->c.x - 28, s->c.y - 36, 56, 12, Justification::centred);
        g.drawText (s->bot, s->c.x - 28, s->c.y + 25, 56, 12, Justification::centred);
        if (s->id == "bypass" || s->id == "stereo" || s->id == "ms")
        {
            const bool on = s->b.getToggleState();
            const Colour col = s->id == "bypass" ? Colour (0xffff3b2a) : Colour (0xffffa63a);
            drawLed (g, { (float) s->c.x + 26.f, (float) s->c.y - 12.f }, on, col);
            drawLed (g, { (float) s->c.x + 26.f, (float) s->c.y + 12.f }, ! on, col);
        }
    }

    // needle meters
    for (int i = 0; i < 2; ++i)
        dispGr[i] += (proc.grMeter[i].load() - dispGr[i]) * .25f;
    drawMeter (g, Rectangle<float> (1036.f, 100.f, 152.f, 91.f), dispGr[0], "gain reduction 1");
    drawMeter (g, Rectangle<float> (1036.f, 250.f, 152.f, 91.f), dispGr[1], "gain reduction 2");
}

//==============================================================================
// May be called from the audio thread (host automation), so hop to the message thread.
void PA3609Editor::parameterChanged (const String& id, float newValue)
{
    if (ignore) return;
    auto* par = proc.apvts.getParameter (id);
    if (par == nullptr) return;
    const float nn = par->convertTo0to1 (newValue);
    MessageManager::callAsync ([safe = Component::SafePointer<PA3609Editor> (this), id, nn]
                               { if (safe != nullptr) safe->applyLink (id, nn); });
}

void PA3609Editor::applyLink (const String& id, float nn)
{
    const float prev = prevNorm[id];
    prevNorm[id] = nn;
    if (ignore) return;

    const bool isLim = id.startsWith ("l_");
    if (proc.apvts.getRawParameterValue (isLim ? "lk_lim" : "lk_cmp")->load() < .5f) return;

    const float d = nn - prev;
    if (std::abs (d) < 1.0e-6f) return;

    const String other = id.dropLastCharacters (1) + (id.endsWith ("1") ? "2" : "1");
    auto* op = proc.apvts.getParameter (other);
    if (op == nullptr) return;

    const bool isSwitch = dynamic_cast<AudioParameterBool*> (op) != nullptr;
    const float target = isSwitch ? nn : jlimit (0.f, 1.f, op->getValue() + d);

    ignore = true;
    op->beginChangeGesture();
    op->setValueNotifyingHost (target);
    op->endChangeGesture();
    ignore = false;
    prevNorm[other] = op->getValue();
}
