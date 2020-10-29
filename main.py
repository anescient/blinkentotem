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
        self.velocity = 0  # nominally in [0.0, 1.0]
        self.laggingvelocity = 0
        self.heat = 0
        self.io = 0

    def update(self, cpuactivity):
        if cpuactivity.busy > 0.2:
            t = (cpuactivity.busy - 0.3) / 0.7
            if t >= self.heat:
                self.heat += (0.05 * (t - self.heat)) ** 2
        if self.heat < 0:
            self.heat = 0
        if self.heat > 1:
            self.heat = 1
        self.heat *= 0.999 - 0.04 * self.heat
        self.io = cpuactivity.io

        self.velocity = 0.7 * self.velocity + 0.3 * cpuactivity.busy
        self.laggingvelocity = 0.9 * self.laggingvelocity + 0.1 * self.velocity

    def set_led(self, led):
        led.r = unitToByte(self.heatcurve.sample(self.heat * 8))
        led.g = unitToByte(0.5 * self.io ** 2)

    def set_spinner(self, spin):
        spin.frequency = max(1, unitToByte(0.1 + 0.9 * self.velocity))
        spin.brightness = max(4, unitToByte(0.1 + 0.9 * self.laggingvelocity))


def main():
    totem = Totem()

    totem.lamps[0].r = totem.lamps[1].r = 30
    totem.lamps[0].g = totem.lamps[1].g = 15
    totem.lamps[0].b = totem.lamps[1].b = 25
    totem.update()

    raiddevices = ['sdb', 'sdc', 'sdd', 'sde']
    rootdevice = 'sda'

    cpuactivites = [CPUActivity(i) for i in range(8)]
    diskactivities = {device: DiskActivity() for device in [rootdevice] + raiddevices}

    cpuindicators = [CPUIndicator() for _ in cpuactivites]

    frame = 0
    while True:
        frame += 1

        cputimes = psutil.cpu_times(percpu=True)
        for activity in cpuactivites:
            activity.update(cputimes)

        diskcounters = psutil.disk_io_counters(perdisk=True, nowrap=True)
        for device, activity in diskactivities.items():
            activity.update(diskcounters[device])

        for activity, indicator in zip(cpuactivites, cpuindicators):
            indicator.update(activity)

        for indicator, led, spin in zip(cpuindicators, totem.cpus, totem.spinners):
            indicator.set_led(led)
            indicator.set_spinner(spin)

        for device, led in zip(raiddevices, totem.raid):
            activity = diskactivities[device]
            led.setrgb(0, 2, 0)
            if activity.bytesread > 0 or activity.byteswritten > 0:
                led.g = 30
            if activity.byteswritten > 0:
                led.r = 5

        activity = diskactivities[rootdevice]
        led1, led2 = totem.aux
        led1.setrgb(28, 33, 15)
        led2.setrgb(15, 17, 4)
        led1.push()
        led2.push()

        a, b = (led1, led2) if frame % 2 else (led2, led1)
        if activity.bytesread > 0:
            a.setrgb(0, 255, 0)
            b.g = 0
        if activity.byteswritten > 0:
            b.setrgb(255, 0, 0)
            a.r = 0

        totem.update()
        time.sleep(0.001)
        led1.pop()
        led2.pop()
        totem.update()

        time.sleep(0.01)

        led1.setrgb(28, 33, 15)
        led2.setrgb(15, 17, 7)
        for led in totem.raid:
            led.g = 2
        totem.update()

        time.sleep(0.04)

    return 0


if __name__ == '__main__':
    main()