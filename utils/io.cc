#include "utils.hpp"
#include <dirent.h>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

std::vector<std::string> readdir(std::string path, bool concat) {
	if (path[path.length() - 1] != '/')
		path.push_back('/');
	DIR *dir =opendir(path.c_str());
	if (!dir)
		fatalx(errno, "failed to open directory %s", path.c_str());

	std::vector<std::string> ents;
	errno = 0;
	struct dirent *dent = readdir(dir);
	while (dent) {
		const char *n = dent->d_name;
		if (strcmp(n, ".") != 0 && strcmp(n, "..") != 0)
			ents.push_back(concat ? path + n : n);
		errno = 0;
		dent = readdir(dir);
	}
	if (errno != 0)
		fatalx(errno, "failed to read directory %s", path.c_str());

	closedir(dir);
	return ents;
}

bool fileexists(const std::string &path) {
	struct stat sb;
	if (stat(path.c_str(), &sb) == -1) {
		if (errno == ENOENT)
			return false;
		fatalx(errno, "failed to stat %s", path.c_str());
	}
	return true;
}
