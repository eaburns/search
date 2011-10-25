#include "utils.hpp"
#include <cstring>
//#include <boost/filesystem.hpp>

//namespace bf = boost::filesystem;
//
//// Get the key for the given path.
//static bool getkey(bf::path, std::string&);
//
//struct RdbAttrs {
//};
//
//std::string pathfor(const char *root, RdbAttrs keys) {
//	bool used[keys.size()];
//	memset(used, 0, sizeof(*used) * keys.size());
//
//	bf::path path(root);
//	
//
//	return path.string();
//}
//
//static bool getkey(bf::path p, std::string &key) {
//	for (bf::directory_iterator it(p); it != bf::directory_iterator(); it++) {
//		const char *fname = it->string().c_str();
//		if (strstr(fname, "KEY=") == fname) {
//			key = fname + strlen("KEY=");
//			return true;
//		}
//	}
//	return false;
//}
