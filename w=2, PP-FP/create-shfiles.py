n = 39

for z in range(1, n):
    file_name = 'num' + str(z) + '.sh'
    file_name1 = 'num' + str(z + 1) + '.sh'
    in_file = open(file_name, 'r')
    line = in_file.readlines()
    line[7] = 'python /home/aspotapova_2/point' + str(z + 1) + '.py'
    save_changes = open(file_name1, 'w')
    save_changes.writelines(line)
    save_changes.close()
    line = []

