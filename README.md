# matrix_daemon
Provides a simple daemon-like program to display CPU/RAM usage via the Framework LED Matrix displays.

Installation instructions for Systemd Linux (most modern distros):
- Compile matrix_daemon.c with any desired changes to the proprocessor directives
- Copy the binary file to /usr/bin/
- Copy matrix_daemon.service (found under /systemd/ in this repo) to /etc/systemd/system/
- Run the following in terminal:  sudo systemctl start matrix_daemon.service && sudo systemctl enable matrix_daemon.serivce

TODO:  Add logging in the event of failure
