import numpy as np
from numpy.linalg import *
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

#Inicialização das variáveis e constantes
def init():
    omega = 100
    beta = 1
    dt = 0.01
    tmax = 3000
    a = 1
    b = 0.2
    A0 = 15
    itmax = int(tmax/dt)
    DrV = 1.25
    AB = []
    C = 0
    xmax = []
    nf = 1
    kf = 1
    return beta,dt,itmax,omega,A0,a,b,DrV,AB,C,xmax,nf,kf,tmax

beta,dt,itmax,omega,A0,a,b,DrV,AB,C,xmax,nf,kf,tmax = init()

x = np.zeros((kf,nf,itmax))
#Varia a amplitude do ruído
for k in range(kf): 
    C += 0.6*DrV
#Cálcula evolução da posição da partícula para um ruído fixo
    for n in range(nf):
        x1 = 1
        i = 0
        t = 0
        t1 = []
        dw = C*np.random.normal(0,1,itmax)
        for it in range(itmax):
            x1 = x1 + a*x1*dt - b*x1*x1*x1*dt + A0*np.cos(omega*t)*dt + beta*np.sqrt(dt)*dw[i]
            t += dt
            x[k][n][i] = x1
            t1.append(t)
            i+= 1
    AB.append(C/DrV)
plt.plot(t1,x[0][0][:])
plt.xlabel('Tempo')
plt.ylabel('Posição')
plt.title('$a = 1, b = 0.2, A_0 = 15, D = 0.6 \Delta V$')
plt.savefig('posicao.png')
plt.show()
