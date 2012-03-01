#include "rdb.hpp"
#include "../utils/utils.hpp"
#include <utility>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>

// makepath builds the path for the given attribute list.
static std::string makepath(std::string, RdbAttrs);

// collect collects all of the files with attributes matching the
// given attribute list to the vector.
static void collect(std::string, RdbAttrs&, std::vector<std::string>&);

// addfiles adds the RDB entries matching the attribute list
// beneath the given directory to the vector.
static void addfiles(const std::string&, RdbAttrs&, std::vector<std::string>&);

// getkey searches the given directory for the
// KEY= file and returns the key name.
static bool getkey(const std::string&, std::string&);

// touch 'ensure that the given file exists.
static void touch(const std::string&);

// frontpair gets the first key=value pair from the argument vector.
// The return value is the number of arguments used to find this pair.
static int frontpair(int, const char*[], std::string&, std::string&);

bool RdbAttrs::push_back(const std::string &key, const std::string &value) {
	if (pairs.find(key) != pairs.end())
		return false;
	pairs[key] = value;
	keys.push_back(key);
	return true;
}

bool RdbAttrs::push_front(const std::string &key, const std::string &value) {
	if (pairs.find(key) != pairs.end())
		return false;
	pairs[key] = value;
	keys.push_front(key);
	return true;
}

bool RdbAttrs::rm(const std::string &key) {
	if (pairs.find(key) == pairs.end())
		return false;

	for (std::deque<std::string>::iterator it = keys.begin(); it != keys.end(); it++) {
		if (*it == key) {
			keys.erase(it);
			break;
		}
	}

	pairs.erase(pairs.find(key));
	return true;	
}

std::string RdbAttrs::string(void) const {
	std::string r;

	for (std::deque<std::string>::const_iterator it = keys.begin(); it != keys.end(); it++) {
		const std::string &k = *it;
		const std::string &v = pairs.find(k)->second;
		if (r.size() > 0)
			r.push_back(' ');
		r += k;
		r.push_back('=');
		r+= v;
	}

	return r;
}

std::vector<std::string> rdbwithattrs(const std::string &root, RdbAttrs attrs) {
	std::vector<std::string> files;
	collect(root, attrs, files);
	return files;
}

static void collect(std::string path, RdbAttrs &attrs, std::vector<std::string> &files) {
	if (!fileexists(path) || !isdir(path))
		return;

	std::string curkey;
	if (!getkey(path, curkey))
		return;

	if (attrs.mem(curkey)) {
		path = pathcat(path, attrs.lookup(curkey));
		return addfiles(path, attrs, files);
	}

	std::vector<std::string> ents = readdir(path);
	for (unsigned int i = 0; i < ents.size(); i++)
		addfiles(ents[i], attrs, files);
}

static void addfiles(const std::string &path, RdbAttrs &attrs, std::vector<std::string> &files) {
	if (strstr(path.c_str(), "KEY=") != NULL)
		return;
	if (!fileexists(path))
		return;
	if (isdir(path))
		return collect(path, attrs, files);
	files.push_back(path);
}

std::string rdbpathfor(const std::string &root, RdbAttrs attrs) {
	std::string path(root);
	std::string curkey;

	while (attrs.size() > 0) {
		if (fileexists(path)) {
			if (!isdir(path) && attrs.size() > 0)
				fatal("%s is not a directory\n", path.c_str());
			if (!getkey(path, curkey))
				return makepath(path, attrs);
			if (!attrs.mem(curkey)) {
				path = pathcat(path, "UNSPECIFIED");
			} else {
				path = pathcat(path, attrs.lookup(curkey));
				attrs.rm(curkey);
			}
		} else {
			return makepath(path, attrs);
		}
	}

	return path;
}

static std::string makepath(std::string root, RdbAttrs attrs) {
	while (attrs.size() > 0) {
		if (!fileexists(root)) {
			if (mkdir(root.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == -1) {
				if (errno != EEXIST)
					fatalx(errno, "failed to create %s\n", root.c_str());
			}
		}

		std::string key = attrs.front();
		std::string vl = attrs.lookup(key);
		std::string kfile = pathcat(root, "KEY=" + key);
		if (!fileexists(kfile))
			touch(kfile);
		attrs.pop_front();
		root = pathcat(root, vl);
	}

	return root;
}

static bool getkey(const std::string &p, std::string &key) {
	std::vector<std::string> ents = readdir(p);
	for (unsigned int i = 0; i < ents.size(); i++) {
		const char *fname = ents[i].c_str();
		const char *keyfile = strstr(fname, "KEY=");
		if (!keyfile)
			continue;
		key = keyfile + strlen("KEY=");
		return true;
	
	}
	return false;
}

static void touch(const std::string &p) {
	FILE *touch = fopen(p.c_str(), "w");
	if (!touch)
		fatalx(errno, "Failed to touch keyfile %s", p.c_str());
	fclose(touch);
}

RdbAttrs pathattrs(std::string path) {
	RdbAttrs attrs;

	while (fileexists(path)) {
		std::string dir = dirname(path);
		std::string file = basename(path);
		std::string key;
		if (!getkey(dir, key))
			break;
		attrs.push_front(key, file);
		path = dir;
	}

	return attrs;
}

RdbAttrs attrargs(int argc, const char *argv[]) {
	RdbAttrs attrs;
	while (argc > 0) {
		std::string key, value;
		int shift = frontpair(argc, argv, key, value);
		if (shift < 0)
			break;
		argc -= shift;
		argv += shift;
		attrs.push_back(key.c_str(), value.c_str());
	}
	return attrs;
}

static int frontpair(int argc, const char *argv[], std::string &key, std::string &vl) {
	key.clear();
	vl.clear();

	for (int i = 0; i < argc; i++) {
		std::string arg = argv[i];
		size_t eq = arg.find('=');
		if (eq == std::string::npos) {
			key += arg;
			key.push_back(' ');
			continue;
		}
		vl = arg.substr(eq + 1, arg.size() - eq - 1);
		if (eq > 0)
			key += arg.substr(0, eq - 1);
		else if (key[key.size()-1] == ' ')
			key.resize(key.size()-1);
		return i + 1;
	}
	return -1;
}