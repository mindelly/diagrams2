Nup = 58
Ndown = 58
num = 15
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
           'NUMBER_EIGENVALUES'        : 2,
           'L'                         : 40,
           'W'                         : 3,
           'MAXSTATES'                 : 400
          } ]

f = list() #list for e1 = E(Nup, Ndown)
g = list() #list for e2 = E(Nup + 1, Ndown + 1)
i = list() #list for e3 = E(Nup + 1, Ndown - 1)
e1 = list()
e2 = list()
e3 = list()

file_name1 = '1point1-' + str(num)
file_name2 = '1point2-' + str(num)
file_name3 = '1point3-' + str(num)

parms[0]['Nup_total'] = Nup
parms[0]['Ndown_total'] = Ndown

input_file = pyalps.writeInputFiles(file_name1, parms)
res = pyalps.runApplication('dmrg', input_file, writexml=True)

data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name1))
for s in data[0]:
    f.append(s.y[0])

for m in range(0, len(f), 2):
    e1.append(f[m])

print(f)
# e2
parms[0]['Nup_total'] = Nup + 1
parms[0]['Ndown_total'] = Ndown - 1

input_file = pyalps.writeInputFiles(file_name2, parms)
res = pyalps.runApplication('dmrg', input_file, writexml=True)

data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name2))
for s in data[0]:
    g.append(s.y[0])

for q in range(0, len(g), 2):
    e2.append(g[q])

print(g)
#e3
parms[0]['Nup_total'] = Nup + 1
parms[0]['Ndown_total'] = Ndown + 1

input_file = pyalps.writeInputFiles(file_name3,parms)
res = pyalps.runApplication('dmrg',input_file,writexml=True)

data = pyalps.loadEigenstateMeasurements(pyalps.getResultFiles(prefix=file_name3))
for s in data[0]:
    i.append(s.y[0])


for d in range(0, len(i), 2):
    e3.append(i[d])

print(i)

h = list()
h.append((e2[0] - e1[0]) / 2)
print(h)

mu = list()
mu.append((e3[0] - e1[0]) / 2)
print(mu)

result = open('1result-W=3.txt', 'a')
stroka = 'point' + str(num) + ' ' + str(mu) + ', ' + str(h) + '\n'
for x in stroka:
    result.write(x)
result.close()

