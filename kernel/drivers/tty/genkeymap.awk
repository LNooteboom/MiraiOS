BEGIN {
	print "#include <uapi/eventcodes.h>\n"
	print "char keymap[NROF_EVENTS] = {"
	print "\t[KEY_ENTER] = '\\n',"
	print "\t[KEY_BACKSPACE] = 127,"
	print "\t[KEY_SPACE] = ' ',"
}

/\#define KEY_/ {
	if (length($2) == 5) {
		print "\t[" $2 "] = '" substr($2, 5, 5) "',"
	}
}

END {
	print "};"
}