/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

#include <Server/Components/Unicode/unicode.hpp>
#include <sdk.hpp>
#include <unicode/ucsdet.h>
#include <unicode/unistr.h>

using namespace Impl;

class UnicodeComponent final : public IUnicodeComponent
{
public:
	void onLoad(ICore* core) override
	{
	}

	OptimisedString toUTF8(StringView input) override
	{
		static UErrorCode detstatus = U_ZERO_ERROR;
		static UCharsetDetector* detector = ucsdet_open(&detstatus);
		if (U_FAILURE(detstatus))
		{
			return OptimisedString(input);
		}
		UErrorCode status = U_ZERO_ERROR;
		ucsdet_setText(detector, input.data(), input.length(), &status);
		const char* cp = ucsdet_getName(ucsdet_detect(detector, &status), &status);
		if (U_FAILURE(status))
		{
			return OptimisedString(input);
		}
		String output;
		icu::UnicodeString(input.data(), input.length(), cp).toUTF8String(output);
		return OptimisedString(output);
	}

	StringView componentName() const override
	{
		return "Unicode";
	}

	SemanticVersion componentVersion() const override
	{
		return SemanticVersion(OMP_VERSION_MAJOR, OMP_VERSION_MINOR, OMP_VERSION_PATCH, BUILD_NUMBER);
	}

	void free() override
	{
		delete this;
	}

	void reset() override
	{
		// Nothing to reset here.
	}
};

COMPONENT_ENTRY_POINT()
{
	return new UnicodeComponent();
}
