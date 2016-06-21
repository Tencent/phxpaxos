def AddLicense(file_path, license_file_path, file_type="code"):

	license_file=open(license_file_path,"r")
	licenses = license_file.readlines()
	license_file.close()

	add_file = open(file_path)
	lines = add_file.readlines()
	add_file.close()

	new_lines = []
	begin = False
	for line in lines:
		read_line = line.strip()

		if( file_type =="code" ):
			if( len( read_line ) > 0 and read_line[0] == '#' ):
				begin = True
		else:
			if( len( read_line ) > 0 and read_line[0] != '#' ):
				begin = True

		if( begin == False ):
			continue
		new_lines.append( line )

	add_file = open(file_path,'w')

	if( file_type == "Makefile" ):
		for license in licenses:
			add_file.write("# "+license)
	else:
		add_file.write("/*\n")
		for license in licenses:
			add_file.write("\t"+license)
		add_file.write("*/\n")
	add_file.write("\n")

	add_file.writelines(new_lines)
	add_file.close()

