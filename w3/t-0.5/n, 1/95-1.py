Nup = 95
Ndown = 1
import pyalps
parms = [ {
           'LATTICE'                   : "open ladder",
           'MODEL'                     : "fermion Hubbard",
           'CONSERVED_QUANTUMNUMBERS'  : 'Nup, Ndown',
           'Nup_total'                  : Nup,
           'Ndown_total'                  : Ndown,
           't0'                         : 1,
	   't1'                         : 0.5,
           'U'                         : -7,
           'SWEEPS'                    : 15,
           'NUMBER_EIGENVALUES'        : 1,
           'L'                         : 40,
           'W'                         : 3,
           'MAXSTATES'                 : 1500
          } ]

file_name = str(Nup) + '-' + str(Ndown)

input_file = pyalps.writeInputFiles(file_name, parms)
res = pyalps.runApplication('dmrg', input_file, writexml=True)
data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name))

en = data[0][0].y[0]
error = data[0][1].y[0]

print(en, error)


