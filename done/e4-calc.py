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

file = open('e4-result.txt', 'a')
# e4
e4 = []
f = []
parms[0]['Nup_total'] = 1
parms[0]['Ndown_total'] = 0

for k in range(0, 10):
    input_file = pyalps.writeInputFiles('e4', parms)
    res = pyalps.runApplication('dmrg', input_file, writexml=True)

    data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix='e4'))
    for s in data[0]:
        f.append(s.y[0])

    parms[0]['Nup_total'] = parms[0]['Nup_total'] + 2

for m in range(0, len(f), 2):
    e4.append(f[m])



e4 = str(e4)
for index in e4:
    file.write(index)
f.close()
