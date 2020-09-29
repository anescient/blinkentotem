#!/usr/bin/env python3

import serial
import time
import random
import psutil


# wacky functions mapping [0.0, 1.0] to [0, 255]
class EightBitCurve:
    pass


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
        def _extendPacket(self, packet):
            packet.extend([self.r, self.g, self.b, self.w])


    class RGBled:
        def __init__(self):
            self.r = 0
            self.g = 0
            self.b = 0
        def _extendPacket(self, packet):
            packet.extend([self.r, self.g, self.b])
    
    def __init__(self, serialport):
        self._serial = serial.Serial(serialport, 115200)
        self.cpu_leds = [self.RGBWled() for _ in range(8)]
        self.raid_leds = [self.RGBled() for _ in range(4)]
        self.sda_led = self.RGBled()
        self.lamps = [self.RGBled() for _ in range(2)]

    def __del__(self):
        self._serial.close()

    def update(self):
        packet = [ord(c) for c in 'xx']
        for rbgw in self.cpu_leds:
            rbgw._extendPacket(packet)
        for rgb in self.lamps:
            rgb._extendPacket(packet)
        self.sda_led._extendPacket(packet)
        for rgb in self.raid_leds:
            rgb._extendPacket(packet)
        self._serial.write(packet)
        self._serial.flush()


def main():
    
    avatar = AvatarDevice('/dev/ttyUSB0')
    
    avatar.lamps[0].r = 10
    avatar.lamps[0].b = 30
    avatar.lamps[1].r = 10
    avatar.lamps[1].b = 30

    raiddevices = ['sdb', 'sdc', 'sdd', 'sde']
    rootdevice = 'sda'

    loads = [CPULoad() for _ in range(8)]
    disks = {device: DiskActivity() for device in ['sda', 'sdb', 'sdc', 'sdd', 'sde']}

    redi = 0

    frame = 0
    while(True):
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

        rr = [0, 1, 2, 5, 12, 20, 50, 120, 255, 255]

        for load, cpu_led in zip(loads, avatar.cpu_leds):
            cpu_led.r = rr[int(load.heat * 8)]
            cpu_led.g = 0 if load.io < 0.1 else 60
            cpu_led.b = 1 + int(load.blue ** 2 * 250)

        for device, raid_led in zip(raiddevices, avatar.raid_leds):
            activity = disks[device]
            raid_led.r = 0
            raid_led.g = 2
            raid_led.b = 0
            if activity.bytesread > 0 or activity.byteswritten > 0:
                raid_led.g = 30
            if activity.byteswritten > 0:
                raid_led.r = 5

        activity = disks[rootdevice]
        led = avatar.sda_led
        led.r = 0
        led.g = 0
        led.b = 0
        if activity.bytesread > 0:
            led.g = 70
        if activity.byteswritten > 0:
            led.r = 30

        avatar.update()
        time.sleep(0.01)
        for i in range(4):
            led = avatar.raid_leds[i]
            led.g = 2
        avatar.update()
            
        time.sleep(0.04)

    return 0


if __name__ == '__main__':
    main()
