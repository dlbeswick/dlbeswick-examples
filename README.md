David Beswick source code examples
==================================

Overview
--------

This source is taken from my personal projects, developed for my own interest in my own time.

Project descriptions
--------------------

### ruby/auanalysis

A music collection cataloguing program that also performs transcoding between different formats. 

It makes use of Wavelet transforms to detect track boundaries in recordings of vinyl records, including using memory-mapping to process large files (see Standard/src/Standard/Wavelet.h)

It contains an implementation of a stack-based Makefile-like task system that is used to run transcoding operations in parallel (see ruby/auanalysis/transform/task_runner.rb)

### ruby/DJ

A DJ program written in Ruby with C++ extensions. It makes use of multithreading to perform audio decoding and streaming. It has basic implementations of DSP operations (lowpass, highpass, pitch, mixing.) It also can act on data from MIDI controllers.

See ruby/std/dsound/src for C++ extensions for audio.

### ruby/std

Common cross-project code for Ruby projects. C++ extensions can be built with GNU autotools. Some notable areas:

#### ruby/std/view

A system for constructing GUI views from data defined in Ruby classes, using wxruby.

#### ruby/std/midiiinterface

Cross-platform wrapper allowing access of MIDI data in Ruby.

#### ruby/std/dsound

C++ extensions providing audio playback to Ruby applications.

### Standard

Common cross-project code for C++ projects. Contains cross-platform wrappers for multithreading, file operations, memory mapping, etc.

The project can be built with GNU autotools.

### RSE

C++ project. "Reality Simulation Engine", DirectX 9 game engine project.

### Pancake Bros

C++ 2.5D action game making use of above game engine.


