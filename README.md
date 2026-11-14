# Paragon PA-3609 (Windows VST3)

JUCE 8 / CMake port of the PA-3609 web prototype DSP: limiter + diode/opto compressor per channel,
stereo link, mid/side, HF-emphasis detector, sidechain HPF, drive, parallel mix, in/out trim, meters.

## Get the installer (easiest): GitHub Actions
1. Create a new GitHub repo and push this folder to it.
2. Open the repo's Actions tab -> "Build Windows installer" -> latest run.
3. Download the artifact "Paragon-PA3609-VST3-Windows" -> run Paragon-PA3609-VST3-Setup.exe.

## Build locally
Requirements: Windows 10/11, Visual Studio 2022 (Desktop C++), CMake 3.22+, Git, Inno Setup 6.
    cmake -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer\PA3609.iss
The installer appears in installer\Output\. Or run installer\Install-VST3-Manual.bat as Administrator next to the .vst3 folder, or copy the "Paragon PA-3609.vst3" folder from
build\PA3609_artefacts\Release\VST3\ into C:\Program Files\Common Files\VST3\ by hand.

## Notes
- JUCE is AGPL-3 or commercial: a closed-source release needs a JUCE commercial license.
- Unsigned installers trigger Windows SmartScreen; sign with a code-signing certificate before shipping.
- The UI is a functional parameter panel; the skinned faceplate from the web version is the next step.
- 64-bit VST3 only. No oversampling yet (the saturation can alias at high drive).

## Code signing (removes the SmartScreen "unknown publisher" warning)
Signing needs a code-signing certificate issued to Paragon Audio; it cannot be generated for you.
1. Buy an OV/EV code-signing certificate (e.g. from SSL.com, Sectigo, DigiCert), or for an open-source
   project apply to SignPath Foundation (free signing for OSS) or use Azure Trusted Signing.
2. Export the certificate with its private key as a .pfx file.
3. In PowerShell: [Convert]::ToBase64String([IO.File]::ReadAllBytes("cert.pfx")) | Set-Clipboard
4. GitHub repo -> Settings -> Secrets and variables -> Actions -> add:
   - WIN_CERT_PFX_BASE64  (the base64 text)
   - WIN_CERT_PASSWORD    (the .pfx password)
5. Re-run the workflow. It then signs the plugin DLL and the installer (SHA-256, timestamped).
Without the secrets the workflow still builds, just unsigned. Never commit the .pfx to the repo.
Note: OV certificates build SmartScreen reputation over time; EV certificates get it immediately.

## L/R link
Two switches in the control grid, "Link Limit L/R" and "Link Comp L/R" (both on by default).
With a link on, moving a control on channel 1 or 2 moves the matching control on the other channel
by the same amount (offsets are kept; switches mirror). The editor must be open for linking to act.

## Interface
The editor is a fixed-layout skinned faceplate (blue knobs with tick scales, chrome toggle levers,
LINK pills, two needle gain-reduction meters, in/out peak bars). All drawing is in Source/PluginEditor.cpp
(PanelLNF + PA3609Editor::paint), so colours and positions are easy to tweak.
