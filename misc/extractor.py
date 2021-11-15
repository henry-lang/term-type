with open('./words.txt', 'r') as file:
    lines = file.readlines()
    print(lines)
    with open('./cwords.txt', 'w') as to_write:
        to_write.write('{\n')
        for line in lines:
            line = line[:-1] # Take off whitespace
            to_write.write(f'    \"{line}\",\n')
        to_write.write('}\n')
