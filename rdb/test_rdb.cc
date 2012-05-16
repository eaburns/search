#include "rdb.hpp"
#include "../utils/utils.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>

static bool hassuffix(const std::string&, const char*);
static void touch(const std::string&);

bool test_pathfor_newpath() {
	char dir[] = "rdb-XXXXXX";
	const char *root = mkdtemp(dir);

	RdbAttrs attrs;
	attrs.push_back("test0", "0");
	attrs.push_back("test1", "1");

	std::string path = pathfor(root, attrs);
	bool ok = hassuffix(path, "0/1");
	if (!ok)
		testpr("Expected test0/test1 suffix on [%s]\n", path.c_str());

	rmrecur(root);

	return ok;		
}

bool test_pathfor_samepath() {
	char dir[] = "rdb-XXXXXX";
	const char *root = mkdtemp(dir);

	RdbAttrs attrs;
	attrs.push_back("test0", "0");
	attrs.push_back("test1", "1");

	std::string path0 = pathfor(root, attrs);
	std::string path1 = pathfor(root, attrs);
	bool ok = path0 == path1;
	if (!ok)
		testpr("Expected [%s] == [%s]\n", path0.c_str(), path1.c_str());

	rmrecur(root);

	return ok;		
}

bool test_pathfor_shareprefix() {
	bool ok = true;
	char dir[] = "rdb-XXXXXX";
	const char *root = mkdtemp(dir);

	RdbAttrs attrs0;
	attrs0.push_back("test0", "0");
	attrs0.push_back("test1", "1");
	attrs0.push_back("test2", "1");
	RdbAttrs attrs1;
	attrs1.push_back("test0", "0");
	attrs1.push_back("test1", "2");
	attrs1.push_back("test2", "1");
	RdbAttrs attrs2;
	attrs2.push_back("test0", "0");
	attrs2.push_back("test1", "1");
	attrs2.push_back("test2", "2");

	std::string path0 = pathfor(root, attrs0);
	if (path0 != std::string(root) + "/0/1/1") {
		testpr("Expected path to be %s/0/1/1, got [%s]\n", root, path0.c_str());
		ok = false;
	}
	std::string path1 = pathfor(root, attrs1);
	if (path1 != std::string(root) + "/0/2/1") {
		testpr("Expected path to be %s/0/2/1, got [%s]\n", root, path1.c_str());
		ok = false;
	}
	std::string path2 = pathfor(root, attrs2);
	if (path2 != std::string(root) + "/0/1/2") {
		testpr("Expected path to be %s/0/1/,, got [%s]\n", root, path2.c_str());
		ok = false;
	}

	rmrecur(root);

	return ok;		
}

bool test_pathfor_existing() {
	bool ok = true;
	char dir[] = "rdb-XXXXXX";
	const char *root = mkdtemp(dir);

	std::string p(root);
	mkdir(p.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
	touch(pathcat(p, "KEY=zero"));
	p = pathcat(p, "0");
	mkdir(p.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
	touch(pathcat(p, "KEY=one"));
	p = pathcat(p, "1");
	mkdir(p.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
	touch(pathcat(p, "KEY=two"));

	RdbAttrs attrs;
	attrs.push_back("two", "2");
	attrs.push_back("zero", "0");
	attrs.push_back("one", "1");

	std::string path = pathfor(root, attrs);
	if (path != std::string(root) + "/0/1/2") {
		testpr("Expected %s/0/1/2, got [%s]\n", root, path.c_str());
		ok = false;
	}

	rmrecur(root);

	return ok;		
}

static bool hassuffix(const std::string &string, const char *suffix) {
	const char *s = string.c_str();
	return strstr(s, suffix) == s + strlen(s) - strlen(suffix);
}

static void touch(const std::string &p) {
	FILE *touch = fopen(p.c_str(), "w");
	if (!touch)
		fatalx(errno, "Failed to touch keyfile %s", p.c_str());
	fclose(touch);
}