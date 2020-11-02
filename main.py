#!/usr/bin/env python3

import time
import psutil

from blinkentotem import Totem


# mapping [0.0, 1.0] to [0.0, 1.0]
class UnitCurve:

    class _Point:
        def __init__(self, x, y):
            self.x = float(x)
            self.y = float(y)

    def __init__(self):
        self.points = []

    def addPoint(self, x, fofx):
        self.points.append(self._Point(x, fofx))
        self.points.sort(key=lambda p: p.x)

    def sample(self, x):
        assert len(self.points) >= 2
        if x <= self.points[0].x:
            return int(self.points[0].y)
        if x >= self.points[-1].x:
            return int(self.points[-1].y)

        lowpoint = self.points[0]
        highpoint = None
        for point in self.points[1:]:
            if point.x > x:
                highpoint = point
                break
            lowpoint = point
        assert lowpoint.x <= x < highpoint.x

        segment = (x - lowpoint.x) / (highpoint.x - lowpoint.x)
        assert 0.0 <= segment <= 1.0
        y = (1.0 - segment) * lowpoint.y + segment * highpoint.y
        return max(0.0, min(1.0, y))


def unitToByte(x):
    if x <= 0:
        return 0
    return max(1, min(255, int(256.0 * x)))


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


class CPUIndicator:

    heatcurve = UnitCurve()
    heatcurve.addPoint(0.0, 0.004)
    heatcurve.addPoint(0.1, 0.02)
    heatcurve.addPoint(0.2, 0.04)
    heatcurve.addPoint(0.3, 0.08)
    heatcurve.addPoint(0.4, 0.16)
    heatcurve.addPoint(0.5, 0.25)
    heatcurve.addPoint(0.9, 0.5)
    heatcurve.addPoint(1.0, 1.0)

    def __init__(self):
        self.busy = 0
        self.io = 0
        self.heat = 0
        self.busyLagA = 0
        self.busyLagB = 0

    def update(self, cpuactivity):
        self.busy = cpuactivity.busy
        self.io = cpuactivity.io

    def step(self):
        if self.busy > 0.2:
            t = (self.busy - 0.3) / 0.7
            if t >= self.heat:
                self.heat += (0.05 * (t - self.heat)) ** 2
        if self.heat < 0:
            self.heat = 0
        if self.heat > 1:
            self.heat = 1
        self.heat *= 0.999 - 0.04 * self.heat

        if self.busy > self.busyLagA:
            self.busyLagA = 0.6 * self.busyLagA + 0.4 * self.busy
        else:
            self.busyLagA = 0.2 * self.busyLagA + 0.8 * self.busy

        if self.busy > self.busyLagB:
            self.busyLagB = 0.8 * self.busyLagB + 0.2 * self.busy
        else:
            self.busyLagB = 0.2 * self.busyLagB + 0.8 * self.busy

    def set_led(self, led):
        led.r = unitToByte(self.heatcurve.sample(self.heat * 8))
        led.g = unitToByte(0.5 * self.io ** 2)

    def set_spinner(self, spin):
        spin.frequency = max(3, unitToByte(0.9 * self.busyLagA - 0.2))
        spin.brightness = max(30, unitToByte(0.2 + 0.7 * self.busyLagB))


def main():
    totem = Totem()
    totem.config.maxPulse = 40
    totem.config.raidRed = 30
    totem.config.raidGreen = 100
    totem.config.drumRed = 150
    totem.config.drumGreen = 200
    totem.pushConfig()

    for led in totem.lamps:
        led.setrgb(30, 15, 25)
    for led in totem.raid:
        led.g = 2
    totem.drum[0].setrgb(28, 33, 15)
    totem.drum[1].setrgb(15, 17, 7)

    totem.pushPieces()

    raiddevices = ['sdb', 'sdc', 'sdd', 'sde']
    rootdevice = 'sda'
    raidDivisor = 150000
    rootDivisor = 800000

    cpuactivities = [CPUActivity(i) for i in range(8)]
    diskactivities = {device: DiskActivity() for device in [rootdevice] + raiddevices}

    cpuindicators = [CPUIndicator() for _ in cpuactivities]

    frame = 0
    while True:
        frame += 1

        if frame % 3 == 0:
            cputimes = psutil.cpu_times(percpu=True)
            for activity in cpuactivities:
                activity.update(cputimes)

            for activity, indicator in zip(cpuactivities, cpuindicators):
                indicator.update(activity)
                indicator.step()

            for indicator, led, spin in zip(cpuindicators, totem.rgbw, totem.spinners):
                indicator.set_led(led)
                indicator.set_spinner(spin)

        diskcounters = psutil.disk_io_counters(perdisk=True, nowrap=True)
        for device, activity in diskactivities.items():
            activity.update(diskcounters[device])

        for device, pulse in zip(raiddevices, totem.raidpulse):
            activity = diskactivities[device]
            if activity.bytesread > 0:
                pulse.read = max(1, min(70, activity.bytesread // raidDivisor))
            if activity.byteswritten > 0:
                pulse.write = max(1, min(70, activity.byteswritten // raidDivisor))
            pulse.read = max(pulse.read, pulse.write // 2)

        activity = diskactivities[rootdevice]
        pulse = totem.drumpulse
        if activity.bytesread > 0:
            pulse.read = max(4, min(70, activity.bytesread // rootDivisor))
        if activity.byteswritten > 0:
            pulse.write = max(4, min(70, activity.byteswritten // rootDivisor))

        totem.pushPieces()
        time.sleep(0.05)

        if frame % 10 == 0:
            totem.ping()

    return 0


if __name__ == '__main__':
    main()
