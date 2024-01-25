ESP32 Camera Server.

Server app developed with delphi 12, using ICS for sockets..
Compiled bin is a 32 bit Windows app.
Needs nothing to run, download, put in folder of your choosing and execute..
Click listen to start connecting cams.
When cam first connects, its window is created top right and small, once it identifies, previous window size and placement are restored..
There's 3 mode of viewing, starts out all video, click on vid to expand window, then click on config to expand again..
Added extensive debug logging mostly due to my poor wifi net but also not so sure it's my net as cams seem to work good after they warm up some, strange..

Server app broadcasts its ip address and port, so cams should all auto connect.
Also changed time out timer to just close window is cam becomes silent, cam will see this and reconnect..

esp folder contains current sketch..
still to test and add..
Web Ip Change
OTA
PTZ controls
Drive Controls


enjoy.. ~q
