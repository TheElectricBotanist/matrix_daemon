# matrix_daemon
Provides a simple daemon-like program to display CPU/RAM usage via the Framework LED Matrix displays.  After compiling (with any desired changes to the proprocessor directives) copy the binary file to /usr/bin/ and matrix_daemon.service (found under /systemd/ in this repo) to /etc/systemd/system/.  Then run the following in terminal:
sudo systemctl start matrix_daemon.service && sudo systemctl enable matrix_daemon.serivce

TODO:  Add logging in the event of failure
