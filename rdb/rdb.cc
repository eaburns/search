#include "rdb.hpp"
#include "../utils/utils.hpp"
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;

static std::string makepath(bf::path, RdbAttrs);
static void walkwithattrs(bf::path, RdbAttrs&, std::vector<std::string>&);
static void addpath(bf::path, RdbAttrs&, std::vector<std::string>&);
// Get the key for the given path.
static bool getkey(const bf::path&, std::string&);
static bf::path keyfile(const std::string&);
static void touch(const bf::path&);

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

std::vector<std::string> rdbwithattrs(const char *root, RdbAttrs attrs) {
	std::vector<std::string> files;
	walkwithattrs(root, attrs, files);
	return files;
}

static void walkwithattrs(bf::path path, RdbAttrs &attrs, std::vector<std::string> &files) {
	if (!bf::exists(path) || !bf::is_directory(path))
		return;

	std::string curkey;
	if (!getkey(path, curkey))
		return;

	if (attrs.mem(curkey)) {
		path /= attrs.lookup(curkey);
		return addpath(path, attrs, files);
	}

	for (bf::directory_iterator it(path); it != bf::directory_iterator(); it++)
		addpath(*it, attrs, files);
}

static void addpath(bf::path path, RdbAttrs &attrs, std::vector<std::string> &files) {
	if (strstr(path.string().c_str(), "KEY=") != NULL)
		return;
	if (!bf::exists(path))
		return;
	if (bf::is_directory(path))
		return walkwithattrs(path, attrs, files);
	files.push_back(path.string());
}

std::string rdbpathfor(const char *root, RdbAttrs attrs) {
	bf::path path(root);
	std::string curkey;

	while (attrs.size() > 0) {
		if (bf::exists(path)) {
			if (!bf::is_directory(path) && attrs.size() > 0)
				fatal("%s is not a directory\n", path.string().c_str());
			if (!getkey(path, curkey))
				return makepath(path, attrs);
			if (!attrs.mem(curkey)) {
				path /= "UNSPECIFIED";
			} else {
				path /= attrs.lookup(curkey);
				attrs.rm(curkey);
			}
		} else {
			return makepath(path, attrs);
		}
	}

	return path.string();
}

static std::string makepath(bf::path root, RdbAttrs attrs) {
	while (attrs.size() > 0) {
		if (!bf::exists(root)) {
			if (mkdir(root.string().c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == -1) {
				if (errno != EEXIST)
					fatalx(errno, "failed to create %s\n", root.string().c_str());
			}
		}

		const std::string &key = attrs.front();
		const std::string &vl = attrs.lookup(key);
		if (!bf::exists(root / keyfile(key)))
			touch(root / keyfile(key));
		attrs.pop_front();
		root /= vl;
	}

	return root.string();
}

static bool getkey(const bf::path &p, std::string &key) {
	for (bf::directory_iterator it(p); it != bf::directory_iterator(); it++) {
		const char *fname = it->path().string().c_str();
		const char *keyfile = strstr(fname, "KEY=");
		if (!keyfile)
			continue;
		key = keyfile + strlen("KEY=");
		return true;
	}
	return false;
}

static bf::path keyfile(const std::string &key) {
	return bf::path(std::string("KEY=") + key);
}

static void touch(const bf::path &p) {
	FILE *touch = fopen(p.string().c_str(), "w");
	if (!touch)
		fatalx(errno, "Failed to touch keyfile %s", p.string().c_str());
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