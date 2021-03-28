Nup = 12
Ndown = Nup
import pyalps
parms = [ {
           'LATTICE'                   : "open square lattice",
           'MODEL'                     : "fermion Hubbard",
           'CONSERVED_QUANTUMNUMBERS'  : 'Nup, Ndown',
           'Nup_total'                  : Nup,
           'Ndown_total'                  : Ndown,
           't'                         : 1,
           'U'                         : -7,
           'SWEEPS'                    : 12,
           'NUMBER_EIGENVALUES'        : 1,
           'L'                         : 40,
           'W'                         : 3,
           'MAXSTATES'                 : 1500
          } ]

################ e6 = E(Nup - 1, 1) ################

file_name = str(Nup) + '-' + str(Ndown)

input_file = pyalps.writeInputFiles(file_name, parms)
res = pyalps.runApplication('dmrg', input_file, writexml=True)
data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name))

en = data[0][0].y[0]
error = data[0][1].y[0]

energy = ['E({0},{1})={2} te={3} \n'.format(Nup, Ndown, en, error)]

file = open('result.txt', 'a')
for y in energy:
    file.write(y)
file.close()



