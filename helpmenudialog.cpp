#include"helpmenudialog.h"
#include <QApplication>
#include<QString>
HelpMenuDialog::HelpMenuDialog(HelpType type, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(getTitle(type));
    setModal(true);
    resize(600, 500);

    setupUI();
    loadContent(type);
}

void HelpMenuDialog::setupUI()
{
    m_layout = new QVBoxLayout(this);

    m_contentArea = new QTextEdit(this);
    m_contentArea->setReadOnly(true);
   m_contentArea->setAcceptRichText(true);


    m_closeButton = new QPushButton("Close", this);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);

    m_layout->addWidget(m_contentArea);
    m_layout->addWidget(m_closeButton);
}

void HelpMenuDialog::loadContent(HelpType type)
{
    QString content;

    switch (type) {
    case HelpType::About:
        content = getAboutContent();
        break;
    case HelpType::Features:
        content = getFeaturesContent();
        break;
    case HelpType::Instructions:
        content = getInstructionsContent();
        break;
    case HelpType::BestPractices:
        content = getBestPracticesContent();
        break;
    case HelpType::WhatsNew:
        content = getWhatsNewContent();
        break;
    case HelpType::Shortcuts:
        content = getShortcutsContent();
        break;
    case HelpType::Security:
        content = getSecurityContent();
        break;
    case HelpType::TwoFA:
        content = getTwoFAContent();
        break;
    case HelpType::DataManagement:
        content = getDataManagementContent();
        break;
    case HelpType::DownloadManagement:
        content = getDownloadManagerContent();
        break;
    case HelpType::onSitesAndSessions:
        content = getSitesSessionsContent();
        break;
    case HelpType::onSecurity:
        content = getOnSecurityContent();
        break;
    case HelpType::onNewStorageSystem:
        content = getOnNewStorageSystemContent();
        break;
    case HelpType::onNamedProfiles:
        content = getOnNamedProfilesContent();
        break;
    case HelpType::onChangelog:
        content = getChangelogContent();
        break;
    case HelpType::supportUs:
        content = getSupportusContent();
        break;
    }


    m_contentArea->setHtml(content);
}

QString HelpMenuDialog::getTitle(HelpType type)
{
    switch (type) {
    case HelpType::About: return "About BinauralPlayer";
    case HelpType::Features: return "Features";
    case HelpType::Instructions: return "Instructions";
    case HelpType::BestPractices: return "Best Practices";
    case HelpType::WhatsNew: return "What's New";
    case HelpType::Shortcuts: return "Keyboard Shortcuts";
    case HelpType::Security: return "Jasmine Security";
    case HelpType::TwoFA: return "Jasmine 2FA Utility";
    case HelpType::DataManagement: return "Jasmine Data Management";
    case HelpType::DownloadManagement: return "Jasmine Download Management";
    case HelpType::onSitesAndSessions: return "On Sites And Sessions";
    case HelpType::onSecurity: return "On Security";
    case HelpType::onNewStorageSystem: return "On the New Storage System";
    case HelpType::onNamedProfiles: return "On Shared Named Profiles";
    case HelpType::onChangelog: return "Changelog";

    case HelpType::supportUs: return "Support Us";

    default: return "Help";
    }
}

QString HelpMenuDialog::getBestPracticesContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">

            <h1 style="color: #8e44ad; text-align: center; margin-bottom: 20px;">
                🧠 Best Practices for Binaural & Isochronic Audio
            </h1>

            <p>
                Brainwave entrainment is the practice of using rhythmic audio—such as binaural beats
                or isochronic tones—to gently guide the brain into specific states of relaxation,
                focus, meditation, or sleep. While extremely helpful when used correctly, it’s important
                to understand how it works and how to use it safely.
            </p>

            <h2 style="color: #3498db;">🌊 What Are Binaural Beats?</h2>
            <p>
                Binaural beats occur when you play slightly different frequencies in each ear.
                The brain detects the difference between the tones and begins synchronizing with
                the resulting “beat frequency.” This effect requires <strong>headphones</strong>.
            </p>

            <h2 style="color: #3498db;">⚡ What Are Isochronic Tones?</h2>
            <p>
                Isochronic tones are single, sharply-pulsed tones that turn on and off at a steady rhythm.
                They do *not* require headphones and are considered one of the strongest forms of
                brainwave entrainment.
            </p>

            <h2 style="color: #27ae60;">✨ Potential Benefits</h2>
            <ul style="padding-left: 20px;">
                <li>Deep relaxation and meditation assistance</li>
                <li>Improved focus and concentration</li>
                <li>Increased creativity and problem-solving</li>
                <li>Enhanced sleep and dream clarity</li>
                <li>Stress and anxiety reduction</li>
                <li>Mood balancing and emotional grounding</li>
            </ul>

            <h2 style="color: #c0392b;">⚠️ Safety & Dangers</h2>
            <ul style="padding-left: 20px;">
                <li>Do <strong>not</strong> use while driving or operating machinery</li>
                <li>Avoid high-intensity frequencies if you are prone to seizures or epilepsy</li>
                <li>Start with short sessions (10–15 minutes) before longer ones</li>
                <li>If you feel dizziness or discomfort, stop immediately</li>
                <li>Use comfortable volume levels—entrainment does not require loud audio</li>
                <li>Never force meditation states; let the brain shift naturally</li>
            </ul>

            <h2 style="color: #2980b9;">📜 Useful Frequency Lists</h2>
            <p>
                A large list of brainwave-related frequencies can be found here:
            </p>
            <ul style="padding-left: 20px;">
                <li><a href="https://www.lunarsight.com/freq.htm">https://www.lunarsight.com/freq.htm</a></li>
                <li><a href="https://docs.preterhuman.net/Brainwave/Cymatic_Frequency_Listing">https://docs.preterhuman.net/Brainwave/Cymatic_Frequency_Listing</a></li>
            </ul>

            <h2 style="color: #16a085;">🌿 Background Ambience</h2>
            <p>
                Atmospheric nature sounds greatly enhance the experience. High-quality,
                royalty-free ambient audio can be found at:
            </p>
            <p>
                <a href="https://pixabay.com/sound-effects/search/nature/">https://pixabay.com/sound-effects/search/nature/</a>
            </p>

            <h2 style="color: #8e44ad;">💡 Best Practices</h2>
            <ul style="padding-left: 20px;">
                <li>Use good headphones for binaural beats</li>
                <li>Use speakers or headphones for isochronic tones</li>
                <li>Meditate or sit comfortably during sessions</li>
                <li>Combine tones with relaxing ambience or maybe some relaxxing music for deeper immersion</li>
                <li>Choose frequencies appropriate for your goal (sleep, focus, creativity, etc.)</li>
                <li>Consistency matters—small daily sessions are better than long sporadic ones</li>
            </ul>

            <div style="background-color: #f8f9fa; padding: 15px; border-radius: 8px; margin-top: 25px; text-align: center;">
                <strong>
                    Brainwave entrainment is a powerful tool—use it mindfully, with intention,
                    and always listen to your body.
                </strong>
            </div>

        </div>
    )";
}


QString HelpMenuDialog::getWhatsNewContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #8e44ad; text-align: center; margin-bottom: 20px;">🌳 What's New - Nature Toolbar</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Introducing the Ambient Sound Control System</h2>
                <p>A powerful new feature that lets you create and control layered nature soundscapes
                for relaxation, focus, meditation, and atmospheric enhancement.</p>
            </div>

            <h2 style="color: #2E8B57; border-bottom: 2px solid #2E8B57; padding-bottom: 5px;">🌿 Nature Toolbar Features</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #2E8B57;">🎚️ 5-Channel Ambient Mixer</h3>
                <p>Five independent sound players that can run simultaneously. Create complex sound environments
                by mixing rain, forest, thunder, ocean, and more.</p>

                <h3 style="color: #2E8B57;">🔘 Global Power Control</h3>
                <p>Single power button to enable/disable all nature sounds at once. Perfect for quick mute
                or instant atmosphere activation.</p>

                <h3 style="color: #2E8B57;">⏯️ Master Transport Controls</h3>
                <p>Play, pause, and stop ALL active nature sounds simultaneously. Color-coded buttons provide
                clear visual feedback: green (play), orange (pause), red (stop).</p>

                <h3 style="color: #2E8B57;">🎛️ Individual Player Buttons</h3>
                <p>Each channel has its own button showing name, state, and status. Single-click toggles
                play/pause. Color indicates status: green (playing), orange (paused), gray (disabled).</p>

                <h3 style="color: #2E8B57;">📊 Master Volume Control</h3>
                <p>Slider controls overall ambient sound level with perceptual volume curve for natural
                human hearing response.</p>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">💾 Preset System</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #9b59b6;">💽 Save Configurations</h3>
                <p>Store complete sound setups including audio files, volumes, names, and enabled states.</p>

                <h3 style="color: #9b59b6;">📂 Load Presets</h3>
                <p>Quickly switch between saved environments like "Rainy Forest", "Ocean Waves", or "Focus Study".</p>

                <h3 style="color: #9b59b6;">🔄 Reset Function</h3>
                <p>One-click restoration to default settings with confirmation dialog for safety.</p>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">🎵 Player Capabilities</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #e67e22;">🎧 Multi-Format Audio</h3>
                <p>Supports MP3, WAV, OGG, FLAC, and M4A files with integrated file browser.</p>

                <h3 style="color: #e67e22;">🔤 Custom Naming</h3>
                <p>Give each channel descriptive names up to 10 characters (e.g., "Rain", "Birds", "Thunder").</p>

                <h3 style="color: #e67e22;">🔁 Auto-Repeat</h3>
                <p>Loop audio files seamlessly for continuous background ambience.</p>

                <h3 style="color: #e67e22;">📏 Individual Volume</h3>
                <p>Fine-tune each channel's volume independently (0-100%).</p>

                <h3 style="color: #e67e22;">⏱️ Progress Tracking</h3>
                <p>See playback progress with seek capability in detailed settings dialog.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🖥️ Interface Design</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #3498db;">🎨 Non-Destructive Dialogs</h3>
                <p>Player settings dialogs hide instead of close, preserving your configuration state.</p>

                <h3 style="color: #3498db;">👁️ Visual Status Indicators</h3>
                <p>Buttons show real-time status with icons and colors for instant recognition.</p>

                <h3 style="color: #3498db;">🔍 Tooltip Guidance</h3>
                <p>All controls have descriptive tooltips explaining their function.</p>

                <h3 style="color: #3498db;">📱 Responsive Layout</h3>
                <p>Clean toolbar design that integrates seamlessly with existing interface.</p>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🚀 Quick Start Guide</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #1abc9c;">⚡ Getting Started</h3>
                <ol style="margin-left: 20px; padding-left: 0;">
                    <li><strong>Power On</strong>: Click the ○ button to activate the system</li>
                    <li><strong>Add Sounds</strong>: Click any player button (P1-P5) to open settings</li>
                    <li><strong>Browse Files</strong>: Select nature sound files for each channel</li>
                    <li><strong>Customize Names</strong>: Give descriptive names to each channel</li>
                    <li><strong>Adjust Volumes</strong>: Set levels to create your perfect mix</li>
                    <li><strong>Control Playback</strong>: Use master controls or click individual buttons</li>
                </ol>


 <h3 style="color: #1abc9c;">🎵 Download Nature Sounds</h3>
                <p>Get high-quality, royalty-free ambient sounds from:</p>
                <div style="background-color: #f0f9ff; padding: 15px; border-radius: 8px; border-left: 4px solid #3498db; margin: 15px 0;">
                    <p><strong>🌐 Pixabay Sound Effects:</strong> <a href="https://pixabay.com/sound-effects/search/nature/" style="color: #3498db; text-decoration: none;">https://pixabay.com/sound-effects/search/nature/</a></p>
                    <p>After downloading, place your audio files in:</p>
                    <div style="background-color: #e8f4f8; padding: 10px; border-radius: 6px; font-family: monospace; margin: 10px 0;">
                        ~/Documents/BinauralPlayer/ambient-tracks/
                    </div>
                    <p>Recommended downloads:</p>
                    <ul style="margin-left: 20px; padding-left: 0;">
                        <li>Rain & Thunderstorms</li>
                        <li>Forest & Birds</li>
                        <li>Ocean Waves</li>
                        <li>Streams & Rivers</li>
                        <li>Wind & Nature Ambience</li>
                    </ul>
                    <p style="font-style: italic; margin-top: 10px;">💡 <strong>Tip:</strong> The ambient player will automatically look for files in this directory when browsing!</p>
                </div>

                <h3 style="color: #1abc9c;">💡 Pro Tips</h3>
                <ul style="margin-left: 20px; padding-left: 0;">
                    <li>Start with 2-3 complementary sounds for best results</li>
                    <li>Save your favorite mixes as presets for quick access</li>
                    <li>Use the power button for instant silence during calls</li>
                    <li>Combine with binaural beats for enhanced meditation</li>
                </ul>
            </div>

            <div style="background-color: #f8f9fa; padding: 20px; border-radius: 8px; text-align: center; margin-top: 30px;">
                <h3 style="color: #2c3e50; margin-top: 0;">🌟 Transform Your Audio Environment</h3>
                <p style="margin-bottom: 0;">The Nature Toolbar brings professional ambient sound control to your fingertips.
                Create your perfect soundscape for relaxation, focus, or atmospheric enhancement in minutes!</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getShortcutsContent()
{
    return QString();  // TODO: Implement
}

QString HelpMenuDialog::getAboutContent() {
    return QString(R"(
        <div style="text-align: center; font-family: Arial, sans-serif;">
            <h1 style="color: #2c3e50; margin-bottom: 10px;">🎧 BinauralPlayer</h1>
            <h3 style="color: #7f8c8d; margin-bottom: 20px;">Multimedia & Brainwave Audio Engine</h3>
            <p style="font-size: 16px; margin-bottom: 20px;">
                A powerful desktop application for media playback, binaural and isochronic
                tone generation, and real-time brainwave entrainment. Built with modern Qt 6
                technologies and a high-performance C++17 audio engine.
            </p>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin: 20px 0;">
                <p style="margin: 5px 0;"><strong>Version:</strong> %1</p>
                <p style="margin: 5px 0;"><strong>Built with:</strong> Qt Framework (Qt 6)</p>
                <p style="margin: 5px 0;"><strong>Platform:</strong> Cross-platform</p>
            </div>

            <div style="margin: 30px 0;">
                <h4 style="color: #2c3e50;">Key Features</h4>

                <p style="text-align: left; margin: 10px 20px;">
                    • Multi-format media playback (MP3, FLAC, OGG, WAV, MP4, MKV)<br>
                    • Real-time binaural & isochronic tone generation<br>
                    • Dynamic waveform engine (Sine, Square, Triangle, Sawtooth)<br>
                    • Tabbed playlist management with drag & drop<br>
                    • Streaming support for direct audio URLs<br>
                    • JSON presets and playlist saving<br>
                    • Safety warnings and customizable UI elements
                </p>

                <p>Available for Linux, Windows, and macOS.</p>
<p>
  Project Repository
  <br>
  <a href="https://github.com/alamahant/BinauralPlayer">https://github.com/alamahant/BinauralPlayer</a>
</p>
            </div>

            <hr style="margin: 30px 0; border: 1px solid #bdc3c7;">

            <div style="margin: 20px 0;">
                <p style="margin: 5px 0; color: #7f8c8d;">
                    <strong>Copyright © 2025 Dharma</strong>
                </p>
                <p style="margin: 5px 0; font-size: 12px; color: #95a5a6;">
                    All rights reserved. This software is provided as-is without warranty.
                </p>
            </div>

            <div style="margin: 20px 0;">
                <p style="font-size: 14px; color: #7f8c8d;">
                    Crafted with ❤️ for audio explorers, meditators, and frequency enthusiasts
                </p>
            </div>
        </div>
    )").arg(QApplication::applicationVersion());
}

QString HelpMenuDialog::getFeaturesContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #8e44ad; text-align: center; margin-bottom: 20px;">🎧 BinauralPlayer Features</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Multimedia Player & Brainwave Generator</h2>
                <p>A powerful Qt-based audio system combining traditional media playback with real-time
                binaural/isochronic tone generation for meditation, focus, and audio experimentation.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🎵 Media Playback Features</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #e67e22;">🎶 Multi-Format Support</h3>
                <p>Plays MP3, WAV, FLAC, OGG, M4A, MP4, M4V, AVI, MKV and more.</p>

                <h3 style="color: #e67e22;">📑 Tabbed Playlists</h3>
                <p>Create, rename, and manage multiple playlists with a tabbed interface.</p>

                <h3 style="color: #e67e22;">🎛 Advanced Controls</h3>
                <p>Shuffle, repeat, seek, mute, and real-time position tracking.</p>

                <h3 style="color: #e67e22;">🌐 Streaming Support</h3>
                <p>Play audio directly from HTTP/HTTPS URLs.</p>

                <h3 style="color: #e67e22;">🔍 In-Playlist Search</h3>
                <p>Quick filtering of tracks inside the active playlist.</p>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🧠 Brainwave Generation</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #9b59b6;">🔊 Three Synthesis Modes</h3>
                <p>Binaural Beats (headphones), Isochronic Tones, and general Audio Generator mode.</p>

                <h3 style="color: #9b59b6;">⚡ Real-Time Engine</h3>
                <p>DynamicEngine processes audio instantly with no pre-rendered buffers.</p>

                <h3 style="color: #9b59b6;">📡 Full Frequency Control</h3>
                <p>Left/right channels independently adjustable from 20Hz–20kHz.</p>

                <h3 style="color: #9b59b6;">📉 Multiple Waveforms</h3>
                <p>Sine, Square, Triangle, and Sawtooth.</p>

                <h3 style="color: #9b59b6;">⏱ Session Timer</h3>
                <p>Optional auto-stop timer from 1–45 minutes with visual countdown.</p>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">📋 Playlist Management</h2>

            <div style="margin: 20px 0;">

                <h3 style="color: #27ae60;">💽 JSON Playlist Storage</h3>
                <p>Save and load playlists with persistent, readable JSON files.</p>

                <h3 style="color: #27ae60;">🗑 Track Operations</h3>
                <p>Add, remove, clear tracks with smart selection tracking.</p>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🎨 User Interface</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #e74c3c;">🧭 Custom Toolbars</h3>
                <p>Color-coded media/binaural/nature sections for clarity.</p>

                <h3 style="color: #e74c3c;">📊 Split Status Bar</h3>
                <p>Playback info on the left, binaural engine status on the right.</p>



                <h3 style="color: #e74c3c;">🪟 Responsive Layout</h3>
                <p>UI adapts cleanly to window resizing.</p>

                <h3 style="color: #e74c3c;">⚠ Safety Notices</h3>
                <p>First-launch warning and optional brainwave activation warnings.</p>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🛠 Technical Highlights</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #1abc9c;">🎧 Dual Audio Pipelines</h3>
                <p>Playback and generated audio run simultaneously with independent volume control.</p>

                <h3 style="color: #1abc9c;">💾 Persistent Settings</h3>
                <p>All preferences stored through QSettings.</p>

                <h3 style="color: #1abc9c;">🧪 JSON Presets</h3>
                <p>Brainwave presets include metadata, versioning, and timestamps.</p>
            </div>

            <div style="background-color: #f8f9fa; padding: 20px; border-radius: 8px; text-align: center; margin-top: 30px;">
                <h3 style="color: #2c3e50; margin-top: 0;">🌟 Harmony of Technology & Sound</h3>
                <p style=\"margin-bottom: 0;\">BinauralPlayer blends powerful multimedia features with real-time
                audio synthesis for meditation, concentration, and audio experimentation.</p>
            </div>
        </div>
    )";
}


QString HelpMenuDialog::getInstructionsContent()
{
    return R"(
<div style=\"font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;\">
    <h1 style=\"color: #3498db; text-align: center; margin-bottom: 20px;\">🎧 How to Use BinauralPlayer</h1>

    <div style=\"background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;\">
        <h2 style=\"color: #2c3e50; margin-top: 0;\">🔊 Step 1: Choose Audio Type</h2>
        <p>BinauralPlayer supports two forms of brainwave audio stimulation:</p>
        <ul style=\"padding-left: 20px;\">
            <li><strong>Binaural Beats</strong> — requires headphones</li>
            <li><strong>Isochronic Tones</strong> — headphones optional</li>
        </ul>
        <p>Choose the mode from the interface before setting frequencies.</p>
    </div>

    <h2 style=\"color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;\">🎧 Step 2: Using Binaural Beats</h2>

    <div style=\"background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;\">
        <h3 style=\"color: #856404; margin-top: 0;\">How Binaural Beats Work</h3>
        <p>Binaural beats are created when the left and right ears receive two <strong>slightly different</strong> frequencies. The brain perceives the difference as the beat.</p>
        <p><strong>Example:</strong> Left = 200 Hz, Right = 208 Hz → Beat = 8 Hz</p>
    </div>

    <h3 style=\"color: #e67e22;\">Setting Up Binaural Beats:</h3>
    <ol style=\"padding-left: 25px;\">
        <li>Set <strong>Left Carrier Frequency</strong> (e.g., 200 Hz)</li>
        <li>Set <strong>Right Carrier Frequency</strong> to create the desired beat (e.g., 208 Hz → 8 Hz beat)</li>
        <li>Press <strong>Play</strong> to start the session</li>
    </ol>

    <p><strong>Tip:</strong> Binaural beats work best with high-quality headphones.</p>
    <p>You can safely use beats outside the 0–30 Hz range, such as 40–100 Hz for stimulation.</p>

    <h2 style=\"color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;\">🔵 Step 3: Using Isochronic Tones</h2>

    <div style=\"background-color: #e8f8f5; padding: 15px; border-radius: 8px; margin: 15px 0;\">
        <h3 style=\"color: #117a65; margin-top: 0;\">How Isochronic Tones Work</h3>
        <p>Isochronic tones use a <strong>single frequency</strong> that pulses on and off rapidly at the chosen beat frequency.</p>
    </div>

    <h3 style=\"color: #e67e22;\">Setting Up Isochronic Tones:</h3>
    <ol style=\"padding-left: 25px;\">
        <li>Set the <strong>Left Frequency</strong> (this will automatically copy to the right when you press Play)</li>
        <li>Enter the <strong>Isochronic Pulse Frequency</strong> (0–100 Hz)</li>
        <li>Press <strong>Play</strong> to generate pulses</li>
    </ol>

    <p><strong>Recommended:</strong> 0–30 Hz is best for brainwave entrainment.
    Headphones are optional for ISO — speakers often work great.</p>

    <h2 style=\"color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;\">⚠️ Safety & Responsibility</h2>

    <div style=\"background-color: #ffebee; border: 2px solid #f44336; padding: 20px; border-radius: 8px; margin-bottom: 25px;\">
        <h3 style=\"color: #c62828; text-align: center; margin-top: 0;\">Important Notice</h3>
        <p style=\"font-weight: bold; text-align: center; color: #b71c1c;\">
            Brainwave entrainment affects your mental state.<br>
            Use responsibly and discontinue immediately if you feel discomfort.
        </p>
        <ul style=\"padding-left: 20px; color: #b71c1c;\">
            <li>Always start at a <strong>low or comfortable volume</strong></li>
            <li>Do NOT use while driving, operating machinery, or in unsafe environments</li>
            <li>If you have epilepsy or neurological conditions, consult a doctor first</li>
            <li>These audios may induce relaxation, drowsiness, or altered consciousness</li>
        </ul>
    </div>

    <h2 style=\"color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;\">🎯 Recommended Beat Ranges</h2>

    <ul style=\"padding-left: 20px;\">
        <li><strong>0.5–4 Hz (Delta):</strong> Deep sleep</li>
        <li><strong>4–8 Hz (Theta):</strong> Meditation & trance</li>
        <li><strong>8–14 Hz (Alpha):</strong> Relaxation & creativity</li>
        <li><strong>14–30 Hz (Beta):</strong> Focus & alertness</li>
        <li><strong>30–60+ Hz (Gamma):</strong> High cognition (binaural only)</li>
    </ul>

    <div style=\"text-align: center; margin: 30px 0; padding: 20px; background-color: #f8f9fa; border-radius: 8px;\">
        <h3 style=\"color: #495057; margin-top: 0;\">🌟 Enjoy Responsibly</h3>
        <p style=\"margin-bottom: 0;\">BinauralPlayer makes it easy to explore powerful audio-based mental states.
        Keep volumes safe, stay aware of your body, and relax into the experience.</p>
    </div>
</div>
    )";
}

QString HelpMenuDialog::getSecurityContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">🔐 Security Features</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Master Password Protection</h2>
                <p>Jasmine includes a comprehensive security system to protect your saved websites, sessions, and sensitive data.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🛡️ Password Protection Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Master password requirement on startup</li>
                    <li>Secure password hashing with salt encryption</li>
                    <li>Failed attempt protection (5 attempts maximum)</li>
                    <li>Factory reset option for forgotten passwords</li>
                </ul>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">🔧 How to Enable Password Protection</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Go to Security menu → "Require Password on Startup"</li>
                    <li>Read the security notice and click OK</li>
                    <li>Enter your new master password</li>
                    <li>Confirm your password</li>
                    <li>Jasmine will now require this password on every startup</li>
                </ol>
            </div>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">⚠️ Important Security Notes</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Choose a strong, memorable password</li>
                    <li>Write it down in a safe place</li>
                    <li>If you forget it, you'll need to factory reset</li>
                    <li>Password is encrypted and stored securely</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">🔄 Changing Your Master Password</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Go to Security menu → "Change Master Password"</li>
                    <li>Enter your new password</li>
                    <li>Confirm the new password</li>
                    <li>Password is updated immediately</li>
                </ol>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🚨 Failed Login Protection</h2>

            <div style="margin: 20px 0;">
                <p><strong>Maximum 5 password attempts allowed</strong></p>
                <p>After 5 failed attempts, you get two options:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Exit Application</strong></li>
                    <li><strong>Factory Reset</strong> (clears all data and security settings)</li>
                </ul>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🔄 Factory Reset</h2>

            <div style="margin: 20px 0;">
                <p>If you forget your master password, factory reset will:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Remove all security settings</li>
                    <li>Clear the master password</li>
                    <li>Reset password protection to disabled</li>
                    <li>Allow you to start fresh</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">⚙️ Security Menu Options</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>"Require Password on Startup"</strong> - Toggle password protection on/off</li>
                    <li><strong>"Change Master Password"</strong> - Update your existing password</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">🔒 Protection States</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #d35400;">When Password Protection is Active:</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Jasmine prompts for password on every startup</li>
                    <li>All your websites, sessions, and data remain encrypted</li>
                    <li>No access to application features without correct password</li>
                </ul>

                <h3 style="color: #d35400;">When Password Protection is Disabled:</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Jasmine starts immediately without password prompt</li>
                    <li>All features accessible without authentication</li>
                    <li>Data remains saved but unprotected</li>
                </ul>
            </div>

            <h2 style="color: #16a085; border-bottom: 2px solid #16a085; padding-bottom: 5px;">💡 Best Practices</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Use a unique password not used elsewhere</li>
                    <li>Include numbers, letters, and special characters</li>
                    <li>Avoid easily guessable information</li>
                    <li>Keep a secure backup of your password</li>
                    <li>Enable password protection if you store sensitive login information</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">🔐 Security Implementation</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>SHA-256 encryption with custom salt</li>
                    <li>No plain text password storage</li>
                    <li>Secure settings storage</li>
                    <li>Memory-safe password handling</li>
                </ul>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🛡️ Complete Protection</h3>
                <p style="margin-bottom: 0;">This security system ensures your browsing profiles, saved websites, sessions, and any stored login references remain protected even if someone gains access to your computer.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getTwoFAContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">🔐 2FA Code Generator</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Two-Factor Authentication Manager</h2>
                <p>Jasmine includes a built-in Two-Factor Authentication (2FA) code generator that helps you manage and generate time-based one-time passwords (TOTP) for your accounts.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">❓ What is 2FA?</h2>

            <div style="margin: 20px 0;">
                <p>Two-Factor Authentication adds an extra layer of security to your accounts by requiring a second form of verification beyond just your password. This usually involves a 6-digit code that changes every 30 seconds.</p>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">🚀 Key Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Generate 6-digit TOTP codes for any 2FA-enabled account</li>
                    <li>Real-time code updates every 30 seconds</li>
                    <li>Visual countdown timer showing when codes refresh</li>
                    <li>One-click code copying to clipboard</li>
                    <li>Secure local storage of account secrets</li>
                    <li>Support for multiple accounts from different services</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">📱 How to Access</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Open the 2FA Manager from the Toolbar icon or the Tools Menu</li>
                    <li>The manager opens in a separate window</li>
                    <li>Resizable interface with accounts list and code display</li>
                </ul>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">➕ Adding 2FA Accounts</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Click "Add Account" button</li>
                    <li>Enter account name (e.g., "GitHub", "Google", "Discord")</li>
                    <li>Paste the secret key from the website's 2FA setup</li>
                    <li>Optionally enter the issuer/company name</li>
                    <li>Click OK to save</li>
                </ol>
            </div>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">🔍 Where to Find Secret Keys</h3>
                <p>When enabling 2FA on websites, they typically show:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>A QR code for mobile apps</li>
                    <li>A text secret key (what you need for Jasmine)</li>
                    <li>Look for "Can't scan QR code?" or "Manual entry" options</li>
                </ul>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🔢 Using Generated Codes</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Select an account from the list</li>
                    <li>Current 6-digit code displays in large text</li>
                    <li>Countdown timer shows seconds until next refresh</li>
                    <li>Click "Copy Code to Clipboard" for easy pasting</li>
                    <li>Codes automatically update every 30 seconds</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">👁️ Visual Indicators</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Progress bar shows time remaining (green → yellow → red)</li>
                    <li>Large, easy-to-read monospace font for codes</li>
                    <li>Clear countdown text showing refresh time</li>
                    <li>Copy button changes to "Copied!" for confirmation</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">⚙️ Account Management</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>View all your 2FA accounts in organized list</li>
                    <li>Delete accounts you no longer need</li>
                    <li>Accounts persist between application restarts</li>
                    <li>Secure local storage (not cloud-synced)</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">🔒 Security Notes</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Secret keys are stored locally on your device</li>
                    <li>No internet connection required for code generation</li>
                    <li>Codes are generated using industry-standard TOTP algorithm</li>
                    <li>Same codes as Google Authenticator, Authy, etc.</li>
                </ul>
            </div>

            <h2 style="color: #16a085; border-bottom: 2px solid #16a085; padding-bottom: 5px;">🌐 Supported Services</h2>

            <div style="margin: 20px 0;">
                <p>Works with any service that supports TOTP 2FA:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Google/Gmail accounts</li>
                    <li>GitHub</li>
                    <li>Discord</li>
                    <li>Microsoft accounts</li>
                    <li>Banking websites</li>
                    <li>Social media platforms</li>
                    <li>And many more</li>
                </ul>
            </div>

            <h2 style="color: #d35400; border-bottom: 2px solid #d35400; padding-bottom: 5px;">📋 Workflow Example</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Enable 2FA on GitHub</li>
                    <li>Copy the secret key from GitHub's setup page</li>
                    <li>Add account in Jasmine's 2FA Manager</li>
                    <li>When logging into GitHub, select the account</li>
                    <li>Copy the current 6-digit code</li>
                    <li>Paste into GitHub's 2FA prompt</li>
                </ol>
            </div>

            <h2 style="color: #7b1fa2; border-bottom: 2px solid #7b1fa2; padding-bottom: 5px;">💡 Benefits Over Mobile Apps</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Access codes directly on your computer</li>
                    <li>No need to grab your phone</li>
                    <li>Larger, easier-to-read display</li>
                    <li>Integrated with your browsing workflow</li>
                    <li>Quick clipboard copying</li>
                </ul>
            </div>

            <h2 style="color: #388e3c; border-bottom: 2px solid #388e3c; padding-bottom: 5px;">⏰ Time Synchronization</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Codes are time-based (30-second intervals)</li>
                    <li>Uses your system clock for accuracy</li>
                    <li>Same timing as other authenticator apps</li>
                    <li>Automatic refresh every second</li>
                </ul>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🌟 Integrated Security</h3>
                <p style="margin-bottom: 0;">This 2FA manager eliminates the need for separate authenticator apps while providing the same security benefits, making it convenient to access your two-factor codes directly within Jasmine.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getDataManagementContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">🗂️ Data Management & Privacy</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Session & Profile Data Management</h2>
                <p>Jasmine provides comprehensive tools to manage your browsing data, sessions, and privacy settings. Control what data is stored and when to clear it.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🧹 Clean Current Session Data</h2>

            <div style="margin: 20px 0;">
                <p><strong>What it does:</strong></p>
                <p>Clears browsing data from all currently active sessions and the shared profile.</p>

                <p><strong>Data removed:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All cookies from active sessions</li>
                    <li>HTTP cache from all profiles</li>
                    <li>Visited links history</li>
                    <li>Temporary browsing data</li>
                </ul>

                <p><strong>When to use:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>After browsing sensitive websites</li>
                    <li>When sharing your computer</li>
                    <li>To free up storage space</li>
                    <li>For privacy after online shopping/banking</li>
                </ul>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">👥 Clean Shared Profile Data</h2>

            <div style="margin: 20px 0;">
                <p><strong>What it does:</strong></p>
                <p>Clears browsing data only from the shared profile, leaving separate tab profiles untouched.</p>

                <p><strong>Data removed:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Shared profile cookies only</li>
                    <li>Shared profile cache</li>
                    <li>Shared profile visited links</li>
                </ul>

                <p><strong>What's preserved:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Individual tab profile data</li>
                    <li>Private profile sessions</li>
                    <li>Separate profile cookies and cache</li>
                </ul>

                <p><strong>When to use:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>When you want to keep private profiles intact</li>
                    <li>To clear general browsing without affecting work profiles</li>
                    <li>Selective privacy cleaning</li>
                </ul>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🏭 Restore Factory Defaults</h2>

            <div style="margin: 20px 0;">
                <p><strong>What it does:</strong></p>
                <p>Completely resets Jasmine to its original state, removing all user data and settings.</p>

                <p><strong>Data removed:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All saved websites and bookmarks</li>
                    <li>All saved sessions</li>
                    <li>All application settings and preferences</li>
                    <li>Security settings and master passwords</li>
                    <li>All browsing data (cookies, cache, history)</li>
                    <li>Application data directories</li>
                    <li>Profile configurations</li>
                </ul>
            </div>

            <div style="background-color: #f8d7da; padding: 15px; border-radius: 8px; margin: 15px 0; border-left: 4px solid #dc3545;">
                <h3 style="color: #721c24; margin-top: 0;">⚠️ Factory Reset Warning</h3>
                <p style="margin-bottom: 0;"><strong>This action cannot be undone!</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All your saved data will be permanently lost</li>
                    <li>Application will close automatically after reset</li>
                    <li>You'll need to restart Jasmine manually</li>
                    <li>All customizations will be lost</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">📍 How to Access These Features</h2>

            <div style="margin: 20px 0;">
                <p>All data management options are located in the <strong>Sessions</strong> menu:</p>
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Click on "Sessions" in the menu bar</li>
                    <li>Scroll to the bottom section</li>
                    <li>Choose your desired cleaning option</li>
                    <li>Confirm the action in the dialog box</li>
                </ol>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🔄 Data Types Explained</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #8e44ad;">Cookies</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Login sessions and preferences</li>
                    <li>Shopping cart contents</li>
                    <li>Website customizations</li>
                </ul>

                <h3 style="color: #8e44ad;">HTTP Cache</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Temporarily stored website files</li>
                    <li>Images, scripts, and stylesheets</li>
                    <li>Speeds up repeat visits</li>
                </ul>

                <h3 style="color: #8e44ad;">Visited Links</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>History of visited websites</li>
                    <li>Link color changes (visited vs unvisited)</li>
                    <li>Navigation history</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🛡️ Privacy Recommendations</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #16a085;">Regular Cleaning (Weekly)</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Use "Clean Shared Profile Data" for routine maintenance</li>
                    <li>Keeps private profiles intact</li>
                    <li>Maintains good performance</li>
                </ul>

                <h3 style="color: #16a085;">Deep Cleaning (Monthly)</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Use "Clean Current Session Data" for thorough cleanup</li>
                    <li>Clears all active session data</li>
                    <li>Good for privacy and storage</li>
                </ul>

                <h3 style="color: #16a085;">Emergency Cleaning</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>After using public computers</li>
                    <li>When selling or giving away device</li>
                    <li>After security concerns</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">💡 Smart Usage Tips</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Before cleaning:</strong> Save any important sessions you want to keep</li>
                    <li><strong>Profile separation:</strong> Use private profiles for sensitive browsing</li>
                    <li><strong>Regular maintenance:</strong> Clean shared profile weekly, all data monthly</li>
                    <li><strong>Factory reset:</strong> Only use when starting completely fresh</li>
                    <li><strong>Backup important data:</strong> Export sessions before major cleaning</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">⚡ Performance Benefits</h2>

            <div style="margin: 20px 0;">
                <p><strong>Regular data cleaning provides:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Faster application startup</li>
                    <li>Reduced memory usage</li>
                    <li>More available storage space</li>
                    <li>Improved browsing performance</li>
                    <li>Better privacy protection</li>
                </ul>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🎯 Choose the Right Tool</h3>
                <p style="margin-bottom: 0;">
                    <strong>Shared Profile Clean:</strong> For routine maintenance<br>
                    <strong>Current Session Clean:</strong> For thorough privacy cleaning<br>
                    <strong>Factory Reset:</strong> For complete fresh start
                </p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getDownloadManagerContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">📥 Download Manager</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Integrated Download Management</h2>
                <p>Jasmine includes a comprehensive download manager that handles all your file downloads with progress tracking, organization, and easy access to downloaded files.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🚀 Key Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Real-time download progress tracking</li>
                    <li>Download speed and time remaining calculations</li>
                    <li>Automatic file organization in dedicated folder</li>
                    <li>Duplicate filename handling</li>
                    <li>One-click access to files and folders</li>
                    <li>Download history management</li>
                    <li>Cancel active downloads</li>
                    <li>Clean interface with visual progress bars</li>
                </ul>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">📍 How to Access Downloads</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #229954;">Opening the Download Manager</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Click the <strong>Downloads</strong> icon in the toolbar</li>
                    <li>Or go to <strong>View → Downloads</strong> in the menu</li>
                    <li>Download window opens showing all current and past downloads</li>
                </ul>

                <h3 style="color: #229954;">Download Location</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Files are saved to: <code>Downloads/Jasmine/</code></li>
                    <li>Organized in your system's default Downloads folder</li>
                    <li>Automatic folder creation if it doesn't exist</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">📊 Download Progress Tracking</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #d68910;">Real-time Information</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>File name and size:</strong> Clear identification of what's downloading</li>
                    <li><strong>Progress bar:</strong> Visual representation of download completion</li>
                    <li><strong>Speed indicator:</strong> Current download speed (KB/s, MB/s)</li>
                    <li><strong>Time remaining:</strong> Estimated completion time</li>
                    <li><strong>Status updates:</strong> Starting, downloading, completed, cancelled</li>
                </ul>

                <h3 style="color: #d68910;">Progress Display</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Percentage completion with visual progress bar</li>
                    <li>Downloaded size vs. total file size</li>
                    <li>Dynamic speed calculations updated every second</li>
                    <li>Color-coded status indicators</li>
                </ul>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🎛️ Download Controls</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #8e44ad;">During Download</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Cancel Button:</strong> Stop active downloads immediately</li>
                    <li><strong>Open Folder:</strong> Access download directory anytime</li>
                    <li><strong>Progress Monitoring:</strong> Watch real-time progress</li>
                </ul>

                <h3 style="color: #8e44ad;">After Download</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Open File:</strong> Launch downloaded file directly</li>
                    <li><strong>Open Folder:</strong> Navigate to file location</li>
                    <li><strong>Remove from List:</strong> Clean up download history</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🗂️ File Organization</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #16a085;">Automatic Organization</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All downloads saved to dedicated Jasmine folder</li>
                    <li>Automatic duplicate filename handling</li>
                    <li>Files renamed with numbers: <code>file.pdf</code>, <code>file (1).pdf</code>, <code>file (2).pdf</code></li>
                    <li>Preserves original file extensions</li>
                </ul>

                <h3 style="color: #16a085;">Smart Naming</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Uses original filename from website</li>
                    <li>Fallback to "download" if no name available</li>
                    <li>Prevents file overwrites automatically</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">🧹 Download Management</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #d35400;">Window Controls</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Clear Finished:</strong> Remove completed/cancelled downloads from list</li>
                    <li><strong>Open Downloads Folder:</strong> Quick access to download directory</li>
                    <li><strong>Individual Remove:</strong> Remove specific items from history</li>
                </ul>

                <h3 style="color: #d35400;">List Management</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Chronological list of all downloads</li>
                    <li>Persistent across application restarts</li>
                    <li>Easy cleanup of old downloads</li>
                    <li>Empty state message when no downloads</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">💡 Download States</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #7b1fa2;">Active Downloads</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Starting:</strong> Download initializing</li>
                    <li><strong>In Progress:</strong> Actively downloading with progress</li>
                    <li><strong>Speed Display:</strong> Real-time transfer rate</li>
                    <li><strong>Cancel Option:</strong> Stop button available</li>
                </ul>

                <h3 style="color: #7b1fa2;">Completed Downloads</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Completed:</strong> Successfully downloaded</li>
                    <li><strong>Cancelled:</strong> User stopped download</li>
                    <li><strong>Interrupted:</strong> Network or system error</li>
                    <li><strong>File Access:</strong> Open file/folder buttons available</li>
                </ul>
            </div>

            <h2 style="color: #d32f2f; border-bottom: 2px solid #d32f2f; padding-bottom: 5px;">⚡ Performance Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Efficient Memory Usage:</strong> Minimal resource consumption</li>
                    <li><strong>Background Downloads:</strong> Continue while browsing</li>
                    <li><strong>Multiple Downloads:</strong> Handle several files simultaneously</li>
                    <li><strong>Speed Calculation:</strong> Accurate transfer rate monitoring</li>
                    <li><strong>Progress Updates:</strong> Smooth, real-time progress tracking</li>
                </ul>
            </div>

            <h2 style="color: #388e3c; border-bottom: 2px solid #388e3c; padding-bottom: 5px;">🔧 Technical Details</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #2e7d32;">File Size Formatting</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Automatic unit conversion: B → KB → MB → GB</li>
                    <li>Decimal precision for readability</li>
                    <li>Speed shown as size per second</li>
                </ul>

                <h3 style="color: #2e7d32;">Time Calculations</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Remaining time based on current speed</li>
                    <li>Format: seconds, minutes, hours as appropriate</li>
                    <li>Dynamic updates as speed changes</li>
                </ul>
            </div>

            <h2 style="color: #5d4037; border-bottom: 2px solid #5d4037; padding-bottom: 5px;">🎯 Usage Tips</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Monitor Progress:</strong> Keep download window open to watch progress</li>
                    <li><strong>Multiple Downloads:</strong> Start several downloads simultaneously</li>
                    <li><strong>Quick Access:</strong> Use "Open Folder" for easy file management</li>
                    <li><strong>Clean History:</strong> Regularly clear finished downloads</li>
                    <li><strong>Cancel if Needed:</strong> Stop unwanted downloads immediately</li>
                    <li><strong>File Organization:</strong> Downloads are automatically organized</li>
                </ul>
            </div>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">📁 Platform-Specific Notes</h3>
                <p><strong>Flathub Version:</strong> Shows download location in dialog</p>
                <p><strong>Standard Version:</strong> Opens file manager directly</p>
                <p style="margin-bottom: 0;"><strong>All Platforms:</strong> Downloads saved to system Downloads folder under "Jasmine" subdirectory</p>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🌟 Integrated Experience</h3>
                <p style="margin-bottom: 0;">The download manager seamlessly integrates with your browsing experience, automatically handling all file downloads while providing full control and visibility over the download process.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getSitesSessionsContent()
{
    return QString(
        "<h3>Managing Sites</h3>"
        "<p><strong>Creating a New Site:</strong></p>"
        "<ul>"
        "<li>Press the <strong>Clear</strong> button to clear existing fields</li>"
        "<li>Fill in the <strong>URL</strong> and <strong>Title</strong> (required fields)</li>"
        "<li>Optionally add <strong>Username</strong>, <strong>Password</strong>, and <strong>Comments</strong></li>"
        "<li>Press the <strong>Add</strong> button to save the site</li>"
        "</ul>"

        "<h3>Managing Sessions</h3>"
        "<p><strong>Creating a New Session:</strong></p>"
        "<ul>"
        "<li>Ensure at least one tab is open in the webview</li>"
        "<li>Select <strong>Save Current Session</strong> from the menu or toolbar</li>"
        "<li>Give your session a name and click <strong>OK</strong></li>"
        "<li>Sessions are automatically assigned a randomly generated SVG icon</li>"
        "</ul>"

        "<h3>Editing Sites and Sessions</h3>"
        "<p><strong>To edit any site or session:</strong></p>"
        "<ul>"
        "<li>Select the item you want to modify</li>"
        "<li>Enter the new values in the appropriate fields</li>"
        "<li>Press the <strong>Update</strong> button to save changes</li>"
        "<li>For sessions: Click the small <strong>Edit</strong> button next to the icon to change it, then click <strong>Update</strong></li>"
        "</ul>"
        );
}

QString HelpMenuDialog::getOnSecurityContent()
{
    return QString(
        "<h3>Security Features Overview</h3>"
        "<p>Jasmine provides several optional security features designed for your convenience. "
        "You are completely free to use or not use any of these features based on your preferences.</p>"

        "<h4>Username & Password Storage</h4>"
        "<ul>"
        "<li>Storing credentials in website entries is <strong>completely optional</strong></li>"
        "<li>Leave these fields blank if you prefer to use your own credentials manager</li>"
        "<li>Stored credentials are saved locally on your device only in binary format</li>"
        "<li>No data is transmitted over the network</li>"
        "</ul>"

        "<h4>Master Password Protection</h4>"
        "<ul>"
        "<li>Optional feature to protect access to Jasmine</li>"
        "<li>When enabled, you'll need to enter your master password on startup</li>"
        "<li>Choose a strong, memorable password and store it safely</li>"
        "<li>If forgotten, you'll need to perform a factory reset</li>"
        "</ul>"

        "<h4>Two-Factor Authentication (2FA) Manager</h4>"
        "<ul>"
        "<li>Optional convenience tool for generating TOTP codes</li>"
        "<li>Helps manage 2FA codes for your various accounts</li>"
        "<li>All secrets are stored locally on your device in binary format</li>"
        "<li>Use only if you're comfortable with local storage</li>"
        "</ul>"

        "<h4>Security Disclaimer</h4>"
        "<p><em>While every reasonable effort has been made to implement a secure framework "
        "and all sensitive info is stored in binary format within Jasmine, "
        "these features are provided as conveniences rather than guarantees. Users are responsible "
        "for deciding what information to store based on their individual security requirements and risk tolerance.</em></p>"

        "<p><strong>Recommendation:</strong> For maximum security, consider using dedicated password managers "
        "and letting your browser handle credential storage.</p>"
        );
}

QString HelpMenuDialog::getOnNewStorageSystemContent() {
    return QString(
        "<h3>New Storage System (Jasmine 1.1.0 and onwards)</h3>"
        "<p>This version of Jasmine uses a new storage system that improves performance and efficiency by using symlinks for profile data. This means that instead of copying entire profile directories, only links are created, saving disk space and speeding up operations.</p>"
        "<h4>Recommendations:</h4>"
        "<p>To fully leverage the new storage system and ensure optimal performance, we highly recommend performing a factory reset. This will clear all old data and provide a clean start. You can find the factory reset option in the 'Sessions' menu.</p>"
        "<h4>Clean up orphaned profile directories on startup:</h4>"
        "<p>As part of the new storage system, a checkbox is available in the 'Tools' menu to automatically clean up unused, orphaned profile directories at application startup. It is STRONGLY RECOMMENDED to keep this option enabled UNLESS you also have old-format sessions saved.</p>"
        "<h4>Handling Old-Format Sessions:</h4>"
        "<p>If you have existing sessions that use the old directory-based storage, you can continue to use them. However, we strongly advise against adding new tabs or saving changes to these older sessions. To ensure optimal performance and avoid potential issues, it's best to create new sessions using the new storage system.</p>"
        );
}

QString HelpMenuDialog::getOnNamedProfilesContent()
{
    return QString(R"(
        <div style="text-align: center; font-family: Arial, sans-serif;">
            <h1 style="color: #2c3e50; margin-bottom: 10px;">Named Shared Profiles</h1>
            <h3 style="color: #7f8c8d; margin-bottom: 20px;">Contextual Browsing Environments</h3>

            <div style="text-align: left; margin: 20px 0; line-height: 1.6;">
                <p>
                    Jasmine's Named Shared Profiles feature takes your browsing organization to the next level by allowing
                    multiple tabs to share the same browsing context under meaningful labels like "Work," "Home," "Shopping,"
                    or "Research." This powerful addition bridges the gap between completely isolated private profiles and a
                    single shared environment.
                </p>

                <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin: 20px 0;">
                    <h4 style="color: #2c3e50; margin-top: 0;">Examples:</h4>
                    <ul style="margin-left: 20px;">
                        <li><strong>Work Profile:</strong> Company email, project management tools, and internal documentation all share cookies and login states</li>
                        <li><strong>Personal Profile:</strong> Social media and entertainment sites kept separate from work accounts</li>
                        <li><strong>Finance Profile:</strong> Banking, investment, and budgeting tools with shared authentication</li>
                        <li><strong>Travel Profile:</strong> Airline, hotel, and rental car sites that need to exchange booking information</li>
                    </ul>
                </div>

                <p>
                    This contextual grouping maintains the perfect balance between isolation and integration: your work accounts
                    never mix with personal browsing, but related tools within each context can seamlessly communicate when needed.
                    Named Shared Profiles eliminate the all-or-nothing approach to profile isolation, giving you granular control
                    over exactly which tabs should share data with each other while maintaining barriers between different areas of your digital life.
                </p>

                <h3 style="color: #2c3e50; margin-top: 30px;">How to Use Named Profiles</h3>

                <ol style="margin-left: 20px;">
                    <li><strong>Create Profiles:</strong> In the profile area of the toolbar, in the profileSelector combobox select "New Profile..." and give it a meaningful name like "Work" or "Personal"</li>
                    <li><strong>Select a Profile:</strong> Choose your desired profile from the dropdown before launching websites</li>
                    <li><strong>Launch Websites:</strong> Any sites launched while a named profile is selected will share the same browsing context</li>
                    <li><strong>Create Sessions:</strong> Save groups of tabs launched under the same named profile for quick restoration</li>
                    <li><strong>Manage Profiles:</strong> Select "Manage Profiles..." to create, delete, or clean profile data</li>
                </ol>

                <div style="background-color: #e8f4f8; padding: 15px; border-radius: 8px; margin: 20px 0; border-left: 4px solid #3498db;">
                <p><strong>Note:</strong> Keeping the profile selector set to "Default" will use the universal shared profile,
                which is the standard shared browsing environment. Private profiles (toggled per tab via the Private Toggle Button) always remain
                completely isolated regardless of named profile selection.</p>

                <p><strong>Simply put:</strong></p>
                <ul style="margin-left: 20px;">
                    <li>To create tabs with private profile, toggle the Private Profile button ON in the toolbar.
                    This will inactivate the Named Profile selector.</li>
                    <li>To use a named shared profile, untoggle the Private Profile button (if toggled) and select a previously
                    created named shared profile from the combobox.</li>
                    <li>Or leave it set to "Default" to use the universal profile.</li>
                </ul>
                </div>



                <h3 style="color: #2c3e50; margin-top: 30px;">Profile Management</h3>
                <p>
                    You can manage your named profiles by selecting "Manage Profiles..." from the profile selector dropdown. This allows you to:
                </p>
                <ul style="margin-left: 20px;">
                    <li><strong>Create new profiles</strong> for different contexts or projects</li>
                    <li><strong>Delete profiles</strong> you no longer need (only if they're not in use by active sessions)</li>
                    <li><strong>Clean profile data</strong> to remove cookies, cache, and browsing history while keeping the profile</li>
                </ul>

                <p>
                    Each named profile maintains its own separate storage for:
                </p>
                <ul style="margin-left: 20px;">
                    <li>Cookies and login sessions</li>
                    <li>Browsing history and cache</li>
                    <li>Local storage and website data</li>
                    <li>Form data and preferences</li>
                </ul>
            </div>

            <hr style="margin: 30px 0; border: 1px solid #bdc3c7;">

            <div style="margin: 20px 0;">
                <p style="font-size: 14px; color: #7f8c8d;">
                    Organize your digital life with contextual browsing environments
                </p>
            </div>
        </div>
    )");

}

QString HelpMenuDialog::getChangelogContent()
{
    return QString(
        "<h3>Changelog</h3>"

        "<h4>[v1.2.2] - 2025-10-31</h4>"
        "<ul>"
        "<li><b>Custom Context Menu Implementation:</b> Subclassed QWebEnginePage and created a fully functional custom context menu with enhanced functionality and improved user experience.</li>"
        "<li><b>Consistent Profile Linking:</b> Ensured links opened from a tab share the same profile type as the calling tab, maintaining profile consistency across navigation and new tab operations.</li>"
        "<li><b>Code Polishes and Improvements:</b> Performed various code optimizations, refactoring, and maintenance improvements for better performance, stability, and maintainability.</li>"
        "</ul>"

        "<h4>[v1.2.1] - 2025-07-23</h4>"
        "<ul>"
        "<li><b>Session Profile Tagging Bug Fixed:</b> Resolved an issue introduced with named profiles where some tabs were not saved or displayed with the correct profile when saving sessions. Now, tabs are reliably saved and shown with their associated profile—Shared Universal, Named, or Private—according to the UI profile settings at launch time.</li>"
        "<li><b>Improved Security Dialogs and User Experience:</b> All security-related dialogs (master password setup, password protection toggle) now include a Cancel button and properly abort the operation if Cancel or the window close (X) is pressed. This ensures that no unintended changes occur if the user cancels at any step, and toggle actions are always reverted if the process is not completed.</li>"
        "<li><b>Audio Support in Flatpak:</b> Added `pulseaudio` permission to the Flatpak manifest, enabling sound playback in the application.</li>"
        "<li><b>Fullscreen Video Support:</b> Implemented full screen functionality for embedded and standalone videos, enhancing the media viewing experience.</li>"
        "<li><b>Open Copied Link in New Tab:</b> Added a toolbar action and F11 shortcut to instantly open a copied link from the clipboard in a new tab.</li>"
        "<li><b>Code Optimizations and Polishes:</b> Performed various code improvements and optimizations for better performance, stability, and maintainability.</li>"
        "</ul>"

        "<h4>[v1.2.0] - 2025-06-20</h4>"
        "<ul>"
        "<li><b>Named Shared Profiles:</b> Implemented contextual browsing environments allowing multiple tabs to share the same browsing context under meaningful labels like \"Work,\" \"Home,\" or \"Research\"</li>"
        "<li><b>Integrated Ad Blocker:</b> Added comprehensive ad blocking functionality with toggle option in Tools menu</li>"
        "<li><b>Enhanced Browser Privacy:</b> Implemented custom request interceptor with standardized headers and secure default headers</li>"
        "<li><b>Profile Management Tools:</b> Added profile manager dialog for creating, deleting, and cleaning Named Shared Profiles</li>"
        "<li><b>UI Improvements:</b> Added visual indicators for active Named profile and improved toolbar layout</li>"
        "</ul>"

        "<h4>[v1.1.0] - 2025-06-12</h4>"
        "<ul>"
        "<li><b>New Symlink-Based Storage System:</b> Dramatically reduces disk space usage by using symbolic links instead of copying entire directories</li>"
        "<li><b>Progress Bar for Session Saving:</b> Added visual feedback to prevent premature closure during session saves</li>"
        "<li><b>Automatic Cleanup:</b> Implemented cleanup of orphaned profile directories at startup (configurable in Tools menu)</li>"
        "<li><b>Help Documentation:</b> Added documentation explaining the new storage system benefits and recommendations</li>"
        "</ul>"

        "<h4>[v1.0.1] - 2025-06-07</h4>"
        "<ul>"
        "<li><b>Enhanced UI and Functionality:</b> Added fully functional URL bar with toggle visibility</li>"
        "<li><b>Website Card Creation:</b> New feature to create website cards directly from currently loaded URL</li>"
        "<li><b>Fixed Dashboard Size:</b> Set dashboard to fixed size while keeping web view resizable</li>"
        "<li><b>Theme Improvements:</b> Enhanced theme handling for help menu items</li>"
        "<li><b>Bug Fixes:</b> Fixed critical QWebEngineView resizing bugs and website card update button connections</li>"
        "</ul>"

        "<h4>[v1.0.0] - 2025-06-01</h4>"
        "<ul>"
        "<li><b>Initial Release:</b> Jasmine Website and Session Manager</li>"
        "<li><b>Core Features:</b> Website and session management with card-based interface, multi-tab restoration, private profile system</li>"
        "<li><b>Security:</b> Master password protection with SHA-256 encryption, two-factor authentication (2FA) code generator</li>"
        "<li><b>Productivity:</b> Built-in download manager, screenshot capture, multi-account support</li>"
        "</ul>"
    );
}

QString HelpMenuDialog::getSupportusContent()
{
    return QString();

}




