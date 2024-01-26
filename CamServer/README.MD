ESP32 Camera Server.

Server app developed with delphi 12, using ICS for sockets..<br>
Compiled bin is a 32 bit Windows app.<br>
Needs nothing to run, download, put in folder of your choosing and execute..<br>
Click listen to start connecting cams.<br>
When cam first connects, its window is created top right and small, once it identifies, previous window size and placement are restored..<br>
There's 3 mode of viewing, starts out all video, click on vid to expand window, then click on config to expand again..<br>
Added extensive debug logging mostly due to my poor wifi net but also not so sure it's my net as cams seem to work good after they warm up some, strange..<br><br>

Server app broadcasts its ip address and port, so cams should all auto connect.<br>
Also changed time out timer to just close window is cam becomes silent, cam will see this and reconnect..<br>
<br>
esp folder contains current sketch..<br>
added SOS state when ssid is not known, onboard leds blinks sos and config is accepted through serial terminal on succesful connect, settings are saved..<br>
To do the SOS blinking I used my AsyncMorseLib, you can find it in ArdsAndEnds, fixed it too, compiling the cam sketch I have warnings shown as errors, so had to clean up 2 things..<br>

still to test and add..<br>
Web Ip Change<br>
OTA<br>
PTZ controls<br>
Drive Controls<br>
<br>

enjoy.. ~q
