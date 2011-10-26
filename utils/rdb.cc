#include "utils.hpp"
#include <cstring>
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;

static std::string makepath(bf::path, RdbAttrs);
// Get the key for the given path.
static bool getkey(bf::path, std::string&);
static bf::path keyfile(const std::string&);
static void touch(bf::path);

std::string pathfor(const char *root, RdbAttrs keys) {
	bool used[keys.size()];
	memset(used, 0, sizeof(*used) * keys.size());

	bf::path path(root);
	std::string curkey;
	while (keys.size() > 0) {
		if (bf::exists(path)) {
			if (!bf::is_directory(path))
				fatal("%s is not a directory\n", path.string().c_str());
			if (!getkey(path, curkey))
				return makepath(path.string().c_str(), keys);
			path /= keys[curkey];
			keys.erase(curkey);
		} else {
			return makepath(path.string().c_str(), keys);
		}
	}

	return path.string();
}

static std::string makepath(bf::path root, RdbAttrs keys) {
	while (keys.size() > 0) {
		if (!bf::exists(root) && keys.size() > 1)
			bf::create_directory(root);

		const std::string &key = keys.begin()->first;
		const std::string &vl = keys.begin()->second;
		touch(root / keyfile(key));

		bf::create_directory(vl);
		root /= vl;
	}

	return root.string();
}

static bool getkey(bf::path p, std::string &key) {
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

static void touch(bf::path p) {
	FILE *touch = fopen(p.string().c_str(), "w");
	if (!touch)
		fatalx(errno, "Failed to touch keyfile %s\n", p.string().c_str());
	fclose(touch);
}