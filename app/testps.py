#!/usr/bin/env python3

import time

from superps import SystemActivity


def main():

    raidDevices = ['sdb', 'sdc', 'sdd', 'sde']
    rootDevice = 'sda'

    systemActivity = SystemActivity([rootDevice] + raidDevices)

    frame = 0
    while True:
        frame += 1

        #cpus = systemActivity.updateCPUs()
        #print(''.join(('{:3}'.format(int(100 * a.busy)) for a in cpus)))

        time.sleep(0.5)
        cpus = systemActivity.updateCPUTemps()
        minmin = min(c.thermal.minvalue for c in cpus)
        maxmax = max(c.thermal.maxvalue for c in cpus)
        print(' '.join('{:1.2f}'.format(c.thermal.relativeValue) for c in cpus), end='')
        print('  {} {}'.format(minmin, maxmax))


    # noinspection PyUnreachableCode
    return 0


if __name__ == '__main__':
    main()
