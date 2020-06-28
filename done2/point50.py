Nup = 100
num = 50

import pyalps
import numpy as np
import matplotlib.pyplot as plt

parms = [ {
           'LATTICE'                   : "open square lattice",
           'MODEL'                     : "fermion Hubbard",
           'CONSERVED_QUANTUMNUMBERS'  : 'Nup, Ndown',
           'Nup_total'                  : 1,
           'Ndown_total'                  : 1,
           't'                         : 1,
           'U'                         : -7,
           'SWEEPS'                    : 8,
           'NUMBER_EIGENVALUES'        : 2,
           'L'                         : 40,
           'W'                         : 3,
           'MAXSTATES'                 : 800
          } ]

f = list() #list for e4 = E(Nup, 0)
g = list() #list for e5 = E(Nup + 1, 1)
i = list() #list for e6 = E(Nup - 1, 1)
e4 = list()
e5 = list()
e6 = list()

file_name1 = 'point4-' + str(num)
file_name2 = 'point5-' + str(num)
file_name3 = 'point6-' + str(num)

parms[0]['Nup_total'] = Nup
parms[0]['Ndown_total'] = 0

input_file = pyalps.writeInputFiles(file_name1, parms)
res = pyalps.runApplication('dmrg', input_file, writexml=True)

data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name1))
for s in data[0]:
    f.append(s.y[0])

for m in range(0, len(f), 2):
    e4.append(f[m])

print(f)
# e5
parms[0]['Nup_total'] = Nup + 1
parms[0]['Ndown_total'] = 1

input_file = pyalps.writeInputFiles(file_name2, parms)
res = pyalps.runApplication('dmrg', input_file, writexml=True)

data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name2))
for s in data[0]:
    g.append(s.y[0])

for q in range(0, len(g), 2):
    e5.append(g[q])

print(g)
#e6
parms[0]['Nup_total'] = Nup - 1
parms[0]['Ndown_total'] = 1

input_file = pyalps.writeInputFiles(file_name3,parms)
res = pyalps.runApplication('dmrg',input_file,writexml=True)

data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name3))
for s in data[0]:
    i.append(s.y[0])


for d in range(0, len(i), 2):
    e6.append(i[d])

print(i)

h2 = list()
h2.append((e6[0] - e4[0]) / (-2))
print(h2)

mu2 = list()
mu2.append((e5[0] - e4[0]) / 2)
print(mu2)

result = open('result-W=3.txt', 'a')
stroka = 'point' + str(num) + ' ' + str(mu2) + ', ' + str(h2) + '\n'
for x in stroka:
    result.write(x)
result.close()
