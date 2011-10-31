#include "utils.hpp"
#include <cstring>
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;

static std::string makepath(bf::path, RdbAttrs);
// Get the key for the given path.
static bool getkey(const bf::path&, std::string&);
static bf::path keyfile(const std::string&);
static void touch(const bf::path&);

bool RdbAttrs::push_back(const std::string &key, const std::string &value) {
	if (pairs.find(key) != pairs.end())
		return false;
	Value &v = pairs[key];
	v.value = value;
	v.ind = keys.size();
	keys.push_back(value);
	return true;
}

bool RdbAttrs::rm(const std::string &key) {
	if (pairs.find(key) == pairs.end())
		return false;
	Value v = pairs[key];
	keys.erase(keys.begin() + v.ind);
	pairs.erase(pairs.find(key));
	return true;	
}

std::string rdbpathfor(const char *root, RdbAttrs attrs) {
	bf::path path(root);
	std::string curkey;

	while (attrs.size() > 0) {
		if (bf::exists(path)) {
			if (!bf::is_directory(path) && attrs.size() > 0)
				fatal("%s is not a directory\n", path.string().c_str());
			if (!getkey(path, curkey))
				return makepath(path.string().c_str(), attrs);
			if (!attrs.mem(curkey)) {
				path /= "UNSPECIFIED";
			} else {
				path /= attrs.lookup(curkey);
				attrs.rm(curkey);
			}
		} else {
			return makepath(path.string().c_str(), attrs);
		}
	}

	return path.string();
}

static std::string makepath(bf::path root, RdbAttrs attrs) {
	while (attrs.size() > 0) {
		if (!bf::exists(root)) {
			if (!bf::create_directory(root))
				fatal("failed to create %s\n", root.string().c_str());
		}

		
		const std::string &key = attrs.front();
		const std::string &vl = attrs.lookup(key);
		if (!bf::exists(root / keyfile(key)))
			touch(root / keyfile(key));
		attrs.rm(key);
		root /= vl;
	}

	return root.string();
}

static bool getkey(const bf::path &p, std::string &key) {
	for (bf::directory_iterator it(p); it != bf::directory_iterator(); it++) {
		const char *fname = it->string().c_str();
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