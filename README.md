# Beepboy 

gnuboy port for Beepy.
Currently:
* Scales original 160 x 144 display to 320 x 240

To do:
* Implement Bayer 2x2 dithering for grayscales

To install on Beepy:
```
curl -s https://raw.githubusercontent.com/TheMediocritist/beepy_gnuboy/master/beepy_setup.sh | bash
```






## GNUBOY 1.0.4 README

Welcome to gnuboy, one of the fastest if not the fastest GB/GBC emu-
lator available.

gnuboy was written well over 20 years ago and achieved full speed
on most machines of that era, on today's machines it runs close to
0% CPU usage.

gnuboy development was mostly stagnant for the past 20 years, but
i revived it using the last released sources and CVS dumps and fixed
numerous minor issues with modern compilers and systems, as well
as the occasional bugfix, plus a new SDL2 backend.

changes to 1.0.3 include:

- new ao audio backend
- new SDL2 video/input/audio backend
- fixed cpu cycle accuracy for all CPU ops
- backported fix for some opcodes
- fixed issues with window in NBA hoopz
- added support for .xz compressed roms
- added support for original BIOS

unlike most modern GB emulators, gnuboy doesn't strive to be
cycle-accurate (except where necessary), but to be fast, portable
and compatible and to provide a good gaming experience.

it works using the following backends:

- linux fbdev
- linux joystick
- SDL1.2
- SDL2
- X11
- AO audio
- OSS audio
- SVGAlib
- DOS/Windows (untested)

old README follows.

  INTRO

Welcome to gnuboy, one of the few pieces of Free Software to emulate
the Game Boy handheld game console. Written in ANSI C with a few
optional assembler optimizations for particular cpus, gnuboy supports
a wide range of host systems, and has been tested successfully on:

  GNU/Linux
  FreeBSD
  OpenBSD
  BeOS
  Linux/390 (IBM S/390 Mainframe)
  SunOS/Sun Ultra60
  IRIX/SGI O2
  IRIX/SGI Indy
  AIX/Unknown
  DR-DOS
  MS-DOS
  Windows DOS box
  Windows 9x/NT/2k

Additionally, gnuboy should run on any other *nix variants that have
ANSI C compilers and that are remotely POSIX compliant. As gnuboy is
Free Software, you're welcome to fix any problems you encounter
building it for a particular system, or to port it to entirely new
systems.


  EMULATION

gnuboy emulates nearly all aspects of the (Color) Gameboy, including
all of the following and much more:

  Full GBZ80 instruction set.
  Scanline-based LCD engine.
  Ten sprites per scanline limit.
  Support for all CGB graphics extensions.
  Sprite DMA, HDMA, and GDMA.
  All four sound channels including digital samples.
  MBC1, MBC2, MBC3 (including clock), and MBC5 mappers.
  Wave pattern memory corruption when sound channel 3 is played.
  Pad, timer, divide counter, and other basic hardware registers.
  CGB double-speed CPU mode.

Aspects not emulated at this time include:

* Serial IO (link cable).
  Undocumented 'extra' ram in OAM space on Gameboy Color.
  All Super Gameboy extensions.
* GBC, HuC1, and HuC3 IR ports.
* Obscure mappers such as TAMA5.
  Sorting sprites by X coordinate in DMG mode.
  HALT instruction skipping in DMG mode.
  CPU stalls during HDMA and GDMA.

Only the two marked by * are known to affect the playability of
actual games or demos; the rest are just listed for completeness'
sake.


  CONFIGURATION

Please refer to docs/CONFIG and the sample files in etc/.
The default search path order for configuration files is:

  $HOME/.gnuboy/gnuboy.rc
  ./gnuboy.rc


  FEATURES

In addition to basic emulation, gnuboy provides the following
features:

  Highly flexible keybinding and configuration subsystem.
  State saving and loading at any point.
  Very precise timing/synchronization, preserved across save/load.
  Joystick support on Linux, DOS, and all SDL-based ports.
  Fully customizable palettes for DMG games.
  Screen scaling by a factor of 2, 3, or 4 in all ports.
  Hardware-based screen scaling on platforms where it's available.
  Debug traces to stdout.
  Dynamic palette allocation when run in 256-color modes...
  OR simulated 3/3/2 bits per channel in 256-color modes.

For information on configuring and using these features, see the
additional documentation in the "docs" directory.


  COMPATIBILITY

Out of over 300 results reported by testers, all games are known to
work perfectly on gnuboy with the following exceptions:

  Fighting Phoenix (Japanese) may or may not work since it uses the
  HuC1 memory controller, which is not implemented properly. There has
  been no report either way so far.

  Pocket Bomberman (Japanese version, which uses HuC1) runs, but can
  be made to crash if the player jumps into the ceiling in the first
  level. It's not clear whether this bug is MBC-related, something
  else, or an actual bug in the original game.

  Monster Go! Go! Go! (Japanese) is unplayable. The cause of the
  problem is not fully known, but it's either a very bad dump or it's
  using some sort of specialized MBC that's not documented.

  Final Fantasy Adventure has visual problems with the fade between
  screens. Does not affect gameplay.

  Bubble Bobble 2 has some minor tile glitches right before gameplay
  actually begins. Cause unknown. Does not affect gameplay.

  Alone in the Dark is reported to have minor visual glitches. I
  haven't seen it myself so I can't judge their severity.

  Both new Zelda games are reported to have a visual glitch at the
  beginning of the game, and on certain other screens. I haven't seen
  the problem myself, but supposedly it impacts gameplay to some
  extent.

Please report any other incompatibilities discovered directly to
gnuboy@unix-fu.org, so that they can be documented and hopefully
fixed.


  FUTURE / WISHLIST

Here's a brief list of what may appear in gnuboy in the future:

  Screenshots.
  Integrated debugger.
  Super Gameboy support.
  Serial link over the internet.
  Serial link to a real Gameboy with a custom cable.
  Configurable color filters to provide more authentic LCD look.
  Custom colorization of DMG games on a per-tile basis.
  Support for more colorspaces in the hardware scaler.
  Recording audio.
  GBS player built from the same source tree.
  Full recording and playback of emulation.
  So-called "high level emulation" of certain typical dumb loops.

Features that are not likely to appear soon or at all include:

  Rumble support - this would be nice, but SDL doesn't seem to support
  force-feedback yet. We'll see about it in the long-term though.

  Eagle/2xSaI/etc. - probably not feasible since these libraries don't
  appear to be compatible with the terms of the GPL. We might work on
  our own interpolation engine eventually, but that's low priority.

  GUI/GUI-like features - such things are best handled by external
  front-ends. We might eventually add a mechanism for external
  programs to communicate with gnuboy and reconfigure it while it's
  running, however.

  Plugins - NO! The way I see it, plugins are just an attempt to work
  around the GPL. In any case, even if you are adding plugin support
  yourself, you are bound by the terms of the GPL when linking ANY
  code to gnuboy, including dynamic-linked modules. However we'd
  rather not deal with this mess to begin with.

  Compressed ROMs/Saves - this one is very iffy. On most systems, this
  is redundant; *nix users can just pipe the rom through a
  decompression program, and Windows users can just double-click or
  drag files from their favorite GUI unzipper program. Linking to zlib
  isn't really acceptable since it's massively bloated and we don't
  want to include it with gnuboy or add external dependencies. We may,
  however, write our own tiny decompressor to use at some point.

Ideas and suggestions for other features are welcome, but won't
necessarily be used. You're of course also free to add features
yourself, and if they fit well into the main tree they may eventually
get included in the official release. See the file HACKING for more
details on modifying and/or contributing.


  THANKS

Thanks goes out to everyone who's expressed interest in gnuboy by
writing -- users, porters, authors of other emulators, and so forth.
Apologies if we don't get a personal response out to everyone, but
either way, consider your feedback appreciated.


  EPILOGUE

OK, that looks like about it. More to come, stick around...



                                          -Laguna  <laguna@aerifal.cx>











