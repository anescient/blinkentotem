#!/usr/bin/env python3

import psutil


class AutoRangeNumber:
    def __init__(self, premin=None, premax=None):
        self.value = None
        self.minvalue = premin
        self.maxvalue = premax
        self.relativeValue = None

    def updateValue(self, value):
        self.value = value
        self.minvalue = min(self.minvalue or self.value, self.value)
        self.maxvalue = max(self.maxvalue or self.value, self.value)
        range = self.maxvalue - self.minvalue
        self.relativeValue = (self.value - self.minvalue) / range if range > 0 else 0


class SystemActivity:

    class DiskActivity:

        def __init__(self):
            self._sdiskio = None
            self.bytesread = 0
            self.byteswritten = 0

        def update(self, sdiskio):
            if self._sdiskio is None:
                self._sdiskio = sdiskio
            self.bytesread = sdiskio.read_bytes - self._sdiskio.read_bytes
            self.byteswritten = sdiskio.write_bytes - self._sdiskio.write_bytes
            self._sdiskio = sdiskio

    # psutil's "cpu_percent" methods changed or broke or something
    # they sucked anyway
    class CPUActivity:

        def __init__(self, scpuindex):
            self._prevtimes = None
            self._scpuindex = scpuindex
            self._coretempindex = 1 + (scpuindex % 4)
            self.busy = 0
            self.io = 0
            self.thermal = AutoRangeNumber(28, 67)

        def update(self, scputimes):
            nexttimes = scputimes[self._scpuindex]
            if self._prevtimes is None:
                self._prevtimes = nexttimes
            prevsum = sum(self._prevtimes)
            nextsum = sum(nexttimes)
            dt = nextsum - prevsum
            if dt < 0.02:
                return

            idletime = nexttimes.idle - self._prevtimes.idle
            iotime = nexttimes.iowait - self._prevtimes.iowait
            busytime = dt - idletime - iotime
            self._prevtimes = nexttimes

            self.busy = busytime / dt
            self.io = iotime / dt

        def updateTemp(self, coretemps):
            self.thermal.updateValue(coretemps[self._coretempindex].current)

    class SwapActivity:

        def __init__(self):
            self._prevsin = 0
            self._prevsout = 0

        def getBytesSince(self):
            swapmem = psutil.swap_memory()
            sin, sout = swapmem.sin, swapmem.sout
            bytes = 0
            if sin > self._prevsin:
                bytes += sin - self._prevsin
                self._prevsin = sin
            if sout > self._prevsout:
                bytes += sout - self._prevsout
                self._prevsout = sout
            return bytes

    # disks is a list of /dev/ file names e.g. 'sda', 'md0'
    def __init__(self, disks):
        self._diskActivities = {dev: self.DiskActivity() for dev in disks}
        self._cpuActivities = [self.CPUActivity(i) for i in range(8)]
        self.swapActivity = self.SwapActivity()

    def updateAll(self):
        self.updateDisks()
        self.updateCPUs()
        self.updateCPUTemps()

    def updateDisks(self):
        diskcounters = psutil.disk_io_counters(perdisk=True, nowrap=True)
        for device, activity in self._diskActivities.items():
            activity.update(diskcounters[device])
        return self._diskActivities

    def updateCPUs(self):
        cputimes = psutil.cpu_times(percpu=True)
        for activity in self._cpuActivities:
            activity.update(cputimes)
        return self._cpuActivities

    def updateCPUTemps(self):
        coretemps = psutil.sensors_temperatures()['coretemp']
        for activity in self._cpuActivities:
            activity.updateTemp(coretemps)
        return self._cpuActivities
