#!/usr/bin/env python3

import time

from blinkentotem import Totem
from superps import SystemActivity


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
                self.heat += 0.001 * (t - self.heat)
        if self.heat < 0:
            self.heat = 0
        if self.heat > 1:
            self.heat = 1
        self.heat *= 0.99

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


class RaidDiskIndicator:
    def __init__(self):
        self._divisor = 150000

    def update(self, diskActivity, outPulse):
        read, written = diskActivity.bytesread, diskActivity.byteswritten
        if read > 0:
            outPulse.read = max(1, min(70, read // self._divisor))
        if written > 0:
            outPulse.write = max(1, min(70, written // self._divisor))
        outPulse.read = max(outPulse.read, outPulse.write // 2)


def setDrumWhite(totem, x):
    totem.drum[0].setrgb(
        unitToByte(x * 0.28),
        unitToByte(x * 0.33),
        unitToByte(x * 0.15))
    totem.drum[1].setrgb(
        unitToByte(x * 0.15),
        unitToByte(x * 0.15),
        unitToByte(x * 0.06))


def main():
    totem = Totem()
    totem.config.maxPulse = 40
    totem.config.raidRed = 30
    totem.config.raidGreen = 100
    totem.config.drumRed = 150
    totem.config.drumGreen = 200
    totem.pushConfig()

    for led in totem.lamps:
        led.setrgb(20, 8, 14)
    for led in totem.raid:
        led.g = 2

    drumBrightness = 0.5
    setDrumWhite(totem, drumBrightness)

    totem.pushPieces()

    raidDevices = ['sdb', 'sdc', 'sdd', 'sde']
    rootDevice = 'sda'
    rootDivisor = 800000

    systemActivity = SystemActivity([rootDevice] + raidDevices)
    cpuIndicators = [CPUIndicator() for _ in range(8)]
    raidIndicators = [RaidDiskIndicator() for _ in raidDevices]

    frame = 0
    while True:
        frame += 1

        if frame % 3 == 0:
            cpus = systemActivity.updateCPUs()
            for activity, indicator, led, spin in zip(
                    cpus, cpuIndicators, totem.rgbw, totem.spinners):
                indicator.update(activity)
                indicator.step()
                indicator.set_led(led)
                indicator.set_spinner(spin)

        disks = systemActivity.updateDisks()

        for device, indicator, pulse in zip(
                raidDevices, raidIndicators, totem.raidpulse):
            activity = disks[device]
            indicator.update(activity, pulse)

        activity = disks[rootDevice]
        pulse = totem.drumpulse
        if activity.bytesread > 0:
            pulse.read = max(4, min(70, activity.bytesread // rootDivisor))
        if activity.byteswritten > 0:
            pulse.write = max(4, min(70, activity.byteswritten // rootDivisor))
        if activity.bytesread or activity.byteswritten:
            drumBrightness = max(0.3, drumBrightness - 0.06)
        else:
            drumBrightness = min(1.0, drumBrightness + 0.02)
        setDrumWhite(totem, min(0.5, drumBrightness))

        totem.pushPieces()
        time.sleep(0.05)

        if frame % 10 == 0:
            totem.ping()

    # noinspection PyUnreachableCode
    return 0


if __name__ == '__main__':
    main()
