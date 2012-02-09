#include "utils.hpp"
#include <dirent.h>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static void rmslash(std::string);

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

bool isdir(const std::string &path) {
	struct stat sb;
	if (stat(path.c_str(), &sb) == -1) {
		if (errno == ENOENT)
			return false;
		fatalx(errno, "failed to stat %s", path.c_str());
	}
	return S_ISDIR(sb.st_mode);
}

void ensuredir(const std::string &p) {
	std::string dir = dirname(p);
	if (dir == "." || dir == "/" || isdir(dir))
		return;

	ensuredir(dirname(dir));

	if (mkdir(dir.c_str(), 0) == -1)
		fatalx(errno, "failed to make directory %s\n", dir.c_str());
}

std::string basename(const std::string &p) {
	rmslash(p);
	const char *str = p.c_str();
	const char *slash = strrchr(str, '/');
	if (!slash)
		return p;
	return slash + 1;
}

std::string dirname(const std::string &p) {
	rmslash(p);
	if (p == "/")
		return p;
	const char *str = p.c_str();
	const char *slash = strrchr(str, '/');
	if (!slash)
		return ".";
	std::string res;
	for (const char *c = str; c < slash; c++)
		res.push_back(*c);
	return res;
}

// rmslash removes the trailing slash at the end of a path.
static void rmslash(std::string s) {
	while (s.length() > 1 && s[s.length() - 1] == '/')
		s.resize(s.length() - 1);
}