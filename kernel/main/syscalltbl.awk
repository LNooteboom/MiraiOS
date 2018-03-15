BEGIN {
	table = 0;
	print "#include <uapi/syscalls.h>\n"
	print "int (*syscallTable[256])() = {"
}
/\/\/END/ {
	table = 0;
}
{
	if (table == 1) {
		print "\t(void *)" substr($3, 0, index($3,"(") - 1) ","
	}
}
/\/\/SYSCALLS/ {
	table = 1;
}
END {
	print "};"
}