///////////////////////////////////////////////////////////////////////////////
//
//   Notify CD Player for Windows NT and Windows 95
//
//   Copyright (c) 1996-1998, Mats Ljungqvist (mlt@cyberdude.com)
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// History:
// 960312 Mats      Initial coding
// 960313 Mats      Added some CDPLAYER.EXE compatibility stuff
// 960314 Mats      Added multi functions on left button click
// 960314 Mats      Added centering of dialogs
// 960314 Mats      Added more functionality to CDGetStatus. We can now 
//                  determine if the inserted CD is aduio or not
// 960314 Mats      Version 0.92
// 960314 Mats      Added a settings dialog
// 960314 Mats      Added some time displaying in tooltip
// 960314 Mats      Added track nuber to tracks meny and tooltip
// 960314 Mats      Added icon in about box
// 960314 Mats      Version 0.93
// 960314 Mats      Added the options Stop On Exit and Stop On Start
// 960314 Mats      Version 0.94
// 960315 Mats      Added enable/disable of a lot of things in the menu
// 960315 Mats      The current track is checked in the Tracks menu
// 960315 Mats      Added the MYWM_WAKEUP command which is sent by additional
//                  instances to the original instance of the application.
// 960315 Mats      Version 0.95
// 960318 Mats      Fixed the missing track number in the tracks menu!
// 960401 Mats      Version 0.96
// 960401 Mats		Added poll timer!
// 960401 Mats		Added tracktime in tracks menu
// 960401 Mats		Added handling of the -TRACK parameter
// 960401 Mats      Version 0.97
// 960402 Mats		Fixed a bug in the pause handling, the track number was 
//				    lost if the CD was in pause mode
// 960402 Mats		Fixed the problem with the CD losing the pause mode
// 960402 Mats      Version 0.98 (First release to the Internet)
// 960403 Mats		Improved the CD Info dialog. Kbd only operation is now
//				    easier when entering a new record.
// 960403 Mats		Added the -PAUSE command line option. Can be used to 
//					create a PAUSE/RESUME shortcut.
// 960403 Mats		Fixed the background color to "transparent" around the icon
// 960403 Mats      Version 0.99
// 960403 Mats		Track text is selected when a track is selctecd in the 
//					track listbox.
// 960404 Mats		TOTAL REWRITE OF ALL CD FUNCTIONS! The player now consumes
//					less CPU power!
// 960404 Mats		Fixed the Resume problem!
// 960404 Mats      Eject/Close now works!
// 960404 Mats		Rewrote the track handling so it is zero based and can use
//					programmed play!
// 960404 Mats		Added programmed play to the CD Info dialog
// 960404 Mats		Added Repeat
// 960404 Mats		Check for CD change in CD Info when OK is pressed
// 960409 Mats		Ampersand (&) made an underscore in menu if included in 
//					track title. Fixed.
// 960409 Mats		Added randomize
// 960409 Mats		Added some tooltip display options
// 960409 Mats		Added settings for left button functions
// 960409 Mats		Decreased tabstops in CD Info listbox
// 960409 Mats		Version 1.00beta2
// 960409 Mats		Fixed some CD change bugs
// 960410 Mats	    -PLAY and -TRACK played the wrong track!
// 960412 Mats		Fixed some topptil not updating bugs due to resume not 
//					sending notify...
// 960412 Mats		Changing track name also changes the name in the playlist
// 960412 Mats		Set track becomes default button if track edit is selected
// 960412 Mats		Version 1.00beta3
// 960413 Mats		When switching CD to a non audio CD or no CD at all the 
//					tooltip was not updated
// 960413 Mats		Eject/Load didn't always work, it now checks the status 
//					for OPEN to know what to do.
// 960415 Mats		Pause/Resume on stopped CD starts CD
// 960417 Mats		Fixed the CD remain and CD position using programmed tracks
// 960417 Mats		Fixed display of N/A if randomize
// 960419 Mats		Version 1.00
// 960420 Mats		Fixed the Quit message appearing even if the CD has been 
//					taken out of the player...
// 960420 Mats      Fixed Infinite repeat if not programmed
// 960420 Mats      Version 1.01
// 960520 Mats      Moved the time display stuff to a function that gets called
//                  when the mouse is moved over the icon. This prevents the
//                  app to query the time all the time...
// 960520 Mats      Icon shows status!
// 960524 Mats      Added Exit on CD remove
// 960524 Mats      Added stop to the left button functions
// 960528 Mats      Added display of programmed playtime in CD Info dialog
// 960528 Mats      Added the CD Database Editor
// 960528 Mats      Added the Export/Import functions to the db editor
// 960528 Mats      Added ODBC support to the CD DB
// 960529 Mats      Rewrote the ODBC support so it resides in a separate DLL
//                  since the CD DB DLL is configurable in the INI file 
//                  theoreticly anyone can create a CD DB this way.
// 960529 Mats      Rewrote a lot of the status check stuff 
// 960529 Mats      Added SKIP dialog
// 960529 Mats      Added SETABSPOS dialog
// 960529 Mats      Added repeat track
// 960529 Mats      Added play whole, when CD is programmed
// 960529 Mats      Version 1.10 Beta 1
// 960530 Mats      Rewrote some of the DB dlg init stuff in order to make 
//                  ODBC to work with one open statement. (SQL Server...)
// 960530 Mats      Moved the DB DLL init after the -PLAY stuff that is sent
//                  to an existing NTFY_CD instance. Otherwise the login 
//                  failed.
// 960531 Mats      Fixed the bug with the popup tracks menu showing the 
//                  wrong current track
// 960606 Mats      Decreased the size of the CD DB dialog
// 960606 Mats      Added statistics to the CD DB dialog
// 960606 Mats      Added TRACKLENGTH to tooltip options
// 960607 Mats      Rewrote the TRACK DB handling for external DB drivers
// 960609 Mats      Fixed a bug with repeat track during play whole
// 960609 Mats      Fixed the bug with a CD in pause mode not detecting 
//                  an eject of the cd.
// 960609 Mats      Added -RANDOM cmd-line option
// 960609 Mats      Added categories. Still some work in order to use it 
//                  everywhere!
// 960609 Mats      Added version handling in the export/import routines
// 960616 Mats      Added context menu for records in the TreeView in DB 
//                  editor
// 960616 Mats      Added aliasing for records
// 960616 Mats      Added category choose dialog for export and report
// 960616 Mats      Splitted the source in order to make the CDDB maintaince 
//                  app.
// 960616 Mats      Version 1.10 Beta 2
// 960715 Mats      Fixed the bug where repeat and repeat track status was 
//                  still there when switching CD!
// 960715 Mats      Fixed a cosmetic bug with the current time field in the
//                  skip dialog being to small to display "No CD"
// 960722 Mats      Added Selected time in the info dialog
// 960730 Mats      Fixed the TreeView problem! The data should be copied into
//                  pszText instead of setting pszText to point on the data!
// 960730 Mats      Version 1.10
// 960812 Mats      Fixed the bug with track names not being placed in the edit
//                  field when selected in the listbox
// 960812 Mats      Version 1.11
// 960827 Mats      Clear ret string in CDDBGetInfo
// 960827 Mats      Progress bar only shows each 10 counted
// 960827 Mats      Added Delete to the Disc popup menu in the CDDB.
// 960903 Mats      Fixed a bug with -RANDOM when no disc is inserted
// 960903 Mats      Fixed a bug with -PLAY when a data disc was inserted
// 960925 Mats      Added support for multiple physical CD-ROMs
// 960925 Mats      Added "Full status check" option to use with older
//                  CD-ROMs without notify functionality
// 960925 Mats      Added "Prev always prev track" that always selects the 
//                  previous track. This is the old functionality. 
//                  The new prev functionality restarts the current track if
//                  track has been playing for more than 5 seconds.
// 960925 Mats      Devices meny only present with more that one CD-ROM
// 960925 Mats      Config is saved when user OK's the options dialog
// 960925 Mats      Status is saved and retrieved if "Remember status" is set
// 960926 Mats      Tracks menu is broken into submenys if nProgrammedTracks
//                  is greater than nMenuBreak.
// 960926 Mats      -PAUSE starts CD in pause mode when this is the first
//                  instance
// 961024 Mats      Playing programmed tracks plays all sequential tracks 
//                  before checking next track, this to prevent abrupting
//                  tracks that have sequential music.
// 961024 Mats      Broke up the Open/Close into two choises since the MCI
//                  API doesn't provide me with a way to figure out if the tray
//                  is closed or not if there's no disc in the tray. 
//                  Windows CD Player can't handle this either.
// 961024 Mats      Doubleclick in Tracks adds to playlist and in playlist
//                  removes from playlist
// 961024 Mats      Disable dialogs in menu when they are open
// 961024 Mats      Added -STOP, -PREV, -NEXT cmd-line options for use with
//                  second instance
// 961025 Mats      1.20 beta 1
// 961123 Mats      Fixed the bug that caused the player not to play programmed 
//                  tracks after the last track on the CD if it was in the 
//                  middle of the programmed list
// 961123 Mats      Added column menu
// 961123 Mats      Added Win95/NT4.0 look a like bitmap
// 961124 Mats      Resizable CDDB dialog
// 961124 Mats      Fixed the bug that caused the player to always play the 
//                  first track when started from the Explorer.
// 961124 Mats      Option to remove the menu bitmap
// 961124 Mats      1.20 beta 2
// 961211 Mats      Editing artist names in the DB dialog now works.
// 961211 Mats      If first track isn't audio, check all tracks for audio
//                  tracks in order to allow mixed data/audio discs.
// 961211 Mats      Fixed the skip dialog so it works correctly with arrow 
//                  keys, page up/page down and home/end
// 961211 Mats      Changed the bitmap to the new look supplied by a user!
//                  vginkel@idirect.com
// 961212 Mats      Added a random buttom to the Info dialog. 
// 961212 Mats      Improved the random play so it don't play the same track
//                  until five other tracks has already been played!
// 961213 Mats      Added the "No menu break" option that cause the player
//                  not to break up track menu in submenus.
// 961213 Mats      Fixed the problem with not resetting to the first track
//                  after finishing playing
// 961213 Mats      Fixed the problem with the menu breakup causing to many 
//                  submenus
// 961215 Mats      Added the categories dialog
// 961216 Mats      Fixed a problem with repeat
// 970102 Mats      Added an option for the default CD device
// 970102 Mats      Fixed the bug with the tracks column menu not dissapering
//                  when removing the CD.
// 970102 Mats      Command line can handle different devices. Autostart from
//                  the explorer for instance.
// 970102 Mats      1.20 beta 3
// 970114 Mats      Fixed the bug with the menubreak in the first splitted
//                  tracks submeny
// 970114 Mats      Rewrote the default device handling
// 970114 Mats      Increased the size of the default device combo
// 970114 Mats      Fixed cancel and ok while editing in the TreeView
// 970114 Mats      Handle WM_ENDSESSION to shut down gracefully
// 970117 Mats      Added an option to UpdateDiscInformation that tells it to do 
//                  extensive track checking because CDGetCurrTrack might 
//                  return the next track after a track has finished.
// 970117 Mats      Added the CDDBSave function
// 970117 Mats      Extended the strings that handle track names
// 970117 Mats      Added a dialog that displays a message for DB DLL's that
//                  doesn't support the DB DLL.
// 970117 Mats      Added DB properties to the options dlg
// 970120 Mats      1.20 beta 4
// 970120 Mats      Fixed an invalid call to RemoveMenu
// 970120 Mats      Fixed some memory leaks!
// 970121 Mats      CDInit doesn't close and reopen each time
// 970121 Mats      Changing device doesn't call CDInit after CDClose and 
//                  fixing the menu. Instead it calls StatusCheck which in
//                  turn calls CDInit if appropriate.
// 970121 Mats      Fixed a bug with the command line and getting the device.
//                  The command line wasn't parsed for device if if it wasn't
//                  -PLAY, the Explorer by default sends /PLAY on autoplay...
// 970124 Mats      Implemented the Get from Internet button
// 970128 Mats      Added more debug info...
// 970128 Mats      Moved disc info storage functions to a separate function
//                  because InternetSend had to use it too!
// 970203 Mats      1.20beta5 (first beta1 of CDDB)
// 970211 Mats      Fixed the problem with the "no disc" stuff!
// 970218 Mats      Disable CLOSE when playing
// 970218 Mats      Started using WM_DEVICECHANGE to detect removal and 
//                  insertion of discs. Seems to work ok.
// 970224 Mats      1.20beta6
// 970225 Mats      Fixed the problem with categories multiplying when doing
//                  an internet get
// 970310 Mats      Pressing the random button didn't update the playlist 
//                  length
// 970310 Mats      Fixed the bug with track titles becoming to long
// 970310 Mats      Rewrote a little of the ID code. Moved CDDBGetDiscID into
//                  CDGetID and removed the nCurrID variable. Only the zCurrCDDBID
//                  is used from now.
// 970312 Mats      Rewrote the random play. Tracks will only repeat themself 
//                  after all other tracks have been played
// 970319 Mats      Added support for WM_DISPLAYCHANGE which removes and 
//                  adds the tray icon to fix the bad looking icon.
// 970401 Mats      Added REPEAT command line option.
// 970401 Mats      Notify closes the device if it finds a data disc
// 970408 Mats      Fixed open after adding the above stuff
// 970408 Mats      Fixed the bug with incorrect track lengths on mixed discs
// 970408 Mats      Fixed a bug with switching devices
// 970408 Mats      Fixed the bug where resume (after restarting the app) 
//                  didn't get the MCI_SUCCESS message due to hMainWnd being
//                  NULL!
// 970411 Mats      Fixed the bug where pressing ESC in the DB DLG caused a 
//                  crash
// 970411 Mats      1.20beta7 released to special testers
// 970415 Mats		InitInfoDlg didn't set the category
// 970423 Mats      Hide categories in options if using the CDDB plug-in
// 970424 Mats		1.20 released
// 970504 Mats		Added code that enable Full Status Check until first
//                  device arrival message appears. 
// 970504 Mats		Fixed the crash bug with programmed discs
// 970504 Mats		Fixed the bug where WM_ENDSESSION didn't stop the cd
// 970510 Mats		If full status check is enabled and we got devicechange
//					we skip the WM_DEVICECHANGE message and trust the full
//					status check
// 970510 Mats		Enabled the categories again. Illegal categories are 
//					refused when submitting to the CDDB repository and this
//					is handled by the plug-in
// 970511 Mats		Renamed the full status check option to no insert 
//					notification
// 970511 Mats		Fixed the repeat track bug where the disc stopped after
//					the track that used to be repeated after the repeat track 
//					function had been reset. Fixed by setting the dwTo 
//					parameter to the very last millisecond of the disc.
// 970511 Mats		Repeat track (bRepeatTrack) wasn't reseted when changing
//					the disc.
// 970511 Mats		Wrote INF file for SETUP
// 970513 Mats		Fixed the NoAudio bug. WM_DEVICECHANGE messages seems to 
//					be sent from time to time without any devicechange!
// 970513 Mats		Added timestamp to logfile
// 970515 Mats      1.21
// 970603 Mats		Fixed repeat bug
// 970604 Mats		1.22
// 970710 Mats      More fixing of the repeat bug
// 970710 Mats		Fixed stuff for supporting the DB Editor with the CDDB 
//					plug-in
// 970727 Mats		Added extended disc and track info to the CD Info dialog
// 9708xx Mats      Starting with an already playing disc that is not 
//                  programmed do not cause an interruption in the music
//                  anymore.
// 970803 Mats      Removed support for plug-ins!
// 970804 Mats      Moved a lot of stuff to separate files
// 970804 Mats      Incorporated the CDDB stuff into the main program
// 970804 Mats      Added MOTD
// 970804 Mats      Choose disc dialog and others now come on top
// 970804 Mats      Store copy in CDPLAYER.INI!
// 970805 Mats      A lot of stuff with the database handling rewritten and
//                  "rethought". You can now use the player without any local
//                  storage but still use the remote CDDB server for example.
//                  You can use the remote server and store in the INI file.
// 970805 Mats      Added the show on caption feature
// 970805 Mats      Added tracks combo to the skip dialog
// 970805 Mats      Added tracks combo to the set abs track pos
// 970811 Mats      CDDB can now use all categories for local storage
// 970811 Mats      1.50beta1
// 970811 Mats      Fixed some crashes
// 970811 Mats      1.50beta2
// 970812 Mats      Fixed proxy
// 970812 Mats      Fixed a bug causing a crash when reporting on an empty 
//                  db.
// 970812 Mats      Fixed the save of new entry bug (CDDB)
// 970812 Mats      Added check for existance of a category when saving to CDDB
// 970812 Mats      Fixed crash when sending to Internet DB
// 970812 Mats      Added customizable caption font
// 970812 Mats      Fixed repainting of caption when string was shorter than 
//                  before
// 970812 Mats      Fixed the bug with query local requiring a CDDB path 
//                  when using CDPLAYER.INI
// 970812 Mats      Fixed a bug with extended disc information
// 970813 Mats      Fixed the bug with importing records to the DB
// 970813 Mats      Changed the handling of "No insert notifcation". The 
//                  option will stick even though the app figures out it 
//                  really has insert notification.
// 970813 Mats      Added a messagebox telling the user of a remote connection
//                  timeout.
// 970813 Mats      Store copy in CDPLAYER.INI only stores if a disc was 
//                  really found...
// 970813 Mats      1.50 beta 3
// 970814 Mats      Fixed the bug with empty disc choose dialog
// 970815 Mats      Fixed the bug with changing device to drive Z:
// 970815 Mats      Read-only checkbox should not be visible on export
// 970818 Mats      Added a DBDelete when category changes in order to prevent
//                  local CDDB to find the old entry.
// 970818 Mats      Fixed the bug with not deleting temporary files when 
//                  saving in CDDB format!
// 970820 Mats      Fixed some painting problems with the "Show on caption" 
//                  option.
// 970820 Mats      Repaint foreground window caption on program exit if
//                  show on caption option is used.
// 970820 Mats      Rewrote parts of the CDDB parsing code to make it faster
// 970821 Mats      Moved the "Show on active caption" option to the tooltip
//                  tab.
// 970821 Mats      Rewrote the options dialog so that tabs work
// 970821 Mats      Fixed some enable/disable stuff in the options dialog
// 970821 Mats      Convert settings from 1.21
// 970824 Mats      Sending a PLAY command to another instance without a 
//                  track specified will NOT restart the disc as it did 
//                  before.
// 970824 Mats      1.50 beta 4
// 970825 Mats      Fixed the bug with database editor showing faulty entries
// 970825 Mats      Fixed the bug with playing track -1
// 970825 Mats      Fixed the bug with failing to save if the entry was in
//                  the currently cached file.
// 970825 Mats      Fixed the bug with caption text overlapping the window 
//                  caption
// 970826 Mats      Fixed the bug with changing category not deleting old
//                  entry
// 970826 Mats      Fixed the crash when using playlist
// 970826 Mats      Rewrote some parts of the CDDB handling so that it uses
//                  the track frames from the entry when saving, except when
//                  submitting to the repository
// 970827 Mats      Added a send choise to the context menu of a disc in
//                  the database editor. This required some changes to the
//                  CDDB handling that might be a problem
// 970827 Mats      Starting first instance with /PLAY wont restart an already
//                  playing disc if the track number supplied is -1
// 970827 Mats      Added separate display options for the caption
// 970902 Mats      Fixed so that Show On Caption doesn't paint on a 
//                  WS_EX_TOOLWINDOW style window.
// 970902 Mats      1.50 beta 5
// 970905 Mats      Added External Command
// 970905 Mats      Double-clicking on icon now uses the system double-click 
//                  delay time
// 970905 Mats		Added MIME headers to the outgoing mail
// 970906 Mats		Fixed the bug that made the DB Editor crash under 
//					Windows 95 (It might ahve caused crashes on NT too)
// 970906 Mats      Fixed the bug that made the DB Editor hang sometimes
// 970906 Mats		Made the DB Editor initialize faster!
// 970907 Mats		Added a new tolltip/caption display option, Track number.
//					This was automaticly shown before when the Track name 
//					option was choosen.
// 970907 Mats		Added option to show artist and CD title in the top of the
//					popup menu.
// 970908 Mats      Added scrollbars to the disc and track information tabs
// 970908 Mats      Fixed so that the installer installs the documentation
// 970910 Mats      Fixed a bug that made it impossible to import to INI file
// 970910 Mats      Fixed a bug in importing where setting the CDDB tracks
//                  could try to set more tracks than allocated.
// 970910 Mats      Fixed more bugs with INI file handling
// 970910 Mats      Fixed a bug with the context menu in the DB Editor
// 970910 Mats      Fixed a bug with changing category in the DB Editor. It 
//                  didn't delete the old entry if the category changed.
// 970911 Mats      Open shareable by default
// 970911 Mats      1.50 beta 6
// 970915 Mats      Fixed the bug with autoplay
// 970915 Mats      Added context sensitive help
// 970915 Mats      1.50 beta 7
// 970915 Mats      Fixed the motd bug with CDDB
// 971015 Mats      Fixed the HTTP/1.1 bug
// 971020 Mats      Added MIME/QP in the submit function
// 971023 Mats      The tracks combo in the skip dialog gets updated if the 
//                  disc is changed.
// 971111 Mats      Added Proxy-Authentication
// 971118 Mats      1.50 rc 1
// 971203 Mats      Added horizontal scroll bar in the choosedisc dialog
// 971209 Mats      Use static RTL to help drop the confusion around the 
//                  MSVCRT*.DLL files some people seems to have.
// 971215 Mats      1.50 rc 2
// 971216 Mats      No track lenght limitation in the Info Dialog
// 971216 Mats      No track lenght limitation in the Database Editor
// 971216 Mats      Added the Ask for password option to the proxy 
//                  authentication support
// 971218 Mats      Fixed so that we can read "unlimited" lenght strings 
//                  from CDDB entries although they should be limited to
//                  76 chars or so according to the spec.
// 971223 Mats      Fixed a bug in the DB Editor. Changes to Artist was never
//                  saved.
// 971223 Mats		Fixed a bug with reading disc entries that caused the
//					last line to be skipped.
// 971223 Mats		Random play doesn't continue after all tracks have played
//					if the repeat flag isn't set.
// 971223 Mats		When we have played all random tracks and are about to
//					repeat the procedure, make sure we don't play the same
//					track as we did just before we reseted the played random
//					tracks list.
// 971223 Mats		If we are in repeat mode and receive a PLAY command from
//					the commandline or another instance, don't play track 1. 
//					Play a random track there also.
// 971227 Mats		Added version check in About box
// 971229 Mats		Fixed a bug in the memory allocation for Track names when
//					using CDPLAYER.INI
// 971230 Mats      Version 1.50
// 980105 Mats      When doing a remote query and receiving an entry we need
//                  to accept any DISCID line since the server might have 
//                  found a fuzzy match that doesn't include our DISCID
// 980105 Mats		Fixed the bug with the crashing OPTIONS dialog
//					When using the retail (4.00.00.950) version of 
//					COMCTL32.DLL, you *MUST* have Popup style on the dialog, or
//					it will CRASH AND BURN!
//					Also, *DO NOT* use Help ID on controls or the page will
//					show up empty! Don't you just love those guys sometimes?
// 980106 Mats      Minor fix in the submit function. Some mail servers screw
//                  up the headers otherwise.
// 980106 Mats      Version 1.51
// 980112 Mats      Found a bug with the disc id that was in uppercase 
//                  sometimes...                 
// 980113 Mats      Found a bug in the Base64 encoding causing problems
//                  if the password lenght was an (exact multiple of 3) + 1
// 980114 Mats      Don't delete entry when category is changed if we use
//                  the INI file as local database
// 980114 Mats      Make query local and store local (INI) default if no
//                  old options are found.
// 980115 Mats      Found and fixed a parsing bug of CDDB entries in Windows
//                  format that caused a crash
// 980210 Mats      Fixed another crash with local query
// 980210 Mats      Fixed the problem with the DB Editor showing multiple 
//                  entries when using the INI file as DB.
// 980210 Mats      Fixed the bug with disc length 0 if a new entry was saved
// 980210 Mats      Fixed sorting of the right pane in the DB Editor
// 980301 Mats		Fixed a bug with the db editor when we had done
//					a remote query and was using the INI file without 
//					automatic local storage. The db editor would find multiple
//					entries of the same disc.
// 980301 Mats		Fixed a possible DB Editor crash
// 980302 Mats      Fixed the problem where information wasn't gathered after
//                  the DB Editor was used.
// 980302 Mats      Hopefully (again) fixed the bug with InternetSend 
//                  complaining about illegal category
//
///////////////////////////////////////////////////////////////////////////////


#define STRICT
#define WIN32_LEAN_AND_MEAN

#pragma warning(disable:4201)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <windows.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <commctrl.h>
#include <dbt.h>

#include "resource.h"

#define ALLOCATE

#include "ntfy_cd.h"
#include "misc.h"
#include "db.h"
#include "mci.h"
#include "options.h"
#include "dbdlg.h"

BOOL bFoundLocal = FALSE;
BOOL bFoundRemote = FALSE;
HWND hLastForegroundWindow;

void InitTracksMenu()
{
    HMENU hTracksMenu;

    if ((nOptions & OPTIONS_TRACKSMENUCOLUMN)) {
        RemoveMenu(GetSubMenu(hTrackMenu, 0), IDM_TRACKS, MF_BYCOMMAND);
        hTracksMenu = GetSubMenu(hTrackMenu, 0);
    }
    else    
        hTracksMenu = GetSubMenu(GetSubMenu(hTrackMenu, 0), nMenuIndexTracks);

    if (hTracksMenu) {
        char szTmp[300];

        if (!(nOptions & OPTIONS_TRACKSMENUCOLUMN)) {
            while (RemoveMenu(hTracksMenu, 0, MF_BYPOSITION))
                ;
        }

        if (nProgrammedTracks < nMenuBreak || (nOptions & OPTIONS_NOMENUBREAK)) {
            bBrokenTracksMenu = FALSE;

            for (int nLoop = 0 ; nLoop < nProgrammedTracks ; nLoop ++) {
                sprintf(szTmp, "%d. %s", pnProgrammedTracks[nLoop]+1, ppzTracks[pnProgrammedTracks[nLoop]]);
			    CheckAmpersand(szTmp);
                if (!nLoop && (nOptions & OPTIONS_TRACKSMENUCOLUMN))
                    AppendMenu(GetSubMenu(hTrackMenu, 0), MF_MENUBARBREAK | MF_STRING, IDM_TRACKS+nLoop, szTmp);
                else
                    InsertMenu(hTracksMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_TRACKS+nLoop, szTmp);
            }

            if ((nOptions & OPTIONS_TRACKSMENUCOLUMN)) {
                for (nLoop = 0 ; nLoop < nProgrammedTracks ; nLoop ++) {
                    sprintf(szTmp, "[%s]", ppzTrackLen[pnProgrammedTracks[nLoop]]);
			        CheckAmpersand(szTmp);
                    if (!nLoop && (nOptions & OPTIONS_TRACKSMENUCOLUMN))
                        AppendMenu(GetSubMenu(hTrackMenu, 0), MF_MENUBREAK | MF_STRING, IDM_TRACKS+nLoop, szTmp);
                    else
                        InsertMenu(hTracksMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_TRACKS+nLoop, szTmp);
                }
            }
        }
        else {
            int nNumSubMenys = nProgrammedTracks / 10;
            if (nProgrammedTracks % 10)
                nNumSubMenys ++;

            bBrokenTracksMenu = TRUE;

            for (int nLoop2 = 0 ; nLoop2 < nNumSubMenys ; nLoop2 ++) {
                int nLoop;
                HMENU hMenu = CreateMenu();
                char zTmp[300];
            
                if (nLoop2*10+10 < nProgrammedTracks)
                    sprintf(zTmp, "Track %d to %d", nLoop2*10 + 1, nLoop2*10+10);
                else
                    sprintf(zTmp, "Track %d to %d", nLoop2*10 + 1, nProgrammedTracks);

                if (!nLoop2 && (nOptions & OPTIONS_TRACKSMENUCOLUMN))
                    AppendMenu(hTracksMenu, MF_MENUBARBREAK | MF_BYPOSITION | MF_POPUP | MF_STRING, (int)hMenu, zTmp);
                else
                    InsertMenu(hTracksMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_POPUP | MF_STRING, (int)hMenu, zTmp);
    
                for (nLoop = nLoop2*10 ; (nLoop - nLoop2*10) < 10 && nLoop < nProgrammedTracks ; nLoop ++) {
                    sprintf(szTmp, "%d. %s", pnProgrammedTracks[nLoop]+1, ppzTracks[pnProgrammedTracks[nLoop]]);
			        CheckAmpersand(szTmp);
                    InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_TRACKS+nLoop, szTmp);
                }
            }
        }
    }
}

void InitMenu()
{
    HMENU hTracksMenu;

	if (hTrackMenu)
        DestroyMenu(hTrackMenu);

    hTrackMenu = LoadMenu(hMainInstance, MAKEINTRESOURCE(IDR_MENU));

    if (!(nOptions & OPTIONS_TRACKSMENUCOLUMN)) {
		if (!(nOptions & OPTIONS_ARTISTINMENU)) {
			nMenuIndexTracks = 7;
			nMenuIndexOther = 14;
			nMenuIndexDevices = 15;
		}
		else {
			nMenuIndexTracks = 9;
			nMenuIndexOther = 16;
			nMenuIndexDevices = 17;
		}
    }
    else {
		if (!(nOptions & OPTIONS_ARTISTINMENU)) {
			nMenuIndexOther = 12;
			nMenuIndexDevices = 13;
		}
		else {
			nMenuIndexOther = 14;
			nMenuIndexDevices = 15;
		}

        RemoveMenu(GetSubMenu(hTrackMenu, 0), 6, MF_BYPOSITION);
        RemoveMenu(GetSubMenu(hTrackMenu, 0), 6, MF_BYPOSITION);
    }

    if (!(nOptions & OPTIONS_NOMENUBITMAP)) {
        SetMenu(hMainWnd, GetSubMenu(hTrackMenu, 0));

        InsertMenu(GetSubMenu(hTrackMenu, 0), 20, MF_STRING | MF_OWNERDRAW | MF_BYPOSITION | MF_MENUBARBREAK, 999, (char*)1);
    
        DrawMenuBar(hMainWnd);
    }

    hTracksMenu = GetSubMenu(hTrackMenu, 0);
    
	if (nOptions & OPTIONS_ARTISTINMENU) {
		char szTmp[256];

		strcpy(szTmp, pzArtist);
		if (pzTitle && *pzTitle) {
			strcat(szTmp, " - ");
			strcat(szTmp, pzTitle);
		}

		InsertMenu(hTracksMenu, 0, MF_STRING | MF_BYPOSITION, 9999, szTmp);
		InsertMenu(hTracksMenu, 1, MF_SEPARATOR | MF_BYPOSITION, 9999, "");
		SetMenuDefaultItem(hTracksMenu, 0, TRUE);
	}

	if (bRepeat)
		ModifyMenu(hTracksMenu, IDM_REPEAT, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_REPEAT, "Repeat");
	else
		ModifyMenu(hTracksMenu, IDM_REPEAT, MF_BYCOMMAND | MF_STRING, IDM_REPEAT, "Repeat");

	if (bRandomize)
		ModifyMenu(hTracksMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_RANDOMIZE, "Random Play");
	else
		ModifyMenu(hTracksMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_STRING, IDM_RANDOMIZE, "Random Play");

    // Add devices to the devices sub menu or remove submenu if only one is present
    
    if (nNumberOfDevices == 1)
        RemoveMenu(GetSubMenu(hTrackMenu, 0), nMenuIndexDevices, MF_BYPOSITION);
    else {
        HMENU hDevicesMenu;
        hDevicesMenu = GetSubMenu(GetSubMenu(hTrackMenu, 0), nMenuIndexDevices);

        if (hDevicesMenu) {
            char szTmp[80];

            RemoveMenu(hDevicesMenu, 0, MF_BYPOSITION);

            for (int nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
                if (abDevices[nLoop - 'A']) {
                    sprintf(szTmp, "Drive %c:", nLoop);
                    if (nLoop - 'A' == nCurrentDevice)
                        InsertMenu(hDevicesMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING | MF_CHECKED, IDM_DEVICES+nLoop, szTmp);
                    else
                        InsertMenu(hDevicesMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_DEVICES+nLoop, szTmp);
                }
            }
        }
    }

    // Fix tracks menu
    
    InitTracksMenu();

    if (!(nOptions & OPTIONS_NOMENUBITMAP) && (nOptions & OPTIONS_TRACKSMENUCOLUMN)) {
        RemoveMenu(GetSubMenu(hTrackMenu, 0), 999, MF_BYCOMMAND);
        InsertMenu(GetSubMenu(hTrackMenu, 0), 21+(nProgrammedTracks*2), MF_STRING | MF_OWNERDRAW | MF_BYPOSITION | MF_MENUBARBREAK, 999, (char*)1);
    
        DrawMenuBar(hMainWnd);
    }
}


BOOL CheckCategory(char* pzCategory, int nCategory)
{
    int nLoop;

    if (nCategory == -1)
        return TRUE;

    for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++) {
        if (!stricmp(ppzCategories[nLoop], pzCategory) && nCategory == nLoop)
            return TRUE;
    }


    return FALSE;
}


void CheckProgrammed()
{
	int nLoop;

    nCurrTrack = CDGetCurrTrack() - 1;

	bProgrammed = FALSE;
	// Check if programmed list is the same as the track order
	if (nProgrammedTracks == nMaxTrack) {
		for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++) {
			if (pnProgrammedTracks[nLoop] != nLoop) {
				bProgrammed = TRUE;
				nLoop = nMaxTrack;
			}
		}
	}
	else {
		bProgrammed = TRUE;
	}

    // Find first match!
	for (nLoop = 0 ; nLoop < nProgrammedTracks; nLoop ++) {
		if (pnProgrammedTracks[nLoop] == nCurrTrack) {
			nCurrTrack = nLoop;
			nLoop = nProgrammedTracks + 1;
		}
	}
	if (nLoop != nProgrammedTracks + 2)
		CDStop();

    if (bProgrammed && bPlaying)
        CDPlay(0);

    if (pnLastRandomTracks)
        free(pnLastRandomTracks);

    pnLastRandomTracks = (int*) malloc(nMaxTrack * sizeof(int));
    nLastRandomTrack = 0;
}


void ResetPlaylist(BOOL bProgram)
{
	char* pzOrder;
    int nLoop;

    pzOrder = NULL;

    if (!bProgram)
        bPlayWhole = TRUE;
    else
        bPlayWhole = FALSE;

    // Get the programmed list!
	if (pnProgrammedTracks)
		free(pnProgrammedTracks);

    DBGetInfo(zCurrCDDBID, zCurrMCIID, "ORDER", &pzOrder);

	nProgrammedTracks = DBGetInfoInt(zCurrCDDBID, zCurrMCIID, "NUMPLAY");
	if (!bProgram || (!nProgrammedTracks || !pzOrder[0])) {
		nProgrammedTracks = nMaxTrack;

		pnProgrammedTracks = (int*) malloc(nProgrammedTracks*sizeof(int));
		for (nLoop = 0 ; nLoop < nProgrammedTracks ; nLoop ++)
			pnProgrammedTracks[nLoop] = nLoop;
	}
	else {
		int nPos;
		char* pzPtr;

		pnProgrammedTracks = (int*) malloc(nProgrammedTracks*sizeof(int));

		nLoop = 0;
		nPos = 0;
		while(pzOrder[nPos] && nLoop < nProgrammedTracks) {
			pzPtr = strchr(&pzOrder[nPos], ' ');
			if (pzPtr) {
				*pzPtr = 0;

				pnProgrammedTracks[nLoop] = atoi(&pzOrder[nPos]);

				nPos += (pzPtr - &pzOrder[nPos]);
				nPos ++;
			}
			else {
				pnProgrammedTracks[nLoop] = atoi(&pzOrder[nPos]);
				nPos += strlen(&pzOrder[nPos]);
			}

			nLoop ++;
		}
	}

    free(pzOrder);

    // Update the "Tracks" menu!

    InitMenu();
}


void GetDiscInfo()
{
    int nLoop;
    char szStr[80];

DebugPrintf("GetDiscInfo()");

    if (ppzTrackLen) {
        for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++)
            free(ppzTrackLen[nLoop]);

        free(ppzTrackLen);
    }

    if (pnTrackLen)
        free(pnTrackLen);

    if (ppzTracks)
        DBFreeTrackTitles(ppzTracks, nMaxTrack);

    if (ppzTracksExt) {
        for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++)
            free(ppzTracksExt[nLoop]);

        free(ppzTracksExt);
    }

	nMaxTrack = DBGetInfoInt(zCurrCDDBID, zCurrMCIID, "NUMTRACKS");
	if (!nMaxTrack)
		nMaxTrack = CDGetTracks();

    nCurrTrack = CDGetCurrTrack() - 1;

    ppzTrackLen = (char**) malloc(nMaxTrack * sizeof(char*));
    for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++) {
        ppzTrackLen[nLoop] = (char*) malloc(10*sizeof(char));
		ppzTrackLen[nLoop][0] = 0;
	}

    ppzTracksExt = (char**) malloc(nMaxTrack * sizeof(char*));
    for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++)
        ppzTracksExt[nLoop] = NULL;

    pnTrackLen = (int*) malloc(nMaxTrack * sizeof(int));

    ppzTracks = DBGetTrackTitles(zCurrCDDBID, zCurrMCIID, nMaxTrack);

	sMCISet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	mciSendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);
	
    for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++) {
		pnTrackLen[nLoop] = CDGetTrackLength(nLoop + 1, szStr);
		strcpy(ppzTrackLen[nLoop], szStr);
    }

	sMCISet.dwTimeFormat = MCI_FORMAT_TMSF;
	mciSendCommand (sMCIOpen.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) (LPVOID) &sMCISet);

    DBGetInfo(zCurrCDDBID, zCurrMCIID, "ARTIST", &pzArtist);
    if (!pzArtist[0]) {
        free(pzArtist);

        if (!bFoundLocal && !bFoundRemote)
            pzArtist = strdup("New Artist");
    }
    DBGetInfo(zCurrCDDBID, zCurrMCIID, "TITLE", &pzTitle);
    if (!pzTitle[0]) {
        free(pzTitle);

        if (!bFoundLocal && !bFoundRemote)
            pzTitle = strdup("New Title");
    }
    DBGetInfo(zCurrCDDBID, zCurrMCIID, "CATEGORY", &pzCategory);
    if (!pzCategory[0]) {
        free(pzCategory);

        pzCategory = strdup("");
    }
    DBGetInfo(zCurrCDDBID, zCurrMCIID, "EXTD", &pzDiscExt);
	FixNewLine(FALSE, &pzDiscExt);
	for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++) {
		sprintf(szStr, "EXTT%d", nLoop);
	    DBGetInfo(zCurrCDDBID, zCurrMCIID, szStr, &ppzTracksExt[nLoop]);
	    FixNewLine(FALSE, &ppzTracksExt[nLoop]);
	}

    ResetPlaylist(TRUE);
}


BOOL SetDiscInfo()
{
    char szTmp[1024];
    char szStr[80];
    int nLoop;
    BOOL bRet;

    // Save the info!

    DebugPrintf("-> SetDiscInfo()");

    DBSetInfoInt(zCurrCDDBID, zCurrMCIID, "NUMTRACKS", nMaxTrack);
    DBSetInfo(zCurrCDDBID, zCurrMCIID, "ARTIST", pzArtist);
    DBSetInfo(zCurrCDDBID, zCurrMCIID, "TITLE", pzTitle);
    DBSetInfo(zCurrCDDBID, zCurrMCIID, "CATEGORY", pzCategory);
    DBSetTrackTitles(zCurrCDDBID, zCurrMCIID, ppzTracks, nMaxTrack);

    szTmp[0] = 0;

	for (nLoop = 0 ; nLoop < nProgrammedTracks ; nLoop ++) {
		if (szTmp[0])
			strcat(szTmp, " ");
		
		itoa(pnProgrammedTracks[nLoop], szStr, 10);
		strcat(szTmp, szStr);
	}

	DBSetInfo(zCurrCDDBID, zCurrMCIID, "ORDER", szTmp);
	DBSetInfoInt(zCurrCDDBID, zCurrMCIID, "NUMPLAY", nProgrammedTracks);

    FixNewLine(TRUE, &pzDiscExt);
	DBSetInfo(zCurrCDDBID, zCurrMCIID, "EXTD", pzDiscExt);
    FixNewLine(FALSE, &pzDiscExt);

	for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++) {
		sprintf(szStr, "EXTT%d", nLoop);
	    FixNewLine(TRUE, &ppzTracksExt[nLoop]);
	    DBSetInfo(zCurrCDDBID, zCurrMCIID, szStr, ppzTracksExt[nLoop]);
	    FixNewLine(FALSE, &ppzTracksExt[nLoop]);
	}
    
	bRet = DBSave();

    DebugPrintf("<- SetDiscInfo()");

    return bRet;
}


void DiscInit()
{
DebugPrintf("DiscInit()");

    if (!bCDOpened && !CDOpen()) {
        DebugPrintf("Open device failed");

        bMediaPresent = FALSE;
	    bAudio = FALSE;
	    zCurrCDDBID[0] = 0;
	    zCurrMCIID[0] = 0;
	    bPaused = FALSE;
	    bPlaying = FALSE;
	    nCurrTrack = 0;
	    bInit = FALSE;

        return;
    }

    bAudio = CDGetAudio();
	if (!bAudio) {
        DebugPrintf("Media not audio!");

        if (!(nOptions & OPTIONS_NOINSERTNOTIFICATION))
            CDClose();

        return;
    }

	DBGetDiscID(zCurrCDDBID, zCurrMCIID, TRUE, &bFoundLocal, &bFoundRemote);

    if (nOptions & OPTIONS_STOPONSTART)
		CDStop();

    GetDiscInfo();
  
    if (((bFoundLocal || bFoundRemote) && (nCDDBOptions & OPTIONS_CDDB_STORECOPYININI) && (nOptions & OPTIONS_USECDDB)) || 
        (bFoundRemote && (nOptions & OPTIONS_STORERESULT)))
        SetDiscInfo();

    CheckProgrammed();

    bInit = TRUE;

DebugPrintf("DiscInit() finished");
}


int GetRandTrack()
{	
	int nTrack, nLoop;
    BOOL bEqual = FALSE;
	int nTmpLast = -1;
    
	DebugPrintf("CDPlayRandTrack()");

    if (nLastRandomTrack == nProgrammedTracks) {
		if (nLastRandomTrack > 0)
			nTmpLast = pnLastRandomTracks[nLastRandomTrack-1];

        nLastRandomTrack = 0;
		DebugPrintf("All tracks in the playlist played on random play");

		if (bRepeat) 
			DebugPrintf("Resetting random played tracks and continue to play random since we have the repeat flag set");
		else {
			DebugPrintf("Stopping random play since we have played all tracks and we don't have repeat flag set");

			return -1;
		}
	}

	nTrack = nCurrTrack;
	while(!bEqual) {
        bEqual = TRUE;

		nTrack = rand() % nProgrammedTracks;

		// Check temporary track if it's not -1 so we don't play the same track twice
		// in a row when the random of the playlist is restarted due to repeat

		if (nTmpLast != -1 && nTrack == nTmpLast) {
			bEqual = FALSE;

			continue;
		}

        // Check last random tracks

        for (nLoop = 0 ; nLoop < nLastRandomTrack ; nLoop ++) {
            if (pnLastRandomTracks[nLoop] == nTrack)
                bEqual = FALSE;
        }
    }

    // Update last random tracks

    pnLastRandomTracks[nLastRandomTrack] = nTrack;
    if (nLastRandomTrack <= nProgrammedTracks)
        nLastRandomTrack ++;

	DebugPrintf("Number of random tracks played are %d", nLastRandomTrack);

	DebugPrintf("Random track to play is %d", nTrack);

	return nTrack;
}


void PlayRandTrack() 
{
	int nTrack;

	nTrack = GetRandTrack();
	if (nTrack != -1)
		CDPlay(nTrack);
}


void UpdateTooltipOrCaption(BOOL bTooltipOrCaption, char* pzStr)
{
    char szStr[512];
    int nCDInfoOptions;
    int nTimeOptions;

    if (bTooltipOrCaption) {
        nTimeOptions = nTooltipTimeOptions;
        nCDInfoOptions = nTooltipCDInfoOptions;
    }
    else {
        nTimeOptions = nCaptionTimeOptions;
        nCDInfoOptions = nCaptionCDInfoOptions;
    }

    if (!zCurrMCIID[0] || !bAudio) {
        strcpy(szStr, "No audio CD");

        if (strcmp(szStr, szToolTip)) {
            strcpy(szToolTip, szStr);

            NotifyModify(hMainWnd, 100, hTrayIcon, szToolTip);
        }

        return;
    }

    if (nCurrTrack < nProgrammedTracks && nProgrammedTracks && nCurrTrack != -1) {
		szStr[0] = 0;
		if (nCDInfoOptions & OPTIONS_CDINFO_ARTIST) {
			strcat(szStr, pzArtist);
		}
		if ((nCDInfoOptions & OPTIONS_CDINFO_TITLE) && pzTitle && *pzTitle) {
			if (szStr[0])
				strcat(szStr, " ");

			strcat(szStr, "(");
			strcat(szStr, pzTitle);
			strcat(szStr, ") ");
		}
        if (nCDInfoOptions & OPTIONS_CDINFO_TRACKNO) {
			char szTmp[32];

			if (szStr[0])
				strcat(szStr, " - ");
			itoa(pnProgrammedTracks[nCurrTrack]+1, szTmp, 10);
			strcat(szStr, szTmp);
			if (nCDInfoOptions & OPTIONS_CDINFO_TRACKTITLE) 
				strcat(szStr, ". ");
		}
        if (nCDInfoOptions & OPTIONS_CDINFO_TRACKTITLE) {
			if (!(nCDInfoOptions & OPTIONS_CDINFO_TRACKNO) && szStr[0])
				strcat(szStr, " - ");
			strcat(szStr, ppzTracks[pnProgrammedTracks[nCurrTrack]]);
		}
		if (nCDInfoOptions & OPTIONS_CDINFO_TRACKLENGTH) {
			char szTmp[32];

			strcat(szStr, " (");
            sprintf(szTmp, "%02d:%02d", pnTrackLen[pnProgrammedTracks[nCurrTrack]] / 60, 
                    pnTrackLen[pnProgrammedTracks[nCurrTrack]] % 60);
            strcat(szStr, szTmp);
			strcat(szStr, ") ");
		}
	}
    else {
		szStr[0] = 0;
		if (nCDInfoOptions & OPTIONS_CDINFO_ARTIST) {
			strcat(szStr, pzArtist);
		}
		if (nCDInfoOptions & OPTIONS_CDINFO_TITLE) {
			if (szStr[0])
				strcat(szStr, " ");

			strcat(szStr, "(");
			strcat(szStr, pzTitle);
			strcat(szStr, ") ");
		}
	}

    if (bPaused)
        strcat(szStr, " [pause]");
    else if (!bPlaying)
        strcat(szStr, " [stop]");
    else {
        if (nTimeOptions) {
            strcat(szStr, " [");
            strcat(szStr, szTime);
            strcat(szStr, "]");
        }
    }

    strcpy(pzStr, szStr);
}


void UpdateMenu(HMENU hMenu)
{
    HMENU hOtherMenu = GetSubMenu(hMenu, nMenuIndexOther);

    if (bAudio && bInit && bMediaPresent) {
	    if (nOptions & OPTIONS_ARTISTINMENU) {
		    char szTmp[256];

		    strcpy(szTmp, pzArtist);
		    if (pzTitle && *pzTitle) {
			    strcat(szTmp, " - ");
			    strcat(szTmp, pzTitle);
		    }

		    ModifyMenu(hMenu, 0, MF_STRING | MF_BYPOSITION, 9999, szTmp);
	    }

        EnableMenuItem(hMenu, IDM_PLAY, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_PAUSE, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_STOP, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_NEXT, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_PREV, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, 7, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(hMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_ENABLED);

        EnableMenuItem(hMenu, IDM_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        // Enable stuff in Other menu

        if (bProgrammed)
            EnableMenuItem(hOtherMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hOtherMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hOtherMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hOtherMenu, IDM_SETABSTRACKPOS, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hOtherMenu, IDM_SKIP, MF_BYCOMMAND | MF_ENABLED);

        if (!bPlayWhole && !bInInfoDlg)
            EnableMenuItem(hMenu, IDM_INFO, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_INFO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!bInDBDlg)
            EnableMenuItem(hMenu, IDM_DB, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_DB, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!bInSkipDlg)
            EnableMenuItem(hMenu, IDM_SKIP, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_SKIP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!bInSetAbsTrackPosDlg)
            EnableMenuItem(hMenu, IDM_SETABSTRACKPOS, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_SETABSTRACKPOS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!bInOptionsDlg)
            EnableMenuItem(hMenu, IDM_OPTIONS, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_OPTIONS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!bInAboutDlg)
            EnableMenuItem(hMenu, IDM_ABOUT, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hMenu, IDM_ABOUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            
        if (!bPlaying && !bPaused) {
            EnableMenuItem(hMenu, IDM_STOP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PAUSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            EnableMenuItem(hMenu, IDM_NEXT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PREV, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

            // Disable stuff in Other menu

            EnableMenuItem(hOtherMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            EnableMenuItem(hOtherMenu, IDM_SKIP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }

		if (ppzTracks && ppzTrackLen) {
			HMENU hSubMenu;
			char szTmp[300];

            // Update curr track
            UpdateDiscInformation(FALSE, TRUE, NULL);

            if (!(nOptions & OPTIONS_TRACKSMENUCOLUMN))
                hSubMenu = GetSubMenu(GetSubMenu(hTrackMenu, 0), nMenuIndexTracks);
            else
                hSubMenu = GetSubMenu(hTrackMenu, 0);
			for (int nLoop = 0 ; nLoop < nProgrammedTracks ; nLoop ++) {
                int nFlags;

                if (!(nOptions & OPTIONS_TRACKSMENUCOLUMN) || bBrokenTracksMenu)
                    sprintf(szTmp, "%d. %s\t[%s]", pnProgrammedTracks[nLoop]+1, ppzTracks[pnProgrammedTracks[nLoop]], ppzTrackLen[pnProgrammedTracks[nLoop]]);
                else
                    sprintf(szTmp, "%d. %s", pnProgrammedTracks[nLoop]+1, ppzTracks[pnProgrammedTracks[nLoop]]);

				CheckAmpersand(szTmp);

                nFlags = MF_BYCOMMAND | MF_STRING;
                if (nLoop == nCurrTrack)
                    nFlags |= MF_CHECKED;
                if (!nLoop && (nOptions & OPTIONS_TRACKSMENUCOLUMN) && !bBrokenTracksMenu)
                    nFlags |= MF_MENUBARBREAK;

                ModifyMenu(hSubMenu, IDM_TRACKS + nLoop, nFlags, IDM_TRACKS + nLoop, szTmp);
			}
		}
    }
    else {
	    if (nOptions & OPTIONS_ARTISTINMENU)
		    ModifyMenu(hMenu, 0, MF_STRING | MF_BYPOSITION, 9999, "No Disc");

        EnableMenuItem(hMenu, IDM_CLOSE, MF_BYCOMMAND | MF_ENABLED);

        EnableMenuItem(hMenu, IDM_PLAY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PAUSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_STOP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_NEXT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PREV, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_INFO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        if (!(nOptions & OPTIONS_TRACKSMENUCOLUMN))
            EnableMenuItem(hMenu, nMenuIndexTracks, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
        else {
            int nLoop;

            for (nLoop = 0 ; nLoop < 99 ; nLoop ++) {
                RemoveMenu(hMenu, IDM_TRACKS + nLoop, MF_BYCOMMAND);
                RemoveMenu(hMenu, IDM_TRACKS + nLoop, MF_BYCOMMAND);
            }
        }

        EnableMenuItem(hMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

        // Disable stuff in Other menu

        if (bProgrammed)
            EnableMenuItem(hOtherMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_ENABLED);
        else
            EnableMenuItem(hOtherMenu, IDM_PLAYWHOLE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hOtherMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hOtherMenu, IDM_SETABSTRACKPOS, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hOtherMenu, IDM_SKIP, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    }
}

void StatusCheck()
{
    BOOL bTmpMediaPresent;
    int nTmp;

    DebugPrintf("Status check!");
    DebugPrintf("Current device = %c", nCurrentDevice + 'A');

    // If CDOpen has failed, try again
    if (!bCDOpened) {
        CDClose();

        if (!CDOpen()) {
            bMediaPresent = FALSE;
	        bAudio = FALSE;
	        zCurrCDDBID[0] = 0;
	        bPaused = FALSE;
	        bPlaying = FALSE;
	        nCurrTrack = 0;
	        bInit = FALSE;

            return;
        }
    }

    nTmp = CDGetStatus();
	if (nStatus != nTmp) {
		nStatus = nTmp;
	}

    bTmpMediaPresent = CDGetMediaPresent();
    if (!bMediaPresent && bTmpMediaPresent) {
        DebugPrintf("Found media!");

        bMediaPresent = TRUE;
	    bPaused = FALSE;
	    bPlaying = FALSE;
	    nCurrTrack = 0;
        
        if (!bInit) {
DebugPrintf("Killing TIMER");
			KillTimer(hMainWnd, 1);

            DiscInit();
            
            SetTimer(hMainWnd, 1, nPollTime*1000, NULL);

            if (zExternalCommand[0] && pzArtist && pzTitle) {
                char zCmd[512];
                STARTUPINFO sStartup;
                PROCESS_INFORMATION sProcessInfo;

                sprintf(zCmd, "%s \"%s\" \"%s\" %s %s", zExternalCommand, pzArtist, pzTitle, zCurrMCIID, zCurrCDDBID);

                DebugPrintf("Running external command '%s'", zCmd);

                memset(&sStartup, 0, sizeof(sStartup));
                sStartup.cb = sizeof(sStartup);
                sStartup.dwFlags = STARTF_USESHOWWINDOW;
                sStartup.wShowWindow = SW_HIDE;

                if (CreateProcess(NULL, zCmd, NULL, NULL, FALSE, 0, NULL, NULL, &sStartup, &sProcessInfo)) {
                    CloseHandle(sProcessInfo.hProcess);
                    CloseHandle(sProcessInfo.hThread);
                }
            }
DebugPrintf("Restarting TIMER");            
        }

	    if (nStatus == 2)
            CDResume();
    }
    else if (bMediaPresent && !bTmpMediaPresent) {
        DebugPrintf("Media lost!");

        bMediaPresent = FALSE;
	    bAudio = FALSE;
	    zCurrCDDBID[0] = 0;
	    zCurrMCIID[0] = 0;
	    bPaused = FALSE;
	    bPlaying = FALSE;
	    nCurrTrack = 0;
	    bInit = FALSE;
		bRepeatTrack = FALSE;

        if (nOptions & OPTIONS_EXITONCDREMOVE)
			DestroyWindow(hMainWnd);
    }
}


void UpdateDiscInformation(BOOL bNotify, BOOL bTooltip, char* pzStr)
{
    // WARNING: This is also used by UpdatePopupMenu
    
    if (pzStr)
        pzStr[0] = 0;

    // If CD is playing
    if (bPlaying) {
        int nOldTrack;
        int nTrack, nMin, nSec, nFrame;

        if (bProgrammed)
            nOldTrack = nCurrTrack;
        else
            nOldTrack = 0;

	    nCurrTrack = CDGetCurrTrack() - 1;

        if (bNotify) {
            CDGetTime(0, TRUE, &nTrack, &nMin, &nSec, &nFrame);
            if (nMin == 0 && nSec < 5)
                nCurrTrack = nTrack - 2;
        }

	    if (bProgrammed) {
            // Scan upwards for active track...

            for (int nLoop = nOldTrack ; nLoop < nProgrammedTracks ; nLoop ++) {
                if (pnProgrammedTracks[nLoop] == nCurrTrack) {
                    nCurrTrack = nLoop;

                    break;
                }
            }
        }

        if (nTooltipTimeOptions || nCaptionTimeOptions) {
            if (bTooltip)
		        CDGetTime(nTooltipTimeOptions);
            else
		        CDGetTime(nCaptionTimeOptions);
        }
    }

    if (pzStr) {
        UpdateTooltipOrCaption(bTooltip, pzStr);

        if (bTooltip && strcmp(pzStr, szToolTip)) {
            strcpy(szToolTip, pzStr);

		    CheckAmpersand(szToolTip);

            NotifyModify(hMainWnd, 100, hTrayIcon, szToolTip);
        }
    }
}



/////////////////////////////////////////////////////////////////////
//
// ABOUT!
//
/////////////////////////////////////////////////////////////////////

HFONT hLinkFont;

BOOL CALLBACK AboutDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch(nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
            char szStr[512];
            
            bInAboutDlg = TRUE;

            CenterWindow(hWnd);

            sprintf(szStr, "Version %s", VERSION);
            SetWindowText(GetDlgItem(hWnd, IDC_VERSION), szStr);
        }
		break;

        case WM_CTLCOLORSTATIC: {
            if ((HWND)lParam == GetDlgItem(hWnd, IDC_EMAIL) || (HWND)lParam == GetDlgItem(hWnd, IDC_HOMEPAGE)) {
                if (!hLinkFont) {
                    LOGFONT lf;

                    hLinkFont = (HFONT)SendDlgItemMessage(hWnd, IDC_EMAIL, WM_GETFONT, 0, 0);

                    GetObject(hLinkFont, sizeof(lf), &lf);

                    lf.lfUnderline = TRUE;

                    hLinkFont = CreateFontIndirect(&lf);
                }

                SelectObject((HDC)wParam, hLinkFont);
                SetTextColor((HDC)wParam, RGB(0, 0, 255));
                SetBkMode((HDC)wParam, TRANSPARENT);

                return (int)GetStockObject(HOLLOW_BRUSH);
            }

            return NULL;
        }
        break;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case IDC_EMAIL:
                    ShellExecute(NULL, "open", "mailto:mlt@cyberdude.com", NULL, ".", SW_SHOWNORMAL);
                break;

                case IDC_HOMEPAGE: {
                    ShellExecute(NULL, "open", "http://www.artech.se/~mlt/software", NULL, ".", SW_SHOWNORMAL);
                }
                break;

				case IDC_CHECKVERSION: {
					if (!CheckForNewVersion()) 
						MessageBox(hWnd, "Check failed. Perhaps HTTP access wasn't acquired", APPNAME, MB_OK);
				}
				break;

                case IDCANCEL:
                case IDOK:
                    bInAboutDlg = FALSE;

                    EndDialog(hWnd, TRUE);
                break;
            }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// INFO
//
/////////////////////////////////////////////////////////////////////


void UpdatePlaylistTime(HWND hWnd)
{
    int* pnTracks = NULL;
    int nLoop;
    char szTmp[300];
	int nNum;
    int nSecs = 0;
    int nMin;

	// Get playlist

	nNum = SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_GETCOUNT, 0, 0);
	if (nNum) {
		pnTracks = (int*) malloc(nNum*sizeof(int));
		for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
			SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_GETTEXT, nLoop, (LONG)szTmp);
			sscanf(szTmp, "%d.\t", &pnTracks[nLoop]);
			pnTracks[nLoop] --;
		}

        for (nLoop = 0 ; nLoop < nNum ; nLoop ++) 
            nSecs += pnTrackLen[pnTracks[nLoop]];
	}

    // Calc time!

    nMin = nSecs / 60;
    nSecs %= 60;

    sprintf(szTmp, "(Play time: %02d:%02d)", nMin, nSecs);

    SetDlgItemText(hWnd, IDC_PLAYTIME, szTmp);

    if (pnTracks)
        free(pnTracks);
}


///////////////////////////////////////////////////////////////////////////
//
// InfoDlg
//
///////////////////////////////////////////////////////////////////////////

#define NUM_TABS 3

struct {
	char* pzTabName;
	int nDialogTemplate;
	HWND hTabWnd;
} asInfoTabs[NUM_TABS] = {
	{"Playlist", IDD_INFO_TAB_PLAYLIST, NULL},
	{"More CD info", IDD_INFO_TAB_DISCINFO, NULL},
	{"More track info", IDD_INFO_TAB_TRACKINFO, NULL},
};

int nCurrInfoTab;
HWND hInfoTabCtrl;
HWND hInfoDlg;

BOOL CALLBACK InfoTabDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam);

void InfoChangeTab()
{
	ShowWindow(asInfoTabs[nCurrInfoTab].hTabWnd, SW_HIDE);

	nCurrInfoTab = TabCtrl_GetCurSel(hInfoTabCtrl);

	ShowWindow(asInfoTabs[nCurrInfoTab].hTabWnd, SW_SHOW);
}


BOOL InfoDlgNotifyHandler(HWND hWnd, UINT /*nMsg*/, WPARAM wParam, LPARAM lParam)
{
    switch(wParam) {
        case IDC_TAB: {
			NMHDR* psNM = (NMHDR*) lParam;
            
            switch(psNM->code) {
                case TCN_SELCHANGE: {
					InfoChangeTab();

                    if (nCurrInfoTab == 1) {
                        EnableWindow(GetDlgItem(hWnd, IDC_SETTRACK), FALSE);
                        EnableWindow(GetDlgItem(hWnd, IDC_TRACK), FALSE);
                    }
                    else {
                        EnableWindow(GetDlgItem(hWnd, IDC_SETTRACK), TRUE);
                        EnableWindow(GetDlgItem(hWnd, IDC_TRACK), TRUE);
                    }
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}


BOOL CALLBACK InfoTabDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    static char zInfoID[10];

    switch(nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
            strcpy(zInfoID, zCurrCDDBID);

            bInInfoDlg = TRUE;

            CenterWindow(hWnd);
        }
		break;

        case WM_COMMAND: {
            if (HIWORD(wParam) == LBN_DBLCLK) {
                switch(LOWORD(wParam)) {
					case IDC_TRACKS:
					case IDC_TRACKS2: {
                        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDC_ADD, BN_CLICKED), 0L);
                    }
                    break;

                    case IDC_PLAYLIST: {
                        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDC_REMOVE, BN_CLICKED), 0L);
                    }
                    break;
                }
            }
            else if (HIWORD(wParam) == EN_KILLFOCUS) {
                switch(LOWORD(wParam)) {
					case IDC_TRACKINFO: {
						int nSel;

						nSel = SendDlgItemMessage(hWnd, IDC_TRACKS2, LB_GETCURSEL, 0, 0);
						if (nSel != LB_ERR) {
							int nLen;

							nLen = GetWindowTextLength(GetDlgItem(hWnd, IDC_TRACKINFO));

							free(ppzTracksExt[nSel]);

							ppzTracksExt[nSel] = (char*) malloc(nLen + 1);
							GetWindowText(GetDlgItem(hWnd, IDC_TRACKINFO), ppzTracksExt[nSel], nLen + 1);
						}
					}
					break;
				}
			}
            else if (HIWORD(wParam) == LBN_SELCHANGE) {
                switch(LOWORD(wParam)) {				    
                    case IDC_TRACKS: {
						if (nCurrInfoTab == 0) {
							int* pnTracks = NULL;
							int* pnSel = NULL;
							int nLoop;
							char szTmp[300];
							int nNum;
							int nSecs = 0;
							int nMin;

							// Get playlist

							nNum = SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELCOUNT, 0, 0);
							if (nNum) {
								pnSel = (int*) malloc(nNum*sizeof(int));
    							SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELITEMS, nNum, (LPARAM)pnSel);

								pnTracks = (int*) malloc(nNum*sizeof(int));
								for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
        							SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETTEXT, pnSel[nLoop], (LPARAM)szTmp);
									sscanf(szTmp, "%d.\t", &pnTracks[nLoop]);
									pnTracks[nLoop] --;
								}

								for (nLoop = 0 ; nLoop < nNum ; nLoop ++) 
									nSecs += pnTrackLen[pnTracks[nLoop]];
							}

							// Calc time!

							nMin = nSecs / 60;
							nSecs %= 60;

							sprintf(szTmp, "(Selected time: %02d:%02d)", nMin, nSecs);

							SetDlgItemText(hWnd, IDC_SELTIME, szTmp);

							if (pnTracks)
								free(pnTracks);
							if (pnSel)
								free(pnSel);

							int nCurrSel;

							nCurrSel = SendMessage((HWND) lParam, LB_GETCURSEL, 0, 0);
							if (nCurrSel != LB_ERR) {
								SetWindowText(GetDlgItem(hInfoDlg, IDC_TRACK), ppzTracks[nCurrSel]);
								SendMessage(GetDlgItem(hInfoDlg, IDC_TRACK), EM_SETSEL, 0, -1);
							}
						}
					}
					break;

                    case IDC_TRACKS2: {
						if (nCurrInfoTab == 2) {
							int nCurrSel;

							nCurrSel = SendMessage((HWND) lParam, LB_GETCURSEL, 0, 0);
                            if (nCurrSel != LB_ERR) {
								SetWindowText(GetDlgItem(hWnd, IDC_TRACKINFO), ppzTracksExt[nCurrSel]);
								
                                SetWindowText(GetDlgItem(hInfoDlg, IDC_TRACK), ppzTracks[nCurrSel]);
								SendMessage(GetDlgItem(hInfoDlg, IDC_TRACK), EM_SETSEL, 0, -1);
                            }
						}
                    }
				    break;
                }
            }
            else if (HIWORD(wParam) == BN_CLICKED) {
                // This is because the tab is it's own dialog and when I click the buttom it seems to
                // decide to set it as the default button for the tab dialog...

                ChangeDefButton(hWnd, IDOK, LOWORD(wParam));

                switch(LOWORD(wParam)) {
				    case IDC_ADD: {
                        int nSelCount;

					    nSelCount = SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELCOUNT, 0, 0);
					    if (nSelCount) {
						    int* pnSel = new int[nSelCount];
						    int nLoop;
						    char szTmp[300];

						    SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELITEMS, nSelCount, (LPARAM) pnSel);
						    for (nLoop = 0 ; nLoop < nSelCount ; nLoop ++) {			
							    sprintf(szTmp, "%d.\t%s", pnSel[nLoop] + 1, ppzTracks[pnSel[nLoop]]);
							    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_ADDSTRING, 0, (LONG)szTmp);
						    }

						    delete [] pnSel;
					    }

                        UpdatePlaylistTime(hWnd);
				    }
				    break;

				    case IDC_INSERT: {
                        int nCurSel;
                        int nSelCount;

                        nCurSel = SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_GETCURSEL, 0, 0);

					    nSelCount = SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELCOUNT, 0, 0);
					    if (nSelCount) {
						    int* pnSel = new int[nSelCount];
						    int nLoop;
						    char szTmp[300];

						    SendMessage(GetDlgItem(hWnd, IDC_TRACKS), LB_GETSELITEMS, nSelCount, (LPARAM) pnSel);
						    for (nLoop = 0 ; nLoop < nSelCount ; nLoop ++) {			
							    sprintf(szTmp, "%d.\t%s", pnSel[nLoop] + 1, ppzTracks[pnSel[nLoop]]);
							    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_INSERTSTRING, nCurSel+nLoop, (LONG)szTmp);
						    }

						    delete [] pnSel;
					    }

                        UpdatePlaylistTime(hWnd);
				    }
				    break;

				    case IDC_REMOVE: {
                        int nSelCount;

					    nSelCount = SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_GETSELCOUNT, 0, 0);
					    while (nSelCount) {
						    int anSel[1];

						    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_GETSELITEMS, 1, (LPARAM) anSel);
						    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_DELETESTRING, anSel[0], 0);

						    nSelCount --;
					    }

                        UpdatePlaylistTime(hWnd);
				    }
				    break;

				    case IDC_CLEAR: {
					    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_RESETCONTENT, 0, 0);

                        UpdatePlaylistTime(hWnd);
				    }
				    break;

				    case IDC_RESET: {
					    int nLoop;
					    char szTmp[300];

					    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_RESETCONTENT, 0, 0);

					    for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++) {
						    sprintf(szTmp, "%d.\t%s", nLoop + 1, ppzTracks[nLoop]);
						    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_ADDSTRING, 0, (LPARAM) szTmp);
					    }

                        UpdatePlaylistTime(hWnd);
				    }
				    break;

                    case IDC_RANDOM: {
                        int* pnRandomTracks = new int[nMaxTrack];
                        int nLoop, nLoop2;
                        int nCount = 0;
                        BOOL bUnique;
                        char zTmp[300];
                        
                        for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++) {
                            bUnique = FALSE;

                            while (!bUnique) {
                                pnRandomTracks[nCount] = rand() % nMaxTrack;

                                bUnique = TRUE;

                                for (nLoop2 = 0 ; nLoop2 < nCount ; nLoop2 ++){
                                    if (pnRandomTracks[nCount] == pnRandomTracks[nLoop2])
                                        bUnique = FALSE;
                                }
                            }

						    sprintf(zTmp, "%d.\t%s", pnRandomTracks[nCount]+1, ppzTracks[pnRandomTracks[nCount]]);
						    SendMessage(GetDlgItem(hWnd, IDC_PLAYLIST), LB_ADDSTRING, 0, (LPARAM) zTmp);

                            nCount ++;
                        }

                        UpdatePlaylistTime(hWnd);
                    }
                    break;

                }
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


void InitInfoDlg(HWND hWnd, BOOL bAddTabs)
{
	TC_ITEM sTabItem;
	int nLoop;
	RECT sRect;
	int nInfoX;
	int nInfoY;

    if (bAddTabs) {
        // Init tabs

	    hInfoTabCtrl = GetDlgItem(hWnd, IDC_TAB);

	    for (nLoop = 0 ; nLoop < NUM_TABS ; nLoop ++) {
		    sTabItem.mask = TCIF_TEXT; 
		    sTabItem.pszText = asInfoTabs[nLoop].pzTabName; 
		    TabCtrl_InsertItem(hInfoTabCtrl, nLoop, &sTabItem); 
	    }
	    
	    GetClientRect(hInfoTabCtrl, &sRect);

	    TabCtrl_AdjustRect(hInfoTabCtrl, FALSE, &sRect); 

	    nInfoX = sRect.left;
	    nInfoY = sRect.top;

	    for (nLoop = 0 ; nLoop < NUM_TABS ; nLoop ++) {
		    nCurrInfoTab = nLoop;

		    asInfoTabs[nLoop].hTabWnd = CreateDialog(hMainInstance, MAKEINTRESOURCE(asInfoTabs[nLoop].nDialogTemplate), hInfoTabCtrl, InfoTabDlgProc);
		    GetClientRect(asInfoTabs[nLoop].hTabWnd, &sRect);

		    MoveWindow(asInfoTabs[nLoop].hTabWnd, nInfoX, nInfoY, sRect.right, sRect.bottom, FALSE);	
	    }

	    nCurrInfoTab = 0;

	    InfoChangeTab();
    }

	char zID[32];
	char zTmp[80];
    int nTabs[2] = {15, 20};

    SendDlgItemMessage(asInfoTabs[0].hTabWnd, IDC_TRACKS, LB_SETTABSTOPS, 1, (LPARAM) nTabs);
    SendDlgItemMessage(asInfoTabs[0].hTabWnd, IDC_TRACKS, LB_RESETCONTENT, 0, 0);

    SendDlgItemMessage(asInfoTabs[2].hTabWnd, IDC_TRACKS2, LB_SETTABSTOPS, 1, (LPARAM) nTabs);
    SendDlgItemMessage(asInfoTabs[2].hTabWnd, IDC_TRACKS2, LB_RESETCONTENT, 0, 0);

    SendDlgItemMessage(asInfoTabs[0].hTabWnd, IDC_PLAYLIST, LB_SETTABSTOPS, 1, (LPARAM) nTabs);
    SendDlgItemMessage(asInfoTabs[0].hTabWnd, IDC_PLAYLIST, LB_RESETCONTENT, 0, 0);

    DBGetAlias(zCurrMCIID, zID);

	sprintf(zTmp, "ID: %s", zCurrCDDBID);
	SetWindowText(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_CURRID), zTmp);
	if (stricmp(zCurrCDDBID, zID)) {
		sprintf(zTmp, "Alias: %s", zID);
		SetWindowText(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_ALIASID), zTmp);
	}
	else
		SetWindowText(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_ALIASID), "");   

    if (bInit && bMediaPresent) {
		char szTmp[300];
		int nLoop;
        int nWidth;
        SIZE sSize;
        HDC hDC;

        if (nCurrInfoTab == 0) {
            nWidth = 0;
            
            hDC = GetDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST));

            for (nLoop = 0 ; nLoop < nProgrammedTracks; nLoop ++) {
				sprintf(szTmp, "%d.\t%s", pnProgrammedTracks[nLoop]+1, ppzTracks[pnProgrammedTracks[nLoop]]);
				SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_ADDSTRING, 0, (LPARAM) szTmp);

                GetTextExtentPoint32(hDC, szTmp, strlen(szTmp), &sSize);

                nWidth = max(nWidth, sSize.cx);
			}

            SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_SETHORIZONTALEXTENT, nWidth, 0);

            ReleaseDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), hDC);
        }

        nWidth = 0;
        
        hDC = GetDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS));

		for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++) {
			sprintf(szTmp, "%d.\t%s", nLoop + 1, ppzTracks[nLoop]);
			SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_ADDSTRING, 0, (LPARAM) szTmp);
			SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_ADDSTRING, 0, (LPARAM) szTmp);

            GetTextExtentPoint32(hDC, szTmp, strlen(szTmp), &sSize);

            nWidth = max(nWidth, sSize.cx);
		}

        ReleaseDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), hDC);

        SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_SETHORIZONTALEXTENT, nWidth, 0);
        SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_SETHORIZONTALEXTENT, nWidth, 0);

		SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_SETCURSEL, 0, 0);
		SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_SETCURSEL, 0, 0);
    }
    
    SendDlgItemMessage(asInfoTabs[0].hTabWnd, IDC_TRACKS, LB_SETCURSEL, 0, -1);
    SendDlgItemMessage(asInfoTabs[2].hTabWnd, IDC_TRACKS2, LB_SETCURSEL, 0, -1);

    UpdatePlaylistTime(asInfoTabs[0].hTabWnd);

    SetWindowText(GetDlgItem(asInfoTabs[1].hTabWnd, IDC_DISCINFO), pzDiscExt);

    SetWindowText(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKINFO), ppzTracksExt[0]);

    if (!bInit || !bMediaPresent) {
		SetWindowText(GetDlgItem(hWnd, IDC_ARTIST), "No audio CD");
		SetWindowText(GetDlgItem(hWnd, IDC_TITLE), "No audio CD");
	}
	else {
		char* pzTmp;
		
        pzTmp = NULL;

		SetWindowText(GetDlgItem(hWnd, IDC_ARTIST), pzArtist);
		SetWindowText(GetDlgItem(hWnd, IDC_TITLE), pzTitle);
		
		SetActiveWindow(GetDlgItem(hWnd, IDC_ARTIST));

		SetWindowText(GetDlgItem(hWnd, IDC_TRACK), ppzTracks[0]);

		// Set category
		DBGetInfo(zCurrCDDBID, zCurrMCIID, "CATEGORY", &pzTmp);
		SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_SELECTSTRING, (WPARAM) -1, (LPARAM)pzTmp);

        free(pzTmp);
	}
}

BOOL SetDiscInfoFromInfoDlg(HWND hWnd)
{
    int nLoop;
    char szTmp[1024];
	int nNum;
    int nLen;
	int nSel;

    free(pzArtist);
    nLen = GetWindowTextLength(GetDlgItem(hWnd, IDC_ARTIST));
    pzArtist = (char*) malloc((nLen+1)*sizeof(char));
    GetWindowText(GetDlgItem(hWnd, IDC_ARTIST), pzArtist, nLen+1);

    free(pzTitle);
    nLen = GetWindowTextLength(GetDlgItem(hWnd, IDC_TITLE));
    pzTitle = (char*) malloc((nLen+1)*sizeof(char));
    GetWindowText(GetDlgItem(hWnd, IDC_TITLE), pzTitle, nLen+1);
    
    free(pzDiscExt);
    nLen = GetWindowTextLength(GetDlgItem(asInfoTabs[1].hTabWnd, IDC_DISCINFO));
    pzDiscExt = (char*) malloc(nLen + 1);
    GetWindowText(GetDlgItem(asInfoTabs[1].hTabWnd, IDC_DISCINFO), pzDiscExt, nLen + 1);

	nSel = SendDlgItemMessage(asInfoTabs[2].hTabWnd, IDC_TRACKS2, LB_GETCURSEL, 0, 0);
	if (nSel != LB_ERR) {
		int nLen;

		nLen = GetWindowTextLength(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKINFO));

		free(ppzTracksExt[nSel]);

		ppzTracksExt[nSel] = (char*) malloc(nLen + 1);
		GetWindowText(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKINFO), ppzTracksExt[nSel], nLen + 1);
	}

    // Get category
    if (SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_GETLBTEXT, SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_GETCURSEL, 0, 0), (LPARAM)szTmp) != CB_ERR) {
        // Category has changed. Delete old entry in case we use CDDB
        if (stricmp(szTmp, pzCategory) && (nOptions & OPTIONS_USECDDB))
            DBDelete(zCurrCDDBID, zCurrMCIID);

        free(pzCategory);
		pzCategory = (char*) malloc((strlen(szTmp)+1)*sizeof(char));
		strcpy(pzCategory, szTmp);
	}

	// Get playlist

	free(pnProgrammedTracks);

    nNum = SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_GETCOUNT, 0, 0);
	if (!nNum) {
		nProgrammedTracks = nMaxTrack;

		pnProgrammedTracks = (int*) malloc(nProgrammedTracks*sizeof(int));
		for (nLoop = 0 ; nLoop < nProgrammedTracks ; nLoop ++)
			pnProgrammedTracks[nLoop] = nLoop;
	}
	else {
		nProgrammedTracks = nNum;

		pnProgrammedTracks = (int*) malloc(nProgrammedTracks*sizeof(int));
		for (nLoop = 0 ; nLoop < nNum ; nLoop ++) {
			SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_GETTEXT, nLoop, (LONG)szTmp);
			sscanf(szTmp, "%d.\t", &pnProgrammedTracks[nLoop]);
			pnProgrammedTracks[nLoop] --;
		}
	}
	
    return SetDiscInfo();
}


void ExitInfoDlg()
{
    int nLoop;

    for (nLoop = 0 ; nLoop < NUM_TABS ; nLoop ++)
		DestroyWindow(asInfoTabs[nLoop].hTabWnd);
}


BOOL CALLBACK InfoDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    static char zInfoID[10];

    switch(nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
            int nLoop;

            hInfoDlg = hWnd;

			strcpy(zInfoID, zCurrCDDBID);

            bInInfoDlg = TRUE;

            CenterWindow(hWnd);

            // Fill category list
            SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_ADDSTRING, 0, (LPARAM)" ");
            for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++)
                SendMessage(GetDlgItem(hWnd, IDC_CATEGORY), CB_ADDSTRING, 0, (LPARAM)ppzCategories[nLoop]);

            // Fix ID's 

            InitInfoDlg(hWnd, TRUE);
        }
		break;

        case WM_NOTIFY: {
            return InfoDlgNotifyHandler(hWnd, nMsg, wParam, lParam);
        }
        break;

        case WM_COMMAND: {
            if (HIWORD(wParam) == EN_SETFOCUS || HIWORD(wParam) == EN_KILLFOCUS) {
                if (LOWORD(wParam) == IDC_TRACK) {
					if (HIWORD(wParam) == EN_SETFOCUS)
						ChangeDefButton(hWnd, IDC_SETTRACK, IDOK);
					else
						ChangeDefButton(hWnd, IDOK, IDC_SETTRACK);
				}
            }
            else if (HIWORD(wParam) == BN_CLICKED) {
                switch(LOWORD(wParam)) {
                    case IDC_SETTRACK: {
                        int nCurSel;
                        char szTmp[300];
                        int nID;
                        HWND hTab;

                        if (nCurrInfoTab == 0) {
                            nID = IDC_TRACKS;
                            hTab = asInfoTabs[nCurrInfoTab].hTabWnd;
                        }
                        else if (nCurrInfoTab == 2) {
                            nID = IDC_TRACKS2;
                            hTab = asInfoTabs[nCurrInfoTab].hTabWnd;
                        }
                        else
                            break;

                        nCurSel = SendDlgItemMessage(hTab, nID, LB_GETCURSEL, 0, 0);
                        if (nCurSel != LB_ERR) {
						    int nLoop;
                            SIZE sSize;
                            HDC hDC;
                            int nWidth;

                            free(ppzTracks[nCurSel]);
                            ppzTracks[nCurSel] = (char*) malloc((GetWindowTextLength(GetDlgItem(hWnd, IDC_TRACK))+1)*sizeof(char));
                            GetWindowText(GetDlgItem(hWnd, IDC_TRACK), ppzTracks[nCurSel], GetWindowTextLength(GetDlgItem(hWnd, IDC_TRACK))+1);

                            SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_DELETESTRING, nCurSel, 0);
                            SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_DELETESTRING, nCurSel, 0);
                            sprintf(szTmp, "%d.\t%s", nCurSel+1, ppzTracks[nCurSel]);
                            SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_INSERTSTRING, nCurSel, (LPARAM) szTmp);
                            SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_INSERTSTRING, nCurSel, (LPARAM) szTmp);

                            hDC = GetDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS));
                            nWidth = SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_GETHORIZONTALEXTENT, 0, 0);
                            GetTextExtentPoint32(hDC, szTmp, strlen(szTmp), &sSize);
                            nWidth = max(nWidth, sSize.cx);
                            SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), LB_SETHORIZONTALEXTENT, nWidth, 0);
                            SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_SETHORIZONTALEXTENT, nWidth, 0);
                            SendMessage(GetDlgItem(asInfoTabs[2].hTabWnd, IDC_TRACKS2), LB_SETHORIZONTALEXTENT, nWidth, 0);
                            ReleaseDC(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_TRACKS), hDC);

						    for (nLoop = 0 ; nLoop < nProgrammedTracks ; nLoop ++) {
							    if (pnProgrammedTracks[nLoop] == nCurSel) {
								    SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_DELETESTRING, nLoop, 0);
								    sprintf(szTmp, "%d.\t%s", nLoop+1, ppzTracks[nCurSel]);
								    SendMessage(GetDlgItem(asInfoTabs[0].hTabWnd, IDC_PLAYLIST), LB_INSERTSTRING, nCurSel, (LPARAM) szTmp);
							    }
						    }

						    if (nCurSel < (nMaxTrack-1))
							    nCurSel ++;
						    else
							    nCurSel = 0;

                            if (nID == IDC_TRACKS)
                                SendMessage(GetDlgItem(hTab, nID), LB_SETSEL, TRUE, nCurSel);

                            SetWindowText(GetDlgItem(hWnd, IDC_TRACK), ppzTracks[nCurSel]);
						    SetActiveWindow(GetDlgItem(hWnd, IDC_TRACK));
					    
						    SendMessage(GetDlgItem(hWnd, IDC_TRACK), EM_SETSEL, 0, -1);
                        }
                    }
                    break;

				    case IDC_INTERNET: {
                        POINT sPoint;
                        HMENU hMenu = LoadMenu(hMainInstance, MAKEINTRESOURCE(IDR_INTERNET));
                        HMENU hPopup = GetSubMenu(hMenu, 0);

                        GetCursorPos(&sPoint);

                        TrackPopupMenu(hPopup, TPM_RIGHTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, sPoint.x, sPoint.y, 0, hWnd, NULL);

                        DestroyMenu(hMenu);                           
                    }
				    break;

                    case IDM_INTERNETGET: {
                        if (DBInternetGet(hWnd)) {
                            GetDiscInfo();

                            InitInfoDlg(hWnd, FALSE);
                        }
                        else
                            MessageBox(hWnd, "No disc info found on remote server", APPNAME, MB_OK);
                    }
                    break;

                    case IDM_INTERNETSEND: {
                        if (SetDiscInfoFromInfoDlg(hWnd)) 
                            DBInternetSend(hWnd);
                    }
                    break;

                    case IDOK: {
                        char zTmpID1[10];
                        char zTmpID2[10];

                        DebugPrintf("-> InfoDlg SAVE, make sure we still have the same disc inserted");

                        DBGetDiscID(zTmpID1, zTmpID2, FALSE, NULL, NULL);

                        if (strcmp(zTmpID1, zInfoID)) {
						    MessageBox(NULL, "The CD has been changed! No CD Info saved!", APPNAME, MB_OK);

                            bInInfoDlg = FALSE;

						    EndDialog(hWnd, TRUE);

                            break;
					    }

                        if (SetDiscInfoFromInfoDlg(hWnd)) {
                            CheckProgrammed();

//					        UpdateToolTip();

					        InitMenu();

                            bInInfoDlg = FALSE;

					        ExitInfoDlg();

                            EndDialog(hWnd, TRUE);
                        }

                        DebugPrintf("<- InfoDlg SAVE");
                    }
                    break;

                    case IDCANCEL: {
                        bInInfoDlg = FALSE;

                        ExitInfoDlg();

                        EndDialog(hWnd, FALSE);
                    }
                    break;
                }
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// SET ABSOLUTE TRACK POSITION
//
/////////////////////////////////////////////////////////////////////

BOOL CALLBACK SetAbsTrackPosDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch(nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
            bInSetAbsTrackPosDlg = TRUE;

			CenterWindow(hWnd);

            UpdateTrackCombo(hWnd, IDC_TRACK);

            SendDlgItemMessage(hWnd, IDC_TRACK, CB_SETCURSEL, nCurrTrack, 0);

            SetDlgItemInt(hWnd, IDC_MIN, 0, FALSE);

            SetDlgItemInt(hWnd, IDC_SEC, 0, FALSE);
        }
		break;

        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case IDOK: {
					int nTrack = SendDlgItemMessage(hWnd, IDC_TRACK, CB_GETCURSEL, 0, 0);
					int nMin = GetDlgItemInt(hWnd, IDC_MIN, NULL, FALSE);
					int nSec = GetDlgItemInt(hWnd, IDC_SEC, NULL, FALSE);

                    if (nMin * 60 + nSec <= pnTrackLen[pnProgrammedTracks[nTrack]])
                        CDPlayPos(nTrack, nMin, nSec);
                    else
                        MessageBox(hWnd, "Invalid time", APPNAME, MB_OK);

                    bInSetAbsTrackPosDlg = FALSE;
                }
                break;

                case IDCANCEL: {
                    bInSetAbsTrackPosDlg = FALSE;

                    EndDialog(hWnd, FALSE);
                }
                break;
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// SKIP
//
/////////////////////////////////////////////////////////////////////

BOOL bInDrag = FALSE;
BOOL bInDrop = FALSE;
char zSkipDlgID[32];

void UpdateSlider(HWND hWnd)
{
	int nTrack;
    int nMin;
    int nSec;
    int nFrame;
    char zTmp[32];
    
    if (!bProgrammed && !bRandomize)
        nCurrTrack = CDGetCurrTrack() - 1;

    if (nCurrTrack != -1) {
        SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, pnTrackLen[pnProgrammedTracks[nCurrTrack]]));
        SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_SETPAGESIZE, 0, 10);
        SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_SETTICFREQ, 30, 0);
    
        SendDlgItemMessage(hWnd, IDC_SPIN, UDM_SETRANGE, 0, MAKELONG(0, pnTrackLen[pnProgrammedTracks[nCurrTrack]]));

        if (!bInDrop)
            SendDlgItemMessage(hWnd, IDC_TRACK, CB_SETCURSEL, nCurrTrack, 0);

        sprintf(zTmp, "%02d:%02d", pnTrackLen[pnProgrammedTracks[nCurrTrack]] / 60, pnTrackLen[pnProgrammedTracks[nCurrTrack]] % 60);

        SendDlgItemMessage(hWnd, IDC_LEN, WM_SETTEXT, 0, (LPARAM) zTmp);

        if (!bInDrag) {
            CDGetTime(0, TRUE, &nTrack, &nMin, &nSec, &nFrame);

            sprintf(zTmp, "%02d:%02d", nMin, nSec);

            SendDlgItemMessage(hWnd, IDC_TIME, WM_SETTEXT, 0, (LPARAM) zTmp);

            nSec += nMin * 60;
  
            SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_SETPOS, TRUE, nSec);
            SendDlgItemMessage(hWnd, IDC_SPIN, UDM_SETPOS, 0, MAKELONG(pnTrackLen[pnProgrammedTracks[nCurrTrack]] - nSec, 0));
        }
    }
    else {
        SendDlgItemMessage(hWnd, IDC_TRACK, CB_SELECTSTRING, (WPARAM)-1, (LPARAM) "No CD");
        SendDlgItemMessage(hWnd, IDC_TIME, WM_SETTEXT, 0, (LPARAM) "No CD");
    }

    if (strcmp(zCurrCDDBID, zSkipDlgID)) {
        UpdateTrackCombo(hWnd, IDC_TRACK);

        strcpy(zSkipDlgID, zCurrCDDBID);
    }
}


BOOL CALLBACK SkipDlgProc(
    HWND  hWnd,
    UINT  nMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch(nMsg) {
        case WM_HELP: {
            DoHelp(hWnd, (LPHELPINFO)lParam);
        }
        break;

    	case WM_INITDIALOG: {
            CenterWindow(hWnd);

            strcpy(zSkipDlgID, zCurrCDDBID);

            UpdateTrackCombo(hWnd, IDC_TRACK);

            UpdateSlider(hWnd);

            bInSkipDlg = TRUE;

            SetTimer(hWnd, 1, 1000, NULL);
        }
		break;

        case WM_TIMER: {
            switch(wParam) {
                case 1:
                    UpdateSlider(hWnd);
                break;
            }
        }
        break;

        case WM_HSCROLL: {
            int nPos = HIWORD(wParam);

            if ((HWND) lParam == GetDlgItem(hWnd, IDC_SLIDER)) {
                switch(LOWORD(wParam)) {
                    case SB_THUMBTRACK: {
                        char zTmp[32];

                        bInDrag = TRUE;

                        sprintf(zTmp, "%02d:%02d", nPos / 60, nPos % 60);

                        SendDlgItemMessage(hWnd, IDC_TIME, WM_SETTEXT, 0, (LPARAM) zTmp);
                    }
                    break;
                    
                    case SB_THUMBPOSITION: {
                        CDPlayPos(nCurrTrack, nPos / 60, nPos % 60);

                        bInDrag = FALSE;
                    }
                    break;

                    case SB_TOP:
                    case SB_BOTTOM:
                    case SB_LINEUP:
                    case SB_LINEDOWN:
                    case SB_PAGEDOWN: 
                    case SB_PAGEUP: {
                        char zTmp[32];

                        bInDrag = TRUE;

                        nPos = SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_GETPOS, 0, 0);

                        sprintf(zTmp, "%02d:%02d", nPos / 60, nPos % 60);

                        SendDlgItemMessage(hWnd, IDC_TIME, WM_SETTEXT, 0, (LPARAM) zTmp);
                    }
                    break;

                    case SB_ENDSCROLL:
                        nPos = SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_GETPOS, 0, 0);
                        CDPlayPos(nCurrTrack, nPos / 60, nPos % 60);
                        bInDrag = FALSE;
                    break;
                }
            }
            else if ((HWND) lParam == GetDlgItem(hWnd, IDC_SPIN)) {
                switch(LOWORD(wParam)) {
                    case SB_ENDSCROLL: {
                        CDPlayPos(nCurrTrack, (pnTrackLen[pnProgrammedTracks[nCurrTrack]] - nPos) / 60, (pnTrackLen[pnProgrammedTracks[nCurrTrack]] - nPos) % 60);
                        bInDrag = FALSE;
                    }
                    break;

                    case SB_THUMBPOSITION: {
                        char zTmp[32];

                        bInDrag = TRUE;

                        SendDlgItemMessage(hWnd, IDC_SLIDER, TBM_SETPOS, TRUE, pnTrackLen[pnProgrammedTracks[nCurrTrack]] - nPos + 1);
                        sprintf(zTmp, "%02d:%02d", (pnTrackLen[pnProgrammedTracks[nCurrTrack]] - nPos + 1) / 60, (pnTrackLen[pnProgrammedTracks[nCurrTrack]] - nPos + 1) % 60);
        
                        SendDlgItemMessage(hWnd, IDC_TIME, WM_SETTEXT, 0, (LPARAM) zTmp);
                    }
                    break;
                }
            }
        }
        break;

        case WM_COMMAND: {
            if ((HWND) lParam == GetDlgItem(hWnd, IDC_TRACK)) {
                if (HIWORD(wParam) == CBN_SELCHANGE && bInSkipDlg) {
                    int nSel;

                    nSel = SendDlgItemMessage(hWnd, IDC_TRACK, CB_GETCURSEL, 0, 0);
                    if (nSel != CB_ERR)
                        CDPlay(nSel);
                }
                else if (HIWORD(wParam) == CBN_DROPDOWN)
                    bInDrop = TRUE;
                else if (HIWORD(wParam) == CBN_CLOSEUP)
                    bInDrop = FALSE;
            }
            else if (HIWORD(wParam) == BN_CLICKED) {
                switch(LOWORD(wParam)) {
                    case IDOK: {
                        bInSkipDlg = FALSE;

					    EndDialog(hWnd, TRUE);
                    }
                    break;

                    case IDCANCEL: {
                        bInSkipDlg = FALSE;

                        EndDialog(hWnd, FALSE);
                    }
                    break;
                }
            }
        }
        break;

        default:
            return FALSE;
    }

    return TRUE;
}



/////////////////////////////////////////////////////////////////////
//
// MAIN WINDOW!
//
/////////////////////////////////////////////////////////////////////

LRESULT CALLBACK MainWindowProc(                       
    HWND hWnd,
    UINT nMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch(nMsg) {
		case WM_CREATE: {
			hTrayIcon = hIconStop;

			NotifyAdd(hWnd, 100, hTrayIcon, szToolTip);

			CDOpen();

			SetTimer(hWnd, 1, nPollTime*1000, NULL);
			SetTimer(hWnd, 3, 1000, NULL);
		}
		break;

		case WM_ENDSESSION: {
			if (wParam) {
				SaveConfig();

				DestroyWindow(hWnd);

				CDClose();
			}
        }
        break;

        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case MM_MCINOTIFY: {
            if (wParam == MCI_NOTIFY_SUCCESSFUL) {
                DebugPrintf("MCI success");

                if (bPlayWhole && !bRepeatTrack) {
                    nCurrTrack = nProgrammedTracks - 1;
                    ResetPlaylist(TRUE);
                }

                bPlaying = FALSE;

				UpdateDiscInformation(TRUE, TRUE, NULL);
                
                if (bRepeat && !bRepeatTrack && !bRandomize && (!bProgrammed || (bProgrammed && nNextProgrammedTrack == nProgrammedTracks))) {
					DebugPrintf("Playing from first track on the playlist");

					CDPlay(0);
				}
                else if (!bRandomize && !bRepeatTrack && bProgrammed && nNextProgrammedTrack < nProgrammedTracks) {
					DebugPrintf("Playing next track on the playlist");

                    CDPlay(nNextProgrammedTrack);
				}
                else if (bRepeatTrack) {
                    DebugPrintf("Repeating track");

					CDPlay(nRepeatTrack);
				}
				else if (bRandomize) {
					DebugPrintf("Playing random track");

					PlayRandTrack();
				}
                else
                    bPlaying = FALSE;

//                UpdateToolTip();
			}
            else if (wParam == MCI_NOTIFY_ABORTED) {
                DebugPrintf("MCI aborted");

                StatusCheck();
            }
            else if (wParam == MCI_NOTIFY_FAILURE) {
                DebugPrintf("MCI failure");

                StatusCheck();
            }
            else if (wParam == MCI_NOTIFY_SUPERSEDED) {
                DebugPrintf("MCI superseeded");

                StatusCheck();
            }

            if (!bPlaying && !bPaused)
                nCurrTrack = 0;
		}
		break;

        case WM_DISPLAYCHANGE: {
            DebugPrintf("WM_DISPLAYCHANGE");

            NotifyDelete(hMainWnd, 100);
            NotifyAdd(hMainWnd, 100, hTrayIcon, szToolTip);
        }
        break;

        case WM_DEVICECHANGE: {
			if ((nOptions & OPTIONS_NOINSERTNOTIFICATION) && bHasNotification)
				DebugPrintf("No insert notification enabled, skipping WM_DEVICECHANGE");
			else {
				switch(wParam) {
					case DBT_DEVICEARRIVAL: {
						DEV_BROADCAST_HDR* p = (DEV_BROADCAST_HDR*) lParam;
						DebugPrintf("Device Arrival: Type = %d", p->dbch_devicetype);

						if (p->dbch_devicetype == DBT_DEVTYP_VOLUME) {
							if (!bHasNotification) {
								bHasNotification = TRUE;

								DebugPrintf("Use of notification found!");
							}
							
							nCurrTrack = 0;

                            StatusCheck();
						}
						else
							DebugPrintf("Not a volume!");
					}
					break;

					case DBT_DEVICEREMOVECOMPLETE: {
						DEV_BROADCAST_HDR* p = (DEV_BROADCAST_HDR*) lParam;
						DebugPrintf("Device Remove Complete: Type = %d", p->dbch_devicetype);

						if (p->dbch_devicetype == DBT_DEVTYP_VOLUME) {
							if (!bHasNotification) {
								bHasNotification = TRUE;

								DebugPrintf("Use of notification found!");

								nOptions &= ~OPTIONS_NOINSERTNOTIFICATION;
							}

							StatusCheck();
						}
						else
							DebugPrintf("Not a volume!");
					}
					break;

//					default:
//				        DebugPrintf("WM_DEVICECHANGE: %x", wParam);
				}
            }
        }
        break;

        case WM_TIMER: {
            if (wParam == 1) {              
                if (bCreateWindowFinished && !bFirstStatusCheckDone) {
                    StatusCheck();

                    bFirstStatusCheckDone = TRUE;
                }
/*                if (!bMediaPresent || !bAudio || (!bPlaying))
                    StatusCheck();
                else*/ if (nOptions & OPTIONS_NOINSERTNOTIFICATION)
                    StatusCheck();

                // Check for correct icon
                if (bMediaPresent && bAudio && bPlaying && !bPaused && hTrayIcon != hIconPlay) {
                    hTrayIcon = hIconPlay;
                    
                    NotifyModify(hMainWnd, 100, hTrayIcon, szToolTip);
                }
                else if (bMediaPresent && bAudio && !bPlaying && !bPaused && hTrayIcon != hIconStop) {
                    hTrayIcon = hIconStop;
                    
                    NotifyModify(hMainWnd, 100, hTrayIcon, szToolTip);
                }
                else if (bMediaPresent && bAudio && !bPlaying && bPaused && hTrayIcon != hIconPause) {
                    hTrayIcon = hIconPause;
                    
                    NotifyModify(hMainWnd, 100, hTrayIcon, szToolTip);
                }
                else if ((!bMediaPresent || !bAudio) && hTrayIcon != hIconNoAudio) {
                    hTrayIcon = hIconNoAudio;
                    
                    NotifyModify(hMainWnd, 100, hTrayIcon, szToolTip);
                }
			}
            else if (wParam == 2) {
				if (bMediaPresent) {
					if (nClicks == nLeftButtonNext) {
						if (!bPaused)
							SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_NEXT, 0), 0L);
						else if (bPaused)
							SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_PAUSE, 0), 0L);
					}
					else if (nClicks == nLeftButtonPause) {
						SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_PAUSE, 0), 0L);
					}
					else if (nClicks == nLeftButtonPrev) {
						SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_PREV, 0), 0L);
					}
					else if (nClicks == nLeftButtonStop) {
						SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_STOP, 0), 0L);
                    }
				}

                nClicks = 0;
                KillTimer(hWnd, 2);
            }
            else if (wParam == 3) {
                if (nOptions & OPTIONS_SHOWONCAPTION) {
                    HWND hForegroundWnd = GetForegroundWindow();
                    char szName[80];
                    int nStyle;
                    int nExStyle;

                    if (!hForegroundWnd)
                        break;

                    // If we have changed the foreground window, 
                    // repaint the last foreground window caption
                    if (hForegroundWnd != hLastForegroundWindow && hLastForegroundWindow && IsWindow(hLastForegroundWindow)) {
/*                        HDC hDC = GetWindowDC(hLastForegroundWindow);
                        DrawCaption(hLastForegroundWindow, hDC, NULL, DC_TEXT);

                        ReleaseDC(hLastForegroundWindow, hDC);*/

                        char* pzText;
                        int nLen;

                        nLen = GetWindowTextLength(hLastForegroundWindow);
                        pzText = (char*)malloc(nLen + 1);

                        GetWindowText(hLastForegroundWindow, pzText, nLen + 1);
                        SetWindowText(hLastForegroundWindow, pzText);

                        free(pzText);
                    }

                    // Find out the actual parent of this foreground window 
                    // in case it is a popup window with a caption
                    if (GetParent(hForegroundWnd)) {
                        HWND hWnd;

                        hWnd = GetParent(hForegroundWnd);
                        while(hWnd) {
                            hForegroundWnd = hWnd;
                            hWnd = GetParent(hForegroundWnd);
                        }
                    }

                    // Check if we are supposed to draw this info

                    GetClassName(hForegroundWnd, szName, 80);
                    nStyle = GetWindowLong(hForegroundWnd, GWL_STYLE);
                    nExStyle = GetWindowLong(hForegroundWnd, GWL_EXSTYLE);

//                    DebugPrintf("ClassName %s", szName);

                    if ((nStyle & WS_CAPTION) == WS_CAPTION && 
                        !(nExStyle & WS_EX_TOOLWINDOW) &&
                        strcmp(szName, "#32770") && 
                        strcmp(szName, "Shell_TrayWnd")) {
                        char szTmp[512];
                        char szText[512];
                        HDC hDC;
                        RECT sRect;
                        SIZE sSize;
                        SIZE sSize2;
                        NONCLIENTMETRICS sMetrics;
                        HFONT hDrawFont;
                        int nLeft;
                        int nLeftSpace = 0;
                        RECT sDrawRect;
                        int nRightSpace = 0;

                        UpdateDiscInformation(FALSE, FALSE, szText);

                        hLastForegroundWindow = hForegroundWnd;

                        hDC = GetWindowDC(hForegroundWnd);

                        GetWindowText(hForegroundWnd, szTmp, 511);

                        sMetrics.cbSize = sizeof(sMetrics);

                        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &sMetrics, FALSE);

                        // First we need to calculate the actual length of the active window text

                        hDrawFont = CreateFontIndirect(&sMetrics.lfCaptionFont);

                        hDrawFont = (HFONT)SelectObject(hDC, hDrawFont);

                        GetTextExtentPoint(hDC, szTmp, strlen(szTmp), &sSize2);

                        hDrawFont = (HFONT)SelectObject(hDC, hDrawFont);

                        DeleteObject(hDrawFont);

                        // Now use the user defined font

                        hDrawFont = CreateFontIndirect(&sCaptionFont);

                        hDrawFont = (HFONT)SelectObject(hDC, hDrawFont);

                        GetWindowRect(hForegroundWnd, &sRect);

                        GetTextExtentPoint(hDC, szText, strlen(szText), &sSize);

                        nRightSpace += 10;
 
//#define DEBUG_SHOW

#ifdef DEBUG_SHOW
                        DebugPrintf("-----");
#endif
                        // Close button
                        if ((nStyle & WS_SYSMENU) == WS_SYSMENU) {
#ifdef DEBUG_SHOW
                            DebugPrintf("WS_SYSMENU");
#endif
                            nRightSpace += GetSystemMetrics(SM_CXSIZE);
                            nLeftSpace += GetSystemMetrics(SM_CXSIZE);
                        }
                        if ((nStyle & WS_MINIMIZEBOX) == WS_MINIMIZEBOX || (nStyle & WS_MAXIMIZEBOX) == WS_MAXIMIZEBOX) {
#ifdef DEBUG_SHOW
                            DebugPrintf("WS_MINIMIZEBOX or WS_MAXIMIZEBOX");
#endif
                            nRightSpace += 2*GetSystemMetrics(SM_CXSIZE);
                        }
                        if ((nStyle & WS_BORDER) == WS_BORDER && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
#ifdef DEBUG_SHOW
                            DebugPrintf("WS_BORDER");
#endif
                            nRightSpace += GetSystemMetrics(SM_CXBORDER);
                            nLeftSpace += GetSystemMetrics(SM_CXBORDER);
                        }
                        if ((nStyle & WS_THICKFRAME) == WS_THICKFRAME && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
#ifdef DEBUG_SHOW
                            DebugPrintf("WS_THICKFRAME");
#endif
                            nRightSpace += GetSystemMetrics(SM_CXFRAME);
                            nLeftSpace += GetSystemMetrics(SM_CXFRAME);
                        }
                        if ((nStyle & WS_DLGFRAME) == WS_DLGFRAME && !((nStyle & WS_MAXIMIZE) == WS_MAXIMIZE)) {
#ifdef DEBUG_SHOW
                            DebugPrintf("WS_DLGFRAME");
#endif
                            nRightSpace += GetSystemMetrics(SM_CXDLGFRAME);
                            nLeftSpace += GetSystemMetrics(SM_CXDLGFRAME);
                        }

                        nLeft = sRect.right - sRect.left - nRightSpace - sSize.cx;

                        if (!((nStyle & WS_DISABLED) == WS_DISABLED))
                            SetBkColor(hDC, GetSysColor(COLOR_ACTIVECAPTION));
                        else
                            SetBkColor(hDC, GetSysColor(COLOR_INACTIVECAPTION));

                        SetTextColor(hDC, nCaptionFontColor);

                        sDrawRect.top = GetSystemMetrics(SM_CYFRAME) + (GetSystemMetrics(SM_CYCAPTION) - sSize.cy) / 2 - 1;
                        sDrawRect.bottom = sDrawRect.top + sSize.cy;

                        if (sSize.cx < nLastCaptionLen && nLastCaptionLen) {
/*                            HDC hDC = GetWindowDC(hForegroundWnd);
                            DrawCaption(hForegroundWnd, hDC, NULL, DC_TEXT);

                            ReleaseDC(hForegroundWnd, hDC);*/
                            char* pzText;
                            int nLen;

                            nLen = GetWindowTextLength(hForegroundWnd);
                            pzText = (char*)malloc(nLen + 1);

                            GetWindowText(hForegroundWnd, pzText, nLen + 1);
                            SetWindowText(hForegroundWnd, pzText);

                            free(pzText);
                        }

                        nLastCaptionLen = sSize.cx;

                        if (nLeft > sSize2.cx + nLeftSpace + 30) {
                            sDrawRect.left = nLeft;
                            sDrawRect.right = sDrawRect.left + sSize.cx;
#ifdef DEBUG_SHOW
                            DebugPrintf("Text fits");
#endif
                        }
                        else {
                            sDrawRect.left = sSize2.cx + 30 + nLeftSpace;
                            sDrawRect.right = nLeft + sSize.cx;
#ifdef DEBUG_SHOW
                            DebugPrintf("Text too large");
#endif
                        }
                       
                        DrawText(hDC, szText, strlen(szText), &sDrawRect, DT_LEFT | DT_VCENTER | DT_NOPREFIX);

                        hDrawFont = (HFONT)SelectObject(hDC, hDrawFont);

                        DeleteObject(hDrawFont);

                        ReleaseDC(hForegroundWnd, hDC);
                        
                        hLastForegroundWindow = hForegroundWnd;
                    }
                    else {
//                        DebugPrintf("ShowOnCaption: No paint");

                        hLastForegroundWindow = hForegroundWnd;
                    }
                }
            }
        }
        break;

        case WM_DESTROY:
            if (nOptions & OPTIONS_STOPONEXIT)
	            CDStop();

            NotifyDelete(hMainWnd, 100);

            PostQuitMessage(1);
        break;

        case WM_COMMAND:
            if (LOWORD(wParam) >= IDM_TRACKS && LOWORD(wParam) < IDM_TRACKS + 100) {
                CDPlay(LOWORD(wParam) - IDM_TRACKS);

//				UpdateToolTip();
            }
            else if (LOWORD(wParam) >= IDM_DEVICES && LOWORD(wParam) <= IDM_DEVICES + 'Z') {
                CDStop();

                CDClose();

                nCurrentDevice = LOWORD(wParam) - IDM_DEVICES - 'A';

                DebugPrintf("Changing device to %c", (char)nCurrentDevice + 'A');

                // Modify devices menu

	            HMENU hDevicesMenu;
                hDevicesMenu = GetSubMenu(GetSubMenu(hTrackMenu, 0), nMenuIndexDevices);

                if (hDevicesMenu) {
                    char szTmp[80];

                    for (int nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
                        if (abDevices[nLoop - 'A']) {
                            sprintf(szTmp, "Drive %c:", nLoop);
                            if (nLoop - 'A' == nCurrentDevice)
                                ModifyMenu(hDevicesMenu, IDM_DEVICES+nLoop, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_DEVICES+nLoop, szTmp);
                            else
                                ModifyMenu(hDevicesMenu, IDM_DEVICES+nLoop, MF_BYCOMMAND | MF_STRING, IDM_DEVICES+nLoop, szTmp);
                        }
                    }
                }

                StatusCheck();

				DiscInit();
            }
            else {
                switch(LOWORD(wParam)) {
                    case IDM_PLAY:
                        // Call status check to determine CD change if another instance
                        // sent us this command and we haven't detected the CD change!

                        StatusCheck();

                        if (bPlayWhole)
                            ResetPlaylist(TRUE);

                        if (bMediaPresent && bAudio) {
							if (!bPaused)
 							    CDPlay(nCurrTrack);
							else
								CDResume();                         
						}

                        StatusCheck();

//                        UpdateToolTip();
                    break;

                    case IDM_PLAYWHOLE: {
                        // Reset playlist

                        ResetPlaylist(FALSE);

                        CDPlay(0);
                    }
                    break;

                    case IDM_STOP:
                        if (bMediaPresent && bAudio)
							CDStop();

//                        UpdateToolTip();
                    break;

                    case IDM_PAUSE:
						if (bMediaPresent && bAudio) {
							if (!bPaused && bPlaying)
								CDPause();
							else if (bPaused)
								CDResume();
							else if (!bPaused && !bPlaying)
								CDPlay(nCurrTrack);

//							UpdateToolTip();
						}
                    break;

                    case IDM_NEXT:
						if (bMediaPresent && bAudio) {
							if (bRandomize)
								PlayRandTrack();
							else {
								if (nCurrTrack < nProgrammedTracks - 1)
									nCurrTrack ++;
								else
									nCurrTrack = 0;
                
								if (!bPaused && bPlaying)
									CDPlay(nCurrTrack);
								else if (!bPaused && !bPlaying)
									CDPlay(0);
							}

//							UpdateToolTip();
						}
                    break;

                    case IDM_PREV: {
						if (bMediaPresent && bAudio) {
							if (bRandomize)
								PlayRandTrack();
							else {
                                int nTrack, nMin, nSec, nFrame;

                                if (!(nOptions & OPTIONS_PREVALWAYSPREV))
                                    CDGetTime(0, TRUE, &nTrack, &nMin, &nSec, &nFrame);

                                if ((nOptions & OPTIONS_PREVALWAYSPREV) || (nMin < 1 && nSec <= 5)) {
                                    if (nCurrTrack > 0)
									    nCurrTrack --;
								    else
									    nCurrTrack = nProgrammedTracks - 1;
                                }
                                
								if (!bPaused)
									CDPlay(nCurrTrack);
							}

//							UpdateToolTip();
						}
				    }
                    break;

                    case IDM_OPEN: {
                        if (!bCDOpened)
                            CDOpen();

                        CDEject();

                        SendMessage(hWnd, WM_TIMER, 1, 0);
						
//						UpdateToolTip();
					}
                    break;

                    case IDM_CLOSE: {
        				CDLoad();

                        SendMessage(hWnd, WM_TIMER, 1, 0);
						
//						UpdateToolTip();
					}
                    break;

					case IDM_REPEAT: {
						HMENU hSubMenu;

						hSubMenu = GetSubMenu(hTrackMenu, 0);

						if (!bRepeat) {
							ModifyMenu(hSubMenu, IDM_REPEAT, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_REPEAT, "Repeat");
							bRepeat = TRUE;
						}
						else {
							ModifyMenu(hSubMenu, IDM_REPEAT, MF_BYCOMMAND | MF_STRING, IDM_REPEAT, "Repeat");
							bRepeat = FALSE;
						}
					}
					break;

					case IDM_REPEATTRACK: {
						HMENU hSubMenu;

                        hSubMenu = GetSubMenu(hTrackMenu, 0);
						hSubMenu = GetSubMenu(hSubMenu, nMenuIndexOther);

						if (!bRepeatTrack) {
							ModifyMenu(hSubMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_REPEATTRACK, "Repeat track");
							bRepeatTrack = TRUE;

                            UpdateDiscInformation(FALSE, TRUE, NULL);

                            nRepeatTrack = nCurrTrack;
						}
						else {
							ModifyMenu(hSubMenu, IDM_REPEATTRACK, MF_BYCOMMAND | MF_STRING, IDM_REPEATTRACK, "Repeat track");
							bRepeatTrack = FALSE;
						}

                        CDResume();
					}
					break;

					case IDM_RANDOMIZE: {
						if (bMediaPresent && bAudio) {
						    HMENU hSubMenu;

						    hSubMenu = GetSubMenu(hTrackMenu, 0);

						    if (!bRandomize) {
							    ModifyMenu(hSubMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_STRING | MF_CHECKED, IDM_RANDOMIZE, "Random Play");
							    bRandomize = TRUE;

                                nLastRandomTrack = 0;

                                PlayRandTrack();

//							    UpdateToolTip();
						    }
						    else {
							    ModifyMenu(hSubMenu, IDM_RANDOMIZE, MF_BYCOMMAND | MF_STRING, IDM_RANDOMIZE, "Random Play");
							    bRandomize = FALSE;
						    }
                        }
					}
					break;

                    case IDM_SETABSTRACKPOS:
                        DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_SETABSTRACKPOS), NULL, (DLGPROC)SetAbsTrackPosDlgProc);
                    break;

                    case IDM_SKIP:
                        DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_SKIP), NULL, (DLGPROC)SkipDlgProc);
                    break;

                    case IDM_CDDB:
                        DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_CDDB), NULL, (DLGPROC)DatabaseDlgProc);
                    break;

                    case IDM_OPTIONS:
                        DoOptions();
                    break;

                    case IDM_ABOUT:
                        DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_ABOUT), NULL, (DLGPROC)AboutDlgProc);
                    break;

                    case IDM_INFO:
                        DialogBox(hMainInstance, MAKEINTRESOURCE(IDD_INFO), NULL, (DLGPROC)InfoDlgProc);
                    break;

                    case IDM_QUIT: {
						if (bProgrammed && bMediaPresent && bPlaying && !(nOptions & OPTIONS_STOPONEXIT)) {
							if (MessageBox(NULL, "The CD is programmed. It will stop after the last sequential track if You continue this action. Are You sure You want to quit?", APPNAME, MB_YESNO) == IDYES)
								DestroyWindow(hWnd);
						}
						else
							DestroyWindow(hWnd);
					}
                    break;
                }
            }
        break;

		case MYWM_WAKEUP: {
            DebugPrintf("Wakeup call!");
            switch(wParam) {
				case IDM_PLAY: {
                    StatusCheck();

                    if (lParam == -1 && bPlaying)
                        DebugPrintf("Ignoring wakeup call to start playing since we are already playing and no track was specified");
                    else {
                        if (bRandomize) {
							DebugPrintf("Skipping requested track since we are in repeat mode!");

							nCurrTrack = GetRandTrack();
						}
						else
							nCurrTrack = LOWORD(lParam);

                        DebugPrintf("Wakeup call to play track %d", nCurrTrack);

                        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_PLAY, 0), 0);

                        SendMessage(hWnd, WM_TIMER, 1, 0);
                    }
				}
				break;

				case IDM_DEVICES: {
                    DebugPrintf("Version is %d", HIWORD(lParam));
                    DebugPrintf("Wakeup call to change device to %d(%c)", LOWORD(lParam), (char)LOWORD(lParam));
                    if (LOWORD(lParam) - 'A' != nCurrentDevice) {
                        DebugPrintf("Changing...");
                        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_DEVICES+LOWORD(lParam), 0), 0);
                    }
                    else
                        DebugPrintf("No change!");
				}
				break;

				case IDM_PAUSE:
                    DebugPrintf("Wakeup call to PAUSE");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_PAUSE, 0), 0);
				break;

                case IDM_RANDOMIZE:
                    DebugPrintf("Wakeup call to RANDOMIZE");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_RANDOMIZE, 0), 0);
                break;

                case IDM_REPEAT:
                    DebugPrintf("Wakeup call to REPEAT");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_REPEAT, 0), 0);
                break;

				case IDM_STOP:
                    DebugPrintf("Wakeup call to STOP");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_STOP, 0), 0);
				break;

                case IDM_NEXT:
                    DebugPrintf("Wakeup call to NEXT");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_NEXT, 0), 0);
				break;

                case IDM_PREV:
                    DebugPrintf("Wakeup call to PREV");

					SendMessage(hWnd, WM_COMMAND, MAKELONG(IDM_PREV, 0), 0);
				break;
            }
		}
		break;

        case WM_MEASUREITEM: {
            MEASUREITEMSTRUCT* psI = (MEASUREITEMSTRUCT*) lParam;
            if (psI->itemID == 999) {
                psI->itemWidth = 15;

                return TRUE;
            }
        }
        break;

        case WM_DRAWITEM: {
            DRAWITEMSTRUCT* psI = (DRAWITEMSTRUCT*) lParam;
            if (psI->itemID == 999 && psI->itemAction == ODA_DRAWENTIRE) {
                HWND hMenuWnd = WindowFromDC(psI->hDC);
                RECT sRect;

                GetClientRect(hMenuWnd, &sRect);

                // TEMP

                HDC hDC = CreateCompatibleDC(psI->hDC);
                hMenuBitmap = (HBITMAP)SelectObject(hDC, hMenuBitmap);

                psI->rcItem.bottom = (sRect.bottom - sRect.top);
                BitBlt(psI->hDC, psI->rcItem.left, psI->rcItem.bottom - 330, 26, 330, hDC, 0, 0, SRCCOPY);
                RealizePalette(psI->hDC);

                if (psI->rcItem.bottom - 330 > 0) {
                    psI->rcItem.top = 0;
                    psI->rcItem.bottom = 330 - psI->rcItem.bottom;

                    FillRect(psI->hDC, &psI->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
                }

                hMenuBitmap = (HBITMAP)SelectObject(hDC, hMenuBitmap);
                DeleteDC(hDC);

                return TRUE;
            }
        }
        break;

        case MYWM_NOTIFYICON: {
		    switch (lParam) {
                case WM_RBUTTONUP: {
                    POINT sPoint;
                    HMENU hSubMenu = GetSubMenu(hTrackMenu, 0);

                    GetCursorPos(&sPoint);

                    SetForegroundWindow(hWnd);

                    UpdateMenu(hSubMenu);
                    TrackPopupMenu(hSubMenu, TPM_RIGHTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, sPoint.x, sPoint.y, 0, hWnd, NULL);
                    
                    PostMessage(hWnd, WM_USER, 0, 0);
                }
                break;

                case WM_LBUTTONUP: {
                    nClicks ++;
                    
                    SetTimer(hWnd, 2, GetDoubleClickTime(), NULL);
                }
                break;

                case WM_MOUSEMOVE: {
                    char szStr[512];

                    UpdateDiscInformation(FALSE, TRUE, szStr);
                }
                break;

                default:
		        break;
            }
        }
        break;

        default: {
            return DefWindowProc(hWnd, nMsg, wParam, lParam);
        }
    }

    return 0L;
}

void WritePrivateProfileInt(const char* pzApp, const char* pzSect, int nNum, const char* pzFile)
{
    char szStr[32];

    sprintf(szStr, "%d", nNum);

    WritePrivateProfileString(pzApp, pzSect, szStr, pzFile);
}


void LoadConfig()
{
    struct {
        char nLen;
        char zPassword[256];
        char xKey;
    } sPassword;
    int nLoop;
    char zKey[32];
    int nConfigVersion;

    bLogfile = GetPrivateProfileInt("NTFY_CD", "Logfile", 0, "CDPLAYER.INI");
    bDebug = GetPrivateProfileInt("NTFY_CD", "Debug", 0, "CDPLAYER.INI");

    if (bLogfile) {
        time_t t;

        time(&t);

        DebugPrintf("--------------------------------------------------------------------------------------------------------");
        DebugPrintf("Starting version %s(built %s-%s) at %s", VERSION, __DATE__, __TIME__, ctime(&t));
    }

    nOptions = GetPrivateProfileInt("NTFY_CD", "Settings", -1, "CDPLAYER.INI");
DebugPrintf("nOptions = %d", nOptions);
    nTooltipTimeOptions = GetPrivateProfileInt("NTFY_CD", "TimeSettings", 0, "CDPLAYER.INI");
    nTooltipCDInfoOptions = GetPrivateProfileInt("NTFY_CD", "CDInfoSettings", 4+2+1, "CDPLAYER.INI");
    nCaptionTimeOptions = GetPrivateProfileInt("NTFY_CD", "CaptionTimeSettings", 0, "CDPLAYER.INI");
    nCaptionCDInfoOptions = GetPrivateProfileInt("NTFY_CD", "CaptionCDInfoSettings", 4+2+1, "CDPLAYER.INI");
    nPollTime = GetPrivateProfileInt("NTFY_CD", "PollTime", 1, "CDPLAYER.INI");
DebugPrintf("nPollTime = %d", nPollTime);
    bRepeat = GetPrivateProfileInt("NTFY_CD", "Repeat", 0, "CDPLAYER.INI");
    nLeftButtonNext = GetPrivateProfileInt("NTFY_CD", "LeftButtonNext", 1, "CDPLAYER.INI");
    nLeftButtonPause = GetPrivateProfileInt("NTFY_CD", "LeftButtonPause", 2, "CDPLAYER.INI");
    nLeftButtonPrev = GetPrivateProfileInt("NTFY_CD", "LeftButtonPrev", 3, "CDPLAYER.INI");
    nLeftButtonStop = GetPrivateProfileInt("NTFY_CD", "LeftButtonStop", 4, "CDPLAYER.INI");
    nMenuBreak = GetPrivateProfileInt("NTFY_CD", "MenuBreak", 20, "CDPLAYER.INI");
    nDefaultDevice = GetPrivateProfileInt("NTFY_CD", "DefaultDevice", nCurrentDevice, "CDPLAYER.INI");
    GetPrivateProfileString("NTFY_CD", "ExternalCommand", "", zExternalCommand, 255, "CDPLAYER.INI");

    nCDDBType = GetPrivateProfileInt("CDDB", "Type", 1, "CDPLAYER.INI");
    nCDDBOptions = GetPrivateProfileInt("CDDB", "Options_CDDB", 0, "CDPLAYER.INI");
DebugPrintf("nCDDBOptions = %d", nCDDBOptions);

    if (!GetPrivateProfileStruct("NTFY_CD", "CaptionFont", &sCaptionFont, sizeof(sCaptionFont), "CDPLAYER.INI") || 
        !(nOptions & OPTIONS_USEFONT)) {
        NONCLIENTMETRICS sMetrics;
        sMetrics.cbSize = sizeof(sMetrics);

        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &sMetrics, FALSE);

        sMetrics.lfCaptionFont.lfItalic = !sMetrics.lfCaptionFont.lfItalic;

        memcpy(&sCaptionFont, &sMetrics.lfCaptionFont, sizeof(sCaptionFont));        
        nCaptionFontColor = GetSysColor(COLOR_CAPTIONTEXT);
    }
    if (nOptions & OPTIONS_USEFONT) {
        nCaptionFontColor = GetPrivateProfileInt("NTFY_CD", "CaptionFontColor", GetSysColor(COLOR_CAPTIONTEXT), "CDPLAYER.INI");
    }

    nConfigVersion = GetPrivateProfileInt("NTFY_CD", "ConfigVersion", 121, "CDPLAYER.INI");
    if (nConfigVersion != CURR_CONFIG_VERSION && nOptions != -1) {
        int nConvertOptions = nOptions;
        int nConvertCDDBOptions = nCDDBOptions;
        int nConvertCDDBType = nCDDBType;
        int nConvertTimeOptions = nTooltipTimeOptions;
        int nConvertCDInfoOptions = nTooltipCDInfoOptions;

        // Convert from version 1.21
        if (nConfigVersion == 121) {
            nOptions = 0;
            nCDDBOptions = 0;
            nCDDBType = 0;
            nTooltipTimeOptions = 0;
            nTooltipCDInfoOptions = 0;
            nCaptionTimeOptions = 0;
            nCaptionCDInfoOptions = 0;

            nConvertCDDBOptions = GetPrivateProfileInt("CDDB", "Options", 0, "CDPLAYER.INI");

            nTooltipTimeOptions = nConvertTimeOptions;
            nTooltipCDInfoOptions = nConvertCDInfoOptions;
            nCDDBType = nConvertCDDBType;

            if (nConvertOptions & V121_SETTINGS_STOPONEXIT)
                nOptions |= OPTIONS_STOPONEXIT;
            if (nConvertOptions & V121_SETTINGS_STOPONSTART)
                nOptions |= OPTIONS_STOPONSTART;
            if (nConvertOptions & V121_SETTINGS_EXITONCDREMOVE)
                nOptions |= OPTIONS_EXITONCDREMOVE;
            if (nConvertOptions & V121_SETTINGS_PREVALWAYSPREV)
                nOptions |= OPTIONS_PREVALWAYSPREV;
            if (nConvertOptions & V121_SETTINGS_REMEMBERSTATUS)
                nOptions |= OPTIONS_REMEMBERSTATUS;
            if (nConvertOptions & V121_SETTINGS_NOINSERTNOTIFICATION)
                nOptions |= OPTIONS_NOINSERTNOTIFICATION;
            if (nConvertOptions & V121_SETTINGS_TRACKSMENUCOLUMN)
                nOptions |= OPTIONS_TRACKSMENUCOLUMN;
            if (nConvertOptions & V121_SETTINGS_NOMENUBITMAP)
                nOptions |= OPTIONS_NOMENUBITMAP;
            if (nConvertOptions & V121_SETTINGS_NOMENUBREAK)
                nOptions |= OPTIONS_NOMENUBREAK;

            if (nConvertCDDBOptions & V121_OPTIONS_QUERYLOCAL)
                nOptions |= OPTIONS_QUERYLOCAL;
            if (nConvertCDDBOptions & V121_OPTIONS_QUERYREMOTE)
                nOptions |= OPTIONS_QUERYREMOTE;
            if (nConvertCDDBOptions & V121_OPTIONS_STORELOCAL)
                nOptions |= OPTIONS_STORELOCAL;
            if (nConvertCDDBOptions & V121_OPTIONS_STORERESULT)
                nOptions |= OPTIONS_STORERESULT;

            if (nConvertCDDBOptions & V121_OPTIONS_USEHTTP)
                nCDDBOptions |= OPTIONS_CDDB_USEHTTP;
            if (nConvertCDDBOptions & V121_OPTIONS_USEPROXY)
                nCDDBOptions |= OPTIONS_CDDB_USEPROXY;

            if (GetPrivateProfileInt("NTFY_CD", "UseDBDLL", 121, "CDPLAYER.INI") && nCDDBOptions)
                nOptions |= OPTIONS_USECDDB;
        }
        else
            nOptions = OPTIONS_STORELOCAL | OPTIONS_QUERYLOCAL;
    }

    // Default options if not used before
    if (nOptions == -1)
        nOptions = OPTIONS_STORELOCAL | OPTIONS_QUERYLOCAL;

    bHasNotification = GetPrivateProfileInt("NTFY_CD", "HasNotification", 0, "CDPLAYER.INI");

    if (nOptions & OPTIONS_REMEMBERSTATUS) {
        int nStatus = GetPrivateProfileInt("NTFY_CD", "Status", 0, "CDPLAYER.INI");
        if (nStatus & STATUS_RANDOM)
            bRandomize = TRUE;
        if (nStatus & STATUS_REPEAT)
            bRepeat = TRUE;
    }

    // Load categories

    nNumCategories = GetPrivateProfileInt("NTFY_CD", "NumCategories", 0, "CDPLAYER.INI");
    // If no categories, add them!
    if (!nNumCategories) {
        nNumCategories = 10;

        WritePrivateProfileString("NTFY_CD", "NumCategories", "10", "CDPLAYER.INI");

        WritePrivateProfileString("NTFY_CD", "Category0", "Blues", "CDPLAYER.INI");
        WritePrivateProfileString("NTFY_CD", "Category1", "Classical", "CDPLAYER.INI");
        WritePrivateProfileString("NTFY_CD", "Category2", "Country", "CDPLAYER.INI");
        WritePrivateProfileString("NTFY_CD", "Category3", "Folk", "CDPLAYER.INI");
        WritePrivateProfileString("NTFY_CD", "Category4", "Jazz", "CDPLAYER.INI");
        WritePrivateProfileString("NTFY_CD", "Category5", "Misc", "CDPLAYER.INI");
        WritePrivateProfileString("NTFY_CD", "Category6", "Newage", "CDPLAYER.INI");
        WritePrivateProfileString("NTFY_CD", "Category7", "Reggae", "CDPLAYER.INI");
        WritePrivateProfileString("NTFY_CD", "Category8", "Rock", "CDPLAYER.INI");
        WritePrivateProfileString("NTFY_CD", "Category9", "Soundtrack", "CDPLAYER.INI");
    }

    ppzCategories = (char**)malloc(nNumCategories * sizeof(char**));
    for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++) {
        ppzCategories[nLoop] = (char*)malloc(80 * sizeof(char*));
        
        sprintf(zKey, "Category%d", nLoop);
        GetPrivateProfileString("NTFY_CD", zKey, "", ppzCategories[nLoop], 80, "CDPLAYER.INI");
    }

    GetPrivateProfileString("CDDB", "Path", "", zCDDBPath, 255, "CDPLAYER.INI");
    GetPrivateProfileString("CDDB", "RemoteServer", "cddb.celestial.com", zRemoteServer, 255, "CDPLAYER.INI");
    GetPrivateProfileString("CDDB", "RemoteHTTPPath", "/~cddb/cddb.cgi", zRemoteHTTPPath, 255, "CDPLAYER.INI");
    GetPrivateProfileString("CDDB", "RemoteProxyServer", "", zRemoteProxyServer, 255, "CDPLAYER.INI");
    GetPrivateProfileString("CDDB", "EmailServer", "", zRemoteEmailServer, 255, "CDPLAYER.INI");
    GetPrivateProfileString("CDDB", "EmailAddress", "", zEmailAddress, 255, "CDPLAYER.INI");
    GetPrivateProfileString("CDDB", "Domain", "", zDomain, 255, "CDPLAYER.INI");
    GetPrivateProfileString("CDDB", "ProxyUser", "", zProxyUser, 255, "CDPLAYER.INI");

    // Password
    if (!(nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD)) {
        memset(zProxyPassword, 0, sizeof(zProxyPassword));
        if (GetPrivateProfileStruct("CDDB", "ProxyPassword", &sPassword, sizeof(sPassword), "CDPLAYER.INI")) {
            for (nLoop = 0 ; nLoop < sPassword.nLen ; nLoop ++)
                zProxyPassword[nLoop] = (char)(sPassword.zPassword[nLoop] + sPassword.xKey);
        }
    }
    else
        zProxyPassword[0] = 0;

    nRemotePort = GetPrivateProfileInt("CDDB", "RemotePort", 888, "CDPLAYER.INI");
    nRemoteProxyPort = GetPrivateProfileInt("CDDB", "RemoteProxyPort", 0, "CDPLAYER.INI");
    nRemoteTimeout = GetPrivateProfileInt("CDDB", "RemoteTimeout", 30, "CDPLAYER.INI");	

    if (zRemoteHTTPPath[0] != 0 && zRemoteHTTPPath[0] != '/') {
        char zTmp[100];

        strcpy(zTmp, zRemoteHTTPPath);
        strcpy(zRemoteHTTPPath, "/");
        strcat(zRemoteHTTPPath, zTmp);
    }

    if (zRemoteHTTPPath[0] != 0 && zRemoteHTTPPath[strlen(zRemoteHTTPPath)-1] == '/')
        zRemoteHTTPPath[strlen(zRemoteHTTPPath)-1] = 0;

    // Fix path

    if (strlen(zCDDBPath) && zCDDBPath[strlen(zCDDBPath) - 1] != '\\')
        strcat(zCDDBPath, "\\");
}


void SaveConfig()
{
    struct {
        char nLen;
        char zPassword[256];
        char xKey;
    } sPassword;
    int nLoop;

    WritePrivateProfileInt("NTFY_CD", "ConfigVersion", CURR_CONFIG_VERSION, "CDPLAYER.INI");

    WritePrivateProfileInt("NTFY_CD", "Settings", nOptions, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "TimeSettings", nTooltipTimeOptions, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "CDInfoSettings", nTooltipCDInfoOptions, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "CaptionTimeSettings", nCaptionTimeOptions, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "CaptionCDInfoSettings", nCaptionCDInfoOptions, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "PollTime", nPollTime, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "Repeat", bRepeat, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "LeftButtonNext", nLeftButtonNext, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "LeftButtonPause", nLeftButtonPause, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "LeftButtonPrev", nLeftButtonPrev, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "LeftButtonStop", nLeftButtonStop, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "DefaultDevice", nDefaultDevice, "CDPLAYER.INI");
    WritePrivateProfileInt("NTFY_CD", "HasNotification", bHasNotification, "CDPLAYER.INI");
    WritePrivateProfileString("NTFY_CD", "ExternalCommand", zExternalCommand, "CDPLAYER.INI");

    WritePrivateProfileString("CDDB", "PATH", zCDDBPath, "CDPLAYER.INI");
    WritePrivateProfileInt("CDDB", "TYPE", nCDDBType, "CDPLAYER.INI");
    WritePrivateProfileInt("CDDB", "OPTIONS_CDDB", nCDDBOptions, "CDPLAYER.INI");
    WritePrivateProfileString("CDDB", "RemoteServer", zRemoteServer, "CDPLAYER.INI");
    WritePrivateProfileString("CDDB", "RemoteHTTPPath", zRemoteHTTPPath, "CDPLAYER.INI");
    WritePrivateProfileString("CDDB", "RemoteProxyServer", zRemoteProxyServer, "CDPLAYER.INI");
    WritePrivateProfileString("CDDB", "EmailServer", zRemoteEmailServer, "CDPLAYER.INI");
    WritePrivateProfileString("CDDB", "EmailAddress", zEmailAddress, "CDPLAYER.INI");
    WritePrivateProfileString("CDDB", "ProxyUser", zProxyUser, "CDPLAYER.INI");

    // Password
    if (nCDDBOptions & OPTIONS_CDDB_ASKFORPASSWORD)
        zProxyPassword[0] = 0;

    memset(&sPassword, 0, sizeof(sPassword));
    sPassword.xKey = (char)(rand() % 250);
    sPassword.nLen = (char)strlen(zProxyPassword);
    for (nLoop = 0 ; nLoop < (signed)strlen(zProxyPassword) ; nLoop ++)
        sPassword.zPassword[nLoop] = (char)(zProxyPassword[nLoop] - sPassword.xKey);
    for ( ; nLoop < 256 ; nLoop ++)
        sPassword.zPassword[nLoop] = (char)(rand() % 250);

    WritePrivateProfileStruct("CDDB", "ProxyPassword", &sPassword, sizeof(sPassword), "CDPLAYER.INI");

    WritePrivateProfileInt("CDDB", "RemotePort", nRemotePort, "CDPLAYER.INI");;
    WritePrivateProfileInt("CDDB", "RemoteProxyPort", nRemoteProxyPort, "CDPLAYER.INI");;
    WritePrivateProfileInt("CDDB", "RemoteTimeout", nRemoteTimeout, "CDPLAYER.INI");;

    if (nOptions & OPTIONS_USEFONT) {
        WritePrivateProfileStruct("NTFY_CD", "CaptionFont", &sCaptionFont, sizeof(sCaptionFont), "CDPLAYER.INI");
        WritePrivateProfileInt("NTFY_CD", "CaptionFontColor", nCaptionFontColor, "CDPLAYER.INI");
    }

    if (nOptions & OPTIONS_REMEMBERSTATUS) {
        int nStatus = 0;
        if (bRandomize)
            nStatus += STATUS_RANDOM;
        if (bRepeat)
            nStatus += STATUS_REPEAT;

        WritePrivateProfileInt("NTFY_CD", "Status", nStatus, "CDPLAYER.INI");
    }
}


int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE /*hPrevInstance*/,
    LPSTR lpCmdLine,
    int /*nShowCmd*/)
{
    WNDCLASS sWndClass;
    MSG sMsg;
    HWND hPrevWindow;
	char* pzTrackPtr;
	int nFoundTrack = 1;
    int nFoundDevice = -1;
    int nLoop;
    OSVERSIONINFO sVersionInfo;

    srand((unsigned)time(NULL));

	//
	// Variable Init
	// 

	hTrackMenu = NULL;
	hMainWnd = NULL;

	nCurrTrack = 0;
	nMaxTrack = 0;
	nClicks = 0;
	nPollTime = 1;
	nStatus = 0;
	nProgrammedTracks = 0;
	nLeftButtonNext	= 0;
	nLeftButtonPause = 0;
	nLeftButtonPrev	= 0;
	nLeftButtonStop	= 0;
	nCategoryChoosen = 0;

	nMenuIndexTracks = 7;
	nMenuIndexOther = 14;
	nMenuIndexDevices = 15;

	bCDOpened = FALSE;
	bPaused = FALSE;
	bPlaying = FALSE;
	bInit = FALSE;
	bAudio = FALSE;
	bRepeat = FALSE;
	bRandomize = FALSE;
	bMediaPresent = FALSE;
	bProgrammed = FALSE;
	bRepeatTrack = FALSE;
	bPlayWhole = FALSE;
	bChooseAllPresent = FALSE;
	bBrokenTracksMenu = FALSE;
	bCreateWindowFinished = FALSE;
	bFirstStatusCheckDone = FALSE;

	bInInfoDlg = FALSE;
	bInDBDlg = FALSE;
	bInSkipDlg = FALSE;
	bInSetAbsTrackPosDlg = FALSE;
	bInOptionsDlg = FALSE;
	bHasNotification = FALSE;

	bDebug = FALSE;
    bInDBDLGBuildList = FALSE;

	pzArtist = NULL;
	pzTitle = NULL;
	pzDiscExt = NULL;
	ppzTracks = NULL;
	ppzTracksExt = NULL;
	ppzTrackLen = NULL;
	pnTrackLen = NULL;
	pnProgrammedTracks = NULL;
	pzCategory = NULL;

	zCurrMCIID[0] = 0;
	zCurrCDDBID[0] = 0;

	nTooltipTimeOptions = 0;
	nTooltipCDInfoOptions = 0;
	nCaptionTimeOptions = 0;
	nCaptionCDInfoOptions = 0;
	nFrameOffset = 0;
	szToolTip[0] = 0;
	szTime[0] = 0;
	ppzCategories = NULL;
	nNumCategories = 0;
	memset(&abDevices, 0, sizeof(abDevices));
	nCurrentDevice = -1;
	nDefaultDevice = -1;
	nNumberOfDevices = 0;
	nMenuBreak = 20;
	nRepeatTrack = 0;
	nNextProgrammedTrack = 0;
	hMenuFont = NULL;
	hMenuBitmap = NULL;
	pnLastRandomTracks = NULL;
	nLastRandomTrack = 0;
	nDiscLenMS = 0;
    zExternalCommand[0] = 0;

    pzArtist = (char*) malloc(1);
    pzArtist[0] = 0;
    pzTitle = (char*) malloc(1);
    pzTitle[0] = 0;
    pzDiscExt = (char*) malloc(1);
    pzDiscExt[0] = 0;
    pzCategory = (char*) malloc(1);
    pzCategory[0] = 0;

    // Check drives for CD-ROM devices

    for (nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
        char zRoot[4];

        sprintf(zRoot, "%c:\\", nLoop);

        if (GetDriveType(zRoot) == DRIVE_CDROM) {
            abDevices[nLoop - 'A'] = TRUE;

            if (nCurrentDevice == -1)
                nCurrentDevice = nLoop - 'A';

            nNumberOfDevices ++;
        }
        else
            abDevices[nLoop - 'A'] = FALSE;
    }

    if (nCurrentDevice == -1) {
        MessageBox(NULL, "No CD audio device found!", APPNAME, MB_OK);

        return 1;
    }

    LoadConfig();

#ifdef _DEBUG
    if (bDebug) {
        // Fake more devices
        nNumberOfDevices ++;
        abDevices['F' - 'A'] = TRUE;
    }
#endif

    DebugPrintf("Current device = %c", nCurrentDevice + 'A');

    DebugPrintf("Number of devices = %d", nNumberOfDevices);

    for (nLoop = 'A' ; nLoop <= 'Z' ; nLoop ++) {
        if (abDevices[nLoop - 'A'])
            DebugPrintf("%c is a CD-ROM device", nLoop);
    }

    // Set current to default. Default is current if default is missing from the INI file!
    // current might change below if a command line with a device in it is used
    nCurrentDevice = nDefaultDevice;

    DebugPrintf("Default device = %c", nDefaultDevice + 'A');

    sVersionInfo.dwOSVersionInfoSize = sizeof(sVersionInfo);
    GetVersionEx(&sVersionInfo);

    if (sVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        DebugPrintf("Running Windows %d.%02d", sVersionInfo.dwMajorVersion, sVersionInfo.dwMinorVersion);
    else
        DebugPrintf("Running Windows NT %d.%02d.%d (%s)", sVersionInfo.dwMajorVersion, sVersionInfo.dwMinorVersion, 
                    sVersionInfo.dwBuildNumber, sVersionInfo.szCSDVersion);

    InitCommonControls();

    // Command line stuff
    
    strupr(lpCmdLine);

    DebugPrintf("Command Line: %s", lpCmdLine);

    pzTrackPtr = strstr(lpCmdLine, "-PLAY");
    if (!pzTrackPtr)
        pzTrackPtr = strstr(lpCmdLine, "/PLAY");
	if (pzTrackPtr) {
		if (strchr(lpCmdLine, '\\')) {
            char szStr[10];

		    pzTrackPtr += 6;

		    nFoundDevice = (int)*pzTrackPtr;
            DebugPrintf("Found device %c on command line", nFoundDevice);

            pzTrackPtr = strstr(pzTrackPtr, "TRACK");
		    if (pzTrackPtr) {
			    strncpy(szStr, pzTrackPtr+5, 2);
			    szStr[2] = 0;
			    nFoundTrack = atoi(szStr);

				DebugPrintf("Found track %d on command-line", nFoundTrack);
		    }
        }
	}

	hPrevWindow = FindWindow("NOTIFY_CD_CLASS", "Notify CD Player");
    if (hPrevWindow) {
        DebugPrintf("Sending to other instance...");

	    if (strstr(lpCmdLine, "-RANDOM"))
			SendMessage(hPrevWindow, MYWM_WAKEUP, IDM_RANDOMIZE, 0);

	    if (strstr(lpCmdLine, "-REPEAT"))
			SendMessage(hPrevWindow, MYWM_WAKEUP, IDM_REPEAT, 0);

        if (strstr(lpCmdLine, "-PREV"))
		    SendMessage(hPrevWindow, MYWM_WAKEUP, IDM_PREV, 0);

        if (strstr(lpCmdLine, "-NEXT"))
		    SendMessage(hPrevWindow, MYWM_WAKEUP, IDM_NEXT, 0);

        if (strstr(lpCmdLine, "-STOP"))
		    SendMessage(hPrevWindow, MYWM_WAKEUP, IDM_STOP, 0);

		if (strstr(lpCmdLine, "-PLAY") || strstr(lpCmdLine, "/PLAY")) {
            if (nFoundDevice != -1) {
                DebugPrintf("...Device = %c", nFoundDevice);
                SendMessage(hPrevWindow, MYWM_WAKEUP, IDM_DEVICES, MAKELONG(nFoundDevice, 1));
            }
			SendMessage(hPrevWindow, MYWM_WAKEUP, IDM_PLAY, nFoundTrack - 1);
        }

		if (strstr(lpCmdLine, "-PAUSE"))
			SendMessage(hPrevWindow, MYWM_WAKEUP, IDM_PAUSE, nFoundTrack - 1);

	    DebugPrintf("Instance ends....");
        DebugPrintf("--------------------------------------------------------------------------------------------------------");
    
        return 0;
    }

    hMainInstance = hInstance;

    DBInit();

	if (strstr(lpCmdLine, "-SETUP")) {
        DoOptions();

		return 0;
	}

    memset(&sWndClass, 0, sizeof(sWndClass));
    sWndClass.lpfnWndProc = MainWindowProc;
    sWndClass.hInstance = hInstance;
    sWndClass.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
    sWndClass.lpszClassName = "NOTIFY_CD_CLASS";

    if (!RegisterClass(&sWndClass)) 
    {
        MessageBox(NULL, "Error when registering class", APPNAME, MB_OK);

        return 0;
    }
    
    hTrayIcon = NULL;

    hIconNoAudio = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NOAUDIO));
    hIconPause = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAUSE));
    hIconStop = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STOP));
    hIconPlay = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLAY));

    strcpy(szToolTip, "Notify CD Player");

    hMenuFont = CreateFont(20, 0, 900, 0, FW_EXTRABOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
                       CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_DONTCARE, "Arial");
    hMenuBitmap = LoadBitmap(hMainInstance, MAKEINTRESOURCE(IDB_NOTIFY));

    hMainWnd = CreateWindow("NOTIFY_CD_CLASS", 
                            "Notify CD Player", 
                            WS_POPUP/* | WS_VISIBLE | WS_CAPTION*/,
                            , 
                            0, 0,
                            200, 200,
                            NULL,
                            NULL,
                            hInstance,
                            NULL);
    
    bCreateWindowFinished = TRUE;

    InitMenu();

    // Command line stuff
    if (strstr(lpCmdLine, "-RANDOM"))
		SendMessage(hMainWnd, MYWM_WAKEUP, IDM_RANDOMIZE, 0);

    if (strstr(lpCmdLine, "-REPEAT"))
		SendMessage(hMainWnd, MYWM_WAKEUP, IDM_REPEAT, 0);

    if (strstr(lpCmdLine, "/PLAY") || strstr(lpCmdLine, "-PLAY")) {
		if (nFoundDevice != -1)
   			SendMessage(hMainWnd, MYWM_WAKEUP, IDM_DEVICES, MAKELONG(nFoundDevice, 1));
		SendMessage(hMainWnd, MYWM_WAKEUP, IDM_PLAY, nFoundTrack - 1);
	}

    if (strstr(lpCmdLine, "-PAUSE")) {
		nCurrTrack = nFoundTrack - 1;

		SendMessage(hMainWnd, MYWM_WAKEUP, IDM_PLAY, nFoundTrack - 1);
		SendMessage(hMainWnd, MYWM_WAKEUP, IDM_PAUSE, 0);
	}

    PostMessage(hMainWnd, WM_TIMER, 1, 0);

    // Run the app
    while(GetMessage(&sMsg, NULL, 0, 0)) 
    {
        TranslateMessage(&sMsg);
        DispatchMessage(&sMsg);
    }

    SaveConfig();

    for (nLoop = 0 ; nLoop < nNumCategories ; nLoop ++)
        free(ppzCategories[nLoop]);
    free(ppzCategories);

    if (ppzTrackLen) {
        for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++)
            free(ppzTrackLen[nLoop]);
        free(ppzTrackLen);
    }
    if (pnTrackLen)
        free(pnTrackLen);

    if (ppzTracks)
        DBFreeTrackTitles(ppzTracks, nMaxTrack);

	if (pnProgrammedTracks)
		free(pnProgrammedTracks);

    if (pnLastRandomTracks)
        free(pnLastRandomTracks);

    if (ppzTracksExt) {
        for (nLoop = 0 ; nLoop < nMaxTrack ; nLoop ++)
            free(ppzTracksExt[nLoop]);

        free(ppzTracksExt);
    }

    free(pzArtist);
    free(pzTitle);
    free(pzDiscExt);
    free(pzCategory);

    DBFree();

    CDClose();

    DeleteObject(hMenuFont);
    DeleteObject(hMenuBitmap);

    DestroyMenu(hTrackMenu);

    if (nOptions & OPTIONS_SHOWONCAPTION) {
        HWND hForegroundWnd = GetForegroundWindow();

        if (hForegroundWnd) {
            char* pzText;
            int nLen;

            nLen = GetWindowTextLength(hForegroundWnd);
            pzText = (char*)malloc(nLen + 1);

            GetWindowText(hForegroundWnd, pzText, nLen + 1);
            SetWindowText(hForegroundWnd, pzText);

            free(pzText);
        }
    }

	DebugPrintf("Program ends....");
    DebugPrintf("--------------------------------------------------------------------------------------------------------");
    
	return 0;
}
