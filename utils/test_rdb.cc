#include "utils.hpp"
#include <cstring>
#include <cstdio>
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;

static bool hassuffix(const std::string&, const char*);
static void touch(const bf::path&);

bool test_rdbpathfor_newpath(void) {
	char dir[] = "rdb-XXXXXX";
	const char *root = mkdtemp(dir);

	RdbAttrs attrs;
	attrs["test0"] = "0";
	attrs["test1"] = "1";

	std::string path = rdbpathfor(root, attrs);
	bool ok = hassuffix(path, "0/1");
	if (!ok)
		testpr("Expected test0/test1 suffix on [%s]\n", path.c_str());

	bf::remove_all(root);

	return ok;		
}

bool test_rdbpathfor_samepath(void) {
	char dir[] = "rdb-XXXXXX";
	const char *root = mkdtemp(dir);

	RdbAttrs attrs;
	attrs["test0"] = "0";
	attrs["test1"] = "1";

	std::string path0 = rdbpathfor(root, attrs);
	std::string path1 = rdbpathfor(root, attrs);
	bool ok = path0 == path1;
	if (!ok)
		testpr("Expected [%s] == [%s]\n", path0.c_str(), path1.c_str());

	bf::remove_all(root);

	return ok;		
}

bool test_rdbpathfor_shareprefix(void) {
	bool ok = true;
	char dir[] = "rdb-XXXXXX";
	const char *root = mkdtemp(dir);

	RdbAttrs attrs0;
	attrs0["test0"] = "0";
	attrs0["test1"] = "1";
	attrs0["test2"] = "1";
	RdbAttrs attrs1;
	attrs1["test0"] = "0";
	attrs1["test1"] = "2";
	attrs1["test2"] = "1";
	RdbAttrs attrs2;
	attrs2["test0"] = "0";
	attrs2["test1"] = "1";
	attrs2["test2"] = "2";

	std::string path0 = rdbpathfor(root, attrs0);
	if (path0 != std::string(root) + "/0/1/1") {
		testpr("Expected path to be %s/0/1/1, got [%s]\n", root, path0.c_str());
		ok = false;
	}
	std::string path1 = rdbpathfor(root, attrs1);
	if (path1 != std::string(root) + "/0/2/1") {
		testpr("Expected path to be %s/0/2/1, got [%s]\n", root, path1.c_str());
		ok = false;
	}
	std::string path2 = rdbpathfor(root, attrs2);
	if (path2 != std::string(root) + "/0/1/2") {
		testpr("Expected path to be %s/0/1/,, got [%s]\n", root, path2.c_str());
		ok = false;
	}

	bf::remove_all(root);

	return ok;		
}

bool test_rdbpathfor_existing(void) {
	bool ok = true;
	char dir[] = "rdb-XXXXXX";
	const char *root = mkdtemp(dir);

	bf::path p(root);
	bf::create_directory(p);
	touch((p / "KEY=zero").string());
	p /= "0";
	bf::create_directory(p);
	touch((p / "KEY=one").string());
	p /= "1";
	bf::create_directory(p);
	touch((p / "KEY=two").string());

	RdbAttrs attrs;
	attrs["two"] = "2";
	attrs["zero"] = "0";
	attrs["one"] = "1";

	std::string path = rdbpathfor(root, attrs);
	if (path != std::string(root) + "/0/1/2") {
		testpr("Expected %s/0/1/2, got [%s]\n", root, path.c_str());
		ok = false;
	}

	bf::remove_all(root);

	return ok;		
}

static bool hassuffix(const std::string &string, const char *suffix) {
	const char *s = string.c_str();
	return strstr(s, suffix) == s + strlen(s) - strlen(suffix);
}

static void touch(const bf::path &p) {
	FILE *touch = fopen(p.string().c_str(), "w");
	if (!touch)
		fatalx(errno, "Failed to touch keyfile %s", p.string().c_str());
	fclose(touch);
}