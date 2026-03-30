# Changelog

## [2026-03-30] - Release v1.5.0
### Added
- **Visual Brainwave Entrainment**: OpenGL-powered flicker engine runs directly on the video screen, pulsing at the active beat frequency for simultaneous audiovisual stimulation
- **Customisable flicker colors**: Pick any on/off color combination for the flicker cycle via the color picker
- **Brightness envelope sync**: Sine, square and sawtooth envelopes sync automatically to the active audio waveform with optional independent override
- **Frequency sync with override**: Flicker frequency locks to the active beat by default, with an optional override spinbox and live brainwave band label (delta / theta / alpha / beta / gamma)
- **Subliminal text overlay**: Display affirmations or intention-setting text during a session — configurable display mode (flash with beat / always / off), font size, foreground color and background color with full alpha support
- **Visual stimulation control panel**: Accessible directly from the video toolbar via the ✦ button
---

## [2025-12-28] - Release v1.4.0

### Added
- **Full-featured video player**: Play video files with a beautiful media player and dedicated video playlist

### Changed
- Various code improvements and UI polish

---

## [2025-12-21] - Release v1.3.0

### Added
- **Multi-Stage Session Manager**: Create programmable sequences of audio tones with timed stages
- **CUE Sheet Import**: Load and navigate structured audio tracks from CUE files
- **Digital Seek Widget**: Jump to precise time positions within tracks
- **Drag-and-Drop Import**: Import audio files by dragging directly into the application

### Notes
Transforms the application from a simple tone generator into a complete audio toolkit with:
- Programmable therapy and meditation sessions
- Structured navigation of long audio files
- Enhanced file management and workflow

---


## [2025-12-16] - Release v1.2.0

### Changed
- **BinauralPlayer goes fully dynamic**: switched tone generation to a real-time dynamic engine.
- Removed buffered tone generation entirely.
- No delays or gaps when increasing or decreasing frequencies — changes are applied instantly.

### Notes
This is a major milestone that opens the road to new and exciting features, such as:
- Programmable multi-stage sessions with smooth transitions between stages
- Per-stage frequency changes
- Support for binaural beats and isochronic pulses within complex session flows

---


## [2025-12-13] - Release v1.1.0

### Added
- Nature Ambient Sounds Toolbar - ambient sound mixer with 5 independent players
- Bottom-right info button for track metadata display

### Changed

- Changed the default screenshot to reflect the current version of the application including the Nature Ambient Sound Toolbar
- Various code improvements and UI polish

---

## [Initial Release - 2025-12-08] - v1.0.0

Binaural Media Player - A Qt-based application for multimedia playback and brainwave audio generation.

**Features:**
- Multi-format audio/video playback (MP3, WAV, FLAC, OGG, M4A, MP4, etc.)
- Real-time binaural and isochronic tone generation
- Tabbed playlist management with search
- Streaming from HTTP/HTTPS URLs
- Brainwave entrainment with session timer
- Customizable interface with color-coded toolbars
- JSON-based preset and playlist storage
