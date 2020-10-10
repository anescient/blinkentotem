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
class CPULoad:

    def __init__(self):
        self._prevscputimes = None
        self.busy = 0
        self.io = 0
        self.blue = 0
        self.heat = 0
        self.longbusy = 0

    def update(self, scputimes):
        if self._prevscputimes is None:
            self._prevscputimes = scputimes
        prevsum = sum(self._prevscputimes)
        nextsum = sum(scputimes)
        dt = nextsum - prevsum
        if dt < 0.02:
            return

        idletime = scputimes.idle - self._prevscputimes.idle
        iotime = scputimes.iowait - self._prevscputimes.iowait
        busytime = dt - idletime - iotime
        self._prevscputimes = scputimes

        self.busy = busytime / dt
        self.io = iotime / dt

        if self.busy >= self.heat:
            self.heat = 0.95 * self.heat + 0.05 * self.busy
        else:
            self.heat *= 0.99
        if self.heat < 0:
            self.heat = 0
        if self.heat > 1:
            self.heat = 1

        self.blue = self.blue * 0.3
        if self.busy > self.blue:
            self.blue = 0.5 * self.blue + 0.5 * self.busy

        self.longbusy = 0.999 * self.longbusy + 0.001 * self.busy


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

    loads = [CPULoad() for _ in range(8)]
    disks = {device: DiskActivity() for device in ['sda', 'sdb', 'sdc', 'sdd', 'sde']}

    redi = 0

    heatcurve = EightBitCurve()
    heatcurve.addPoint(0.0, 0)
    heatcurve.addPoint(0.2, 0)
    heatcurve.addPoint(0.8, 10)
    heatcurve.addPoint(1.0, 255)

    frame = 0
    while True:
        frame += 1

        cputimes = psutil.cpu_times(percpu=True)
        for i in range(8):
            loads[i].update(cputimes[i])

        redi += 0.02
        for load in loads:
            redi += 0.3 * load.busy
        redi %= 8

        diskio = psutil.disk_io_counters(perdisk=True, nowrap=True)
        for device, activity in disks.items():
            activity.update(diskio[device])

        for load, cpu_led in zip(loads, avatar.cpus):
            cpu_led.r = heatcurve.sample(load.heat * 8)
            cpu_led.g = 0 if load.io < 0.1 else 60
            cpu_led.b = 1 + int(load.blue ** 2 * 250)

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
