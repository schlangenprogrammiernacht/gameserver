#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as pp

mass = np.arange(0, 500e3, 100)

radius1 = np.sqrt(mass)/2
radius2 = (20*mass+100)**0.3 - 100**0.3
#radius2 = 30*(np.log2((mass+10000)/100) - np.log2(100))

pp.plot(mass, radius1, 'r')
pp.plot(mass, radius2, 'g')

pp.show()
