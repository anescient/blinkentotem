#!/usr/bin/env python3

from collections import namedtuple
import time
import serial


class Totem:

    _leadin = ord('$')

    class _Endpoint:
        def __init__(self, addr):
            self._addr = addr
            self.dirty = False
            self._data = None

        def updateRaw(self, data):
            self.dirty |= data != self._data
            self._data = data

        def update(self, items):
            data = []
            for item in items:
                data.extend(item.getPayload())
            self.updateRaw(data)

        def write(self, _serial):
            if self._data is None:
                self._data = []
            _serial.write([Totem._leadin, ord(self._addr), 0] + self._data)
            self.dirty = False

    class _PulseEndpoint(_Endpoint):
        def __init__(self, addr):
            super().__init__(addr)

        def update(self, items):
            if any(item.nonzero() for item in items):
                super().update(items)
                self.dirty = True
            for item in items:
                item.clear()

    class RGBWled:
        def __init__(self):
            self.r = 0
            self.g = 0
            self.b = 0
            self.w = 0

        def clear(self):
            self.r = self.g = self.b = self.w = 0

        def getPayload(self):
            return [self.r, self.g, self.b, self.w]

    class RGBled:
        def __init__(self):
            self.r = 0
            self.g = 0
            self.b = 0

        def clear(self):
            self.r = self.g = self.b = 0

        def setrgb(self, r, g, b):
            self.r, self.g, self.b = r, g, b

        def getPayload(self):
            return [self.r, self.g, self.b]

    class Spinner:
        def __init__(self):
            self.frequency = 0
            self.brightness = 0

        def getPayload(self):
            return [self.frequency, self.brightness]

    class SlowGlow:
        def __init__(self):
            self.targetvalue = 0

        def getPayload(self):
            return [self.targetvalue]

    class Fader:
        def __init__(self):
            self.pumpvalue = 0
            self.decayrate = 0

        def nonzero(self):
            return self.pumpvalue > 0

        def clear(self):
            self.pumpvalue = 0

        def getPayload(self):
            return [self.pumpvalue, self.decayrate]

    class IOPulse:
        def __init__(self):
            self.read = 0
            self.write = 0

        def nonzero(self):
            return self.read > 0 or self.write > 0

        def clear(self):
            self.read = 0
            self.write = 0

        def getPayload(self):
            return [min(255, self.read),
                    min(255, self.write)]

    class Configuration:
        def __init__(self):
            self.raidRed = 30
            self.raidGreen = 100
            self.drumRed = 150
            self.drumGreen = 200

        def getPayload(self):
            return [self.raidRed,
                    self.raidGreen,
                    self.drumRed,
                    self.drumGreen]

    def __init__(self, serialport='/dev/ttyUSB0'):
        self._serial = serial.Serial(serialport, 56000)
        self.config = self.Configuration()
        self.rgbw = [self.RGBWled() for _ in range(8)]
        self.bluespins = [self.Spinner() for _ in self.rgbw]
        self.greenfades = [self.Fader() for _ in self.rgbw]
        self.whitefades = [self.Fader() for _ in self.rgbw]
        self.redglows = [self.SlowGlow() for _ in self.rgbw]
        self.rgb = [self.RGBled() for _ in range(8)]
        self.raid = self.rgb[4:8]
        self.drum = self.rgb[2:4]
        self.lamps = self.rgb[0:2]
        self.raidpulse = [self.IOPulse() for _ in self.raid]
        self.drumpulse = self.IOPulse()

        self._ep = namedtuple(
            'Endpoints',
            'rgb rgbw \
            bluespins greenfades whitefades redglows \
            raid drum lamps raidpulse drumpulse')
        self._ep.rgb = self._Endpoint('1')
        self._ep.rgbw = self._Endpoint('2')
        self._ep.bluespins = self._Endpoint('s')
        self._ep.greenfades = self._PulseEndpoint('i')
        self._ep.whitefades = self._PulseEndpoint('z')
        self._ep.redglows = self._Endpoint('g')
        self._ep.raid = self._Endpoint('r')
        self._ep.drum = self._Endpoint('d')
        self._ep.lamps = self._Endpoint('l')
        self._ep.raidpulse = self._PulseEndpoint('f')
        self._ep.drumpulse = self._PulseEndpoint('m')

        # arduino resets when comms begin
        # gotta wait for the bootloader to timeout and run the rom
        self.ping()
        time.sleep(0.4)
        self.pushFrames(True)

    def __del__(self):
        self._serial.close()

    # WDT reset
    def ping(self):
        self._serial.write([self._leadin, ord(' '), 0])
        self._serial.flush()

    def flush(self):
        self._serial.write([self._leadin, ord(';'), 0])
        self._serial.flush()

    def pushConfig(self):
        self._serial.write([self._leadin, ord('p'), 0] + self.config.getPayload())
        self._serial.flush()

    def pushFrames(self, force=False):
        self._ep.rgb.update(self.rgb)
        self._ep.rgbw.update(self.rgbw)
        updated = False
        for ep in [self._ep.rgb, self._ep.rgbw]:
            if ep.dirty or force:
                ep.write(self._serial)
                self._serial.flush()
                updated = True
        return updated

    def pushPieces(self, force=False):
        self._ep.bluespins.update(self.bluespins)
        self._ep.greenfades.update(self.greenfades)
        self._ep.whitefades.update(self.whitefades)
        self._ep.redglows.update(self.redglows)

        self._ep.rgbw.update(self.rgbw)

        self._ep.raid.update(self.raid)
        self._ep.drum.update(self.drum)
        self._ep.lamps.update(self.lamps)

        self._ep.raidpulse.update(self.raidpulse)
        self._ep.drumpulse.update([self.drumpulse])
        updated = False
        for ep in [self._ep.bluespins,
                   self._ep.greenfades, self._ep.whitefades, self._ep.redglows,
                   self._ep.rgbw,
                   self._ep.raid, self._ep.drum, self._ep.lamps,
                   self._ep.raidpulse, self._ep.drumpulse]:
            if ep.dirty or force:
                ep.write(self._serial)
                self._serial.flush()
                updated = True
        return updated
