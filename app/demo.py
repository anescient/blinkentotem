#!/usr/bin/env python3

from random import random, randint, choice as randomchoice
import time

from blinkentotem import Totem


class _Demo:
    def __init__(self, totem):
        self.totem = totem
        self.framecount = 0

    def step(self):
        self.framecount += 1


class FlashyCounters(_Demo):
    def __init__(self, totem):
        super().__init__(totem)

    def step(self):
        super().step()
        for led in self.totem.rgbw:
            led.clear()
        for i in range(8):
            do_b = (self.framecount % 0xff) & (1 << i) != 0
            do_r = ((self.framecount // 3 + 73) % 0xff) & (1 << i) != 0
            if do_b:
                self.totem.rgbw[7 - i].b = 40
            if do_r:
                self.totem.rgbw[i].r = 100
        for led in self.totem.rgbw:
            if led.b and led.r and random() < 0.1:
                led.w = 250
        self.totem.rgbw[randint(0, 7)].g = 250
        self.totem.pushPieces()
        time.sleep(0.01)


class RandomMess(_Demo):
    def __init__(self, totem):
        super().__init__(totem)

    @staticmethod
    def _random_x():
        return 0 if random() < 0.7 else randint(0, 255)

    def step(self):
        super().step()
        led = randomchoice(self.totem.rgb)
        led.r = self._random_x()
        led.g = self._random_x()
        led.b = self._random_x()

        led = randomchoice(self.totem.rgbw)
        led.r = self._random_x()
        led.g = self._random_x()
        led.b = self._random_x()
        led.w = self._random_x()

        self.totem.pushPieces()

        time.sleep(0.01)


class Chasers(_Demo):
    def __init__(self, totem):
        super().__init__(totem)
        self._leds = totem.rgb + totem.rgbw
        self.redi = 0
        self.bluei = 0
        self.greeni = 0

    def step(self):
        super().step()
        for led in self._leds:
            led.clear()
        self._leds[self.redi].r = 255
        self._leds[self.bluei].b = 200
        self._leds[self.greeni].g = 200
        self.redi = (self.redi + 1) % len(self._leds)
        if random() < 0.5:
            self.bluei = (self.bluei + 1) % len(self._leds)
        if random() < 0.1:
            self.greeni = (self.greeni + 1) % len(self._leds)
        totem.pushPieces()


class FadeTest(_Demo):
    def __init__(self, totem):
        super().__init__(totem)
        self._fades = [led.r_fade for led in totem.rgbw]
        for fade in self._fades:
            fade.uprate = 100
            fade.downrate = 100

    def step(self):
        super().step()
        for fade in self._fades:
            fade.targetvalue = randint(0, 200)
        totem.pushPieces()
        time.sleep(0.7)


if __name__ == '__main__':
    totem = Totem()
    demos = [FlashyCounters(totem),
             RandomMess(totem),
             Chasers(totem),
             FadeTest(totem)]

    demos = [Chasers(totem)]

    while True:
        for demo in demos:
            if len(demos) > 1:
                totem.clear()
            endtime = time.monotonic() + 4
            while time.monotonic() < endtime:
                demo.step()
