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

    _heatcurve = UnitCurve()
    _heatcurve.addPoint(0.0, 0.01)
    _heatcurve.addPoint(0.2, 0.05)
    _heatcurve.addPoint(0.8, 0.25)
    _heatcurve.addPoint(1.0, 1.0)

    def __init__(self, cpuindex, totem):
        self._cpuindex = cpuindex
        self._led = totem.rgbw[cpuindex]
        self._bluespin = totem.bluespins[cpuindex]
        self._greenfade = totem.greenfades[cpuindex]
        self._whitefade = totem.whitefades[cpuindex]
        self._redglow = totem.redglows[cpuindex]
        self._heat = 0
        self._frequency = 0
        self._brightness = 0

    def update(self, cpuactivity):
        busy = cpuactivity.busy

        if busy > self._heat:
            self._heat += 0.02 * (busy - self._heat)
        if busy < 0.2:
            self._heat *= 0.97 + 0.03 * (busy / 0.2)
        self._redglow.targetvalue = unitToByte(self._heatcurve.sample(self._heat))
        if self._heat > 0.8:
            self._whitefade.pumpvalue = unitToByte((self._heat - 0.8) / 0.2)
            self._whitefade.decayrate = 5

        if cpuactivity.io > 0.05:
            self._greenfade.pumpvalue = unitToByte(0.5 + 0.5 * cpuactivity.io)
            self._greenfade.decayrate = 40

        if busy > self._frequency:
            self._frequency = 0.8 * self._frequency + 0.2 * busy
        else:
            self._frequency = 0.6 * self._frequency + 0.4 * busy

        if busy > self._brightness:
            self._brightness = 0.8 * self._brightness + 0.2 * busy
        else:
            self._brightness = 0.2 * self._brightness + 0.8 * busy
        self._bluespin.frequency = max(1, unitToByte(0.8 * self._frequency - 0.2))
        self._bluespin.brightness = max(20, unitToByte(0.2 + 0.6 * self._brightness))


class RaidDiskIndicator:
    def __init__(self, pulse):
        self._divisor = 150000
        self._pulse = pulse

    def update(self, diskactivity):
        read, written = diskactivity.bytesread, diskactivity.byteswritten
        if read > 0:
            self._pulse.read = max(1, min(70, read // self._divisor))
        if written > 0:
            self._pulse.write = max(1, min(70, written // self._divisor))
        self._pulse.read = max(self._pulse.read, self._pulse.write // 2)


class DrumIndicator:
    def __init__(self, totem):
        self._ssdDivisor = 800000
        self._totem = totem

    def update(self, diskactivity):
        read, written = diskactivity.bytesread, diskactivity.byteswritten
        pulse = self._totem.drumpulse
        if read > 0:
            pulse.read = max(4, min(70, read // self._ssdDivisor))
        if written > 0:
            pulse.write = max(4, min(70, written // self._ssdDivisor))

        x = 0.5 if read or written else 0.4
        self._totem.drum[0].setrgb(
            unitToByte(x * 0.28),
            unitToByte(x * 0.33),
            unitToByte(x * 0.15))
        self._totem.drum[1].setrgb(
            unitToByte(x * 0.15),
            unitToByte(x * 0.15),
            unitToByte(x * 0.06))


def main():
    totem = Totem()
    totem.config.raidRed = 30
    totem.config.raidGreen = 100
    totem.config.drumRed = 150
    totem.config.drumGreen = 200
    totem.pushConfig()

    for led in totem.lamps:
        led.setrgb(20, 8, 14)
    for led in totem.raid:
        led.g = 4
    totem.pushPieces()

    raidDevices = ['sdb', 'sdc', 'sdd', 'sde']
    rootDevice = 'sda'

    systemActivity = SystemActivity([rootDevice] + raidDevices)
    cpuIndicators = [CPUIndicator(i, totem) for i in range(8)]
    raidIndicators = [RaidDiskIndicator(pulse) for pulse in totem.raidpulse]
    rootIndicator = DrumIndicator(totem)

    frame = 0
    while True:
        frame += 1

        if frame % 3 == 0:
            cpus = systemActivity.updateCPUs()
            for activity, indicator in zip(cpus, cpuIndicators):
                indicator.update(activity)

        disks = systemActivity.updateDisks()
        for device, indicator in zip(raidDevices, raidIndicators):
            indicator.update(disks[device])
        rootIndicator.update(disks[rootDevice])

        totem.pushPieces()
        time.sleep(0.05)

        if frame % 10 == 0:
            totem.ping()

    # noinspection PyUnreachableCode
    return 0


if __name__ == '__main__':
    main()
