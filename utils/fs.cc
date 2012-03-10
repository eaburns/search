#include "utils.hpp"
#include <dirent.h>
#include <cstring>
#include <cerrno>
#include <cstdarg>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

enum { MaxLine = 4096 };

static const char *sepstr = "/";
static const char sepchar = sepstr[0];

boost::optional<std::string> readline(FILE *in, bool echo) {
	std::string line;

	int c = fgetc(in);
	while (c != '\n' && c != EOF) {
		if (line.size() == MaxLine)
			fatal("Your line is too big");
		line.push_back(c);
		c = fgetc(in);
	}

	if (c == EOF && ferror(in))
		fatal("failed to read line");

	if (line.size() == 0 && feof(in))
		return boost::optional<std::string>();

	if (echo)
		puts(line.c_str());

	if (line[line.size()-1] == '\n')
		line.resize(line.size()-1);
	if (line[line.size()-1] == '\r')
		line.resize(line.size()-1);

	return boost::optional<std::string>(line);
}

std::vector<std::string> readdir(std::string path, bool concat) {
	if (path[path.length() - 1] != sepchar)
		path.push_back(sepchar);
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

	if (dir == "." || dir == sepstr || isdir(dir))
		return;

	ensuredir(dir);

	if (mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == -1) {
		if (errno == EEXIST)
			return;
		fatalx(errno, "failed to make directory %s", dir.c_str());
	}
}

bool rmrecur(const std::string &path) {
	if (!isdir(path)) {
		if (remove(path.c_str()) != 0 && errno != ENOENT) {
			warnx(errno, "failed to remove %s", path.c_str());
			return false;
		}
		return true;
	}

	std::vector<std::string> ents = readdir(path);
	bool ok = true;
	for (unsigned int i = 0; i < ents.size(); i++) {
		if (!rmrecur(ents[i]))
			ok = false;
	}
	if (!ok)
		return false;
	if (remove(path.c_str()) != 0 && errno != ENOENT) {
		warnx(errno, "failed to remove %s", path.c_str());
		return false;
	}
	return true;
}

std::string basename(const std::string &p) {
	if (p.size() == 0 || p == ".")
		return ".";

	const char *str = p.c_str();
	const char *slash = strrchr(str, sepchar);
	if (!slash)
		return p;

	if (strlen(slash + 1) == 0)
		return ".";

	return slash + 1;
}

std::string dirname(const std::string &p) {
	if (p.size() == 0 || p == ".")
		return ".";

	if (p == sepstr)
		return sepstr;

	const char *str = p.c_str();
	const char *slash = strrchr(str, sepchar);
	if (!slash)
		return ".";

	std::string res;
	for (const char *c = str; c <= slash; c++)
		res.push_back(*c);

	if (res.length() > 1 && res[res.length()-1] == sepchar)
		res.resize(res.length()-1);

	return res;
}

std::string pathcat(const std::string &dir, const std::string &base) {
	if (dir[dir.length()-1] == sepchar)
		return dir + base;
	std::string s(dir);
	s.push_back(sepchar);
	s += base;
	return s;
}

std::string pathcatn(unsigned int n, ...) {
	std::string path;
	va_list ap;

	va_start(ap, n);
	for (unsigned int i = 0; i < n; i++) {
		const char *ent = va_arg(ap, const char *);
		if (path[path.length()-1] != sepchar)
			path.push_back(sepchar);
		path += ent;
	}
	va_end(ap);

	return path;
}