// Copyright 2008,2009 Curaga
// A small app to be swallowed into JWM tray, to show cpu,mem,swap usage
// What do you know, mixing C with fltk C++ works just fine

// Licensed under the GPLv2, as scanf did not work for me, so
// I benefited from Open Source and copied the fgets block from
// wmbluemem :)
// Thank You Mihai Drãghicioiu

// v1.8: update battery support for linux 3.0

// Changes from Softwaregurl:
// Moved letters after the values with percentages
// Gigabytes for swap and mem

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/filename.H>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char version[] = "1.9";

float timeout = 1.5, mem = 0, swap = 0, cpu = 0, used = 0, oldused = 0,
	cputotal = 0, oldcputotal = 0;
float batnow = 0, batfull = 0;
int arg_i = 0, fontsize = 10, mib = 0, showswap = 1;
char memind[2] = "m", swapind[2] = "m", batname[6] = "", longpath[PATH_MAX] = "",
	batmsg[10] = "";

void batcheckup() {

	FILE *batinfo;

	sprintf(longpath, "/sys/class/power_supply/%s/charge_now", batname);
	if ((batinfo = fopen(longpath, "r")) == NULL) {
		fprintf(stderr, "Error opening batinfo %s\n", longpath);
		exit(1);
	}
	fscanf(batinfo, "%f", &batnow);
	fclose(batinfo);

	sprintf(longpath, "/sys/class/power_supply/%s/charge_full", batname);
	if ((batinfo = fopen(longpath, "r")) == NULL) {
		fprintf(stderr, "Error opening batinfo %s\n", longpath);
		exit(1);
	}
	fscanf(batinfo, "%f", &batfull);
	fclose(batinfo);

	sprintf(batmsg, " %.1f%%B", (float)(batnow / batfull) * 100);
}

void checkup() {

	FILE *meminfo, *cpuinfo;

	if ((meminfo = fopen("/proc/meminfo", "r")) == NULL) {
		fprintf(stderr, "Error opening meminfo");
		exit(1);
	}
	if ((cpuinfo = fopen("/proc/stat", "r")) == NULL) {
		fprintf(stderr, "Error opening stat");
		exit(1);
	}

	unsigned long memtotal, memfree, buffers, cache, swaptotal, swapfree;
	unsigned long user, nice, sys, idle, iowait, irq, softirq, virt, virt2;
	char buf[60] = "", buf2[190];
	char *p;

	while (!strstr(buf, "MemTotal:"))
		fgets(buf, 60, meminfo);
	strtok(buf, " "); p = strtok(NULL, " ");
	memtotal = strtol(p, NULL, 10);

	while (!strstr(buf, "MemFree:"))
		fgets(buf, 60, meminfo);
	strtok(buf, " "); p = strtok(NULL, " ");
	memfree = strtol(p, NULL, 10);

	while (!strstr(buf, "Buffers:"))
		fgets(buf, 60, meminfo);
	strtok(buf, " "); p = strtok(NULL, " ");
	buffers = strtol(p, NULL, 10);

	while (!strstr(buf, "Cached:"))
		fgets(buf, 60, meminfo);
	strtok(buf, " "); p = strtok(NULL, " ");
	cache = strtol(p, NULL, 10);

	while (!strstr(buf, "SwapTotal:"))
		fgets(buf, 60, meminfo);
	strtok(buf, " "); p = strtok(NULL, " ");
	swaptotal = strtol(p, NULL, 10);

	fgets(buf, 60, meminfo); strtok(buf, " "); p = strtok(NULL, " ");
	swapfree = strtol(p, NULL, 10);
	fclose(meminfo);

	fgets(buf2, 190, cpuinfo); strtok(buf2, " ");
	p = strtok(NULL, " "); user = strtol(p, NULL, 10);
	p = strtok(NULL, " "); nice = strtol(p, NULL, 10);
	p = strtok(NULL, " "); sys = strtol(p, NULL, 10);
	p = strtok(NULL, " "); idle = strtol(p, NULL, 10);
	p = strtok(NULL, " "); iowait = strtol(p, NULL, 10);
	p = strtok(NULL, " "); irq = strtol(p, NULL, 10);
	p = strtok(NULL, " "); softirq = strtol(p, NULL, 10);
	p = strtok(NULL, " "); virt = strtol(p, NULL, 10);
	p = strtok(NULL, " "); virt2 = strtol(p, NULL, 10);
	fclose(cpuinfo);

	used = (float)(user + nice + sys + irq + softirq + virt + virt2);
	cputotal = used + (float)idle + (float)iowait;

	cpu = ((used - oldused) / (cputotal - oldcputotal)) * (float)100;

	if (mib == 1) {
		mem = (float)(memtotal - (memfree + buffers + cache)) / (float)1024;
		swap = (float)(swaptotal - swapfree) / (float)1024;
	}
	if (mib == 2) {
		mem = (float)(memfree + buffers + cache) / (float)1024;
		swap = (float)swapfree / (float)1024;
	}
	if (mib == 0) {
		mem = ((float)(memtotal - (memfree + buffers + cache)) /
			(float)memtotal) * (float)100;
		swap = ((float)(swaptotal - swapfree) / (float)swaptotal) * (float)100;
	}

	oldused = used;
	oldcputotal = cputotal;

	if (mem > 999.9) {
		mem = mem / 1024;
		strcpy(memind, "g");
	}
	if (swap > 999.9) {
		swap = swap / 1024;
		strcpy(swapind, "g");
	}
	if (swaptotal == 0) {
		swap = 0;
	}
}

void tick(void *v) {
	checkup();
	if (batname[0] != '\0')
		batcheckup();
	Fl_Box *box = (Fl_Box *) v;
	char yeah[45] = "";
	if (showswap) {
		if (mib)
			sprintf(yeah, "C%.1f%% M%.1f%s  S%.1f%s%s", cpu, mem,
				memind, swap, swapind, batmsg);
		else
			sprintf(yeah, "%.1f%%C  %.1f%%M  %.1f%%S%s", cpu, mem,
				swap, batmsg);
	} else {
		if (mib)
			sprintf(yeah, "C%.1f%% M%.1f%s%s", cpu, mem, memind,
				batmsg);
		else
			sprintf(yeah, "%.1f%%C  %.1f%%M%s", cpu, mem, batmsg);
	}
	box->copy_label(yeah);
	Fl::repeat_timeout(timeout, tick, box);
}

int parser(int argc, char **argv, int &z) {
	if (strcmp(argv[z], "-h") == 0) {
		printf("Watcher %s\n"
		       "(C) Curaga\n"
		       "Fixes from softwaregurl\n\n"
		       "Switches: \n"
		       "\t-bg <color> background color \n"
		       "\t-fg <color> text color\n"
		       "\t-bat <battery> show battery (eg BAT0)\n"
		       "\t-m show used MiB instead of percent on mem/swap\n"
		       "\t-r show remaining MiB instead of percent on mem/swap\n"
		       "\t-s <float> check every s secs, default %.2f\n"
		       "\t-f <size> use font size f (default %d)\n"
		       "\t-x don't show swap\n"
		       "The color can be either named (green) or rgb ('#00ff00')\n",
		       version, timeout, fontsize);
		exit(0);
	}

	if (strcmp(argv[z], "-s") == 0) {
		timeout = strtof(argv[(z + 1)], NULL);
		z += 2;
		return 1;
	}

	if (strcmp(argv[z], "-f") == 0) {
		fontsize = atoi(argv[(z + 1)]);
		z += 2;
		return 1;
	}

	if (strcmp(argv[z], "-m") == 0) {
		mib = 1;
		z++;
		return 1;
	}

	if (strcmp(argv[z], "-x") == 0) {
		showswap = 0;
		z++;
		return 1;
	}

	if (strcmp(argv[z], "-bat") == 0) {
		strncpy(batname, argv[(z + 1)], 5);
		z += 2;
		return 1;
	}

	if (strcmp(argv[z], "-r") == 0) {
		mib = 2;
		z++;
		return 1;
	}

	return 0;
}

int main(int argc, char **argv) {

	int w = 124;

	// Graphic stuff
	Fl::args(argc, argv, arg_i, &parser);

	if (batname[0] != '\0' && showswap)
		w = 164;

	Fl_Window *window = new Fl_Window(w, 18);
	Fl_Box *box = new Fl_Box(FL_NO_BOX, 0, 0, w, 18, "Starting...");
	box->labelsize(fontsize);
	window->border(0);
	window->end();
	window->show(argc, argv);

	Fl::add_timeout(timeout, tick, box);
	return Fl::run();
}
