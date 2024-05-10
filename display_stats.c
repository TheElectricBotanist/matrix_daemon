/*
 * display_stats.c
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
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>

#define BRIGHTNESS 3  // Sets brightness on scale from 0 to 100
#define UPDATE_RATE 500000000L  // Update rate in ns
#define CPU_DISPLAY "/dev/ttyACM1"  // Port for Matrix displaying CPU use
#define RAM_DISPLAY "/dev/ttyACM0"  // Port for Matrix displaying RAM use
#define USE_CPU 1  // Display CPU.  Set to 0 to turn off
#define USE_RAM 1  // Display RAM.  Set to 0 to turn off

int main(void)
{
	const char tokenString[2] = " ";
	struct timespec remaining, request = { 0, (UPDATE_RATE / 2)};
	struct sysinfo systemInfo;
	char procStats[100];
	char * token;
	char displayString[100];
	int totalTime;
	int idleTime = 0;
	int lastTotal = 0;
	int lastIdle = 0;
	int cpuUsed = 0;
	int ramUsed = 0;
	int i;
	sprintf(displayString, "inputmodule-control led-matrix --brightness %i", BRIGHTNESS);
	system(displayString);
	nanosleep(&request, &remaining);
	memset(displayString, 0, sizeof(displayString));
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
			cpuUsed = 100.0f - (100.0f * ((float)(idleTime - lastIdle) / (float)(totalTime - lastTotal)));
			lastIdle = idleTime;
			lastTotal = totalTime;
			sprintf(displayString, "inputmodule-control --serial-dev %s led-matrix --percentage %i", CPU_DISPLAY, cpuUsed);
			system(displayString);
			memset(displayString, 0, sizeof(displayString));
		}
		nanosleep(&request, &remaining);
		if(USE_RAM)
		{
			sysinfo(&systemInfo);
			ramUsed = 100.0f - (100.0f * ((float)systemInfo.freeram) / ((float)systemInfo.totalram));
			sprintf(displayString, "inputmodule-control --serial-dev %s led-matrix --percentage %i", RAM_DISPLAY, ramUsed);
			system(displayString);
			memset(displayString, 0, sizeof(displayString));
		}
		nanosleep(&request, &remaining);
	}while(1);
	
	return 0;
}

