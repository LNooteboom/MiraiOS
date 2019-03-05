BEGIN {
	table = 0;
	print "BITS 64\n"
}
/\/\/END/ {
	table = 0;
}
{
	if (table == 1) {
		funct = substr($3, 0, index($3,"("));
		print "global " funct ":function"
		print funct ":"
		print "\tmov eax, 0x" substr($1, 3, 2)
		print "\tmov r10, rcx"
		print "\tsyscall"
		print "\tret"
	}
}
/\/\/SYSCALLS/ {
	table = 1;
}
END {
	print ""
}