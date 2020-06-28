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
           'SWEEPS'                    : 12,
           'NUMBER_EIGENVALUES'        : 2,
           'L'                         : 40,
           'W'                         : 2,
           'MAXSTATES'                 : 400
          } ]


# e4
e5 = []
f = []
parms[0]['Nup_total'] = 2 + 2*20
parms[0]['Ndown_total'] = 1

for k in range(0, 5):
    input_file = pyalps.writeInputFiles('e5-2', parms)
    res = pyalps.runApplication('dmrg', input_file, writexml=True)

    data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix='e5-2'))
    for s in data[0]:
        f.append(s.y[0])

    parms[0]['Nup_total'] = parms[0]['Nup_total'] + 2

for m in range(0, len(f), 2):
    e5.append(f[m])

file = open('e5-2-result.txt', 'a')

e5 = str(e5)
for index in e5:
    file.write(index)
file.close()
