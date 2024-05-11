# matrix_daemon
Contains a simple daemon-like program to display CPU/RAM usage via the Framework LED Matrix displays.

Note:  Do not use the older display_stats program since it requires the installation of the input-module control program
which it uses as a middleman for the LED Matrix displays via the system() function.  Since this was seen as redundant and
inefficient the program was changed to directly send the desired commands.  Additionally, this version also uses the
setsid() function allowing it to run as a "proper" daemon.

I have not tested this software under all circumstances but it appears to work under normal use.
TODO:  Make sure it continues to function after a suspend.
