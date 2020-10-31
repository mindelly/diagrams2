Nup = 78
num = 39
import pyalps
parms = [ {
           'LATTICE'                   : "open square lattice",
           'MODEL'                     : "fermion Hubbard",
           'CONSERVED_QUANTUMNUMBERS'  : 'Nup, Ndown',
           'Nup_total'                  : 1,
           'Ndown_total'                  : 1,
           't'                         : 1,
           'U'                         : -7,
           'SWEEPS'                    : 12,
           'NUMBER_EIGENVALUES'        : 1,
           'L'                         : 40,
           'W'                         : 2,
           'MAXSTATES'                 : 1800
          } ]

file_name1 = 'e4-' + str(num)
file_name2 = 'e5-' + str(num)
file_name3 = 'e6-' + str(num)

################ e4 = E(Nup, 0) ################

parms[0]['Nup_total'] = Nup
parms[0]['Ndown_total'] = 0

input_file = pyalps.writeInputFiles(file_name1, parms)
res = pyalps.runApplication('dmrg', input_file, writexml=True)
data1 = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name1))

e4 = data1[0][0].y[0]
error_e4 = data1[0][1].y[0]

energy_list = ['E({0}, {1}) = {2}; te = {3} \n'.format(Nup, 0, e4, error_e4)]
file2 = open('result-en-te-W=2.txt', 'a')
for y in energy_list:
    file2.write('point {0} '.format(num) + y)
file2.close()


################ e5 = E(Nup + 1, 1) ################

parms[0]['Nup_total'] = Nup + 1
parms[0]['Ndown_total'] = 1

input_file = pyalps.writeInputFiles(file_name2, parms)
res = pyalps.runApplication('dmrg', input_file, writexml=True)
data2 = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name2))

e5 = data2[0][0].y[0]
error_e5 = data2[0][1].y[0]

energy_list = ['E({0}, {1}) = {2}; te = {3} \n'.format(Nup + 1, 1, e5, error_e5),]
file2 = open('result-en-te-W=2.txt', 'a')
for y in energy_list:
    file2.write('point {0} '.format(num) + y)
file2.close()


################ e6 = E(Nup - 1, 1) ################

parms[0]['Nup_total'] = Nup - 1
parms[0]['Ndown_total'] = 1

input_file = pyalps.writeInputFiles(file_name3,parms)
res = pyalps.runApplication('dmrg',input_file,writexml=True)
data3 = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name3))

e6 = data3[0][0].y[0]
error_e6 = data3[0][1].y[0]

energy_list = ['E({0}, {1}) = {2}; te = {3} \n'.format(Nup - 1, 1, e6, error_e6)]

file2 = open('result-en-te-W=2.txt', 'a')
for y in energy_list:
    file2.write('point {0} '.format(num) + y)
file2.close()


############################################

h = ((e6 - e4) / (-2))
print(h)

mu = ((e5 - e4) / 2)
print(mu)

###########################################

mu_h = str(num) + ' ' + str(mu) + ' ' + str(h) + '\n'

file1 = open('result-W=2.txt', 'a')
for x in mu_h:
    file1.write(x)
file1.close()
