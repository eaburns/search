#include "utils.hpp"

static const struct { const char *str, *enc; } base64tst[] = {
	{ "Man", "TWFu" },
	{ "pleasure.", "cGxlYXN1cmUu" },
	{ "leasure.", "bGVhc3VyZS4=" },
	{ "easure.", "ZWFzdXJlLg==" },
	{ "asure.", "YXN1cmUu" },
	{ "sure.", "c3VyZS4=" },
	{ "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.",
	"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=" },
};

bool test_base64enc(void) {

	bool ok = true;
	for (unsigned int i = 0; i < sizeof(base64tst) / sizeof(base64tst[0]); i++) {
		std::string enc = base64enc(base64tst[i].str);
		if (enc == base64tst[i].enc)
			continue;
		testpr("Failed to correctly encode [%s], expected [%s], got [%s]\n",
			base64tst[i].str, base64tst[i].enc, enc.c_str());
		ok = false;
	}

	return ok;
}

bool test_base64dec(void) {

	bool ok = true;
	for (unsigned int i = 0; i < sizeof(base64tst) / sizeof(base64tst[0]); i++) {
		std::string str = base64dec(base64tst[i].enc);
		if (str == base64tst[i].str)
			continue;
		testpr("Failed to correctly decode [%s], expected [%s], got [%s]\n",
			base64tst[i].enc, base64tst[i].str, str.c_str());
		ok = false;
	}

	return ok;
}