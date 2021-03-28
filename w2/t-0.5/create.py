n = 39
for z in range(1, n):
    file_name = 'point' + str(z) + '.py'
    file_name1 = 'point' + str(z + 1) + '.py'
    in_file = open(file_name, 'r')
    line = in_file.readlines()
    Nup = int(line[0][6:-1])
    num = int(line[1][6:-1])
    line[0] = 'Nup = ' + str(Nup + 2) + '\n'
    line[1] = 'num = ' + str(num + 1) + '\n'
    in_file.close()
    save_changes = open(file_name1, 'w')
    save_changes.writelines(line)
    save_changes.close()
    line = []

