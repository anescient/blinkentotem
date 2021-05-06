#!/usr/bin/env python3

import time

from blinkentotem import Totem, unitToByte
from superps import SystemActivity


class CPUIndicator:
    def __init__(self, cpuindex, led, personality):
        self._cpuindex = cpuindex
        self._led = led
        self._fakeheat = 0
        self._frequency = 0
        self._brightness = 0
        self._personality = personality

    def update(self, cpuactivities):
        cpu = cpuactivities[self._cpuindex]
        busy = cpu.busy
        heat = cpu.thermal.relativeValue

        if busy > self._fakeheat:
            self._fakeheat += 0.01 * (busy - self._fakeheat)
        if busy < 0.2:
            cooling = 0.95 + 0.03 * self._personality
            self._fakeheat *= cooling + 0.03 * (busy / 0.2)
        if heat is None:
            heat = self._fakeheat

        self._led.r_fade.targetvalue = max(30, unitToByte(heat))

        self._led.w_fade.targetvalue = 0
        if heat > 0.8 and busy > 0.8:
            self._led.w_fade.targetvalue = unitToByte((busy - 0.8) / 0.2)

        io = cpu.io if cpu.io > 0.05 else 0
        self._led.g_fade.targetvalue = unitToByte(io)

        self._frequency = 0.8 * self._frequency + 0.2 * busy

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

    def update(self, diskactivity, blue):
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
            unitToByte(x * 0.15 + blue))
        self._totem.drum[1].setrgb(
            unitToByte(x * 0.15),
            unitToByte(x * 0.15),
            unitToByte(x * 0.06 + blue))


class MixedClocks:
    class TickClock:
        def __init__(self, ticksper):
            self._ticksPer = ticksper
            self._ticks = 0

        def tick(self, _):
            self._ticks += 1

        def popTock(self):
            if self._ticks >= self._ticksPer:
                self._ticks -= self._ticksPer
                return True
            return False

    class IntervalClock:
        def __init__(self, interval):
            self._interval = interval
            self._seconds = 0

        def tick(self, seconds):
            self._seconds += seconds

        def popTock(self):
            if self._seconds >= self._interval:
                self._seconds -= self._interval
                return True
            return False

    def __init__(self):
        self._clocks = []

    def getTickClock(self, ticksper):
        clock = MixedClocks.TickClock(ticksper)
        self._clocks.append(clock)
        return clock

    def getIntervalClock(self, interval):
        clock = MixedClocks.IntervalClock(interval)
        self._clocks.append(clock)
        return clock

    def tickSleep(self, seconds):
        for clock in self._clocks:
            clock.tick(seconds)
        time.sleep(seconds)


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
        led.g = 6
    totem.pushPieces()

    raidDevices = ['sdb', 'sdc', 'sdd', 'sde']
    rootDevice = 'sda'

    systemActivity = SystemActivity([rootDevice] + raidDevices)
    cpuIndicators = [CPUIndicator((i * 4 + i // 2) % 8, totem.rgbw[i], i / 8) for i in range(8)]
    raidIndicators = [RaidDiskIndicator(pulse) for pulse in totem.raidpulse]
    rootIndicator = DrumIndicator(totem)

    systemActivity.updateAll()

    clocks = MixedClocks()
    clockCPU = clocks.getTickClock(2)
    clockCPUTemp = clocks.getIntervalClock(1.0)
    while True:
        clocks.tickSleep(0.07)

        if clockCPU.popTock():
            cpus = systemActivity.updateCPUs()
            if clockCPUTemp.popTock():
                cpus = systemActivity.updateCPUTemps()
            for indicator in cpuIndicators:
                indicator.update(cpus)

        disks = systemActivity.updateDisks()
        for device, indicator in zip(raidDevices, raidIndicators):
            indicator.update(disks[device])
        swapping = 1 if systemActivity.swapActivity.getBytesSince() > 0 else 0
        rootIndicator.update(disks[rootDevice], swapping)

        totem.pushPieces()

    # noinspection PyUnreachableCode
    return 0


if __name__ == '__main__':
    main()
