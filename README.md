# matrix_daemon
Provides a simple daemon-like program to display CPU/RAM usage via the Framework LED Matrix displays.

Note:  Do not use the older display_stats as it requires the installation of the input-module control program,
which is used as a "middleman" for the LED Matrix displays via the system() function.  Since this was overly
redundant and inefficient the program was changed to directly send the desired commands over the USB interface.
This version also makes use of the setsid() function allowing it to run as a "proper" daemon.

I have not tested this software under all circumstances but it appears to work under normal use.  The testing environment
is Debian 12.5.0 with kernel 6.6.13+bpo-amd64.
