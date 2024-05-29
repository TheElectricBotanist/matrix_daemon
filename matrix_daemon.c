/*
 * matrix_daemon.c
 * 
 * Copyright 2024 Lukas Fischer
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <math.h>

#define BRIGHTNESS 3  // Sets brightness on scale from 0 to 100
#define UPDATE_RATE 500000000L  // Update rate in ns
#define CPU_DISPLAY "/dev/ttyACM1"  // Port for Matrix displaying CPU use
#define RAM_DISPLAY "/dev/ttyACM0"  // Port for Matrix displaying RAM use
#define USE_CPU 1  // Display CPU.  Set to 0 to turn off
#define USE_RAM 1  // Display RAM.  Set to 0 to turn off

int set_interface_attribs (int fd, int speed, int parity);
int set_blocking (int fd, int should_block);
int initialize_usb(const char * usb_path);

int main(void)
{
	setsid();
	const char tokenString[2] = " ";
	char setBrightness[4] = {0x32, 0xAC, 0x00, BRIGHTNESS};
	struct timespec remaining, request = { 0, (UPDATE_RATE / 2)};
	volatile char dataPacket[5] = {0x32, 0xAC, 0x01, 0x00, 0};
	volatile struct sysinfo systemInfo;
	volatile char procStats[100];
	volatile char * token;
	volatile int totalTime;
	volatile int idleTime = 0;
	volatile int lastTotal = 0;
	volatile int lastIdle = 0;
	volatile int i;
	int fd_cpu, fd_ram;
	if(USE_CPU)
	{
		fd_cpu = initialize_usb(CPU_DISPLAY);
		write(fd_cpu, setBrightness, 4);
		close(fd_cpu);
	}
	if(USE_RAM)
	{
		fd_ram = initialize_usb(RAM_DISPLAY);
		write(fd_ram, setBrightness, 4);
		close(fd_ram);
	}
	nanosleep(&request, &remaining);
	do{
		if(USE_CPU)
		{
			FILE * fp = fopen("/proc/stat","r");
			fgets(procStats, 100, fp);
			fclose(fp);
			token = strtok(procStats, tokenString);
			totalTime = 0;
			i = 0;
			while(token != NULL)
			{
				token = strtok(NULL, tokenString);
				if(token != NULL)
				{
					totalTime = totalTime + atoi(token);
					if(i == 3)
					{
						idleTime = atoi(token);
					}
					i = i + 1;
				}
			}
			dataPacket[4] = round((100.0f - (100.0f * ((float)(idleTime - lastIdle) / (float)(totalTime - lastTotal)))));
			lastIdle = idleTime;
			lastTotal = totalTime;
			fd_cpu = initialize_usb(CPU_DISPLAY);
			if(fd_cpu != -1)
			{
				write(fd_cpu, dataPacket, 5);
				close(fd_cpu);
			}
		}
		nanosleep(&request, &remaining);
		if(USE_RAM)
		{
			sysinfo(&systemInfo);
			dataPacket[4] = round((100.0f - (100.0f * ((long double)(systemInfo.freeram + systemInfo.bufferram) / (long double)systemInfo.totalram))));
			fd_ram = initialize_usb(RAM_DISPLAY);
			if(fd_ram != -1)
			{
				write(fd_ram, dataPacket, 5);
				close(fd_ram);
			}
		}
		nanosleep(&request, &remaining);
	}while(1);
	return 0;
}

int set_interface_attribs (int fd, int speed, int parity)
{
	struct termios tty;
	if (tcgetattr (fd, &tty) != 0)
	{
		// error_message ("error %d from tcgetattr", errno);
		return -1;
	}
	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
									// no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
									// enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;
	if (tcsetattr (fd, TCSANOW, &tty) != 0)
	{
		// error_message ("error %d from tcsetattr", errno);
		return -1;
	}
	return 0;
}

int set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		// error_message ("error %d from tggetattr", errno);
		return -1;
	}
	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout
	// if (tcsetattr (fd, TCSANOW, &tty) != 0)
			// error_message ("error %d setting term attributes", errno);
	return 0;
}

int initialize_usb(const char * usb_path)
{
	int fd = open(usb_path, O_RDWR | O_NOCTTY | O_SYNC);
	if ((fd < 0) || (set_interface_attribs(fd, B115200, 0) == -1) || (set_blocking(fd, 0) == -1))
	{
		// error_message ("error %d opening %s: %s", errno, CPU_DISPLAY, strerror (errno));
		return -1;
	}
	else
	{
		return fd;
	}
}

