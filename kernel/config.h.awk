BEGIN {
	print "\
#ifndef INCLUDE_CONFIG_H\n\
#define INCLUDE_CONFIG_H\n"
}
{
	if ($0 != "" && substr($0, index($0, "=") + 1) != "n" && substr($0, 1,1) != "#") {
		print "#define "substr($0, 0, index($0, "="))" "substr($0, index($0, "=") + 1)
	}
}
END {
	print "\n#endif"
}