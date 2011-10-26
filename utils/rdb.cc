#include "utils.hpp"
#include <cstring>
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;

static std::string makepath(bf::path, RdbAttrs);
// Get the key for the given path.
static bool getkey(const bf::path&, std::string&);
static bf::path keyfile(const std::string&);
static void touch(const bf::path&);

std::string pathfor(const char *root, RdbAttrs keys) {
	bf::path path(root);
	std::string curkey;

	while (keys.size() > 0) {
		if (bf::exists(path)) {
			if (!bf::is_directory(path) && keys.size() > 0)
				fatal("%s is not a directory\n", path.string().c_str());
			if (!getkey(path, curkey))
				return makepath(path.string().c_str(), keys);
			RdbAttrs::iterator it = keys.find(curkey);
			if (it == keys.end()) {
				path /= "UNSPECIFIED";
			} else {
				path /= it->second;
				keys.erase(it);
			}
		} else {
			return makepath(path.string().c_str(), keys);
		}
	}

	return path.string();
}

static std::string makepath(bf::path root, RdbAttrs keys) {
	if (!bf::exists(root) && keys.size() > 1)
		bf::create_directory(root);

	while (keys.size() > 0) {
		const std::string &key = keys.begin()->first;
		const std::string &vl = keys.begin()->second;
		touch(root / keyfile(key));
		keys.erase(key);

		root /= vl;
		if (keys.size() > 0)
			bf::create_directory(vl);
	}

	return root.string();
}

static bool getkey(const bf::path &p, std::string &key) {
	for (bf::directory_iterator it(p); it != bf::directory_iterator(); it++) {
		const char *fname = it->string().c_str();
		if (strstr(fname, "KEY=") == fname) {
			key = fname + strlen("KEY=");
			return true;
		}
	}
	return false;
}

static bf::path keyfile(const std::string &key) {
	return bf::path(std::string("KEY=") + key);
}

static void touch(const bf::path &p) {
	FILE *touch = fopen(p.string().c_str(), "w");
	if (!touch)
		fatalx(errno, "Failed to touch keyfile %s\n", p.string().c_str());
	fclose(touch);
}