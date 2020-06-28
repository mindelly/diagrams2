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



e6 = []
a = []
parms[0]['Nup_total'] = 40
parms[0]['Ndown_total'] = 1

for k in range(0, 5):
    input_file = pyalps.writeInputFiles('e6-2', parms)
    res = pyalps.runApplication('dmrg', input_file, writexml=True)

    data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix='e6-2'))
    for s in data[0]:
        a.append(s.y[0])

    parms[0]['Nup_total'] = parms[0]['Nup_total'] + 2

for i in range(0, len(a), 2):
    e6.append(a[i])

file = open('e6-2-result.txt', 'a')

e6 = str(e6)
for index in e6:
    file.write(index)
file.close()
