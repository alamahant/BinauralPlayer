# Changelog

## [2026-06-23] - Release v1.6.2

### Added
- **Radionics Console (Experimental)**
  - Intention-based frequency generation tool
  - Three interactive dials for creating unique frequency signatures
  - Save/Load sessions with images and settings
  - Continuous broadcast mode
  - Help menu with theory and usage guide

---

## [2026-06-09] - Release v1.6.1

### Added
- **47+ Brainwave Preset Library**
  - Bundled preset collection: sleep, meditation, focus, altered states, Solfeggio frequencies
  - Presets as `.bsession` files in tar archive (`brainwave_presets.tar.xz`)
  - Auto-copy to user data directory on first run via `showPresetExtractionNotice()`
  - Dialog with folder open, command copy, and "do not show again" checkbox

- **Preset Description Files**
  - `session_descriptions.txt` with category headers, frequency patterns, and use cases
  - `unlimited_duration_notice.txt` explaining sessions exceeding 45-min default limit

- **Cover Art Implementation**
  - `QProcess` + `ffmpeg` extraction (Qt Multimedia couldn't handle embedded art)
  - `QMap` caching stores original full-quality images per track
  - `QLabel` in track info dialog with 1.15x hover zoom (scaled from original, no quality loss)
  - Clear old art when switching to tracks without images
  - Dialog hides on close (not delete) to preserve event filters

### Fixed
- `QDoubleSpinBox` manual entry below 0.5 Hz now works with `setSingleStep(0.1)` and `setRange(0.10, 100.0)`
- Pulse frequency spinbox validation now matches engine's 0.10 Hz minimum

---

## [2026-06-01] - Release v1.6.0

### Added
- **Decoupled Video & Flicker Widgets from Tab System**
  - Video player now opens in independent floating window (no tab embedding)
  - Visual Stimulation (Flicker) now opens in independent floating window
  - Video and Flicker can be used simultaneously without mode switching
  - Removed all `m_isVideoEnabled` conditional logic

- **Playlist Track Persistence**
  - Each playlist now remembers its last selected track when switching tabs
  - Restores selection after tab switching, track removal, and playlist operations
  - Uses `QMap<QString, int>` to store last track index per playlist

- **Unified File Extension System**
  - Single master list `ConstantGlobals::allMediaExtensions` for all media formats
  - Duplicate file detection using `QSet` prevents adding same file twice
  - User feedback dialogs for unsupported formats and duplicates
  - Applies to: Load Music, Drag & Drop, File Association, and Streams

- **Improved Playlist Management**
  - Load playlist directly into current tab with confirmation before overwriting
  - Save All Playlists now provides detailed success/error summary with playlist names
  - Auto-selects first track after loading a playlist

- **Flicker Window Improvements**
  - Fullscreen mode removes titlebar for immersive experience
  - VisStimDialog correctly reappears when window is reopened
  - Window opens larger (1024x600) and remembers size after fullscreen exit

### Fixed
- Fixed duplicate file detection inconsistencies between file dialog and drag & drop
- Fixed event filter crash when accessing null `m_videoPlayerContainer`
- Fixed playlist tab close confirmation removing map entries before user confirmation
- Fixed missing map entry rename when renaming playlists
- Fixed `onRemoveTrackClicked` using wrong track index variable

### Changed
- Simplified `loadPlaylistFromFile` to load into current tab (no automatic new tab creation)
- Removed redundant extension validation in `processDroppedFiles` (dragEnterEvent handles it)
- Cleaned up `onSaveAllPlaylistsClicked` with better user messaging
- Removed overcomplicated `QLocalServer` single-instance code (back to simple multi-instance)

### Removed
- `m_isVideoEnabled` and all related conditional logic
- Video Player and Video Playlist tabs from tab widget
- `m_flickerOriginalTabIndex` and tab visibility code
- Hardcoded extension lists replaced with `allMediaExtensions`

---

## [2026-05-17] - Release v1.5.3

### Added
- **Enhanced Streaming Support for YouTube & Video Sites**
  - Added unified stream extraction for YouTube, Dailymotion, Rumble, Odysee, and Vimeo
  - Improved extraction reliability using timeout-based yt-dlp process with buffer accumulation
  - Added proper process cleanup and cancellation for stream extraction
  - Added "Clear Stream" button to cancel ongoing extraction and unload media player

- **Improved "Add Stream" Feature**
  - Menu option and shortcut (`Ctrl+U`) to add any URL
  - Better direct media detection for common audio/video formats (.mp4, .mkv, .mp3, .m3u8, .webm, .flac, etc.)
  - Unified extraction logic for all streaming sites (no more separate handlers)
  - Added fullscreen prevention when loading streams

- **Player Improvements**
  - Stream extraction now uses single-process approach with 9-second timeout
  - Proper buffer accumulation for reliable URL extraction
  - Stream extraction can now be cancelled without restarting the app
  - Media player properly unloads source when clearing streams

### Fixed
- Fixed crashes caused by nested yt-dlp processes
- Fixed memory leaks from improper process cleanup
- Fixed stream extraction hanging indefinitely on certain URLs

### Changed
- Unified YouTube and generic stream extraction into single robust function
- Updated yt-dlp arguments to use `-f best` for all streaming sites
- Added user-agent header for better site compatibility

---

## [2026-05-13] - Release v1.5.2

### Fixed
- **Critical Crash**: Application no longer crashes when changing waveform from Sine to Square/Triangle/Sawtooth
  - Root cause: Null pointer access to `m_visStimDialog` in waveform change handlers
  - Added proper null checks before calling `syncWaveType()`
  - Affected: `onWaveformChanged()` and `onToneTypeComboIndexChanged()`

### Improved
- Visual stimulation dialog now safely handles uninitialized states
- Stability improvements during tone type switching (BINAURAL/ISOCHRONIC/GENERATOR)

---

## [2026-04-20] - Release v1.5.1
### Added
- **Dark Theme Mode**: Full dark theme support with View → Dark Theme toggle for reduced eye strain in low-light environments
- **Persistent Theme Preference**: Theme setting saved to user preferences and restored on application restart
- **Comprehensive Dark Styling**: Dark theme applied consistently across main window, all dialogs, splitter handles, buttons, and text editor

---

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
