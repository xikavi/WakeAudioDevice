# WakeAudioDevice
 
Small utility to prevent audio devices turning off after some time from last sound was played.
Utility plays sound file (sound.wav in the workdir by default) on timer, that starts after detecting sound volume that exceeded threshold value.
Also it plays sound after system was resumed from low-power state.
Arguments: /hide for minimize to tray, /start for immediatly start on program launch.

Compiled with Qt 5.15.2
Requires Windows SDK