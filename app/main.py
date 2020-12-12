#!/usr/bin/env python3

import time

from blinkentotem import Totem, unitToByte
from superps import SystemActivity


class CPUIndicator:

    def __init__(self, cpuindex, totem, personality):
        self._cpuindex = cpuindex
        self._led = totem.rgbw[cpuindex]
        self._heat = 0
        self._frequency = 0
        self._brightness = 0
        self._personality = personality

    def update(self, cpuactivity):
        busy = cpuactivity.busy

        if busy > self._heat:
            self._heat += 0.02 * (busy - self._heat)
        if busy < 0.2:
            cooling = 0.95 + 0.03 * self._personality
            self._heat *= cooling + 0.03 * (busy / 0.2)
        self._led.r_fade.targetvalue = max(30, unitToByte(self._heat))

        self._led.w_fade.targetvalue = 0
        if busy > 0.9:
            self._led.w_fade.targetvalue = unitToByte((busy - 0.9) / 0.1)

        io = cpuactivity.io if cpuactivity.io > 0.05 else 0
        self._led.g_fade.targetvalue = unitToByte(io)

        self._frequency = 0.5 * self._frequency + 0.5 * busy

        if busy > self._brightness:
            self._brightness = 0.8 * self._brightness + 0.2 * busy
        else:
            self._brightness = 0.2 * self._brightness + 0.8 * busy

        freq = 0.6 * self._frequency + 0.05 * self._personality
        self._led.b_spin.frequency = max(1, unitToByte(freq - 0.3))
        self._led.b_spin.brightness = max(20, unitToByte(0.2 + 0.6 * self._brightness))


class RaidDiskIndicator:
    def __init__(self, pulse):
        self._divisor = 250000
        self._pulse = pulse

    def update(self, diskactivity):
        read, written = diskactivity.bytesread, diskactivity.byteswritten
        total = read + written
        if total == 0:
            return
        read = read / total
        written = written / total
        if written > 0.9:
            read, written = 0.1, 0.9

        x = max(1, total // self._divisor)
        self._pulse.read = min(100, int(read * x))
        self._pulse.write = min(70, int(written * x))


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
    totem.clear()
    totem.config.raidRed = 20
    totem.config.raidGreen = 120
    totem.config.drumRed = 150
    totem.config.drumGreen = 200
    totem.pushConfig()

    for led in totem.lamps:
        led.setrgb(80, 40, 50)
    for led in totem.raid:
        led.g = 10
    totem.pushPieces()

    raidDevices = ['sdb', 'sdc', 'sdd', 'sde']
    rootDevice = 'sda'

    systemActivity = SystemActivity([rootDevice] + raidDevices)
    cpuIndicators = [CPUIndicator(i, totem, i / 8) for i in range(8)]
    raidIndicators = [RaidDiskIndicator(pulse) for pulse in totem.raidpulse]
    rootIndicator = DrumIndicator(totem)

    frame = 0
    while True:
        frame += 1

        if frame % 2 == 0:
            cpus = systemActivity.updateCPUs()
            for activity, indicator in zip(cpus, cpuIndicators):
                indicator.update(activity)

        disks = systemActivity.updateDisks()
        for device, indicator in zip(raidDevices, raidIndicators):
            indicator.update(disks[device])
        rootIndicator.update(disks[rootDevice])

        totem.pushPieces()
        time.sleep(0.07)

    # noinspection PyUnreachableCode
    return 0


if __name__ == '__main__':
    main()
