#!/usr/bin/env python
"""
This script first plots measured temperatures VS ADC readings for JBC T210 and T245 handles.
The fitted 3rd degree plynomial for each curve is then printed to the terminal.

Author: Axel Johansson
Date: 2023-07-21
"""
import numpy as np
import matplotlib.pyplot as plt

# Measured values for T210 handle with C210-007 cartridge
x_t210 = [0, 525, 621, 713, 834, 988, 1155, 1355, 1533, 1680, 1742]
y_t210 = [25, 168, 191, 206, 248, 279, 319, 364, 404, 434, 440]

#  Fit a polynomal equation with degree 3
model = np.poly1d(np.polyfit(x_t210, y_t210, 3))
plt.plot(x_t210,y_t210, '*-')

X_t210 = np.linspace(0, 1800, 20)
Y_t210 = X_t210*X_t210*X_t210*model[3] + X_t210*X_t210*model[2] + X_t210*model[1] + model[0]

plt.plot(X_t210,Y_t210, '*-')

# Print eq
temp = "Equation_T210: " + str(model[3]) + " * x^3 " + str(model[2]) + " * x^2 + " + str(model[1]) + "* x + " + str(model[0]) 
print(temp)


# Measured values for T245 handle with C245-945 cartridge
x_t245 = [0,1690, 2150, 2360, 2780, 3200, 3700]
y_t245 = [25, 200, 240, 265, 300, 348, 399, ]

#  Fit a polynomal equation with degree 3
model = np.poly1d(np.polyfit(x_t245, y_t245, 3))
plt.plot(x_t245,y_t245, '*-')

X_t245 = np.linspace(0, 3500, 20)
Y_t245 = X_t245*X_t245*X_t245*model[3] + X_t245*X_t245*model[2] + X_t245*model[1] + model[0]

plt.plot(X_t245,Y_t245, '*-')
plt.grid(True)
plt.show()

# Print eq
temp = "Equation_T245: " + str(model[3]) + " * x^3 " + str(model[2]) + " * x^2 + " + str(model[1]) + "* x + " + str(model[0]) 
print(temp)
