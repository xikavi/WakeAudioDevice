# WakeAudioDevice
 
Small utility to wake up audio devices that go to sleep mode after some time without sound input.
It plays sound file (sound.wav in the workdir by default) after desired time from last detected audio ouptut.
Also it plays sound after system is resumed from low-power state.
Windows "stereo mixer" input audio device must be configured to listen your audio output.
Arguments: /hide for hiding window, /start for immediatly start on program launch.

Compiled with Qt 5.15.2