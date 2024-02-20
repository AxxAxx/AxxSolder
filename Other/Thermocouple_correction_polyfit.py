#!/usr/bin/env python
"""
This script first plots measured temperatures VS ADC readings for JBC T210 and T245 handles.
The fitted 3rd degree plynomial for each curve is then printed to the terminal.

Author: Axel Johansson
Date: 2023-07-21
"""
import numpy as np
import matplotlib.pyplot as plt


#########################################################
# Measured values for T115 handle with C115-007 cartridge
#########################################################

x_t115 = [0, 500, 630, 734, 825, 930]
y_t115 = [25, 240, 300, 360, 415, 446]

#  Fit a polynomal equation with degree 2
model = np.poly1d(np.polyfit(x_t115, y_t115, 2))
plt.plot(x_t115,y_t115, 'or')

X_t115 = np.linspace(0, 1000, 20)
Y_t115 = X_t115*X_t115*model[2] + X_t115*model[1] + model[0]

plt.plot(X_t115,Y_t115, '--r')

# Print eq
temp = "Equation_T115: " + str(model[2]) + " * x^2 + " + str(model[1]) + "* x + " + str(model[0]) 
print(temp)

#########################################################
# Measured values for T210 handle with C210-007 cartridge
#########################################################
x_t210 = [0, 500, 570, 660, 740, 810, 880, 950, 1015, 1231]
y_t210 = [25, 180, 198, 235, 263, 285, 307, 326, 345, 420]

#  Fit a polynomal equation with degree 2
model = np.poly1d(np.polyfit(x_t210, y_t210, 2))
plt.plot(x_t210,y_t210, 'ob')

X_t210 = np.linspace(0, 1400, 20)
Y_t210 = X_t210*X_t210*model[2] + X_t210*model[1] + model[0]

plt.plot(X_t210,Y_t210, '--b')

# Print eq
temp = "Equation_T210: " + str(model[2]) + " * x^2 + " + str(model[1]) + "* x + " + str(model[0]) 
print(temp)

#########################################################
# Measured values for T245 handle with C245-945 cartridge
#########################################################
x_t245 = [0,1220, 1570, 1980, 2450, 3270]
y_t245 = [25, 170, 212, 256, 312, 410]

#  Fit a polynomal equation with degree 2
model = np.poly1d(np.polyfit(x_t245, y_t245, 2))
plt.plot(x_t245,y_t245, 'og')

X_t245 = np.linspace(0, 3500, 20)
Y_t245 = X_t245*X_t245*model[2] + X_t245*model[1] + model[0]

plt.plot(X_t245,Y_t245, '--g')

# Print eq
temp = "Equation_T245: " + str(model[2]) + " * x^2 + " + str(model[1]) + "* x + " + str(model[0]) 
print(temp)




plt.title("Temperature calibration data")
plt.xlabel("ADC Value")
plt.ylabel("Temperature [deg C]")
plt.legend(('C115 Cartridge Measured', 'C115 Polyfitted 2nd order Eq.','C210 Cartridge Measured', 'C210 Polyfitted 2nd order Eq.','C245 Cartridge Measured', 'C245 Polyfitted 2nd order Eq.'),
           loc='lower right')

plt.grid(True)
plt.show()


