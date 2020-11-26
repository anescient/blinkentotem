#!/usr/bin/env python3

from collections import namedtuple
import time
import serial


def unitToByte(x):
    if x <= 0:
        return 0
    return max(1, min(255, int(256.0 * x)))


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
            _serial.write([Totem._leadin, ord(self._addr)] + self._data)
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
            self.r_fade = Totem.Fader()
            self.g = 0
            self.g_fade = Totem.Fader()
            self.b = 0
            self.b_spin = Totem.Spinner()
            self.w = 0
            self.w_fade = Totem.Fader()

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

    class Fader:
        def __init__(self):
            self.targetvalue = 0
            self.uprate = 10
            self.downrate = 10

        def getPayload(self):
            return [self.targetvalue,
                    self.uprate,
                    self.downrate]

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
        self._r_fades = [led.r_fade for led in self.rgbw]
        self._g_fades = [led.g_fade for led in self.rgbw]
        self._b_spins = [led.b_spin for led in self.rgbw]
        self._w_fades = [led.w_fade for led in self.rgbw]

        for fade in self._g_fades:
            fade.uprate = 255
            fade.downrate = 50
        for fade in self._r_fades:
            fade.uprate = 10
            fade.downrate = 2
        for fade in self._w_fades:
            fade.uprate = 5
            fade.downrate = 20

        self.rgb = [self.RGBled() for _ in range(8)]
        self.raid = self.rgb[4:8]
        self.drum = self.rgb[2:4]
        self.lamps = self.rgb[0:2]
        self.raidpulse = [self.IOPulse() for _ in self.raid]
        self.drumpulse = self.IOPulse()

        self._ep = namedtuple(
            'Endpoints',
            'rgb rgbw \
            r_fades g_fades b_spins w_fades \
            raid drum lamps raidpulse drumpulse')
        self._ep.rgb = self._Endpoint('1')
        self._ep.rgbw = self._Endpoint('2')
        self._ep.r_fades = self._Endpoint('g')
        self._ep.g_fades = self._Endpoint('i')
        self._ep.b_spins = self._Endpoint('s')
        self._ep.w_fades = self._Endpoint('z')
        self._ep.raid = self._Endpoint('r')
        self._ep.drum = self._Endpoint('d')
        self._ep.lamps = self._Endpoint('l')
        self._ep.raidpulse = self._PulseEndpoint('f')
        self._ep.drumpulse = self._PulseEndpoint('m')

        self._pushEndpoints = [
            self._ep.r_fades, self._ep.g_fades, self._ep.b_spins,
            self._ep.w_fades, self._ep.rgbw,
            self._ep.raid, self._ep.drum, self._ep.lamps,
            self._ep.raidpulse, self._ep.drumpulse]

        # arduino resets when comms begin
        # gotta wait for the bootloader to timeout and run the rom
        self.ping()
        time.sleep(0.5)
        self.pushFrames(True)

    def __del__(self):
        self._serial.close()

    # WDT reset
    def ping(self):
        self._serial.write([self._leadin, ord(' ')])
        self._serial.flush()

    def clear(self):
        self._serial.write([self._leadin, ord('c')])
        self._serial.flush()

    def flush(self):
        self._serial.write([self._leadin, ord(';')])
        self._serial.flush()

    def pushConfig(self):
        self._serial.write([self._leadin, ord('p')] + self.config.getPayload())
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
        if updated:
            self.flush()
        return updated

    def pushPieces(self, force=False):
        self._ep.r_fades.update(self._r_fades)
        self._ep.g_fades.update(self._g_fades)
        self._ep.b_spins.update(self._b_spins)
        self._ep.w_fades.update(self._w_fades)

        self._ep.rgbw.update(self.rgbw)

        self._ep.raid.update(self.raid)
        self._ep.drum.update(self.drum)
        self._ep.lamps.update(self.lamps)

        self._ep.raidpulse.update(self.raidpulse)
        self._ep.drumpulse.update([self.drumpulse])
        updated = False
        for ep in self._pushEndpoints:
            if ep.dirty or force:
                ep.write(self._serial)
                self._serial.flush()
                updated = True
        if updated:
            self.flush()
        return updated
