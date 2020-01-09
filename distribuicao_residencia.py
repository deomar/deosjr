import numpy as np
from numpy.linalg import *
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

#Inicialização das variáveis e constantes
def init():
    omega = 100
    beta = 1
    dt = 0.001
    tmax = 100
    a = 15
    b = 1
    A0 = 4
    itmax = int(tmax/dt)
    DrV = 50
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
    C += 0.4*DrV
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
axisx = []
txaux = []
#Varia os índices da matriz que guarda as posições de todas as realizações
for no in range(kf):
    tx = []
    for ki in range(nf):
        taux0 = 0
        taux1 = 0
        t2 = []
        subup = 1
        subdown = 0
#Calcula o número de transições entre os dois estados
        for j in range(itmax-1):
            if ((x[no][ki][j] > 0 and x[no][ki][j+1] < 0) or (x[no][ki][j] < 0 and x[no][ki][j+1] > 0)):
                taux1 = t1[j-1]
            else:
                if (x[no][ki][j] > 0 and x[no][ki][j+1] > 0):
                    if(x[no][ki][j] > 2 and subdown == 1):
                        subup = 1
                        subdown = 0
                        t2.append(taux1 - taux0)
                        taux0 = taux1
                    else:
                        continue
                else:
                    if(x[no][ki][j] < -2 and subup == 1):
                        subup = 0
                        subdown = 1
                        t2.append(taux1 - taux0)
                        taux0 = taux1
                    else:
                        continue
        tx.append(t2)
    txaux.append(np.average(tx)/tmax)
plt.hist(tx,bins=200)
plt.xlabel('T(s)')
plt.ylabel('$N$')
plt.title('$a = 15, b = 1, A_0 = 1, D = 0.4 \Delta V$')
plt.savefig('time_dist.png')
plt.show()
