#!/usr/bin/env python3

from random import random, randint, choice as randomchoice
import time

from blinkentotem import Totem


def rgbw_flashycounters():
    totem = Totem()
    frame = 0
    while True:
        frame += 1

        for led in totem.rgbw:
            led.clear()
        for i in range(8):
            do_b = (frame % 0xff) & (1 << i) != 0
            do_r = ((frame // 3 + 73) % 0xff) & (1 << i) != 0
            if do_b:
                totem.rgbw[7 - i].b = 40
            if do_r:
                totem.rgbw[i].r = 100
        for led in totem.rgbw:
            if led.b and led.r and random() < 0.1:
                led.w = 250
        totem.rgbw[randint(0, 7)].g = 250
        totem.pushPieces()

        time.sleep(0.01)


def randommess_x():
    return 0 if random() < 0.7 else randint(0, 255)


def randommess():
    totem = Totem()
    while True:
        led = randomchoice(totem.rgb)
        led.r = randommess_x()
        led.g = randommess_x()
        led.b = randommess_x()

        led = randomchoice(totem.rgbw)
        led.r = randommess_x()
        led.g = randommess_x()
        led.b = randommess_x()
        led.w = randommess_x()

        totem.pushPieces()

        time.sleep(0.01)


def sequential():
    totem = Totem()
    leds = totem.rgb + totem.rgbw
    redi = 0
    bluei = 0
    greeni = 0
    while True:
        for led in leds:
            led.clear()
        leds[redi].r = 255
        leds[bluei].b = 200
        leds[greeni].g = 200
        redi = (redi + 1) % len(leds)
        if random() < 0.5:
            bluei = (bluei + 1) % len(leds)
        if random() < 0.1:
            greeni = (greeni + 1) % len(leds)
        totem.pushPieces()
        totem.flush()


if __name__ == '__main__':
    sequential()
