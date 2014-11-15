///
///	@file player.h	@brief A play plugin header file.
///
///	Copyright (c) 2012, 2013 by Johns.  All Rights Reserved.
///
///	Contributor(s): Dennis Bendlin
///
///	License: AGPLv3
///
///	This program is free software: you can redistribute it and/or modify
///	it under the terms of the GNU Affero General Public License as
///	published by the Free Software Foundation, either version 3 of the
///	License.
///
///	This program is distributed in the hope that it will be useful,
///	but WITHOUT ANY WARRANTY; without even the implied warranty of
///	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///	GNU Affero General Public License for more details.
///
///	$Id: 0c86be2575387d12e692ffa0cc8ce515bbc6f5e5 $
//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif
    /// C callback feed key press
    extern void FeedKeyPress(const char *, const char *, int, int);

    /// C callback enable dummy device
    extern void EnableDummyDevice(void);
    /// C callback disable dummy device
    extern void DisableDummyDevice(void);

    /// C plugin get osd size and ascpect
    extern void GetOsdSize(int *, int *, double *);

    /// C plugin open osd
    extern void OsdOpen(void);
    /// C plugin close osd
    extern void OsdClose(void);
    /// C plugin clear osd
    extern void OsdClear(void);
    /// C plugin draw osd pixmap
    extern void OsdDrawARGB(int, int, int, int, const uint8_t *);

    /// C plugin play audio packet
    extern int PlayAudio(const uint8_t *, int, uint8_t);
    /// C plugin play TS audio packet
    extern int PlayTsAudio(const uint8_t *, int);
    /// C plugin set audio volume
    extern void SetVolumeDevice(int);

    /// C plugin play video packet
    extern int PlayVideo(const uint8_t *, int);
    /// C plugin play TS video packet
    extern void PlayTsVideo(const uint8_t *, int);
    /// C plugin grab an image
    extern uint8_t *GrabImage(int *, int, int, int, int);

    /// C plugin set play mode
    extern int SetPlayMode(int);
    /// C plugin get current system time counter
    extern int64_t GetSTC(void);
    /// C plugin get video stream size and aspect
    extern void GetVideoSize(int *, int *, double *);
    /// C plugin set trick speed
    extern void TrickSpeed(int);
    /// C plugin clears all video and audio data from the device
    extern void Clear(void);
    /// C plugin sets the device into play mode
    extern void Play(void);
    /// C plugin sets the device into "freeze frame" mode
    extern void Freeze(void);
    /// C plugin mute audio
    extern void Mute(void);
    /// C plugin display I-frame as a still picture.
    extern void StillPicture(const uint8_t *, int);
    /// C plugin poll if ready
    extern int Poll(int);
    /// C plugin flush output buffers
    extern int Flush(int);

    /// C plugin command line help
    extern const char *CommandLineHelp(void);
    /// C plugin process the command line arguments
    extern int ProcessArgs(int, char *const[]);

    /// C plugin exit + cleanup
    extern void PlayExit(void);
    /// C plugin start code
    extern int Start(void);
    /// C plugin stop code
    extern void Stop(void);
    /// C plugin house keeping
    extern void Housekeeping(void);
    /// C plugin main thread hook
    extern void MainThreadHook(void);

    /// Browser root=start directory
    extern const char *ConfigBrowserRoot;
    ///< Disable remote during external play
    extern char ConfigDisableRemote;
    extern const char *X11DisplayName;	///< x11 display name
    extern char PlayerDvdNav;		///< dvdnav active
    extern char PlayerPaused;		///< player paused
    extern char PlayerSpeed;		///< player playback speed
    extern int PlayerCurrent;		///< current postion in seconds
    extern int PlayerTotal;		///< total length in seconds
    extern char PlayerTitle[256];	///< title from meta data
    extern char PlayerFilename[256];	///< filename

    /// Start external player
    extern void PlayerStart(const char *name);
    /// Stop external player
    extern void PlayerStop(void);
    /// Is external player still running
    extern int PlayerIsRunning(void);

    /// Set player volume
    extern void PlayerSetVolume(int);

    /// Player send quit command
    extern void PlayerSendQuit(void);
    /// Player send toggle pause command
    extern void PlayerSendPause(void);
    /// Player send set play speed
    extern void PlayerSendSetSpeed(int);
    /// Player send seek
    extern void PlayerSendSeek(int);
    /// Player send switch audio track
    extern void PlayerSendSwitchAudio(void);
    /// Player send select subtitle
    extern void PlayerSendSubSelect(void);
    /// Player send dvd-nav up
    extern void PlayerSendDvdNavUp(void);
    /// Player send dvd-nav down
    extern void PlayerSendDvdNavDown(void);
    /// Player send dvd-nav left
    extern void PlayerSendDvdNavLeft(void);
    /// Player send dvd-nav right
    extern void PlayerSendDvdNavRight(void);
    /// Player send dvd-nav menu select
    extern void PlayerSendDvdNavSelect(void);
    /// Player send dvd-nav menu prev
    extern void PlayerSendDvdNavPrev(void);
    /// Player send dvd-nav prev
    extern void PlayerSendDvdNavMenu(void);
    /// Get length in seconds.
    extern void PlayerGetLength(void);
    /// Get current position in seconds.
    extern void PlayerGetCurrentPosition(void);
    /// Get title from meta data.
    extern void PlayerGetMetaTitle(void);
    /// Get filename.
    extern void PlayerGetFilename(void);

#ifdef __cplusplus
}
#endif
