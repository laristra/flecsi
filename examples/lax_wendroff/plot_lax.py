#!/usr/bin/python
"""
Plot lax.out from src/main.cc
"""

import matplotlib.pyplot as plt
import numpy as np
import os

def save_image(x, y, phi, time):

    fig, ax = plt.subplots()
    cax = ax.imshow(phi, vmin = -0.2, vmax = 1.4)
    ax.set_title(time)
    cbar = fig.colorbar(cax, ticks = np.arange(-0.2,1.6,0.2))
    plt.savefig(time+'.png')

def plot_snapshot(x, y, phi, time):

    vel = 1.0
    anal_phi = np.zeros((len(x), len(y)))
    for i in range(len(x)):
        for j in range(len(y)):
            if ((x[i]-time)<=0.5) & ((y[j]-vel*time)<=0.5) & ((x[i]-time)>=0.0) & ((y[j]-vel*time)>=0.0):
                anal_phi[i,j] = 1.0

    plt.figure()
    plt.xlabel('x')
    plt.plot(x,phi[:,len(y)/2 -2],'--')
    plt.plot(x,anal_phi[:,len(y)/2 -2],':',label='analytic')
    plt.legend(loc='best')

    plt.figure()
    plt.xlabel('y')
    plt.plot(y,phi[len(x)/2 -2,:],'--')
    plt.plot(y,anal_phi[len(x)/2 -2,:],':',label='analytic')
    plt.legend(loc='best')

    fig, ax = plt.subplots()
    cax = ax.imshow(anal_phi, vmin = -0.2, vmax = 1.4)
    ax.set_title('analytic')
    cbar = fig.colorbar(cax, ticks = np.arange(-0.2,1.6,0.2))

    fig, ax = plt.subplots()
    cax = ax.imshow(phi, vmin = -0.2, vmax = 1.4)
    ax.set_title('Split Lax-Wendroff')
    cbar = fig.colorbar(cax, ticks = np.arange(-0.2,1.6,0.2))

if __name__ == "__main__":

    for file in os.listdir("./"):
        time = file.split(".out")
        if (len(time) > 1):
            print time[0],file
            with open(file) as f:
                content = f.readlines();

            x = []
            y = []
            content = [line.strip('\n') for line in content]
            for line in content:
                entries = line.split()
                x.append(float(entries[0]))
                y.append(float(entries[1]))

            x = list(set(x))
            x.sort()
            x = np.array(x)
            NX = len(x)
            x = x / (NX - 1.0)
            y = list(set(y))
            y.sort()
            y = np.array(y)
            NY = len(y)
            y = y / (NY - 1.0)

            phi = np.zeros( (NX,NY) )
            for line in content:
                entries = line.split()
                i = int(entries[0])
                j = int(entries[1])
                phi[i, j] = float(entries[2])

            plot_snapshot(x, y, phi, 0.166667)
            #save_image(x, y, phi, time[0])

            plt.show()

