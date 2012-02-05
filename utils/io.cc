#include "utils.hpp"

boost::optional<std::string> readline(FILE *in, bool echo) {
	std::string line;

	int c = fgetc(in);
	while (c != '\n' && c != EOF) {
		line.push_back(c);
		c = fgetc(in);
	}

	if (c == EOF && ferror(in))
		fatal("failed to read line");

	if (line.size() == 0 && feof(in))
		return boost::optional<std::string>();

	if (line[line.size()] == '\n')
		line.resize(line.size()-1);

	if (echo)
		puts(line.c_str());

	return boost::optional<std::string>(line);
}