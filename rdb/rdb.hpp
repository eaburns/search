#include <map>
#include <deque>
#include <string>
#include <vector>

// The RdbAttrs struct holds attributes for the RDB database.
// It behaves as a FIFO queue of attributes.
struct RdbAttrs {

	// push_back adds a new key=value pair to the back of the
	// attribute list.  It returns false if the key is already added.
	bool push_back(const std::string &key, const std::string &val);

	// push_front adds a new key=value pair to the front of the
	// attribute list.  It returns false if the key is already added.
	bool push_front(const std::string &key, const std::string &val);

	// pop_front removes the first attribute that was added to the
	// list.
	void pop_front(void) {
		const std::string &k = keys.front();
		pairs.erase(pairs.find(k));
		keys.pop_front();
	}

	// front returns the key for the next attribute pair.
	const std::string &front(void) { return keys.front(); }

	// size returns the number of key=value pairs.
	unsigned int size(void) const { return keys.size(); }

	// rm removes the pair for the given key.
	bool rm(const std::string &key);

	// lookup returns the value associated with the given
	// key.
	const std::string &lookup(const std::string &key) { return pairs[key]; }

	// mem returns true if there is a value bound to the given
	// key.
	bool mem(const std::string &key) const {
		return pairs.find(key) != pairs.end();
	}

	// string returns the string representation of this attribute
	// list.
	std::string string(void) const;

private:
	std::map<std::string, std::string> pairs;
	std::deque<std::string> keys;
};

// rdbpathfor returns the filesystem path for the given attribute
// set rooted at the given root directory.
std::string rdbpathfor(const char *, RdbAttrs);

// rdbwithattrs returns a vector of all file paths that have the given
// attributes under the given root directory.
std::vector<std::string> rdbwithattrs(const char*, RdbAttrs);

// pathattrs returns the attributes for the given file path.
RdbAttrs pathattrs(std::string);