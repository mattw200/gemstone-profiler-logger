# GemStone-Profiler Logger

Project created: 02 June 2017
[Matthew J. Walker](mailto:mw9g09@ecs.soton.ac.uk)

This project records hardware performance counters (PMCs) (and power, voltage and temperature depending on the platform) for ARMv7 and ARMv8 platforms.

More details, instructions and tutorials available at [GemStone](http://gemstone.ecs.soton.ac.uk)

You may also be interested in the [GemStone-Profiler-Automate](https://github.com/mattw200/gemstone-profiler-automate) project,
which uses this project and automates the process of running workloads (at specified, CPU rates and 
core masks) while collecting Performance Counters (PMCs) (and optionally temperature, power etc.) 
and handles experiment repetition to collect many PMC events, and to ensure experiment consistency. 

## Overview

This project collects performance counters (PMCs) and other stats (e.g. CPU
frequency, temperature, power measurements, if supported). 

This project works on both ARMv7 and ARMv8 (64-bit) platforms and with varying
CPU configurations (detects the CPU types and number of counters at run-time).
It also supports monitoring different PMCs for different CPU types. 

The main purpose of this project is to record PMCs for experiments with very log overhead. However, it 
has been extended to also work for run-time purposes (e.g. calculates the
 PMC rate) and handles counter overflows at run-time (see USAGE - runtime data). 

It collects:
* PMC events and the cycle count (per core),
* time stamp in milliseconds
* datetime stamp
* cluster frequency
* (ODROID-XU3) little cluster, big cluster, GPU, memory voltage/power
* (ODROID-XU3) per-(big)core temperature, GPU temperature
* (ODROID-C2, RPi3) CPU temperature
* (runtime) PMC rate

**NOTE:** While this is a standalone project, it is designed to work with 
the [GemStone-Profiler-Automate](https://github.com/mattw200/gemstone-profiler-automate) project, 
which uses this project to automate the running of workloads on an Arm-based platform. 


## Compiling

To make the basic version:
```
cd src
make
```

Some platforms have temperature sensors, power sensors, etc.
Sensors for specific platforms (currently ODROID C2 and XU3, and the Raspberry Pi 3) are supported
and can be enabled by compiling with the 'odroid_c2', 'odroid_xu3', or `rpi3` option. 

E.g. 
```
make odroid_xu3
```

Make sure a `make clean` is done before compiling with a different option. 

## Enabling PMCs

**IMPORTANT:** PMCs must be enabled after each reboot. 

Go to the `enable_pmcs` directory and make the kernel modules (requires kernel source). 
It works on ARMv7 and ARMv8 and with any CPU. 
After compiling the LKM it can be loaded:
```
sudo insmod perf.ko
```

If, for any reason, PMC access from userspace needs to be disabled:
```
sudo rmmod perf.ko
```

It should be possible to force-load (i.e. with `modprobe -f`, look online for documentation on this) one of the existing kernel modules if your existing kernel supports this. For ARMv7 use the [powmon-enable-pmcs.ko.rpi3-32](https://github.com/mattw200/gemstone-profiler-logger/blob/master/enable-pmcs/powmon-enable-pmcs.ko.rpi3-32) and for ARMv8 use [powmon-enable-pmcs.ko.precompiled.armv8](https://github.com/mattw200/gemstone-profiler-logger/blob/master/enable-pmcs/powmon-enable-pmcs.ko.precompiled.armv8) (rename them to `perf.ko`)

If you are using the ODROID-XU3 or ODROID-XU4 board and the provided disk image from [Powmon](http://www.powmon.ecs.soton.ac.uk/powermodeling/) then 
 the pre-compiled `perf.ko.precompiled.xu3` can be used without any re-compilation or force-loading:
```
mv perf.ko.precompiled.xu3 perf.ko
sudo insmod perf.ko
```


## Usage - general

The main purpose of this software is to record data for experiments 
(e.g. logging the data in a raw format and post-processing later). The key steps 
(assuming `perf.ko` has already been loaded) are:

1. Select PMCs using the events.config file. 

   Each row represents a CPU type (e.g. Cortex-A7, Cortex-A53 etc.)
   The comma-separated fields in the row contain the CPUID_CODE 
   (a unique number to identify the CPU, as per the CPU's Reference
   Manuals). This is the number the software uses to identify the CPU.
   The next field is the CPU name (this is just there for convenience
   in identifying the CPU type, it is not used by the software). 

   The remaining columns are the PMC event numbers in hexadecimal format. 

   Some CPU types have 4 counters, some have 6.  The software checks the 
   correct number of PMCs have been specified. 

   The cycle count is automatically counted in addition to the selected
   events. 

2. Run PMC setup

   From the top directory of the project, run:
   ```
   ./bin/pmc-setup
   ```

   This sets the registers in the PMU (performance monitoring unit) to
   the correct values and sets the specified PMC events. 

   (It also derives the CPU configuration [e.g how many cores, the core type
   and the number of counters each core has] and saves it to file 
   [`cpu-data.csv`]. This is used when a core is 
   offline; when a core is offline the number of counters can't be read from 
   the CPU registers, and so this file is used instead to insert the correct
   number of columns in the results). 

   **NOTE:** all CPUs must be online when running pmc-setup

3. Get the column headings

   The header row of the results is obtained by running:
   ```
   ./bin/pmc-get-header
   ```

   The output is a tab-separated CSV header row.

   As well as printing the header row of the results to follow, it also provides
   information on the platform and confirms the PMC event selection (they are 
   read back from hardware). An example of the output of this program is found 
   in the `header.example` file. Each PMC column heading looks like:
   CPU 0 (id:0x07) cntr 3 (0x04), where 
   - CPU 0: means it is CPU 0. 
   - (id:0x07): identifies the type of CPU (e.g. Cortex-A7 in this case). The 
     IDCODE is defined in the ARM Reference Manual for the CPU in question. 
   - cntr 3: specifies which counter is used to count the specified event. E.g.
     the ARM Cortex-A7 has 4 counters (0-3). The Cortex-A15 has 6 (0-5). 
   - (0x04): identifies the event ID. In this case it is Event ID 0x04:
     "Data read or write operation that causes a cache access at (at least) the
       lowest level of data or unified cache. This includes speculative reads."
       (from the Cortex-A7 Technical Reference Manual)

4. Read (instantaneous) PMCs (and optionally sensor data, freq etc.)

   Running:
   ```
   sudo ./bin/pmc-get-pmcs
   ```

   Optionally, another argument can be specified, acting as a label in the
   results. E.g.:
   ```
   sudo ./bin/pmc-get-pmcs "whetstone start"
   ```

   sudo is required for reading `/sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_cur_freq`
   and possibly other sensor files (platform-dependent). 

   An example of the output from this program is shown in: `get-pmcs-pmcs-output.example`

   The pmc-get-pmcs gives an instantaneous snapshot of the time, CPU frequency
   and the PMCs. However, the PMC registers overflow. Therefore, just sampling
   start and end of a workload is not enough if the execution of the workload is
   longer than ~0.5 seconds (the 32-bit counters could overflow). Therefore, as
   well as the start and end snapshots, there needs to be a constant log.
   (see 5, next). 

5. Continuously log PMCs. 

   To get continuous samples of the PMCs:
   ```
   sudo ./bin/pmc-run 100000
   ```

   The argument is the sample period in microseconds. 

   
## Usage - recording experiments
(Example)
 
  Create two output files: one for logging events, one for a continuous sample
  ```
  ./bin/pmc-setup
  sudo ./bin/pmc-run 100000 > continuous.csv &
  ./bin/pmc-get-header > events.csv
  ./bin/pmc-get-pmcs "basicmath start" >> events.csv
  /* RUN BASICMATH
  ./bin/pmcs-get-pmcs "basicmath finish" >> events.csv
  ./bin/pmcs-get-pmcs "bitcount start" >> events.csv
  ```
  etc.
  
  The results are in two tab-separated CSV files, ready to be post-processed.
  Note that if the pmc-run is stopped, the final sample (last line in csv file)
  may not have completed, and therefore may confuse software opening the csv
  file. Delete the last line of this file to solve this. 
  

## Usage - runtime data

An example program has been created to provide run-time PMC output using the
previously described programs. 

It uses the time between consecutive samples and handles counter overflows to 
calculate the PMC rate (counts per second) over the sample period. 

It provides all of the output of the previous programs, but adds extra columns 
for each PMC (and cycle count) column where the rate for that PMC is given. 

Usage:
  ```
  sudo ./bin/pmc-runtime 100000
  ```

(pmc-setup still needs to be run beforehand)

Note that if the pmc-runtime is stopped, the final sample (last line in csv file)
may not have completed, and therefore may confuse software opening the csv
file. Delete the last line of this file to solve this. 

## Authors

[Matthew J. Walker](mailto:mw9g09@ecs.soton.ac.uk) - [University of Southampton](https://www.southampton.ac.uk)

This project supports the paper:
>M. J. Walker, S. Bischoff, S. Diestelhorst, G V. Merrett, and B M. Al-Hashimi,
>["Hardware-Validated CPU Performance and Energy Modelling"](http://www.ispass.org/ispass2018/),
>in IEEE International Symposium on Performance Analysis of Systems and Software (ISPASS), 
> Belfast, Northern Ireland, UK, April, 2018 [Accepted]

This work is supported by [Arm Research](https://developer.arm.com/research), 
[EPSRC](https://www.epsrc.ac.uk), and the [PRiME Project](http://www.prime-project.org).


## License

This project is licensed under the 3-clause BSD license. See LICENSE.md for details.

