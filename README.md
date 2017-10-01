# Rosegarden

[Rosegarden](http://www.rosegardenmusic.com/) is a MIDI and audio sequencer and musical notation editor for Linux.

This is a fork of [Ted Felix's working space](https://github.com/tedfelix/rosegarden),
which is a fork of [the original repository](https://sourceforge.net/projects/rosegarden/).

## Recommended usage

- Install Ubuntu Studio or KX Studio.
    - Otherwise, set up JACK.
- Find some SoundFont 2 (SF2) files.
- Start Fluidsynth and load your SF2 files.
- Plug your MIDI keyboard controller.
- Start Rosegarden.
- Open the menu `Studio > Manage MIDI devices`.
    - Select your MIDI keyboard controller as the input device.
    - Select Fluidsynth as the output device.
- Record your performance, track by track.

## Building on Ubuntu 14.04

This was last tested on 2017-10-01.

Assume that the operating system is Ubuntu 14.04 and the shell is Bash.

Open a new terminal.

Install build dependencies.
This takes some time.

```
sudo apt-get install \
build-essential \
cmake \
dssi-dev \
g++ \
git \
ladspa-sdk \
libasound2-dev \
libfftw3-dev \
libjack-jackd2-dev \
liblo-dev \
liblrdf0-dev \
libsamplerate0-dev \
libsm-dev \
libsndfile1-dev \
qt4-default \
;
```

If that doesn't work, try this (taken from [wiki](http://www.rosegardenmusic.com/wiki/development_from_svn)):

```
sudo apt-get build-dep rosegarden
```

Close that terminal.
Don't reuse it.

Open a new terminal.

Clone the repository.
Get into it.

```
git clone https://github.com/tedfelix/rosegarden.git
cd rosegarden
```

Configure.
Use Qt 4.
Don't waste your time building with Qt 5 on Ubuntu 14.04.

```
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Debug -D USE_QT4=1 ..
```

Build.
Replace `4` with how many processor cores you have.
This takes some time.

```
make -j 4
```

Optional:
Enable core dumps for this session:

```
ulimit -c unlimited
```

Run the application.
No need to install.

```
./rosegarden
```

## Generating source code documentation

This takes some time.

```
cd <directory-containing-Doxyfile>
doxygen
```

The generated documentation begins at `build/doc/html/index.html` or `/tmp/html/index.html`.

## Things to do

- Make it easier for people to contribute.
    - Establish Github presence.
    - Move the [wiki](http://www.rosegardenmusic.com/wiki/) into this repository as Markdown files.
        - Everyone can send a patch to fix the documentation.
        - Everyone can browse the documentation on Github.
        - Let Github host the documents and keep spammers away.
    - Update outdated information.
        - [FAQ](http://rosegardenmusic.com/wiki/frequently_asked_questions) for known problems and workarounds
        - [How to report bugs](http://rosegardenmusic.com/tutorials/bug-guidelines.html)
        - [old readme](README)
        - [developers readme](README.DEVELOPERS)
        - [how to contribute](http://www.rosegardenmusic.com/wiki/dev:contributing)
        - [build dependencies](http://www.rosegardenmusic.com/wiki/dev:contributing#prepare_the_build_environment)
            - Those dependencies are for Debian 5.
            - The build system has changed from autotools to cmake.
    - Move from Subversion to Git.
- Unknown
    - Scripting, text user interface, and command line
        - [SWIG](http://www.swig.org/) ([swig/swig](https://github.com/swig/swig)) takes C header files and generates wrappers for several programming languages
        - [chjj/blessed](https://github.com/chjj/blessed), a nodejs alternative to ncurses

## Other resources

- Similar software
    - [Ardour](https://github.com/Ardour/ardour)
    - LMMS
    - [Muse Sequencer](http://muse-sequencer.org/)
    - [OpenOctave](https://github.com/ccherrett/oom), [forked](http://www.openoctave.org/about) from Muse Sequencer on 2011-01-09
    - [Qtractor](https://qtractor.sourceforge.io/)
    - [Rosegarden](http://www.rosegardenmusic.com/)
- Related software
    - [Fluidsynth](https://github.com/FluidSynth/fluidsynth)
- Wikipedia articles
    - [Comparison of free software for audio](https://en.wikipedia.org/wiki/Comparison_of_free_software_for_audio)

## License

Rosegarden as a whole seems to be licensed under GNU General Public License version 2.

Everything here made by Erik Dominikus is licensed under
BSD 3-clause license, MIT license, ISC License,
and/or Apache Software License version 2.0,
at your choice.
