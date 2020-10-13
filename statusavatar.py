#!/usr/bin/env python3

import time
import serial
import psutil


# wacky functions mapping [0.0, 1.0] to [0, 255]
class EightBitCurve:

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
        return max(0, min(255, int(y)))


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

    heatcurve = EightBitCurve()
    heatcurve.addPoint(0.0, 1)
    heatcurve.addPoint(0.1, 5)
    heatcurve.addPoint(0.2, 10)
    heatcurve.addPoint(0.3, 20)
    heatcurve.addPoint(0.4, 40)
    heatcurve.addPoint(0.5, 70)
    heatcurve.addPoint(0.9, 120)
    heatcurve.addPoint(1.0, 255)

    def __init__(self):
        self.blue = 0
        self.heat = 0
        self.longbusy = 0
        self.io = False

    def update(self, cpuactivity):
        if cpuactivity.busy >= self.heat:
            self.heat += (0.05 * (cpuactivity.busy - self.heat)) ** 2
        if self.heat < 0:
            self.heat = 0
        if self.heat > 1:
            self.heat = 1
        self.heat *= 0.999 - 0.04 * self.heat

        self.blue = self.blue * 0.3
        if cpuactivity.busy > self.blue:
            self.blue = 0.5 * self.blue + 0.5 * cpuactivity.busy

        self.longbusy = 0.999 * self.longbusy + 0.001 * cpuactivity.busy

        self.io = cpuactivity.io > 0.1

    def set_led(self, led):
        led.r = self.heatcurve.sample(self.heat * 8)
        led.g = 60 if self.io else 0
        led.b = 1 + int(self.blue ** 2 * 250)


class AvatarDevice:

    class RGBWled:
        def __init__(self):
            self.r = 0
            self.g = 0
            self.b = 0
            self.w = 0

        def extend_packet(self, packet):
            packet.extend([self.r, self.g, self.b, self.w])

    class RGBled:
        def __init__(self):
            self.r = 0
            self.g = 0
            self.b = 0

        def extend_packet(self, packet):
            packet.extend([self.r, self.g, self.b])

    def __init__(self, serialport):
        self._serial = serial.Serial(serialport, 115200)
        self.cpus = [self.RGBWled() for _ in range(8)]
        self.raid = [self.RGBled() for _ in range(4)]
        self.aux = [self.RGBled() for _ in range(2)]
        self.lamps = [self.RGBled() for _ in range(2)]

    def __del__(self):
        self._serial.close()

    def update(self):
        packet = [ord(c) for c in 'xx']
        for rbgw in self.cpus:
            rbgw.extend_packet(packet)
        for rgb in self.lamps:
            rgb.extend_packet(packet)
        for rgb in self.aux:
            rgb.extend_packet(packet)
        for rgb in self.raid:
            rgb.extend_packet(packet)
        self._serial.write(packet)
        self._serial.flush()


def main():
    avatar = AvatarDevice('/dev/ttyUSB0')

    avatar.lamps[0].r = avatar.lamps[1].r = 30
    avatar.lamps[0].g = avatar.lamps[1].g = 15
    avatar.lamps[0].b = avatar.lamps[1].b = 25

    raiddevices = ['sdb', 'sdc', 'sdd', 'sde']
    rootdevice = 'sda'

    cpus = [CPUActivity(i) for i in range(8)]
    disks = {device: DiskActivity() for device in ['sda', 'sdb', 'sdc', 'sdd', 'sde']}

    cpuindicators = [CPUIndicator() for _ in cpus]

    frame = 0
    while True:
        frame += 1

        cputimes = psutil.cpu_times(percpu=True)
        for cpu, indicator, led in zip(cpus, cpuindicators, avatar.cpus):
            cpu.update(cputimes)
            indicator.update(cpu)
            indicator.set_led(led)

        diskio = psutil.disk_io_counters(perdisk=True, nowrap=True)
        for device, activity in disks.items():
            activity.update(diskio[device])

        for device, raid_led in zip(raiddevices, avatar.raid):
            activity = disks[device]
            raid_led.r = 0
            raid_led.g = 2
            raid_led.b = 0
            if activity.bytesread > 0 or activity.byteswritten > 0:
                raid_led.g = 30
            if activity.byteswritten > 0:
                raid_led.r = 5

        activity = disks[rootdevice]
        for led in avatar.aux:
            led.r = 0
            led.g = 0
            led.b = 5

        if activity.bytesread > 0:
            avatar.aux[frame % 2].b = 35
        if activity.byteswritten > 0:
            avatar.aux[1 - frame % 2].r = 20

        avatar.update()
        time.sleep(0.01)
        for i in range(4):
            led = avatar.raid[i]
            led.g = 2
        for led in avatar.aux:
            led.r = 0
            led.g = 0
            led.b = 5
        avatar.update()

        time.sleep(0.04)

    return 0


if __name__ == '__main__':
    main()
