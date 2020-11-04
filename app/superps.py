#!/usr/bin/env python3

import psutil


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
            self.busy = 0
            self.io = 0

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

    # disks is a list of /dev/ file names e.g. 'sda', 'md0'
    def __init__(self, disks):
        self._diskActivities = {dev: self.DiskActivity() for dev in disks}
        self._cpuActivities = [self.CPUActivity(i) for i in range(8)]

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
