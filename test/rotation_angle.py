#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as pp

def sim_movement(radius, rotation_per_step, color):
    heading = 90 # degrees

    step = np.array(range(8*radius))

    pos = np.zeros(2)

    for s in step:
        heading -= rotation_per_step

        mov_vector = np.array([np.cos(heading * np.pi/180), np.sin(heading * np.pi/180)])

        pos += mov_vector

        circle = pp.Circle((pos[0], pos[1]), radius, color=color, fill=False)
        pp.gca().add_artist(circle)

radius = np.arange(0, 200, 1)

rotation1 = 10.0 / (radius/10.0 + 1) # degrees
rotation2 = 10.0 / (radius/3 + 1) # degrees

pp.figure()
pp.plot(radius, rotation1, 'r')
pp.plot(radius, rotation2, 'g')

pp.figure()
idx = 1
pp.xlim([-1000*radius[idx]/199, 1000*radius[idx]/199])
pp.ylim([-1000*radius[idx]/199, 1000*radius[idx]/199])
sim_movement(radius[1], rotation1[1], 'r')
sim_movement(radius[1], rotation2[1], 'g')

pp.figure()
idx = 20
pp.xlim([-1000*radius[idx]/199, 1000*radius[idx]/199])
pp.ylim([-1000*radius[idx]/199, 1000*radius[idx]/199])
sim_movement(radius[idx], rotation1[idx], 'r')
sim_movement(radius[idx], rotation2[idx], 'g')

pp.figure()
idx = 199
pp.xlim([-1000*radius[idx]/199, 1000*radius[idx]/199])
pp.ylim([-1000*radius[idx]/199, 1000*radius[idx]/199])
sim_movement(radius[idx], rotation1[idx], 'r')
sim_movement(radius[idx], rotation2[idx], 'g')

pp.show()
