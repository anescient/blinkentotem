#!/usr/bin/env python3

import time
import serial


class Totem:

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
            self._stack = []

        def setrgb(self, r, g, b):
            self.r, self.g, self.b = r, g, b

        def push(self):
            self._stack.append((self.r, self.g, self.b))

        def pop(self):
            self.r, self.g, self.b = self._stack.pop()

        def extend_packet(self, packet):
            packet.extend([self.r, self.g, self.b])

    class Spinner:
        def __init__(self):
            self.frequency = 0
            self.b_min = 0
            self.b_max = 0

        def extend_packet(self, packet):
            packet.extend([self.frequency, self.b_min, self.b_max])

    def __init__(self, serialport='/dev/ttyUSB0'):
        self._leadin = '$'
        self._serial = serial.Serial(serialport, 56000)
        self._lastrgbw = None
        self._lastrgb = None
        self._lastspins = None
        self.cpus = [self.RGBWled() for _ in range(8)]
        self.spinners = [self.Spinner() for _ in range(8)]
        self.raid = [self.RGBled() for _ in range(4)]
        self.aux = [self.RGBled() for _ in range(2)]
        self.lamps = [self.RGBled() for _ in range(2)]

        # arduino resets when comms begin
        # gotta wait for the bootloader to timeout and run the rom
        self.ping()
        time.sleep(0.4)
        self.update()

    def __del__(self):
        self._serial.close()

    def ping(self):
        self._send([ord(c) for c in self._leadin + ' \0'])

    def update(self):
        noop = True

        packet = [ord(c) for c in self._leadin + '1\0']
        for rgb in self.lamps:
            rgb.extend_packet(packet)
        for rgb in self.aux:
            rgb.extend_packet(packet)
        for rgb in self.raid:
            rgb.extend_packet(packet)
        if packet != self._lastrgb:
            self._lastrgb = packet
            self._send(packet)
            noop = False

        packet = [ord(c) for c in self._leadin + '2\0']
        for rbgw in self.cpus:
            rbgw.extend_packet(packet)
        if packet != self._lastrgbw:
            self._lastrgbw = packet
            self._send(packet)
            noop = False

        packet = [ord(c) for c in self._leadin + 's\0']
        for spin in self.spinners:
            spin.extend_packet(packet)
        if packet != self._lastspins:
            self._lastspins = packet
            self._send(packet)
            noop = False

        if noop:
            self.ping()

    def _send(self, packet):
        self._serial.write(packet)
        self._serial.flush()
