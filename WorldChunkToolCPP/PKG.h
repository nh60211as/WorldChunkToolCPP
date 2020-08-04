#ifndef PKG_H
#define PKG_H

#include <string>

class PKG
{
public:
	static void ExtractPKG(const std::string& FileInput, bool FlagAutoConfirm, bool FlagUnpackAll, bool OnlyLog);

};

#endif
